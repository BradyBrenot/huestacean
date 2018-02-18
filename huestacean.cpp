#include "huestacean.h"
#include "ScreenCapture.h"

#include <QQmlApplicationEngine>
#include <QElapsedTimer>

///ENTERTAINMENT ------------------

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORC)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf     printf
#define mbedtls_fprintf    fprintf
#endif

#include <string.h>
#include <math.h>

#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/timing.h"

///END ENTERTAINMENT ------------------

QNetworkAccessManager qnam(nullptr);

Huestacean::Huestacean(QObject *parent) 
    : QObject(parent),
    syncing(false),
    eThread(nullptr)
{
    bridgeDiscovery = new BridgeDiscovery(this);
    bridgeDiscovery->startSearch();
    detectMonitors();

    connect(bridgeDiscovery, SIGNAL(modelChanged()),
        this, SLOT(connectBridges()));

    eImageProvider = new EntertainmentImageProvider(this);
    extern QQmlApplicationEngine* engine;
    engine->addImageProvider("entimage", eImageProvider);

    emit hueInit();

    //ENTERTAINMENT
    skip = 32;
    captureInterval = 25;
}

Huestacean::~Huestacean()
{
    if (eThread != nullptr)
    {
        eThread->stop();
        while (eThread->isRunning())
        {
            QThread::msleep(100);
        }
    }
}

int Huestacean::getMessageSendElapsed()
{
    return eThread ? eThread->messageSendElapsed : 0;
}

void Huestacean::setCaptureInterval(int interval)
{
    captureInterval = interval;
    if (framegrabber)
    {
        framegrabber->setFrameChangeInterval(std::chrono::milliseconds(captureInterval));
    }
    emit captureParamsChanged();
}

void Huestacean::detectMonitors()
{
    activeMonitorIndex = 0;

    std::vector<SL::Screen_Capture::Monitor> mons = SL::Screen_Capture::GetMonitors();
    foreach(QObject* monitor, monitors)
    {
        monitor->deleteLater();
    }
    monitors.clear();

    for (SL::Screen_Capture::Monitor& mon : mons)
    {
        Monitor* newMonitor = new Monitor(this, mon.Id, mon.Height, mon.Width, mon.OffsetX, mon.OffsetY, QString::fromUtf8(mon.Name, sizeof(mon.Name)), mon.Scaling);
        monitors.push_back(newMonitor);
    }

    emit monitorsChanged();
}

void Huestacean::startScreenSync(EntertainmentGroup* eGroup)
{
    syncing = true;
    emit syncingChanged();
    eGroup->toggleStreaming(true);

    runSync(eGroup);
}
void Huestacean::stopScreenSync()
{
    if (eThread == nullptr)
    {
        return;
    }

    eThread->stop();

    //TODO: this seems like a flaw...
    for (QObject* Obj : bridgeDiscovery->getModel())
    {
        HueBridge* bridge = qobject_cast<HueBridge*>(Obj);
        if (bridge != nullptr)
        {
            for (auto& group : bridge->EntertainmentGroups)
            {
                group.toggleStreaming(false);
            }
        }
    }
}

void Huestacean::setActiveMonitor(int index)
{
    activeMonitorIndex = index;
}

void Huestacean::updateEntertainmentGroups()
{
    entertainmentGroups.clear();

    for (QObject* Obj : bridgeDiscovery->getModel())
    {
        HueBridge* bridge = qobject_cast<HueBridge*>(Obj);
        if (bridge != nullptr)
        {
            for (auto& group : bridge->EntertainmentGroups)
            {
                entertainmentGroups.push_back(&group);
            }
        }
    }

    emit entertainmentGroupsChanged();
}

void Huestacean::connectBridges()
{
    for (QObject* Obj : bridgeDiscovery->getModel())
    {
        HueBridge* bridge = qobject_cast<HueBridge*>(Obj);
        if (bridge != nullptr)
        {
            connect(bridge, SIGNAL(entertainmentGroupsChanged()),
                this, SLOT(updateEntertainmentGroups()));
            connect(bridge, SIGNAL(lightsChanged()),
                this, SIGNAL(entertainmentGroupsChanged()));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// Entertainment API

void rgb_to_xyz(double& r, double& g, double& b, double& x, double& y, double& brightness)
{
    brightness = std::min(std::max(std::max(r, g), b)*1.2, 1.0);

    r = (r > 0.04045f) ? pow((r + 0.055f) / (1.0f + 0.055f), 2.4f) : (r / 12.92f);
    g = (g > 0.04045f) ? pow((g + 0.055f) / (1.0f + 0.055f), 2.4f) : (g / 12.92f);
    b = (b > 0.04045f) ? pow((b + 0.055f) / (1.0f + 0.055f), 2.4f) : (b / 12.92f);

    float X = r * 0.664511f + g * 0.154324f + b * 0.162028f;
    float Y = r * 0.283881f + g * 0.668433f + b * 0.047685f;
    float Z = r * 0.000088f + g * 0.072310f + b * 0.986039f;

    x = X / (X + Y + Z);
    y = Y / (X + Y + Z);

    //brightness = Y;
}

void Huestacean::runSync(EntertainmentGroup* eGroup)
{
    Q_ASSERT(eGroup != nullptr);

    if (eThread != nullptr) {
        return;
    }

    HueBridge* bridge = eGroup->bridgeParent();
    Q_ASSERT(bridge != nullptr);

    if (bridge->username.isEmpty() || bridge->clientkey.isEmpty())
    {
        qWarning() << "Can't start entertainment thread unless we're logged into the bridge";
        return;
    }

    eThreadMutex.lock();
    eThread = new EntertainmentCommThread(this, bridge->username, bridge->clientkey, bridge->address.toString(), eGroup);
    eThreadMutex.unlock();

    connect(eThread, &EntertainmentCommThread::messageSendElapsedChanged, this, &Huestacean::messageSendElapsedChanged);
    connect(eThread, &EntertainmentCommThread::finished, this, &Huestacean::entertainmentThreadFinished);
    eThread->start();

    int monitorIndex = activeMonitorIndex;

    framegrabber = SL::Screen_Capture::CreateCaptureConfiguration([monitorIndex]() {
        auto allMonitors = SL::Screen_Capture::GetMonitors();
        std::vector<SL::Screen_Capture::Monitor> chosenMonitor;
        for (auto& monitor : allMonitors)
        {
            if (monitor.Index == monitorIndex)
            {
                chosenMonitor.push_back(monitor);
                break;
            }
        }
        
        return chosenMonitor;
    })->onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
        Q_UNUSED(monitor);

        static QElapsedTimer timer;

        const int Height = SL::Screen_Capture::Height(img);
        const int Width = SL::Screen_Capture::Width(img);

        const int RowPadding = SL::Screen_Capture::RowPadding(img);
        const int Pixelstride = img.Pixelstride;

        const int ScreenWidth = 16;
        const int ScreenHeight = 9;

        const int numPixels = Height * Width;

        EntertainmentScreen eScreen(ScreenWidth, ScreenHeight);
        auto& screen = eScreen.screen;

        const int s = skip;

        const unsigned char* src = SL::Screen_Capture::StartSrc(img);
        for (int y = 0; y < Height; y += 1 + s)
        {
            src = SL::Screen_Capture::StartSrc(img) + (Pixelstride * Width + RowPadding)*y;
            int screenY = static_cast<int>((static_cast<uint64_t>(y) * static_cast<uint64_t>(ScreenHeight)) / static_cast<uint64_t>(Height));

            for (int x = 0; x < Width; x += 1 + s)
            {
                const int screenX = static_cast<int>((static_cast<uint64_t>(x) * static_cast<uint64_t>(ScreenWidth)) / static_cast<uint64_t>(Width));
                const int screenPos = screenY * eScreen.width + screenX;
                screen[screenPos].B += (src[0]);
                screen[screenPos].G += (src[1]);
                screen[screenPos].R += (src[2]);
                ++screen[screenPos].samples;
                src += Pixelstride * (1 + s);
            }
        }

        eThread->threadsafe_setScreen(eScreen);

        frameReadElapsed = timer.restart();
        qDebug() << timer.clockType();
        emit frameReadElapsedChanged(); //TODO: make sure this is thread-safe. Pretty sure it is from the docs I'm reading...

    })->start_capturing();

    framegrabber->setFrameChangeInterval(std::chrono::milliseconds(captureInterval));
    framegrabber->setMouseChangeInterval(std::chrono::milliseconds(0));
}

void Huestacean::entertainmentThreadFinished()
{
    framegrabber.reset();

    eThreadMutex.lock();
    eThread->deleteLater();
    eThread = nullptr;
    eThreadMutex.unlock();

    syncing = false;
    emit syncingChanged();
}

EntertainmentCommThread::EntertainmentCommThread(QObject *parent, QString inUsername, QString inClientkey, QString inAddress, EntertainmentGroup* inGroup)
    : QThread(parent),
    screenMutex(), screen(), username(inUsername), clientkey(inClientkey), address(inAddress), eGroup(nullptr), stopRequested(false)
{
    eGroup.name = inGroup->name;
    eGroup.id = inGroup->id;
    eGroup.lights = inGroup->lights;
    for (auto& light : eGroup.lights)
    {
        light.setParent(nullptr);
    }
}

void EntertainmentCommThread::threadsafe_setScreen(const EntertainmentScreen& inScreen)
{
    screenMutex.lock();
    screen = inScreen;
    screenMutex.unlock();
}

EntertainmentScreen& EntertainmentCommThread::getScreenForPixmap()
{
    static EntertainmentScreen retScreen;
    screenMutex.lock();
    retScreen = screen;
    screenMutex.unlock();
    return retScreen;
}

void EntertainmentCommThread::stop()
{
    stopRequested = true;
}

EntertainmentImageProvider::EntertainmentImageProvider(Huestacean* parent)
    : QQuickImageProvider(QQmlImageProviderBase::Image), huestaceanParent(parent)
{

}

QImage EntertainmentImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id);

    huestaceanParent->eThreadMutex.lock();
    if (huestaceanParent->eThread == nullptr)
    {
        huestaceanParent->eThreadMutex.unlock();

        if (size)
        {
            size->setWidth(1);
            size->setHeight(1);
        }

        return QImage(1, 1, QImage::Format_RGB16);
    }
    EntertainmentScreen& screen = huestaceanParent->eThread->getScreenForPixmap();
    //"safe": it's a ref to a static variable, eThread can go away now
    huestaceanParent->eThreadMutex.unlock();

    if (size)
    {
        size->setWidth(screen.width);
        size->setHeight(screen.height);
    }

    QImage img = QImage(screen.width, screen.height, QImage::Format_RGB16);
    int pixel = 0;
    for (int y = 0; y < screen.height; ++y)
    {
        for (int x = 0; x < screen.width; ++x)
        {
            img.setPixelColor(QPoint(x, y), 
                QColor(
                    screen.screen[pixel].R / screen.screen[pixel].samples,
                    screen.screen[pixel].G / screen.screen[pixel].samples,
                    screen.screen[pixel].B / screen.screen[pixel].samples
                ));

            ++pixel;
        }
    }

    //i.setPixelColor(QPoint())

    if(requestedSize.isValid())
        img = img.scaled(requestedSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    //p.scaled

    return img;
}

//simple dtls test using mbedtls. Never ends!
void EntertainmentCommThread::run()
{
    /*
    * The following is modified from the Simple DTLS client demonstration program
    */
    /*
    *  Simple DTLS client demonstration program
    *
    *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
    *  SPDX-License-Identifier: Apache-2.0
    *
    *  Licensed under the Apache License, Version 2.0 (the "License"); you may
    *  not use this file except in compliance with the License.
    *  You may obtain a copy of the License at
    *
    *  http://www.apache.org/licenses/LICENSE-2.0
    *
    *  Unless required by applicable law or agreed to in writing, software
    *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
    *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    *  See the License for the specific language governing permissions and
    *  limitations under the License.
    *
    *  This file is part of mbed TLS (https://tls.mbed.org)
    */

#define READ_TIMEOUT_MS 1000
#define MAX_RETRY       5
#define DEBUG_LEVEL 2000
#define SERVER_PORT "2100"
#define SERVER_NAME "Hue"

    int ret;
    mbedtls_net_context server_fd;
    const char *pers = "dtls_client";
    int retry_left = MAX_RETRY;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    mbedtls_timing_delay_context timer;

    mbedtls_debug_set_threshold(0);

    /*
    * -1. Load psk
    */
    QByteArray pskArray = clientkey.toUtf8();
    QByteArray pskRawArray = QByteArray::fromHex(pskArray);


    QByteArray pskIdArray = username.toUtf8();
    QByteArray pskIdRawArray = pskIdArray;

    /*
    * 0. Initialize the RNG and the session data
    */
    mbedtls_net_init(&server_fd);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    qDebug() << "Seeding the random number generator...";

    mbedtls_entropy_init(&entropy);
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
        (const unsigned char *)pers,
        strlen(pers))) != 0)
    {
        mbedtls_printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
        goto exit;
    }

    /*
    * 1. Start the connection
    */
    qDebug() << "Connecting to udp" << address << SERVER_PORT;

    if ((ret = mbedtls_net_connect(&server_fd, address.toUtf8(),
        SERVER_PORT, MBEDTLS_NET_PROTO_UDP)) != 0)
    {
        qCritical() << "mbedtls_net_connect FAILED" << ret;
        goto exit;
    }

    if (stopRequested)
        goto exit;

    /*
    * 2. Setup stuff
    */
    qDebug() << "Setting up the DTLS structure...";

    if ((ret = mbedtls_ssl_config_defaults(&conf,
        MBEDTLS_SSL_IS_CLIENT,
        MBEDTLS_SSL_TRANSPORT_DATAGRAM,
        MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        qCritical() << "mbedtls_ssl_config_defaults FAILED" << ret;
        goto exit;
    }

    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        qCritical() << "mbedtls_ssl_setup FAILED" << ret;
        goto exit;
    }

    if (0 != (ret = mbedtls_ssl_conf_psk(&conf, (const unsigned char*)pskRawArray.data(), pskRawArray.length() * sizeof(char),
        (const unsigned char *)pskIdRawArray.data(), pskIdRawArray.length() * sizeof(char))))
    {
        qCritical() << "mbedtls_ssl_conf_psk FAILED" << ret;
    }

    int ciphers[2];
    ciphers[0] = MBEDTLS_TLS_PSK_WITH_AES_128_GCM_SHA256;
    ciphers[1] = 0;
    mbedtls_ssl_conf_ciphersuites(&conf, ciphers);

    if ((ret = mbedtls_ssl_set_hostname(&ssl, SERVER_NAME)) != 0)
    {
        qCritical() << "mbedtls_ssl_set_hostname FAILED" << ret;
        goto exit;
    }

    mbedtls_ssl_set_bio(&ssl, &server_fd,
        mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    mbedtls_ssl_set_timer_cb(&ssl, &timer, mbedtls_timing_set_delay,
        mbedtls_timing_get_delay);

    if (stopRequested)
        goto exit;

    /*
    * 4. Handshake
    */
    qDebug() << "Performing the DTLS handshake...";

    for(int attempt = 0; attempt < 4; ++attempt)
    {
        mbedtls_ssl_conf_handshake_timeout(&conf, 400, 1000);
        do ret = mbedtls_ssl_handshake(&ssl);
        while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
            ret == MBEDTLS_ERR_SSL_WANT_WRITE);

        if (ret == 0)
            break;

        msleep(200);
    }

    if (ret != 0)
    {
        qCritical() << "mbedtls_ssl_handshake FAILED" << ret;
        goto exit;
    }

    if (stopRequested)
        goto exit;

    /*
    * 6. Write the echo request
    */
send_request:
    while (true)
    {
        screenMutex.lock();

        static QElapsedTimer timer;
        timer.restart();

        static const unsigned char HEADER[] = {
            'H', 'u', 'e', 'S', 't', 'r', 'e', 'a', 'm', //protocol

            0x01, 0x00, //version 1.0

            0x01, //sequence number 1

            0x00, 0x00, //Reserved write 0’s

            0x01,

            0x00, // Reserved, write 0’s
        };

        static const unsigned char PAYLOAD_PER_LIGHT[] =
        {
            0x01, 0x00, 0x06, //light ID

            //color: 16 bpc
            0xff, 0xff,
            0xff, 0xff,
            0xff, 0xff,
            /*
            (message.R >> 8) & 0xff, message.R & 0xff,
            (message.G >> 8) & 0xff, message.G & 0xff,
            (message.B >> 8) & 0xff, message.B & 0xff
            */
        };

        QByteArray Msg;
        Msg.reserve(sizeof(HEADER) + sizeof(PAYLOAD_PER_LIGHT) * eGroup.lights.size());

        Msg.append((char*)HEADER, sizeof(HEADER));

        for (auto& light : eGroup.lights)
        {
            const double width = std::max(1.0 - std::abs(light.x), 0.3);
            const double height = std::max(1.0 - std::abs(light.z), 0.3);

            const int minX = std::round((std::max(light.x - width, -1.0) + 1.0) * screen.width / 2.0);
            const int maxX = std::round((std::min(light.x + width, 1.0) + 1.0) * screen.width / 2.0);

            const int minY = std::round((std::max(light.z - height, -1.0) + 1.0) * screen.height / 2.0);
            const int maxY = std::round((std::min(light.z + height, 1.0) + 1.0) * screen.height / 2.0);

            quint64 R = 0;
            quint64 G = 0;
            quint64 B = 0;
            quint64 samples = 0;

            for (int y = minY; y < maxY; ++y)
            {
                int rowOffset = y * screen.width;
                for (int x = minX; x < maxX; ++x)
                {
                    R += screen.screen[rowOffset + x].R;
                    G += screen.screen[rowOffset + x].G;
                    B += screen.screen[rowOffset + x].B;
                    samples += screen.screen[rowOffset + x].samples;
                }
            }

            double X, Y, L;
            double dR = (double)R / samples / 255.0;
            double dG = (double)G / samples / 255.0;
            double dB = (double)B / samples / 255.0;

            rgb_to_xyz(dR, dG, dB, X, Y, L);

            R = X * 0xffff;
            G = Y * 0xffff;
            B = L * 0xffff;

            const unsigned char payload[] = {
                0x00, 0x00, ((uint8_t)light.id.toInt()),

                (R >> 8) & 0xff, (R & 0xff),
                (G >> 8) & 0xff, (G & 0xff),
                (B >> 8) & 0xff, (B & 0xff)
            };

            Msg.append((char*)payload, sizeof(payload));
        }


        screenMutex.unlock();

        int len = Msg.size();

        do ret = mbedtls_ssl_write(&ssl, (unsigned char *) Msg.data(), len);
        while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
            ret == MBEDTLS_ERR_SSL_WANT_WRITE);

        if (ret < 0)
        {
            break;
        }

        messageSendElapsed  = timer.elapsed();
        emit messageSendElapsedChanged();
        emit lightColorsChanged();

        QThread::msleep(5);

        if (stopRequested)
        {
            break;
        }
    }

    if (ret < 0)
    {
        switch (ret)
        {
        case MBEDTLS_ERR_SSL_TIMEOUT:
            qWarning() << " timeout";
            if (retry_left-- > 0)
                goto send_request;
            goto exit;

        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
            qWarning() << " connection was closed gracefully";
            ret = 0;
            goto close_notify;

        default:
            qWarning() << " mbedtls_ssl_read returned" << ret;
            goto exit;
        }
    }

    /*
    * 8. Done, cleanly close the connection
    */
close_notify:
    qDebug() << "Closing the connection...";

    /* No error checking, the connection might be closed already */
    do ret = mbedtls_ssl_close_notify(&ssl);
    while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);
    ret = 0;

    qDebug() << "Done";

    /*
    * 9. Final clean-ups and exit
    */
exit:

#ifdef MBEDTLS_ERROR_C
    if (ret != 0)
    {
        char error_buf[100];
        mbedtls_strerror(ret, error_buf, 100);
        mbedtls_printf("Last error was: %d - %s\n\n", ret, error_buf);
    }
#endif

    mbedtls_net_free(&server_fd);

    mbedtls_x509_crt_free(&cacert);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
}

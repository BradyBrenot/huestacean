#include <QNetworkReply>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QThread>
#include <QDateTime>

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

#include "huebridge.h"

QString SETTING_USERNAME = "Bridge/Username";
QString SETTING_CLIENTKEY = "Bridge/clientkey";

HueBridge::HueBridge(QObject *parent)
    : QObject(parent),
    eThread(nullptr),
    manuallyAdded(false)
{
    connect(&qnam, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(replied(QNetworkReply*)));

    connect(this, SIGNAL(connectedChanged()),
        this, SLOT(requestGroups()));

    setMessage("Not connected.");
    setConnected(false);
}

void HueBridge::connectToBridge()
{
    setMessage("Connecting.");
    setConnected(false);

    QSettings settings;
    if (settings.contains(SETTING_USERNAME) && settings.contains(SETTING_CLIENTKEY))
    {
        //Verify existing registration
        //QNetworkRequest qnr(QUrl(QString("http://192.168.0.102/api/%1/config").arg(settings.value("username").toString())));
        QNetworkRequest qnr(QUrl(QString("http://192.168.0.102/api/%1/config").arg(settings.value(SETTING_USERNAME).toString())));
        qnam.get(qnr);
    }
    else
    {
        //Register
        QNetworkRequest qnr(QUrl("http://192.168.0.102/api"));
        qnr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject json;
        json.insert("devicetype", "huestacean#windows");
        json.insert("generateclientkey", true);

        qnam.post(qnr, QJsonDocument(json).toJson());
    }
}
void HueBridge::resetConnection()
{
    QSettings settings;
    settings.remove(SETTING_USERNAME);
    settings.remove(SETTING_CLIENTKEY);
    connectToBridge();
}
void HueBridge::requestGroups()
{
    QSettings settings;
    QNetworkRequest qnr(QUrl(QString("http://192.168.0.102/api/%1/groups").arg(settings.value(SETTING_USERNAME).toString())));
    qnam.get(qnr);
}

void HueBridge::replied(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->request().url().toString().endsWith("/api"))
    {
        QSettings settings;
        QByteArray data = reply->readAll();

        qDebug() << "We got: \n" << QString::fromUtf8(data.data());

        QJsonDocument replyJson = QJsonDocument::fromJson(data);
        if (!replyJson.isArray() || replyJson.array().size() == 0)
        {
            setMessage("Bad reply from bridge");
            return;
        }

        QJsonObject obj = replyJson.array()[0].toObject();
        if (obj.contains(QString("success")))
        {
            //Connected!
            QString username = obj["success"].toObject()["username"].toString();
            QString clientkey = obj["success"].toObject()["clientkey"].toString();

            qDebug() << "Registered with bridge. Username:" << username << "Clientkey:" << clientkey;

            settings.setValue(SETTING_USERNAME, username);
            settings.setValue(SETTING_CLIENTKEY, clientkey);

            setMessage("Registered and connected to bridge!");

            setConnected(true);
        }
        else
        {
            if (obj[QString("error")].toObject()[QString("type")].toInt() == 101)
            {
                setMessage("Press the link button!");
                emit wantsLinkButton();
            }
        }
    }
    else if (reply->request().url().toString().endsWith("/config"))
    {
        QByteArray data = reply->readAll();

        QJsonDocument replyJson = QJsonDocument::fromJson(data);
        if (!replyJson.isObject() || !replyJson.object().contains("whitelist"))
        {
            qDebug() << "Connection failed" << replyJson.isObject() << replyJson.object().contains("whitelist");
            resetConnection();
        }
        else
        {
            setMessage("Connected! Reused old connection!");
            qDebug() << "Reply" << replyJson;
            setConnected(true);
        }
    }
    else if (reply->request().url().toString().endsWith("/groups"))
    {
        QByteArray data = reply->readAll();
        QJsonDocument replyJson = QJsonDocument::fromJson(data);
        qDebug().noquote() << replyJson.toJson(QJsonDocument::Indented);
    }
    else if (reply->request().url().toString().endsWith("/groups/6"))
    {
        qDebug() << "ENABLED";
        QByteArray data = reply->readAll();
        QJsonDocument replyJson = QJsonDocument::fromJson(data);
        qDebug().noquote() << replyJson.toJson(QJsonDocument::Indented);

        handleStreamingEnabled();
    }
    else
    {
        qCritical() << "Received reply to unknown request" << reply->request().url();
    }
}

void HueBridge::testEntertainment()
{
    QSettings settings;

    QNetworkRequest qnr(QUrl(QString("http://192.168.0.102/api/%1/groups/6").arg(settings.value(SETTING_USERNAME).toString())));
    qnr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject stream;
    stream.insert("active", true);

    QJsonObject body;
    body.insert("stream", stream);

    qnam.put(qnr, QJsonDocument(body).toJson());
}


//---------------------------------------------------------------------------------
// Entertainment API

//http://www.easyrgb.com/en/math.php
void rgb_to_xyz(double& r, double& g, double& b, double& x, double& y, double& z)
{
    //sR, sG and sB (Standard RGB) input range = 0 ÷ 255
    //X, Y and Z output refer to a D65/2° standard illuminant.

    double var_R = (r / 255.0);
    double var_G = (g / 255.0);
    double var_B = (b / 255.0);

    if (var_R > 0.04045) var_R = std::pow(((var_R + 0.055) / 1.055), 2.4);
    else                   var_R = var_R / 12.92;
    if (var_G > 0.04045) var_G = std::pow(((var_G + 0.055) / 1.055), 2.4);
    else                   var_G = var_G / 12.92;
    if (var_B > 0.04045) var_B = std::pow(((var_B + 0.055) / 1.055), 2.4);
    else                   var_B = var_B / 12.92;

    var_R = var_R * 100.;
    var_G = var_G * 100.;
    var_B = var_B * 100.;

    x = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
    y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
    z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
}

void HueBridge::handleStreamingEnabled()
{
    if (eThread != nullptr) {
        return;
    }

    eThread = new EntertainmentCommThread(this);
    connect(eThread, &EntertainmentCommThread::finished, this, &HueBridge::entertainmentThreadFinished);
    eThread->start();

    framegrabber = SL::Screen_Capture::CreateCaptureConfiguration([]() {
        auto monitors = SL::Screen_Capture::GetMonitors();
        return monitors;
    })->onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
        Q_UNUSED(monitor);

        double r = 0.;
        double g = 0.;
        double b = 0.;

        const int Height = SL::Screen_Capture::Height(img);
        const int Width = SL::Screen_Capture::Width(img);

        const int RowPadding = SL::Screen_Capture::RowPadding(img);
        const int Pixelstride = img.Pixelstride;

        const int skip = 20;
        int s = 0;

        const float n = Height * Width * 1.0 / skip;

        const unsigned char* src = SL::Screen_Capture::StartSrc(img);
        for (int y = 0; y < Height; ++y)
        {
            src += s;
            for (int x = 0; x < Width; x += skip)
            {
                b += (src[0]) / n;
                g += (src[1]) / n;
                r += (src[2]) / n;
                src += Pixelstride * skip;
            }

            src += RowPadding;
        }

        static EntertainmentMessage msg;

        msg.isXY = true;

        double x, y, z;

        //todo:
        //ideally average would be of x/y/z values, not of rgb (do this earlier)
        rgb_to_xyz(r, g, b, x, y, z);

        msg.R = static_cast<uint16_t>((x / (x + y + z)) * 0xffff);
        msg.G = static_cast<uint16_t>((y / (x + y + z)) * 0xffff);
        msg.B = static_cast<uint16_t>((std::max(std::max(r, g), b) / 255.) * 0xffff);
        eThread->threadsafe_setMessage(msg);
        //qDebug() << r << g << b << msg.B;

    })->start_capturing();

    framegrabber->setFrameChangeInterval(std::chrono::milliseconds(5));
}

void HueBridge::entertainmentThreadFinished()
{
    setMessage("Entertainment over");

    framegrabber.reset();

    eThread->deleteLater();
    eThread = nullptr;
}

EntertainmentCommThread::EntertainmentCommThread(QObject *parent) : QThread(parent)
{

}

void EntertainmentCommThread::threadsafe_setMessage(const EntertainmentMessage& inMessage)
{
    messageMutex.lock();
    message = inMessage;
    messageMutex.unlock();
}

//simple dtls test using mbedtls. Never ends!
void EntertainmentCommThread::run()
{
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
#define SERVER_ADDR "192.168.0.102"

    int ret, len;
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
    QSettings settings;
    QByteArray pskArray = settings.value(SETTING_CLIENTKEY).toString().toUtf8();
    QByteArray pskRawArray = QByteArray::fromHex(pskArray);


    QByteArray pskIdArray = settings.value(SETTING_USERNAME).toString().toUtf8();
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
    qDebug() << "Connecting to udp" << SERVER_ADDR << SERVER_PORT;

    if ((ret = mbedtls_net_connect(&server_fd, SERVER_ADDR,
        SERVER_PORT, MBEDTLS_NET_PROTO_UDP)) != 0)
    {
        qCritical() << "mbedtls_net_connect FAILED" << ret;
        goto exit;
    }

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

    /*
     * 4. Handshake
     */
    qDebug() << "Performing the DTLS handshake...";

    do ret = mbedtls_ssl_handshake(&ssl);
    while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
        ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (ret != 0)
    {
        qCritical() << "mbedtls_ssl_handshake FAILED" << ret;
        goto exit;
    }

    /*
     * 6. Write the echo request
     */
send_request:
    while (true)
    {
        messageMutex.lock();

        char MESSAGE[] = {
              'H', 'u', 'e', 'S', 't', 'r', 'e', 'a', 'm', //protocol

              0x01, 0x00, //version 1.0

              0x01, //sequence number 1

              0x00, 0x00, //Reserved write 0’s

              message.isXY ? 0x01 : 0x00,

              0x00, // Reserved, write 0’s

              0x001, 0x00, 0x06, //light ID

              //color: 16 bpc
              (message.R >> 8) & 0xff, message.R & 0xff,
              (message.G >> 8) & 0xff, message.G & 0xff,
              (message.B >> 8) & 0xff, message.B & 0xff
        };

        messageMutex.unlock();

        len = sizeof(MESSAGE);

        do ret = mbedtls_ssl_write(&ssl, (unsigned char *)MESSAGE, len);
        while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
            ret == MBEDTLS_ERR_SSL_WANT_WRITE);

        if (ret < 0)
        {
            break;
        }

        QThread::msleep(5);
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

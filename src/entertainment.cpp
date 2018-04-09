#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QElapsedTimer>

//----------- mbedtls

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

//----------- END mbedtls

#include "huestacean.h"
#include "huebridge.h"
#include "entertainment.h"
#include "utility.h"

EntertainmentGroup::EntertainmentGroup(HueBridge *parent)
    : BridgeObject(parent),
    name(),
    lights(),
    isStreaming(false),
    eThread(nullptr)
{
    emit propertiesChanged();

    connect(qnam, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(replied(QNetworkReply*)));
}

EntertainmentGroup::~EntertainmentGroup()
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

void EntertainmentGroup::updateLightXZ(int index, float x, float z, bool save)
{
    lights[index].x = x;
    lights[index].z = z;

    if (eThread != nullptr) {
        EntertainmentCommThreadEGroupScopedLock g(eThread);
        g->updateLightXZ(index, x, z, false);
    }

    if (!save || bridgeParent() == nullptr) {
        return;
    }

    QNetworkRequest qnr = bridgeParent()->makeRequest(QString("/groups/%1").arg(id));
    qnr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject locations;
    for (EntertainmentLight& light : lights)
    {
        QJsonArray arr;
        arr.append(light.x);
        arr.append(light.y);
        arr.append(light.z);
        locations.insert(light.id, arr);
    }

    QJsonObject body;
    body.insert("locations", locations);

    qnam->put(qnr, QJsonDocument(body).toJson());
}

void EntertainmentGroup::startStreaming(GetColorFunction getColorFunc)
{
    getColor = getColorFunc;
    askBridgeToToggleStreaming(true);
}

void EntertainmentGroup::stopStreaming()
{
    if (eThread)
    {
        eThread->stop();
    }
    else
    {
        askBridgeToToggleStreaming(false);
    }
}

void EntertainmentGroup::shutDownImmediately()
{
    if (eThread)
    {
        eThread->stop();
    }
}
bool EntertainmentGroup::hasRunningThread()
{
    if (eThread)
    {
        return eThread->isRunning();
    }
    return false;
}

void EntertainmentGroup::askBridgeToToggleStreaming(bool enable)
{
    QNetworkRequest qnr = bridgeParent()->makeRequest(QString("/groups/%1").arg(id));
    qnr.setOriginatingObject(this);
    qnr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject stream;
    stream.insert("active", enable);

    QJsonObject body;
    body.insert("stream", stream);

    qnam->put(qnr, QJsonDocument(body).toJson());
}

void EntertainmentGroup::entertainmentThreadConnected()
{
    isStreaming = true;
    emit isStreamingChanged(true);
}
void EntertainmentGroup::entertainmentThreadFinished()
{
    if (!isStreaming)
    {
        emit udpConnectFailed();
    }

    askBridgeToToggleStreaming(false);
}

void EntertainmentGroup::replied(QNetworkReply *reply)
{
    if (reply->request().originatingObject() != this)
        return;

    QByteArray data = reply->readAll();

    if (!isStreaming)
    {
        if (data.contains("false"))
        {
            emit failedToStartStreaming();
        }
        else if (data.contains("true"))
        {
            eThread = new EntertainmentCommThread(this, bridgeParent()->username, bridgeParent()->clientkey, bridgeParent()->address.toString(), *this, getColor);

            connect(eThread, SIGNAL(connected()),
                this, SLOT(entertainmentThreadConnected()));

            connect(eThread, SIGNAL(finished()),
                this, SLOT(entertainmentThreadFinished()));

            eThread->start();
        }
    }
    else
    {
        if (data.contains("false"))
        {
            isStreaming = false;
            emit isStreamingChanged(false);
        }
    }
}

EntertainmentCommThread::EntertainmentCommThread(QObject *parent, QString inUsername, QString inClientkey, QString inAddress, const EntertainmentGroup& inEGroup, GetColorFunction getColorFunc)
    : QThread(parent),
    username(inUsername),
    clientkey(inClientkey),
    stopRequested(false),
    address(inAddress),
    eGroup(inEGroup),
    getColor(getColorFunc)
{
    //We make a local copy of the group and lights for thread safety, but
    //must prevent their being destroyed if inEGroup's parent is destroyed
    eGroup.setParent(nullptr);
    for (auto& light : eGroup.lights)
    {
        light.setParent(nullptr);
    }
}

void EntertainmentCommThread::stop()
{
    stopRequested = true;
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

    mbedtls_debug_set_threshold(4);

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

    for (int attempt = 0; attempt < 4; ++attempt)
    {
        qDebug() << "handshake attempt" << attempt;
        mbedtls_ssl_conf_handshake_timeout(&conf, 400, 1000);
        do ret = mbedtls_ssl_handshake(&ssl);
        while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
            ret == MBEDTLS_ERR_SSL_WANT_WRITE);

        if (ret == 0)
            break;

        msleep(200);
    }

    qDebug() << "handshake result" << ret;

    if (ret != 0)
    {
        qCritical() << "mbedtls_ssl_handshake FAILED" << ret;
        goto exit;
    }

    qDebug() << "Handshake successful. Connected!";

    if (stopRequested)
        goto exit;

    emit connected();

    /*
    * 6. Send messages repeatedly until we lose connection or are told to stop
    */
send_request:
    while (true)
    {
        static QElapsedTimer timer;
        timer.restart();

        static const uint8_t HEADER[] = {
            'H', 'u', 'e', 'S', 't', 'r', 'e', 'a', 'm', //protocol

            0x01, 0x00, //version 1.0

            0x01, //sequence number 1

            0x00, 0x00, //Reserved write 0’s

            0x01,

            0x00, // Reserved, write 0’s
        };

        static const uint8_t PAYLOAD_PER_LIGHT[] =
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

        eGroupMutex.lock();
        Msg.reserve(sizeof(HEADER) + sizeof(PAYLOAD_PER_LIGHT) * eGroup.lights.size());

        Msg.append((char*)HEADER, sizeof(HEADER));

        for (auto& light : eGroup.lights)
        {
            double newL = 0.0;
            double newC = 0.0;
            double newh = 0.0;

			double minBrightness, maxBrightness, brightnessBoost;

            if (!getColor(light, light.L, light.C, light.h, newL, newC, newh, minBrightness, maxBrightness, brightnessBoost))
                continue;

			light.L = newL;
			light.C = newC;
			light.h = newh;

			double X, Y, Z, x, y;
			Color::LCh_to_XYZ(newL, newC, newh, X, Y, Z);
			Color::XYZ_to_xy(X, Y, Z, x, y);
			Color::FitInGamut(x, y);

            quint64 R = x * 0xffff;
            quint64 G = y * 0xffff;

			double brightness = std::min(1.0, Y * brightnessBoost);
			brightness = brightness * (maxBrightness - minBrightness) + minBrightness;
            quint64 B = brightness * 0xffff;

            const uint8_t payload[] = {
                0x00, 0x00, ((uint8_t)light.id.toInt()),

                static_cast<uint8_t>((R >> 8) & 0xff), static_cast<uint8_t>(R & 0xff),
                static_cast<uint8_t>((G >> 8) & 0xff), static_cast<uint8_t>(G & 0xff),
                static_cast<uint8_t>((B >> 8) & 0xff), static_cast<uint8_t>(B & 0xff)
            };

            Msg.append((char*)payload, sizeof(payload));
        }
        eGroupMutex.unlock();

        int len = Msg.size();

        do ret = mbedtls_ssl_write(&ssl, (unsigned char *)Msg.data(), len);
        while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
            ret == MBEDTLS_ERR_SSL_WANT_WRITE);

        if (ret < 0)
        {
            break;
        }

        messageSendElapsed = timer.elapsed();
        emit messageSendElapsedChanged();

        //TODO: make this delay customizable?
        QThread::msleep(30);

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

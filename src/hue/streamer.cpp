#include "hue/streamer.h"
#include "common/math.h"

#include <thread>
#include <chrono>

#include <QDebug>
#include <QHostAddress>

#define READ_TIMEOUT_MS 1000
#define MAX_RETRY       5
#define DEBUG_LEVEL 2000
#define SERVER_PORT "2100"
#define SERVER_NAME "Hue"

using namespace Hue;
using namespace std::chrono_literals;
using namespace Math;

Streamer::Streamer(const Bridge& bridge) :
	pers("dtls_client"),
	retry_left(5)
{
	isValid = true;

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

	mbedtls_debug_set_threshold(4);

	/*
	* -1. Load psk
	*/
	QByteArray pskArray = bridge.clientkey.c_str();
	QByteArray pskRawArray = QByteArray::fromHex(pskArray);


	QByteArray pskIdArray = bridge.username.c_str();
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
		(const unsigned char*)pers.c_str(),
		pers.size())) != 0)
	{
		mbedtls_printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
		goto exit;
	}

	/*
	* 1. Start the connection
	*/
	qDebug() << "Connecting to udp" << QHostAddress(bridge.address).toString().toUtf8() << SERVER_PORT;

	if ((ret = mbedtls_net_connect(&server_fd, QHostAddress(bridge.address).toString().toUtf8(),
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
		(const unsigned char*)pskIdRawArray.data(), pskIdRawArray.length() * sizeof(char))))
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

	for (int attempt = 0; attempt < 4; ++attempt)
	{
		qDebug() << "handshake attempt" << attempt;
		mbedtls_ssl_conf_handshake_timeout(&conf, 400, 1000);
		do ret = mbedtls_ssl_handshake(&ssl);
		while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
			ret == MBEDTLS_ERR_SSL_WANT_WRITE);

		if (ret == 0)
			break;

		std::this_thread::sleep_for(200ms);
	}

	qDebug() << "handshake result" << ret;

	if (ret != 0)
	{
		qCritical() << "mbedtls_ssl_handshake FAILED" << ret;
		goto exit;
	}

	qDebug() << "Handshake successful. Connected!";
	return;

	exit:
		isValid = false;
}

Streamer::~Streamer()
{
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

void Streamer::Upload(const std::vector<std::tuple<uint32_t, Math::XyyColor>>& LightsToUpload)
{
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

	Msg.reserve(static_cast<int>(sizeof(HEADER) + sizeof(PAYLOAD_PER_LIGHT) * LightsToUpload.size()));

	Msg.append((char*)HEADER, sizeof(HEADER));

	for (auto& light : LightsToUpload)
	{
		auto& [ id, color ] = light;
		quint64 R = color.x * 0xffff;
		quint64 G = color.y * 0xffff;
		double brightness = color.Y;
		quint64 B = brightness * 0xffff;

		qDebug() << "sending" << id << " || " << R << G << B;

		const uint8_t payload[] = {
			0x00, 0x00, static_cast<uint8_t>(id),

			static_cast<uint8_t>((R >> 8) & 0xff), static_cast<uint8_t>(R & 0xff),
			static_cast<uint8_t>((G >> 8) & 0xff), static_cast<uint8_t>(G & 0xff),
			static_cast<uint8_t>((B >> 8) & 0xff), static_cast<uint8_t>(B & 0xff)
		};

		Msg.append((char*)payload, sizeof(payload));
	}

	int len = Msg.size();

	do ret = mbedtls_ssl_write(&ssl, (unsigned char*)Msg.data(), len);
	while (ret == MBEDTLS_ERR_SSL_WANT_READ ||
		ret == MBEDTLS_ERR_SSL_WANT_WRITE);
	
	bool bWantsToCloseCleanly = false;
	bool bWantsToExit = false;

	if (ret < 0)
	{
		switch (ret)
		{
		case MBEDTLS_ERR_SSL_TIMEOUT:
			if (retry_left-- > 0)
				return;
			bWantsToExit = true;
			break;

		case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
			qWarning() << " connection was closed gracefully";
			ret = 0;
			bWantsToCloseCleanly = true;
			break;

		default:
			qWarning() << " mbedtls_ssl_read returned" << ret;
			bWantsToExit = true;
		}
	}

	if (bWantsToCloseCleanly)
	{
		/* No error checking, the connection might be closed already */
		do ret = mbedtls_ssl_close_notify(&ssl);
		while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);
		ret = 0;
		bWantsToExit = true;
	}
	
	if (bWantsToExit)
	{
		isValid = false;
	}
	
}
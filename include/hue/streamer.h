#pragma once

#include "hue/bridge.h"
#include "common/math.h"

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

#include <atomic>

#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/timing.h"

//----------- END mbedtls

namespace Hue
{
	struct Streamer
	{
		std::atomic_bool isValid;

		int ret;
		mbedtls_net_context server_fd;
		std::string pers = "dtls_client";
		int retry_left;

		mbedtls_entropy_context entropy;
		mbedtls_ctr_drbg_context ctr_drbg;
		mbedtls_ssl_context ssl;
		mbedtls_ssl_config conf;
		mbedtls_x509_crt cacert;
		mbedtls_timing_delay_context timer;

		Streamer(const Bridge& bridge);
		~Streamer();
		void Upload(const std::vector<std::tuple<uint32_t, Math::XyyColor>>& LightsToUpload);
	};
};

/*
 * Copyright 2019 Cypress Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file
 *  cy_tls_stack_specific.h
 *  MBEDTLS specific definitions
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "ssl.h"
#include "ssl_internal.h"
#include "ctr_drbg.h"
#include "entropy.h"
#include "cipher.h"
#include "md4.h"
#include "sha1.h"
#include "des.h"


/* IMPLEMENTATION NOTE: Core supplicant implementation should not access any of the structure members defined in this file */

typedef struct mbedtls_ssl_context cy_tls_workspace_t;
typedef struct mbedtls_ssl_session cy_tls_session_t;
typedef struct mbedtls_x509_crt cy_x509_crt_t;
typedef struct mbedtls_pk_context cy_pk_context_t;
typedef struct mbedtls_entropy_context  cy_entropy_context_t;
typedef struct mbedtls_ctr_drbg_context cy_ctr_drbg_context_t;

/**
 * Secure TCP identity
 */
typedef struct
{
    cy_pk_context_t private_key; /**< Private key context */
    cy_x509_crt_t   certificate; /**< X.509 certificate */
} cy_tls_identity_t;

#ifdef __cplusplus
} /*extern "C" */
#endif

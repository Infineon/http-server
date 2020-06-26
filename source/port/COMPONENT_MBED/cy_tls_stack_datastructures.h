/*
 * Copyright 2020 Cypress Semiconductor Corporation
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
 *  cy_tls_stack_datastructures.h
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


#define TLS_MASTER_SESSION_KEY_LENGTH         (48)
#define TLS_RANDOM_BYTES_LENGTH               (64)

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

/**
 * Secure TCP context
 */
typedef struct
{
    void                    *usr_data;                                      /**< User data */
    char*                   peer_cn;                                        /**< Peer common name (optional) */
    cy_tls_session_t        *session;                                       /**< Session pointer for TLS resumption */
    cy_tls_workspace_t      context;                                        /**< Internal SSL context object */
    cy_tls_identity_t       *identity;                                      /**< TLS identity */
    cy_x509_crt_t*          root_ca_certificates;                           /**< Context specific root CA chain */
    cy_entropy_context_t    entropy;                                        /**< Internal entropy context */
    cy_ctr_drbg_context_t   ctr_drbg;                                       /**< Internal context for RNG */
    unsigned char           randbytes[TLS_RANDOM_BYTES_LENGTH];             /**< Initial TLS handshake random bytes */
    unsigned char           master_session_key[TLS_MASTER_SESSION_KEY_LENGTH]; /**< Master key for TLS handshake */
    int                     resume;                                         /**< Flag for session resumption */
    int                     (*supplicant_tls_prf)( const unsigned char *, size_t, const char *, const unsigned char *, size_t, unsigned char *, size_t ); /**< Internal PRF function pointer */
} cy_tls_context_t;

#ifdef __cplusplus
} /*extern "C" */
#endif

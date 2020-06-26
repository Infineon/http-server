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
 *  cy_tls_wrapper.h
 *  TLS wrapper functions
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cy_tls_stack_datastructures.h"
#include "cy_result.h"

typedef enum
{
    CY_TRUE = 0,
    CY_FALSE
} cy_bool_t;

typedef enum
{
    TLS_CERT_NO_VERIFICATION       = 0,
    TLS_CERT_VERIFICATION_OPTIONAL = 1,
    TLS_CERT_VERIFICATION_REQUIRED = 2,
} cy_tls_certificate_verification_t;


/** Initializes TLS context handle
 *
 * @param[in] context   : A pointer to a cy_tls_context_t context object that will be initialized.
 *                        The context object is analogous to a cookie which has all the information to process a TLS message.
 *                        This is the entity that has all the book-keeping information (TLS handshake state, TLS session etc.).
 * @param[in]  identity : A pointer to a cy_tls_identity_t object initialized with @ref cy_tls_init_identity.
 * @param[in]  peer_cn  : Expected peer CommonName (or NULL)
 *
 * @return cy_rslt_t    : CY_RESULT_SUCCESS on success, refer to cy_result_mw.h in connectivity-utilities for error
 */
cy_rslt_t cy_tls_init_context( cy_tls_context_t* context, cy_tls_identity_t* identity, char* peer_cn );

/** De-initialize a previously initialized TLS context
 *
 * @param[in] context : A pointer to a cy_tls_context_t context object
 *
 * @return cy_rslt_t    : CY_RESULT_SUCCESS on success, refer to cy_result_mw.h in connectivity-utilities for error
 */
cy_rslt_t cy_tls_deinit_context( cy_tls_context_t* context );

/** Initialize the trusted root CA certificates specific to the TLS context.
 *
 * @param[in] context                 : A pointer to a cy_tls_context_t context object
 * @param[in] trusted_ca_certificates : A chain of x509 certificates in PEM or DER format.
 *                                      This chain of certificates comprise the public keys of the signing authorities.
 *                                      During the handshake, these public keys are used to verify the authenticity of the peer
 * @param[in] cert_length             : Certificate length excluding 'null' termination character.
 *
 * @return cy_rslt_t    : CY_RESULT_SUCCESS on success, refer to cy_result_mw.h in connectivity-utilities for error
 */
cy_rslt_t cy_tls_set_context_root_ca_certificates( cy_tls_context_t* context, const char* trusted_ca_certificates, const uint32_t cert_length );

/** Initialise the trusted root CA certificates
 *
 *  Initialises the collection of trusted root CA certificates used to verify received certificates
 *
 * @param[in] trusted_ca_certificates : A chain of x509 certificates in PEM or DER format.
 *                                                   This chain of certificates comprise the public keys of the signing authorities.
 *                                                   During the handshake, these public keys are used to verify the authenticity of the peer
 * @param[in] cert_length             : Certificate length excluding 'null' termination character.
 *
 * @return cy_rslt_t    : CY_RESULT_SUCCESS on success, refer to cy_result_mw.h in connectivity-utilities for error
 */
cy_rslt_t cy_tls_init_root_ca_certificates( const char* trusted_ca_certificates, const uint32_t cert_length );

/** De-initialise the trusted root CA certificates
 *
 * @return cy_rslt_t    : CY_RESULT_SUCCESS on success, refer to cy_result_mw.h in connectivity-utilities for error
 */
cy_rslt_t cy_tls_deinit_root_ca_certificates( void );

/** Initializes a TLS identity using a supplied certificate and private key
 *
 * @param[in]  identity           : A pointer to a cy_tls_identity_t object that will be initialized
 *                                  The identity is a data structure that encompasses the device's own certificate/key.
 * @param[in]  private_key        : The server private key in binary format. This key is used to sign the handshake message
 * @param[in]  key_length         : Private key length excluding 'null' termination character.
 * @param[in]  certificate_data   : The server x509 certificate in PEM or DER format
 * @param[in]  certificate_length : The length of the certificate excluding 'null' termination character.
 *
 * @return cy_rslt_t    : CY_RESULT_SUCCESS on success, refer to cy_result_mw.h in connectivity-utilities for error
 */
cy_rslt_t cy_tls_init_identity( cy_tls_identity_t* identity, const char* private_key, const uint32_t key_length, const uint8_t* certificate_data, uint32_t certificate_length );

/** DeiInitializes a TLS identity
 *
 * @param[in] tls_identity    : A pointer to a cy_tls_identity_t object that will be de-initialised
 *
 * @return cy_rslt_t    : CY_RESULT_SUCCESS on success, refer to cy_result_mw.h in connectivity-utilities for error
 */
cy_rslt_t cy_tls_deinit_identity( cy_tls_identity_t* identity );

/** Start TLS on a TCP Connection with a particular set of cipher suites
 *
 * Start Transport Layer Security (successor to SSL) on a TCP Connection
 * (Cipher suites are configured using config.h in mbedtls)
 *
 * @param[in,out] tls_context  : The tls context to work with
 * @param[in,out] referee      : Transport reference - e.g. TCP socket
 * @param[in]     verification : Indicates whether to verify the certificate chain against a root server.
 *
 * @return cy_rslt_t    : CY_RESULT_SUCCESS on success, refer to cy_result_mw.h in connectivity-utilities for error
 *
 */
cy_rslt_t cy_tls_generic_start_tls_with_ciphers( cy_tls_context_t* tls_context, void* referee, cy_tls_certificate_verification_t verification );

/** @} */


#ifdef __cplusplus
} /*extern "C" */
#endif

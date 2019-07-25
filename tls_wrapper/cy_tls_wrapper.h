/*
 * Copyright 2019, Cypress Semiconductor Corporation or a subsidiary of
 * Cypress Semiconductor Corporation. All Rights Reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software"), is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
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

#include "cy_tls_stack_specific.h"
#include "cy_result.h"

#define TLS_WRAPPER_DEBUG( x )  //printf x
#define TLS_WRAPPER_INFO( x )   //printf x

#define TLS_MASTER_SESSION_KEY_LENGTH         (48)
#define TLS_RANDOM_BYTES_LENGTH               (64)

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


typedef struct
{
    void                    *usr_data;
    char*                   peer_cn;
    cy_tls_session_t        *session;                  /* This session pointer is only used to resume connection for client, If application/library wants to resume connection it needs to pass pointer of previous stored session */
    cy_tls_workspace_t      context;
    cy_tls_identity_t       *identity;
    cy_x509_crt_t*          root_ca_certificates;      /* Context specific root-ca-chain */
    cy_entropy_context_t    entropy;
    cy_ctr_drbg_context_t   ctr_drbg;
    unsigned char           randbytes[TLS_RANDOM_BYTES_LENGTH];        /*!<  random bytes            */
    unsigned char           master_session_key[TLS_MASTER_SESSION_KEY_LENGTH];
    int                     resume;
    int                     (*supplicant_tls_prf)( const unsigned char *, size_t, const char *, const unsigned char *, size_t, unsigned char *, size_t );

} cy_tls_context_t;

/** Initializes TLS context handle
 *
 * @param[in] context   : A pointer to a cy_tls_context_t context object that will be initialized.
 *                        The context object is analogous to a cookie which has all the information to process a TLS message.
 *                        This is the entity that has all the book-keeping information (TLS handshake state, TLS session etc.).
 * @param[in]  identity : A pointer to a cy_tls_identity_t object initialized with @ref cy_tls_init_identity.
 * @param[in]  peer_cn  : Expected peer CommonName (or NULL)
 *
 * @return @ref cy_rslt_t
 */
cy_rslt_t cy_tls_init_context( cy_tls_context_t* context, cy_tls_identity_t* identity, char* peer_cn );

/** De-initialize a previously initialized TLS context
 *
 * @param[in] context : A pointer to a cy_tls_context_t context object
 *
 * @return @ref cy_rslt_t
 */
cy_rslt_t cy_tls_deinit_context( cy_tls_context_t* context );

/** Initialize the trusted root CA certificates specific to the TLS context.
 *
 * @param[in] context                 : A pointer to a cy_tls_context_t context object
 * @param[in] trusted_ca_certificates : A chain of x509 certificates in PEM or DER format.
 *                                      This chain of certificates comprise the public keys of the signing authorities.
 *                                      During the handshake, these public keys are used to verify the authenticity of the peer
 * @param[in] cert_length             : Certificate length
 *
 * @return @ref cy_rslt_t
 */
cy_rslt_t cy_tls_set_context_root_ca_certificates( cy_tls_context_t* context, const char* trusted_ca_certificates, const uint32_t cert_length );

/** Initialise the trusted root CA certificates
 *
 *  Initialises the collection of trusted root CA certificates used to verify received certificates
 *
 * @param[in] trusted_ca_certificates : A chain of x509 certificates in PEM or DER format.
 *                                                   This chain of certificates comprise the public keys of the signing authorities.
 *                                                   During the handshake, these public keys are used to verify the authenticity of the peer
 * @param[in] cert_length             : Certificate length
 *
 * @return @ref cy_rslt_t
 */
cy_rslt_t cy_tls_init_root_ca_certificates( const char* trusted_ca_certificates, const uint32_t cert_length );

/** De-initialise the trusted root CA certificates
 *
 * @return @ref cy_rslt_t
 */
cy_rslt_t cy_tls_deinit_root_ca_certificates( void );

/** Initializes a TLS identity using a supplied certificate and private key
 *
 * @param[in]  identity           : A pointer to a cy_tls_identity_t object that will be initialized
 *                                  The identity is a data structure that encompasses the device's own certificate/key.
 * @param[in]  private_key        : The server private key in binary format. This key is used to sign the handshake message
 * @param[in]  key_length         : Private key length
 * @param[in]  certificate_data   : The server x509 certificate in PEM or DER format
 * @param[in]  certificate_length : The length of the certificate
 *
 * @return @ref cy_rslt_t
 */
cy_rslt_t cy_tls_init_identity( cy_tls_identity_t* identity, const char* private_key, const uint32_t key_length, const uint8_t* certificate_data, uint32_t certificate_length );

/** DeiInitializes a TLS identity
 *
 * @param[in] tls_identity    : A pointer to a cy_tls_identity_t object that will be de-initialised
 *
 * @return @ref cy_rslt_t
 */
cy_rslt_t cy_tls_deinit_identity( cy_tls_identity_t* tls_identity );

/** Start TLS on a TCP Connection with a particular set of cipher suites
 *
 * Start Transport Layer Security (successor to SSL) on a TCP Connection
 * (Cipher suites are configured using config.h in mbedtls)
 *
 * @param[in,out] tls_context  : The tls context to work with
 * @param[in,out] referee      : Transport reference - e.g. TCP socket
 * @param[in]     verification : Indicates whether to verify the certificate chain against a root server.
 *
 */
cy_rslt_t cy_tls_generic_start_tls_with_ciphers( cy_tls_context_t* tls_context, void* referee, cy_tls_certificate_verification_t verification );

/** @} */


#ifdef __cplusplus
} /*extern "C" */
#endif

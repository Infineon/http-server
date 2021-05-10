/*
 * Copyright 2021, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
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
 *  cy_tls_port_mbedtls.c
 *  TLS wrapper functions
 *
 */

#include "cy_tls_port.h"
#include "cy_tcpip_port.h"
#include "cy_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nsapi_types.h"

#ifdef ENABLE_HTTP_SERVER_LOGS
#define hs_cy_log_msg cy_log_msg
#else
#define hs_cy_log_msg(a,b,c,...)
#endif

#define MIN(x,y) ((x) < (y) ? (x) : (y))

#define TIMEOUT                  1000
#define DFL_READ_TIMEOUT         0
#define DFL_MIN_VERSION         -1
#define DFL_MAX_VERSION         -1
#define DFL_AUTH_MODE           -1
#define DFL_MFL_CODE            MBEDTLS_SSL_MAX_FRAG_LEN_NONE
#define DFL_TRANSPORT           MBEDTLS_SSL_TRANSPORT_STREAM
#define DFL_HS_TO_MIN            0
#define DFL_HS_TO_MAX            0

static struct options
{
    const char *server_name;    /* hostname of the server (client only)     */
    const char *server_addr;    /* address of the server (client only)      */
    const char *server_port;    /* port on which the ssl service runs       */
    int debug_level;            /* level of debugging                       */
    int nbio;                   /* should I/O be blocking?                  */
    int event;                  /* loop or event-driven IO? level or edge triggered? */
    uint32_t read_timeout;      /* timeout on mbedtls_ssl_read() in milliseconds     */
    int max_resend;             /* DTLS times to resend on read timeout     */
    const char *request_page;   /* page on server to request                */
    int request_size;           /* pad request with header to requested size */
    const char *ca_file;        /* the file with the CA certificate(s)      */
    const char *ca_path;        /* the path with the CA certificate(s) reside */
    const char *crt_file;       /* the file with the client certificate     */
    const char *key_file;       /* the file with the client key             */
    const char *psk;            /* the pre-shared key                       */
    const char *psk_identity;   /* the pre-shared key identity              */
    const char *ecjpake_pw;     /* the EC J-PAKE password                   */
    int ec_max_ops;             /* EC consecutive operations limit          */
    int force_ciphersuite[2];   /* protocol/ciphersuite to use, or all      */
    int renegotiation;          /* enable / disable renegotiation           */
    int allow_legacy;           /* allow legacy renegotiation               */
    int renegotiate;            /* attempt renegotiation?                   */
    int renego_delay;           /* delay before enforcing renegotiation     */
    int exchanges;              /* number of data exchanges                 */
    int min_version;            /* minimum protocol version accepted        */
    int max_version;            /* maximum protocol version accepted        */
    int arc4;                   /* flag for arc4 suites support             */
    int allow_sha1;             /* flag for SHA-1 support                   */
    int auth_mode;              /* verify mode for connection               */
    unsigned char mfl_code;     /* code for maximum fragment length         */
    int trunc_hmac;             /* negotiate truncated hmac or not          */
    int recsplit;               /* enable record splitting?                 */
    int dhmlen;                 /* minimum DHM params len in bits           */
    int reconnect;              /* attempt to resume session                */
    int reco_delay;             /* delay in seconds before resuming session */
    int reconnect_hard;         /* unexpectedly reconnect from the same port */
    int tickets;                /* enable / disable session tickets         */
    const char *curves;         /* list of supported elliptic curves        */
    const char *alpn_string;    /* ALPN supported protocols                 */
    int transport;              /* TLS or DTLS?                             */
    uint32_t hs_to_min;         /* Initial value of DTLS handshake timer    */
    uint32_t hs_to_max;         /* Max value of DTLS handshake timer        */
    int dtls_mtu;               /* UDP Maximum tranport unit for DTLS       */
    int fallback;               /* is this a fallback connection?           */
    int dgram_packing;          /* allow/forbid datagram packing            */
    int extended_ms;            /* negotiate extended master secret?        */
    int etm;                    /* negotiate encrypt then mac?              */
} opt;


static uint8_t buffered_data[1600];

uint8_t *buffer_to_use = buffered_data;
int total_bytes = 0;

mbedtls_x509_crt* root_ca_certificates = NULL;

#define MBEDTLS_DEBUG_C
#if defined(MBEDTLS_DEBUG_C)
static void mbedtls_debug( void *ctx, int level, const char *file, int line, const char *str )
{
    ( (void) level );
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG,"%s:%04d: %s", file, line, str );
}
#endif /* MBEDTLS_DEBUG_C */

static void tls_deinit_root_ca_certificates(mbedtls_x509_crt* root_ca_certs)
{
    if (root_ca_certs != NULL)
    {
        mbedtls_x509_crt_free(root_ca_certs);
        free(root_ca_certs);
    }
}

static cy_rslt_t tls_init_root_ca_certificates(mbedtls_x509_crt** root_ca_certs, const char* trusted_ca_certificates, const uint32_t cert_length)
{
    int result;

    if (root_ca_certs == NULL)
    {
        return CY_RSLT_MODULE_TLS_BAD_INPUT_DATA;
    }

    if (trusted_ca_certificates == NULL || cert_length == 0)
    {
        return CY_RSLT_MODULE_TLS_BAD_INPUT_DATA;
    }

    tls_deinit_root_ca_certificates(*root_ca_certs);

    *root_ca_certs = malloc(sizeof(mbedtls_x509_crt));
    if (*root_ca_certs == NULL)
    {
        return CY_RSLT_MODULE_TLS_OUT_OF_HEAP_SPACE;
    }

    mbedtls_x509_crt_init(*root_ca_certs);

    /* Parse RootCA Certificate */
    result = mbedtls_x509_crt_parse(*root_ca_certs, (const unsigned char *)trusted_ca_certificates, cert_length + 1);
    if (result != 0)
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "Result from mbedtls_x509_crt_parse is %lx (%ld) (~%lx)\n", (uint32_t)result, (uint32_t)result, ~((uint32_t)result));
        mbedtls_x509_crt_free(*root_ca_certs);
        free(*root_ca_certs);
        *root_ca_certs = NULL;
        return CY_RSLT_MODULE_TLS_PARSE_CERTIFICATE;
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tls_init_root_ca_certificates( const char* trusted_ca_certificates, const uint32_t length )
{
    return tls_init_root_ca_certificates(&root_ca_certificates, trusted_ca_certificates, length);
}

int ssl_receive_packet( void *ctx, unsigned char *buf, size_t len )
{
    int recv= 0;
    uint32_t elapsed_time, current_time, diff;

    cy_tcp_socket_t*  socket = (cy_tcp_socket_t*) ctx;
    cy_tls_context_t* tls_context = socket->context;

    cy_rtos_get_time(&elapsed_time);

    /* This is required for chrome & IE browser. As those browser forces strict timing on handshake. Here we make sure
     * that all the data is sent and then only return. And after handshake, when data transfer starts we dont need to
     * do continuous read.
     */

    do
    {
        recv = cy_tcp_recv( socket, (char*) buf, len );

        cy_rtos_get_time(&current_time);

        diff = current_time - elapsed_time;

    } while( recv < 0 && diff <= TIMEOUT && tls_context->context.state != MBEDTLS_SSL_HANDSHAKE_OVER );

    return recv;
}

int ssl_flush_output( void* context, const uint8_t* buffer, size_t length )
{
    int ret = 0;
    uint32_t elapsed_time, current_time, diff;

    cy_tcp_socket_t*  socket = (cy_tcp_socket_t*) context;
    cy_tls_context_t* tls_context = socket->context;

    cy_rtos_get_time(&elapsed_time);

    /* This is required for chrome & IE browser. As those browser forces strict timing on handshake. Here we make sure
     * that all the data is sent and then only return. And after handshake, when data transfer starts we dont need to
     * do continuous write as that is handled by application
     */

    do
    {
        ret = cy_tcp_send(socket, (char*) buffer, length);

        cy_rtos_get_time(&current_time);

        diff = current_time - elapsed_time;

    } while(ret < 0 && diff <= TIMEOUT && tls_context->context.state != MBEDTLS_SSL_HANDSHAKE_OVER);

    return ret;
}


cy_rslt_t cy_tls_generic_start_tls_with_ciphers( cy_tls_context_t* tls_context, void* referee, cy_tls_certificate_verification_t verification )
{

    mbedtls_ssl_context *ssl = &tls_context->context;
    mbedtls_ssl_config  *conf = (mbedtls_ssl_config*) ssl->conf;
    int ret = 0;
    cy_rslt_t result = CY_RSLT_SUCCESS;


    const char *pers = "supplicant_client";

    /*
     * Make sure memory references are valid.
     */
    mbedtls_ssl_init( ssl );

    mbedtls_ssl_config_init( conf );

    mbedtls_ctr_drbg_init( &tls_context->ctr_drbg );

    opt.read_timeout        = DFL_READ_TIMEOUT;
    opt.min_version         = DFL_MIN_VERSION;
    opt.max_version         = DFL_MAX_VERSION;

    /*
     * 0. Initialize the RNG and the session data
     */
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\n  . Seeding the random number generator..." );

    mbedtls_entropy_init( &tls_context->entropy );
    if( ( ret = mbedtls_ctr_drbg_seed( &tls_context->ctr_drbg, mbedtls_entropy_func,
                                       &tls_context->entropy, (const unsigned char *) pers,
                                       strlen( pers ) ) ) != 0 )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " failed\n  ! mbedtls_ctr_drbg_seed returned -0x%x\n",
                        -ret );
        return CY_RSLT_MODULE_TLS_ERROR;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " ok\n" );


    /*
     * 3. Setup stuff
     */
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "  . Setting up the SSL/TLS structure..." );

    if( ( ret = mbedtls_ssl_config_defaults( conf,
                    MBEDTLS_SSL_IS_SERVER,
                    opt.transport,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " failed\n  ! mbedtls_ssl_config_defaults returned -0x%x\n\n",
                        -ret );
        result = CY_RSLT_MODULE_TLS_ERROR;
        goto exit_with_inited_context;
    }

    mbedtls_ssl_conf_rng( conf, mbedtls_ctr_drbg_random, &tls_context->ctr_drbg );

    mbedtls_ssl_conf_dbg( conf, mbedtls_debug, stdout );

    mbedtls_ssl_conf_read_timeout( conf, opt.read_timeout );

    if (tls_context->root_ca_certificates != NULL)
    {
        mbedtls_ssl_conf_ca_chain( conf, tls_context->root_ca_certificates, NULL );
        mbedtls_ssl_conf_authmode( conf, MBEDTLS_SSL_VERIFY_REQUIRED );
    }
    else if ( root_ca_certificates != NULL )
    {
        mbedtls_ssl_conf_ca_chain( conf, root_ca_certificates, NULL );
        mbedtls_ssl_conf_authmode( conf, MBEDTLS_SSL_VERIFY_REQUIRED );
    }
    else
    {
        mbedtls_ssl_conf_authmode( conf, MBEDTLS_SSL_VERIFY_NONE );
    }


    if (tls_context->identity != NULL) {
        if ((ret = mbedtls_ssl_conf_own_cert(conf,&tls_context->identity->certificate,
                &tls_context->identity->private_key)) != 0) {
            result = CY_RSLT_MODULE_TLS_ERROR;
            goto exit_with_inited_context;
        }

    } else {
        conf->key_cert = NULL;
    }


    if( opt.min_version != DFL_MIN_VERSION )
        mbedtls_ssl_conf_min_version( conf, MBEDTLS_SSL_MAJOR_VERSION_3,
                                      opt.min_version );

    if( opt.max_version != DFL_MAX_VERSION )
        mbedtls_ssl_conf_max_version( conf, MBEDTLS_SSL_MAJOR_VERSION_3,
                                      opt.max_version );

    if( ( ret = mbedtls_ssl_setup( ssl, conf ) ) != 0 )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " failed\n  ! mbedtls_ssl_setup returned -0x%x\n\n",
                        -ret );
        result = CY_RSLT_MODULE_TLS_ERROR;
        goto exit_with_inited_context;
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    if( ( ret = mbedtls_ssl_set_hostname( ssl, NULL) ) != 0 )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n",
                        ret );
        result = CY_RSLT_MODULE_TLS_ERROR;
        goto exit_with_inited_context;
    }
#endif

    mbedtls_ssl_set_bio( ssl, referee, ssl_flush_output, ssl_receive_packet, NULL );

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " ok\n" );

    /*
     * 4. Handshake
     */
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "  . Performing the SSL/TLS handshake..." );

    while( ( ret = mbedtls_ssl_handshake( ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE &&
            ret != MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS )
        {
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n", -ret );
            
            result = CY_RSLT_MODULE_TLS_HANDSHAKE_FAILURE;
            goto exit_with_inited_context;

        }
    }

    return( CY_RSLT_SUCCESS );

exit_with_inited_context:
    mbedtls_ssl_close_notify( &tls_context->context );
    mbedtls_entropy_free(&tls_context->entropy );
    mbedtls_ctr_drbg_free( &tls_context->ctr_drbg );
    if ( tls_context->context.conf != NULL )
    {
        mbedtls_ssl_config_free( (mbedtls_ssl_config*) tls_context->context.conf  );
    }

    mbedtls_ssl_free(&tls_context->context);

    /* FIXME : For now, conf is malloced in tls_init_context, and mbedtls_ssl_free does memset of context so assigning conf pointer back so it can be resused. Need to fix it properly */
    tls_context->context.conf = conf;

    return result;
}


static cy_rslt_t tls_load_certificate_key ( cy_tls_identity_t* identity,  const uint8_t* certificate_data, uint32_t certificate_length, const char* private_key, const uint32_t key_length )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int ret = 0;

    if(identity == NULL)
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " [%s][%s][%d] Invalid TLS identity\r\n",__FILE__,__func__,__LINE__ );
        return CY_RSLT_MODULE_TLS_BAD_INPUT_DATA;
    }

    if ( ( certificate_data != NULL ) && ( certificate_length != 0 ) )
    {
        /* load x509 certificate */
        mbedtls_x509_crt_init( &identity->certificate );

        ret = mbedtls_x509_crt_parse( &identity->certificate, (const unsigned char *) certificate_data, certificate_length + 1 );

        if ( ret != 0 )
        {
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " [%s][%s][%d] Unable to parse TLS certificate ret %d\r\n",__FILE__,__func__,__LINE__,ret);
            result = CY_RSLT_MODULE_TLS_PARSE_CERTIFICATE;
            goto ERROR_CERTIFICATE_INIT;
        }

    }

    if ( ( private_key != NULL ) && ( key_length != 0 ) )
    {
        /* load key */
        mbedtls_pk_init( &identity->private_key );

        ret = mbedtls_pk_parse_key( &identity->private_key, (const unsigned char *) private_key, key_length+1, NULL, 0 );
        if ( ret != 0 )
        {
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " [%s][%s][%d] Unable to parse TLS private key ret %d\r\n",__FILE__,__func__,__LINE__,ret);
            result = CY_RSLT_MODULE_TLS_PARSE_KEY;
            goto ERROR_KEY_INIT;
        }

    }

    return CY_RSLT_SUCCESS;

ERROR_KEY_INIT:
    mbedtls_pk_free( &identity->private_key );

ERROR_CERTIFICATE_INIT:
    mbedtls_x509_crt_free( &identity->certificate );

    return result;
}

cy_rslt_t cy_tls_init_context(cy_tls_context_t* tls_context, cy_tls_identity_t* identity, char* peer_cn)
{
    if (tls_context == NULL) {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " [%s][%s][%d] Invalid TLS context\r\n",__FILE__,__func__,__LINE__ );
        return CY_RSLT_MODULE_TLS_BAD_INPUT_DATA;
    }
    memset(tls_context, 0, sizeof(cy_tls_context_t));

    tls_context->context.conf = malloc(sizeof(mbedtls_ssl_config));
    if (tls_context->context.conf == NULL) {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " [%s][%s][%d] Unable to allocate memory for TLS configuration\r\n",__FILE__,__func__,__LINE__ );
        return CY_RSLT_MODULE_TLS_OUT_OF_HEAP_SPACE;
    }

    memset((mbedtls_ssl_config*) tls_context->context.conf, 0,
            sizeof(mbedtls_ssl_config));

    tls_context->identity = identity;
    tls_context->peer_cn = peer_cn;
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tls_deinit_context( cy_tls_context_t* tls_context )
{
    if ( tls_context == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " [%s][%s][%d] Invalid TLS context\r\n",__FILE__,__func__,__LINE__ );
        return CY_RSLT_MODULE_TLS_BAD_INPUT_DATA;
    }

    mbedtls_entropy_free( &tls_context->entropy );
    mbedtls_ctr_drbg_free( &tls_context->ctr_drbg );

    mbedtls_ssl_config_free( (mbedtls_ssl_config*) tls_context->context.conf );
    if ( tls_context->context.conf != NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " [%s][%s][%d] Freeing TLS configuration\r\n",__FILE__,__func__,__LINE__ );
        free((mbedtls_ssl_config*) tls_context->context.conf );
        tls_context->context.conf = NULL;
    }
    mbedtls_ssl_free( &tls_context->context );

    if (tls_context->root_ca_certificates != NULL)
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " [%s][%s][%d] Freeing Root ca certificate \r\n",__FILE__,__func__,__LINE__ );
        mbedtls_x509_crt_free( tls_context->root_ca_certificates );
        free( tls_context->root_ca_certificates );
        tls_context->root_ca_certificates = NULL;
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tls_set_context_root_ca_certificates( cy_tls_context_t* context, const char* trusted_ca_certificates, const uint32_t cert_length )
{
    return tls_init_root_ca_certificates(&context->root_ca_certificates, trusted_ca_certificates, cert_length);
}

cy_rslt_t cy_tls_deinit_root_ca_certificates( void )
 {
    tls_deinit_root_ca_certificates(root_ca_certificates);
    root_ca_certificates = NULL;

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tls_init_identity( cy_tls_identity_t* identity, const char* private_key, const uint32_t key_length, const uint8_t* certificate_data, uint32_t certificate_length )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    if ( identity == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " [%s][%s][%d] Invalid identity \n",__FILE__,__func__,__LINE__ );
        return CY_RSLT_MODULE_TLS_BAD_INPUT_DATA;
    }

    memset( identity, 0, sizeof( *identity ) );

    result = tls_load_certificate_key( identity, certificate_data, certificate_length, private_key, key_length );
    if ( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " [%s][%s][%d]  Failed to load certificate & private key \n",__FILE__,__func__,__LINE__ );
    }

    return result;
}

cy_rslt_t cy_tls_deinit_identity(cy_tls_identity_t* identity)
{
    if (identity == NULL) {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " [%s][%s][%d] Invalid identity \n",__FILE__,__func__,__LINE__ );
        return CY_RSLT_MODULE_TLS_BAD_INPUT_DATA;
    }

    mbedtls_x509_crt_free(&identity->certificate);
    mbedtls_pk_free(&identity->private_key);

    return CY_RSLT_SUCCESS;
}

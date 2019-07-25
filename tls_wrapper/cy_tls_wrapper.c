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
 *  cy_tls_wrapper.c
 *  TLS wrapper functions
 *
 */

#include "cy_tls_wrapper.h"
#include "cy_tcpip.h"
#include <stdlib.h>
#include <string.h>
#include "nsapi_types.h"

#define MIN(x,y) ((x) < (y) ? (x) : (y))

#define TIMEOUT                  1000
#define MAX_REQUEST_SIZE         20000
#define MAX_REQUEST_SIZE_STR    "20000"
#define ALPN_LIST_SIZE  10
#define CURVE_LIST_SIZE 20
#define DFL_SERVER_NAME         "localhost"
#define DFL_SERVER_ADDR         NULL
#define DFL_SERVER_PORT         "4433"
#define DFL_REQUEST_PAGE        "/"
#define DFL_REQUEST_SIZE        -1
#define DFL_DEBUG_LEVEL          0
#define DFL_NBIO                 0
#define DFL_EVENT                0
#define DFL_READ_TIMEOUT         0
#define DFL_MAX_RESEND           0
#define DFL_CA_FILE              ""
#define DFL_CA_PATH              ""
#define DFL_CRT_FILE             ""
#define DFL_KEY_FILE             ""
#define DFL_PSK                  ""
#define DFL_PSK_IDENTITY         "Client_identity"
#define DFL_ECJPAKE_PW           NULL
#define DFL_EC_MAX_OPS          -1
#define DFL_FORCE_CIPHER         0
#define DFL_RENEGOTIATION        MBEDTLS_SSL_RENEGOTIATION_DISABLED
#define DFL_ALLOW_LEGACY        -2
#define DFL_RENEGOTIATE          0
#define DFL_EXCHANGES            1
#define DFL_MIN_VERSION         -1
#define DFL_MAX_VERSION         -1
#define DFL_ARC4                -1
#define DFL_SHA1                -1
#define DFL_AUTH_MODE           -1
#define DFL_MFL_CODE            MBEDTLS_SSL_MAX_FRAG_LEN_NONE
#define DFL_TRUNC_HMAC          -1
#define DFL_RECSPLIT            -1
#define DFL_DHMLEN              -1
#define DFL_RECONNECT            0
#define DFL_RECO_DELAY           0
#define DFL_RECONNECT_HARD       0
#define DFL_TICKETS              MBEDTLS_SSL_SESSION_TICKETS_ENABLED
#define DFL_ALPN_STRING          NULL
#define DFL_CURVES               NULL
#define DFL_TRANSPORT            MBEDTLS_SSL_TRANSPORT_STREAM
#define DFL_HS_TO_MIN            0
#define DFL_HS_TO_MAX            0
#define DFL_DTLS_MTU            -1
#define DFL_DGRAM_PACKING        1
#define DFL_FALLBACK            -1
#define DFL_EXTENDED_MS         -1
#define DFL_ETM                 -1

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
    TLS_WRAPPER_DEBUG(("%s:%04d: %s", file, line, str ));
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
        TLS_WRAPPER_DEBUG(("Result from mbedtls_x509_crt_parse is %lx (%ld) (~%lx)\n", (uint32_t)result, (uint32_t)result, ~((uint32_t)result)));
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

#if defined(MBEDTLS_KEY_EXCHANGE__SOME__PSK_ENABLED)
    unsigned char psk[MBEDTLS_PSK_MAX_LEN];
    size_t psk_len = 0;
#endif
#if defined(MBEDTLS_SSL_ALPN)
    const char *alpn_list[ALPN_LIST_SIZE];
#endif
#if defined(MBEDTLS_ECP_C)
    mbedtls_ecp_group_id curve_list[CURVE_LIST_SIZE];
#endif

    const char *pers = "supplicant_client";

    /*
     * Make sure memory references are valid.
     */
    mbedtls_ssl_init( ssl );

    mbedtls_ssl_config_init( conf );

    mbedtls_ctr_drbg_init( &tls_context->ctr_drbg );

#if defined(MBEDTLS_SSL_ALPN)
    memset( (void * ) alpn_list, 0, sizeof( alpn_list ) );
#endif

    opt.server_name         = DFL_SERVER_NAME;
    opt.server_addr         = DFL_SERVER_ADDR;
    opt.server_port         = DFL_SERVER_PORT;
    opt.debug_level         = DFL_DEBUG_LEVEL;
    opt.nbio                = DFL_NBIO;
    opt.event               = DFL_EVENT;
    opt.read_timeout        = DFL_READ_TIMEOUT;
    opt.max_resend          = DFL_MAX_RESEND;
    opt.request_page        = DFL_REQUEST_PAGE;
    opt.request_size        = DFL_REQUEST_SIZE;
    opt.ca_file             = DFL_CA_FILE;
    opt.ca_path             = DFL_CA_PATH;
    opt.crt_file            = DFL_CRT_FILE;
    opt.key_file            = DFL_KEY_FILE;
    opt.psk                 = DFL_PSK;
    opt.psk_identity        = DFL_PSK_IDENTITY;
    opt.ecjpake_pw          = DFL_ECJPAKE_PW;
    opt.ec_max_ops          = DFL_EC_MAX_OPS;
    opt.force_ciphersuite[0]= DFL_FORCE_CIPHER;
    opt.renegotiation       = DFL_RENEGOTIATION;
    opt.allow_legacy        = DFL_ALLOW_LEGACY;
    opt.renegotiate         = DFL_RENEGOTIATE;
    opt.exchanges           = DFL_EXCHANGES;
    opt.min_version         = DFL_MIN_VERSION;
    opt.max_version         = DFL_MAX_VERSION;
    opt.arc4                = DFL_ARC4;
    opt.allow_sha1          = DFL_SHA1;
    opt.auth_mode           = DFL_AUTH_MODE;
    opt.mfl_code            = DFL_MFL_CODE;
    opt.trunc_hmac          = DFL_TRUNC_HMAC;
    opt.recsplit            = DFL_RECSPLIT;
    opt.dhmlen              = DFL_DHMLEN;
    opt.reconnect           = DFL_RECONNECT;
    opt.reco_delay          = DFL_RECO_DELAY;
    opt.reconnect_hard      = DFL_RECONNECT_HARD;
    opt.tickets             = DFL_TICKETS;
    opt.alpn_string         = DFL_ALPN_STRING;
    opt.curves              = DFL_CURVES;
    opt.transport           = DFL_TRANSPORT;
    opt.hs_to_min           = DFL_HS_TO_MIN;
    opt.hs_to_max           = DFL_HS_TO_MAX;
    opt.dtls_mtu            = DFL_DTLS_MTU;
    opt.fallback            = DFL_FALLBACK;
    opt.extended_ms         = DFL_EXTENDED_MS;
    opt.etm                 = DFL_ETM;
    opt.dgram_packing       = DFL_DGRAM_PACKING;

    /*
     * 0. Initialize the RNG and the session data
     */
    TLS_WRAPPER_DEBUG(( "\n  . Seeding the random number generator..." ));

    mbedtls_entropy_init( &tls_context->entropy );
    if( ( ret = mbedtls_ctr_drbg_seed( &tls_context->ctr_drbg, mbedtls_entropy_func,
                                       &tls_context->entropy, (const unsigned char *) pers,
                                       strlen( pers ) ) ) != 0 )
    {
        TLS_WRAPPER_DEBUG(( " failed\n  ! mbedtls_ctr_drbg_seed returned -0x%x\n",
                        -ret ));
        return CY_RSLT_MODULE_TLS_ERROR;
    }

    TLS_WRAPPER_DEBUG(( " ok\n" ));


    /*
     * 3. Setup stuff
     */
    TLS_WRAPPER_DEBUG(( "  . Setting up the SSL/TLS structure..." ));

    if( ( ret = mbedtls_ssl_config_defaults( conf,
                    MBEDTLS_SSL_IS_SERVER,
                    opt.transport,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        TLS_WRAPPER_DEBUG(( " failed\n  ! mbedtls_ssl_config_defaults returned -0x%x\n\n",
                        -ret ));
        return CY_RSLT_MODULE_TLS_ERROR;
    }

    if( opt.debug_level > 0 )
        mbedtls_ssl_conf_verify( conf, NULL, NULL );


#if defined(MBEDTLS_SSL_MAX_FRAGMENT_LENGTH)
    if( ( ret = mbedtls_ssl_conf_max_frag_len( conf, opt.mfl_code ) ) != 0 )
    {
        TLS_WRAPPER_DEBUG(( " failed\n  ! mbedtls_ssl_conf_max_frag_len returned %d\n\n",
                        ret ));
        return CY_RSLT_MODULE_TLS_ERROR;
    }
#endif

#if defined(MBEDTLS_SSL_TRUNCATED_HMAC)
    if( opt.trunc_hmac != DFL_TRUNC_HMAC )
        mbedtls_ssl_conf_truncated_hmac( conf, opt.trunc_hmac );
#endif

#if defined(MBEDTLS_SSL_EXTENDED_MASTER_SECRET)
    if( opt.extended_ms != DFL_EXTENDED_MS )
        mbedtls_ssl_conf_extended_master_secret( conf, opt.extended_ms );
#endif

#if defined(MBEDTLS_SSL_ENCRYPT_THEN_MAC)
    if( opt.etm != DFL_ETM )
        mbedtls_ssl_conf_encrypt_then_mac( conf, opt.etm );
#endif

#if defined(MBEDTLS_SSL_CBC_RECORD_SPLITTING)
    if( opt.recsplit != DFL_RECSPLIT )
        mbedtls_ssl_conf_cbc_record_splitting( conf, opt.recsplit
                                  ? MBEDTLS_SSL_CBC_RECORD_SPLITTING_ENABLED
                                  : MBEDTLS_SSL_CBC_RECORD_SPLITTING_DISABLED );
#endif

#if defined(MBEDTLS_DHM_C)
    if( opt.dhmlen != DFL_DHMLEN )
        mbedtls_ssl_conf_dhm_min_bitlen( conf, opt.dhmlen );
#endif

#if defined(MBEDTLS_SSL_ALPN)
    if( opt.alpn_string != NULL )
        if( ( ret = mbedtls_ssl_conf_alpn_protocols( conf, alpn_list ) ) != 0 )
        {
            TLS_WRAPPER_DEBUG(( " failed\n  ! mbedtls_ssl_conf_alpn_protocols returned %d\n\n",
                            ret ));
            return CY_RSLT_MODULE_TLS_ERROR;
        }
#endif

    mbedtls_ssl_conf_rng( conf, mbedtls_ctr_drbg_random, &tls_context->ctr_drbg );

    mbedtls_ssl_conf_dbg( conf, mbedtls_debug, stdout );

    mbedtls_ssl_conf_read_timeout( conf, opt.read_timeout );

#if defined(MBEDTLS_SSL_SESSION_TICKETS)
    mbedtls_ssl_conf_session_tickets( conf, opt.tickets );
#endif

    if( opt.force_ciphersuite[0] != DFL_FORCE_CIPHER )
        mbedtls_ssl_conf_ciphersuites( conf, opt.force_ciphersuite );

#if defined(MBEDTLS_ARC4_C)
    if( opt.arc4 != DFL_ARC4 )
        mbedtls_ssl_conf_arc4_support( &conf, opt.arc4 );
#endif


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

#if defined(MBEDTLS_ECP_C)
    if( opt.curves != NULL &&
        strcmp( opt.curves, "default" ) != 0 )
    {
        mbedtls_ssl_conf_curves( conf, curve_list );
    }
#endif

#if defined(MBEDTLS_KEY_EXCHANGE__SOME__PSK_ENABLED)
    if( ( ret = mbedtls_ssl_conf_psk( conf, psk, psk_len,
                             (const unsigned char *) opt.psk_identity,
                             strlen( opt.psk_identity ) ) ) != 0 )
    {
        TLS_WRAPPER_DEBUG(( " failed\n  ! mbedtls_ssl_conf_psk returned %d\n\n",
                        ret ));
        return CY_RSLT_MODULE_TLS_ERROR;
    }
#endif

    if (tls_context->identity != NULL) {
        if ((ret = mbedtls_ssl_conf_own_cert(conf,&tls_context->identity->certificate,
                &tls_context->identity->private_key)) != 0) {
            result = CY_RSLT_MODULE_TLS_ERROR;
            return result;
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

#if defined(MBEDTLS_SSL_FALLBACK_SCSV)
    if( opt.fallback != DFL_FALLBACK )
        mbedtls_ssl_conf_fallback( &conf, opt.fallback );
#endif

    if( ( ret = mbedtls_ssl_setup( ssl, conf ) ) != 0 )
    {
        TLS_WRAPPER_DEBUG(( " failed\n  ! mbedtls_ssl_setup returned -0x%x\n\n",
                        -ret ));
        return CY_RSLT_MODULE_TLS_ERROR;
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    if( ( ret = mbedtls_ssl_set_hostname( ssl, NULL) ) != 0 )
    {
        TLS_WRAPPER_DEBUG(( " failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n",
                        ret ));
        return CY_RSLT_MODULE_TLS_ERROR;
    }
#endif

#if defined(MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED)
    if( opt.ecjpake_pw != DFL_ECJPAKE_PW )
    {
        if( ( ret = mbedtls_ssl_set_hs_ecjpake_password( ssl,
                        (const unsigned char *) opt.ecjpake_pw,
                                        strlen( opt.ecjpake_pw ) ) ) != 0 )
        {
            TLS_WRAPPER_DEBUG(( " failed\n  ! mbedtls_ssl_set_hs_ecjpake_password returned %d\n\n",
                            ret ));
            return CY_RSLT_MODULE_TLS_ERROR;
        }
    }
#endif

         mbedtls_ssl_set_bio( ssl, referee, ssl_flush_output, ssl_receive_packet, NULL );

#if defined(MBEDTLS_ECP_RESTARTABLE)
    if( opt.ec_max_ops != DFL_EC_MAX_OPS )
        mbedtls_ecp_set_max_ops( opt.ec_max_ops );
#endif

    TLS_WRAPPER_DEBUG(( " ok\n" ));

    /*
     * 4. Handshake
     */
    TLS_WRAPPER_DEBUG(( "  . Performing the SSL/TLS handshake..." ));

    while( ( ret = mbedtls_ssl_handshake( ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE &&
            ret != MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS )
        {
            TLS_WRAPPER_DEBUG(( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n",
                            -ret ));
            if( ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED ){
                TLS_WRAPPER_DEBUG((
                    "    Unable to verify the server's certificate. "
                        "Either it is invalid,\n"
                    "    or you didn't set ca_file or ca_path "
                        "to an appropriate value.\n"
                    "    Alternatively, you may want to use "
                        "auth_mode=optional for testing purposes.\n" ));
            TLS_WRAPPER_DEBUG(( "\n" ));
            }

            return ret;

        }
    }

#if defined(MBEDTLS_ECP_RESTARTABLE)
        if( ret == MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS )
            continue;
#endif

    return( CY_RSLT_SUCCESS );

}


static cy_rslt_t tls_load_certificate_key ( cy_tls_identity_t* identity,  const uint8_t* certificate_data, uint32_t certificate_length, const char* private_key, const uint32_t key_length )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int ret = 0;

    if(identity == NULL)
    {
        TLS_WRAPPER_DEBUG( (" [%s][%s][%d] Invalid TLS identity\r\n",__FILE__,__func__,__LINE__));
        return CY_RSLT_MODULE_TLS_BAD_INPUT_DATA;
    }

    if ( ( certificate_data != NULL ) && ( certificate_length != 0 ) )
    {
        /* load x509 certificate */
        mbedtls_x509_crt_init( &identity->certificate );

        ret = mbedtls_x509_crt_parse( &identity->certificate, (const unsigned char *) certificate_data, certificate_length + 1 );

        if ( ret != 0 )
        {
            TLS_WRAPPER_DEBUG( (" [%s][%s][%d] Unable to parse TLS certificate ret %d\r\n",__FILE__,__func__,__LINE__,ret));
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
            TLS_WRAPPER_DEBUG( (" [%s][%s][%d] Unable to parse TLS private key ret %d\r\n",__FILE__,__func__,__LINE__,ret));
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
        TLS_WRAPPER_DEBUG( (" [%s][%s][%d] Invalid TLS context\r\n",__FILE__,__func__,__LINE__));
        return CY_RSLT_MODULE_TLS_BAD_INPUT_DATA;
    }
    memset(tls_context, 0, sizeof(cy_tls_context_t));

    tls_context->context.conf = malloc(sizeof(mbedtls_ssl_config));
    if (tls_context->context.conf == NULL) {
        TLS_WRAPPER_DEBUG( (" [%s][%s][%d] Unable to allocate memory for TLS configuration\r\n",__FILE__,__func__,__LINE__));
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
        TLS_WRAPPER_DEBUG( (" [%s][%s][%d] Invalid TLS context\r\n",__FILE__,__func__,__LINE__));
        return CY_RSLT_MODULE_TLS_BAD_INPUT_DATA;
    }
    mbedtls_ssl_config_free( (mbedtls_ssl_config*) tls_context->context.conf );
    if ( tls_context->context.conf != NULL )
    {
        TLS_WRAPPER_DEBUG( (" [%s][%s][%d] Freeing TLS configuration\r\n",__FILE__,__func__,__LINE__));
        free((mbedtls_ssl_config*) tls_context->context.conf );
        tls_context->context.conf = NULL;
    }
    mbedtls_ssl_free( &tls_context->context );
    mbedtls_ctr_drbg_free( &tls_context->ctr_drbg );
    mbedtls_entropy_free( &tls_context->entropy );

    if (tls_context->root_ca_certificates != NULL)
    {
        TLS_WRAPPER_DEBUG( (" [%s][%s][%d] Freeing Root ca certificate \r\n",__FILE__,__func__,__LINE__));
        mbedtls_x509_crt_free( tls_context->root_ca_certificates );
        free( tls_context->root_ca_certificates );
        tls_context->root_ca_certificates = NULL;
    }

    memset( tls_context, 0, sizeof( *tls_context ) );

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
        TLS_WRAPPER_DEBUG( (" [%s][%s][%d] Invalid identity \n",__FILE__,__func__,__LINE__));
        return CY_RSLT_MODULE_TLS_BAD_INPUT_DATA;
    }

    memset( identity, 0, sizeof( *identity ) );

    result = tls_load_certificate_key( identity, certificate_data, certificate_length, private_key, key_length );
    if ( result != CY_RSLT_SUCCESS )
    {
        TLS_WRAPPER_DEBUG((" [%s][%s][%d]  Failed to load certificate & private key \n",__FILE__,__func__,__LINE__));
        return result;
    }

    return result;
}

cy_rslt_t cy_tls_deinit_identity(cy_tls_identity_t* identity)
{
    if (identity == NULL) {
        TLS_WRAPPER_DEBUG( (" [%s][%s][%d] Invalid identity \n",__FILE__,__func__,__LINE__));
        return CY_RSLT_MODULE_TLS_BAD_INPUT_DATA;
    }
    if (identity != NULL) {

        mbedtls_x509_crt_free(&identity->certificate);
        mbedtls_pk_free(&identity->private_key);

    }
    return CY_RSLT_SUCCESS;

}

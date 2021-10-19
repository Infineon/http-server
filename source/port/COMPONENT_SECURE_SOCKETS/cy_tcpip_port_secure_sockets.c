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
 *  Implements TCP server APIs using secure socket library.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cy_secure_sockets.h"
#include "cy_tcpip_port.h"
#include "cy_tls_port.h"
#include "cy_result_mw.h"
#include "cy_log.h"

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef ENABLE_HTTP_SERVER_LOGS
#define hs_cy_log_msg cy_log_msg
#else
#define hs_cy_log_msg(a,b,c,...)
#endif
/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *                 Static Variables
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
cy_rslt_t cy_tcp_server_network_init( void )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Initialize secure socket library. */
    result = cy_socket_init();
    if ( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR,  "Secure Socket initialization failed!\n" );
        return result;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "Secure Socket initialization completed\n" );
    return result;
}

cy_rslt_t cy_tcp_server_network_deinit( void )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* DeInitialize secure socket library. */
    result = cy_socket_deinit();
    if ( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "Secure Socket deinitialization failed!\n" );
        return result;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "Secure Socket deinitialization completed!\n" );
    return result;
}

cy_rslt_t cy_tls_init_root_ca_certificates( const char* trusted_ca_certificates,
                                            const uint32_t length )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = cy_tls_load_global_root_ca_certificates( trusted_ca_certificates, length );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\ncy_tls_load_global_root_ca_certificates failed with error : %ld\n", result );
        return result;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_tls_load_global_root_ca_certificates completed successfully" );
    return result;
}

cy_rslt_t cy_tls_deinit_root_ca_certificates( void )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = cy_tls_release_global_root_ca_certificates();
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\ncy_tls_release_global_root_ca_certificates failed with error : %ld\n", result );
        return result;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_tls_release_global_root_ca_certificates completed successfully" );
    return result;
}

cy_rslt_t cy_tls_init_identity( cy_tls_identity_t* identity,
                                const char* private_key,
                                const uint32_t key_length,
                                const uint8_t* certificate_data,
                                uint32_t certificate_length )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = cy_tls_create_identity( (const char *)certificate_data,
                                     certificate_length,
                                     private_key,
                                     key_length,
                                     ( identity ) );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\ncy_tls_create_identity failed with error : %ld\n", result );
        return result;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_tls_create_identity completed successfully" );
    return result;
}

cy_rslt_t cy_tls_deinit_identity( cy_tls_identity_t* identity )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = cy_tls_delete_identity( *identity );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\ncy_tls_delete_identity failed with error : %ld\n", result );
        return result;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_tls_delete_identity completed successfully" );
    return result;
}

cy_rslt_t cy_tcp_server_start( cy_tcp_server_t* server,
                               cy_network_interface_t* network_interface,
                               uint16_t port,
                               uint16_t max_sockets,
                               cy_server_type_t type)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_network_interface_t *interface;
    cy_socket_sockaddr_t *tcp_server_addr;
    bool socketcreated = false;
    cy_socket_tls_auth_mode_t mode = CY_SOCKET_TLS_VERIFY_NONE;

    if( server == NULL || network_interface == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_server_start" );
        return CY_RSLT_TCPIP_ERROR;
    }

    interface = ( cy_network_interface_t* )network_interface;
    tcp_server_addr = ( cy_socket_sockaddr_t* )interface->object;

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nStarting TCP server" );
    result = cy_rtos_init_mutex( &server->mutex );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nMutex init failed in cy_tcp_server_start" );
        return result;
    }

    /* Intialize tcp server socket list */
    cy_linked_list_init( &server->socket_list );

    if( type == CY_HTTP_SERVER_TYPE_SECURE )
    {
        result = cy_socket_create( CY_SOCKET_DOMAIN_AF_INET, CY_SOCKET_TYPE_STREAM,
                                CY_SOCKET_IPPROTO_TLS, &(server->server_socket.socket) );
        if( result != CY_RSLT_SUCCESS )
        {
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nTLS Socket create failed with error : [0x%X]", ( unsigned int )result );
            goto exit;
        }

        socketcreated = true;
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nTLS Socket created : %p ", server->server_socket.socket );

        /* FALSE-POSITIVE:
         * CID:264305: Sizeof not portable (SIZEOF_MISMATCH) Wrong sizeof argument
         * The last argument is expected to be the size of the pointer itself which is 4 bytes, hence this is a false positive.
         */
        result = cy_socket_setsockopt( server->server_socket.socket, CY_SOCKET_SOL_TLS,
                                    CY_SOCKET_SO_TLS_IDENTITY, *(server->identity),
                                    (uint32_t) sizeof( server->identity ) );
        if( result != CY_RSLT_SUCCESS )
        {
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nSet TLS identity failed with error : [0x%X]", ( unsigned int )result );
            goto exit;
        }

        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nSet TLS identity completed successfully" );

        if( server->root_ca_certificate != NULL )
        {
            mode = CY_SOCKET_TLS_VERIFY_REQUIRED;
            result = cy_socket_setsockopt( server->server_socket.socket, CY_SOCKET_SOL_TLS,
                                        CY_SOCKET_SO_TLS_AUTH_MODE, (const void *) &mode,
                                        (uint32_t) sizeof( mode ) );
            if( result != CY_RSLT_SUCCESS)
            {
                hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nSet TLS AUTH mode failed with error : [0x%X]", ( unsigned int )result );
                goto exit;
            }

            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nTLS AUTH mode set to CY_SOCKET_TLS_VERIFY_REQUIRED successfully" );
        }
        else
        {
            mode = CY_SOCKET_TLS_VERIFY_NONE;
            result = cy_socket_setsockopt( server->server_socket.socket, CY_SOCKET_SOL_TLS,
                                        CY_SOCKET_SO_TLS_AUTH_MODE, (const void *) &mode,
                                        (uint32_t) sizeof( mode ) );
            if( result != CY_RSLT_SUCCESS)
            {
                hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nSet TLS AUTH mode failed with error : [0x%X]", ( unsigned int )result );
                goto exit;
            }
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nTLS AUTH mode set to CY_SOCKET_TLS_VERIFY_NONE successfully" );
        }

    }
    else
    {
        result = cy_socket_create( CY_SOCKET_DOMAIN_AF_INET, CY_SOCKET_TYPE_STREAM,
                                   CY_SOCKET_IPPROTO_TCP, &(server->server_socket.socket) );
        if( result != CY_RSLT_SUCCESS )
        {
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nTCP Socket create failed with error : [0x%X]", ( unsigned int )result );
            goto exit;
        }

        socketcreated = true;
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nTCP Socket created : %p ", server->server_socket.socket );
    }

    tcp_server_addr->port = port;
    result = cy_socket_bind( server->server_socket.socket,
                             tcp_server_addr,
                             sizeof(cy_socket_sockaddr_t) );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nSocket bind failed with error : [0x%X]", ( unsigned int )result );
        result = CY_RSLT_TCPIP_ERROR_SOCKET_BIND;
        goto exit;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nSocket binds with Port : %d successfully", tcp_server_addr->port );

    /* Start listening on the secure TCP socket. */
    result = cy_socket_listen( server->server_socket.socket, server->max_tcp_connections );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nSocket listen failed with error : [0x%X]", ( unsigned int )result );
        result = CY_RSLT_TCPIP_ERROR_SOCKET_LISTEN;
        goto exit;
    }

    server->type = type;
    return result;

exit :
    cy_rtos_deinit_mutex( &server->mutex );
    cy_linked_list_deinit( &server->socket_list );
    if( socketcreated == true )
    {
        cy_socket_delete( server->server_socket.socket );
    }
    return result;
}

cy_rslt_t cy_tcp_server_accept( cy_tcp_server_t* server,
                                cy_tcp_socket_t** accepted_socket )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_tcp_socket_t *client_socket;
    cy_socket_sockaddr_t  peer_addr;
    uint32_t peer_addr_len = 0;

    if( server == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_server_accept" );
        return CY_RSLT_TCPIP_ERROR;
    }

    memset( &peer_addr, 0x00, sizeof( cy_socket_sockaddr_t ) );

    /* Allocate memory to accept client socket */
    *accepted_socket = ( cy_tcp_socket_t* ) malloc( sizeof(cy_tcp_socket_t) );
    if( *accepted_socket == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nFailed to allocate memory for new client in cy_tcp_server_accept" );
        return CY_RSLT_TCPIP_ERROR_NO_MEMORY;
    }

    client_socket = *accepted_socket;

    memset( client_socket, 0x00, sizeof( cy_tcp_socket_t ) );

    result = cy_socket_accept( server->server_socket.socket, &peer_addr,
                               &peer_addr_len, &(client_socket->socket) );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nFailed to accept incoming client connection. Error: [0x%X]\n", ( unsigned int )result );
        free( *accepted_socket );
        *accepted_socket = NULL;
        return result;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_INFO, "\nNew client connection accepted : %p\n", client_socket->socket );
    /* Set the client connection flag as true. */
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\n %d : %s() : --- Lock the mutex\r\n", __LINE__, __FUNCTION__ );
    cy_rtos_get_mutex( &server->mutex, CY_RTOS_NEVER_TIMEOUT );
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\n %d : %s() : --- Locked the mutex\r\n", __LINE__, __FUNCTION__ );
    cy_linked_list_set_node_data( &client_socket->socket_node, client_socket );
    cy_linked_list_insert_node_at_rear( &server->socket_list, &client_socket->socket_node );
    server->active_tcp_connections = server->active_tcp_connections + 1;
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_INFO, "\nNumber of active client connections : %ld", server->active_tcp_connections );
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\n %d : %s() : --- Unlock the mutex\r\n", __LINE__, __FUNCTION__ );
    cy_rtos_set_mutex( &server->mutex );
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\n %d : %s() : --- Unlocked the mutex\r\n", __LINE__, __FUNCTION__ );
    return result;
}

int cy_tcp_server_recv( cy_tcp_socket_t* tcp_socket, char* buffer, int length )
{
    cy_rslt_t result = CY_RSLT_SUCCESS ;
    cy_socket_t handle;
    size_t bytesReceived = 0;
    size_t bytesTobeRead = length;

    if( tcp_socket == NULL || tcp_socket->socket == NULL || buffer == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_server_recv" );
        return CY_HTTP_SERVER_SOCKET_ERROR;
    }

    handle = ( cy_socket_t )tcp_socket->socket;

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_socket_recv requested : %d bytes tcp_socket = %p Socket handle = %p", length, tcp_socket, handle );

    result = cy_socket_recv( handle, buffer, bytesTobeRead, 0, (uint32_t *)&bytesReceived );
    if( result != CY_RSLT_SUCCESS )
    {
        /* No data is available, wait for the next event */
        if( result == CY_RSLT_MODULE_SECURE_SOCKETS_TIMEOUT )
        {
            return CY_HTTP_SERVER_SOCKET_NO_DATA;
        }
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_socket_recv failed with Error = [0x%X]\n", ( unsigned int )result );
        return CY_HTTP_SERVER_SOCKET_ERROR;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_socket_recv received : %d bytes......", bytesReceived );
    return bytesReceived;
}

cy_rslt_t cy_tcp_stream_init( cy_tcp_stream_t* stream, void* socket )
{
    if( stream == NULL || socket == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_stream_init" );
        return CY_RSLT_TCPIP_ERROR;
    }
    stream->socket = ( cy_tcp_socket_t* ) socket;
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_tcp_stream_init - Stream = %p tcp_socket = %p", stream , stream->socket );
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tcp_stream_deinit( cy_tcp_stream_t* stream )
{
    if( stream == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_stream_deinit" );
        return CY_RSLT_TCPIP_ERROR;
    }
    stream->socket = NULL ;
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_tcp_stream_deinit - stream->socket = %p", stream->socket );
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tcp_stream_write( cy_tcp_stream_t* stream, const void* data, uint32_t data_length )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    size_t bytesSent = 0;
    cy_socket_t sockethandle;

    if( (stream == NULL) || (stream->socket == NULL) )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid stream or already stream is closed..!\n" );
        return CY_RSLT_TCPIP_ERROR;
    }

    if( data == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_stream_write" );
        return CY_RSLT_TCPIP_ERROR;
    }

    sockethandle = ( cy_socket_t ) stream->socket->socket;

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_tcp_stream_write - %lu bytes to be sent for stream = %p  tcp_socket = %p Socket Handle  %p", ( unsigned long ) data_length ,stream, stream->socket, sockethandle );
    result = cy_socket_send( sockethandle, data, data_length, 0, (uint32_t *)&bytesSent );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_tcp_stream_write failed with Error : [0x%X]\n", ( unsigned int )result );
        return result;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_tcp_stream_write - %lu bytes sent", ( unsigned long ) bytesSent );
    return result;
}

cy_rslt_t cy_tcp_stream_flush( cy_tcp_stream_t* stream )
{
    /* This function will be needed for packet driven data approach ( which are not used currently ) */
    return CY_RSLT_SUCCESS;
}

static void _receiveCallbackWrapper( cy_socket_t socket_handle, void *pArgument )
{
    cy_tcp_socket_t *tcp_handle = (cy_tcp_socket_t *) pArgument;
    if( tcp_handle != NULL )
    {
        tcp_handle->receive_cb( tcp_handle );
    }
}

cy_rslt_t cy_register_socket_callback( cy_tcp_socket_t* socket, receive_callback rcv_callback )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_socket_opt_callback_t tcp_receive_option;
    cy_socket_t server_socket;

    if( (socket == NULL) || (socket->socket == NULL) )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_register_socket_callback" );
        return CY_RSLT_TCPIP_ERROR;
    }

    socket->receive_cb = rcv_callback;
    server_socket = ( cy_socket_t ) socket->socket;

    memset( &tcp_receive_option, 0x00, sizeof( cy_socket_opt_callback_t ) );

    /* Register the callback function to handle messages received from a TCP client. */
    tcp_receive_option.callback = ( cy_socket_callback_t )_receiveCallbackWrapper;
    tcp_receive_option.arg = socket;

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nSet the receive callback for socket handle = %p ", server_socket );
    result = cy_socket_setsockopt( server_socket, CY_SOCKET_SOL_SOCKET,
                                   CY_SOCKET_SO_RECEIVE_CALLBACK,
                                   &tcp_receive_option, sizeof( cy_socket_opt_callback_t ) );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nRegistering receive callback for socket handle = %p failed with Error : [0x%X]", server_socket, ( unsigned int )result );
        return result;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nSuccessfully registered receive callback for socket handle = %p ", server_socket );
    return result;
}

cy_rslt_t cy_register_connect_callback( cy_tcp_socket_t* socket, connect_callback callback )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_socket_opt_callback_t tcp_connection_option;
    cy_socket_t server_socket;

    if( (socket == NULL) || (socket->socket == NULL) )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_register_connect_callback" );
        return CY_RSLT_TCPIP_ERROR;
    }

    server_socket = ( cy_socket_t ) socket->socket;
    memset( &tcp_connection_option, 0x00, sizeof( cy_socket_opt_callback_t ) );

    /* Register the callback function to handle connection request from a TCP client. */
    tcp_connection_option.callback = ( cy_socket_callback_t )callback;
    tcp_connection_option.arg = NULL;

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nSet the connect callback for socket handle = %p ", server_socket );
    result = cy_socket_setsockopt( server_socket, CY_SOCKET_SOL_SOCKET,
                                  CY_SOCKET_SO_CONNECT_REQUEST_CALLBACK,
                                  &tcp_connection_option, sizeof( cy_socket_opt_callback_t ) );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nRegistering connect callback for socket handle = %p failed with Error : [0x%X]", server_socket, ( unsigned int )result );
        return result;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nSuccessfully registered connect callback for socket handle = %p ", server_socket );
    return result;
}

static void _disconnectCallbackWrapper( cy_socket_t socket_handle, void *pArgument )
{
    cy_tcp_socket_t *tcp_handle = (cy_tcp_socket_t *) pArgument;
    if( tcp_handle != NULL )
    {
        tcp_handle->disconnect_cb( tcp_handle );
    }
}

cy_rslt_t cy_register_disconnect_callback( cy_tcp_socket_t* socket, disconnect_callback callback )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_socket_opt_callback_t tcp_disconnection_option;
    cy_socket_t server_socket;

    if( (socket == NULL) || (socket->socket == NULL) )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_register_disconnect_callback" );
        return CY_RSLT_TCPIP_ERROR;
    }

    socket->disconnect_cb = callback;
    server_socket = ( cy_socket_t ) socket->socket;
    memset( &tcp_disconnection_option, 0x00, sizeof( cy_socket_opt_callback_t ) );

    /* Register the callback function to handle disconnection request from a TCP client. */
    tcp_disconnection_option.callback = ( cy_socket_callback_t )_disconnectCallbackWrapper;
    tcp_disconnection_option.arg = socket;

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nSet the disconnect callback for socket handle = %p ", server_socket );
    result = cy_socket_setsockopt( server_socket, CY_SOCKET_SOL_SOCKET,
                                   CY_SOCKET_SO_DISCONNECT_CALLBACK,
                                   &tcp_disconnection_option, sizeof( cy_socket_opt_callback_t ) );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nRegistering disconnect callback for socket handle = %p failed with Error : [0x%X]", server_socket, ( unsigned int )result );
        return result;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nSuccessfully registered disconnect callback for socket handle = %p ", server_socket );
    return result;
}

cy_rslt_t cy_set_socket_recv_timeout( cy_tcp_socket_t* socket, uint32_t timeout )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_socket_t server_socket;
    uint32_t timeout_milliseconds = timeout;

    if( (socket == NULL) || (socket->socket == NULL) )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_set_socket_recv_timeout" );
        return CY_RSLT_TCPIP_ERROR;
    }

    server_socket = ( cy_socket_t ) socket->socket;

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nSet socket receive timeout for socket handle = %p ", server_socket );
    result = cy_socket_setsockopt( server_socket, CY_SOCKET_SOL_SOCKET,
                                   CY_SOCKET_SO_RCVTIMEO,
                                   &timeout_milliseconds, sizeof( timeout_milliseconds ) );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nRegistering disconnect callback for socket handle = %p failed with Error : [0x%X]", server_socket, ( unsigned int )result );
        return result;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nSuccessfully registered disconnect callback for socket handle = %p ", server_socket );
    return result;
}

cy_rslt_t cy_tcp_server_stop( cy_tcp_server_t* server )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_socket_t server_socket;
    cy_linked_list_node_t* current_client_socket_node = NULL;
    cy_linked_list_node_t* current_client_socket_next_node = NULL;

    if( ( server == NULL ) || ( server->server_socket.socket == NULL ) )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_server_stop" );
        return CY_RSLT_TCPIP_ERROR;
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_tcp_server_stop called" );

    server_socket = ( cy_socket_t ) server->server_socket.socket;

    cy_linked_list_get_front_node( &server->socket_list, ( cy_linked_list_node_t** ) &current_client_socket_node );
    while( current_client_socket_node != NULL )
    {
        cy_tcp_socket_t* current_client_socket = ( cy_tcp_socket_t* ) current_client_socket_node->data;
        current_client_socket_next_node = current_client_socket_node->next;

        cy_tcp_server_disconnect_socket( server, current_client_socket );

        current_client_socket_node = current_client_socket_next_node;
    }

    /* Disconnect the network connection. */
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nDisconnect the network connection for socket handle : %p", server_socket );
    result = cy_socket_disconnect( server_socket, 0 );
    if( ( result !=  CY_RSLT_SUCCESS ) && ( result != CY_RSLT_MODULE_SECURE_SOCKETS_NOT_CONNECTED ) )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_socket_disconnect failed with Error : [0x%X]", ( unsigned int )result );
        /* Fall through: In case of unexpected network disconnection, cy_socket_disconnect API always returns failure. Hence ignore the error and fall through the flow to perform the remaining operations. */
    }

    /* Clean up the context of secure socket connections. */
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nDelete socket handle : %p", server_socket );
    result = cy_socket_delete( server_socket );
    if( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "cy_socket_delete failed with Error : [0x%X]", ( unsigned int )result );
        /* Fall through: cy_socket_delete returns error only in case of invalid socket handle. Even though TCP server handle is invalid, resource allocated along with server handle needs to be freed as part of stop TCP server. */
    }

    server->server_socket.socket = NULL;
    cy_linked_list_deinit( &server->socket_list );
    cy_rtos_deinit_mutex( &server->mutex );

    return result;
}

cy_rslt_t cy_tcp_server_disconnect_socket( cy_tcp_server_t* server, cy_tcp_socket_t* client_socket )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_socket_t tcp_socket;
    cy_socket_t server_socket;
    cy_tcp_socket_t* current_client_node = NULL;
    cy_socket_t client_handle = NULL;
    cy_socket_sockaddr_t  peer_addr;
    uint32_t peer_addr_len = 0;

    if( (server == NULL) || (server->server_socket.socket == NULL) ||
        (client_socket == NULL) || (client_socket->socket == NULL) )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_server_disconnect_socket" );
        return CY_RSLT_TCPIP_ERROR;
    }

    tcp_socket = (cy_socket_t) client_socket->socket;
    server_socket = (cy_socket_t) server->server_socket.socket;

    memset( &peer_addr, 0x00, sizeof( cy_socket_sockaddr_t ) );

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " %d : %s() : --- Lock the mutex\r\n", __LINE__, __FUNCTION__ );
    cy_rtos_get_mutex( &server->mutex, CY_RTOS_NEVER_TIMEOUT );
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " %d : %s() : --- Locked the mutex\r\n", __LINE__, __FUNCTION__ );

    cy_linked_list_get_front_node( &server->socket_list, (cy_linked_list_node_t**) &current_client_node );
    while( current_client_node != NULL )
    {
        cy_tcp_socket_t* current_client_socket = (cy_tcp_socket_t*) current_client_node->socket_node.data;
        if( current_client_socket == client_socket )
        {
            cy_linked_list_remove_node( &server->socket_list, &current_client_node->socket_node );

            /* Disconnect the network connection. */
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_INFO, "\nDisconnecting client socket : %p", tcp_socket );
            result = cy_socket_disconnect( tcp_socket, 0 );
            if( ( result !=  CY_RSLT_SUCCESS ) && ( result != CY_RSLT_MODULE_SECURE_SOCKETS_NOT_CONNECTED ) )
            {
                hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_socket_disconnect failed with Error : [0x%X]", ( unsigned int )result );
            }

            /* Delete the client socket handle. */
            result = cy_socket_delete( tcp_socket );
            if( result != CY_RSLT_SUCCESS )
            {
                hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\ncy_socket_delete failed with Error : [0x%X]", ( unsigned int )result );
            }

            /*
             * In case of unexpected network disconnection, cy_socket_disconnect API always returns failure.
             * Return value of cy_socket_disconnect API is not checked here to avoid calling multiple times the API.
             */
            free( client_socket );
            current_client_node = NULL;

            server->active_tcp_connections = server->active_tcp_connections - 1;
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_INFO, "\nNumber of active client connections : %ld", server->active_tcp_connections );
        }
        else
        {
            current_client_node = (cy_tcp_socket_t*) current_client_node->socket_node.next;
        }
    }

    /* once server socket listen backlog is exhausted, need to call accept to get further signals */
    if( server->listen_backlog_exhausted )
    {
        server->listen_backlog_exhausted = false;
        result = cy_socket_accept( server_socket,
                                   &peer_addr,
                                   &peer_addr_len,
                                   &client_handle );
        if( result != CY_RSLT_SUCCESS )
        {
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "\nAccept call on listen exhaust succeeded" );
        }
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " %d : %s() : --- Unlock the mutex\r\n", __LINE__, __FUNCTION__ );
    cy_rtos_set_mutex( &server->mutex );
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " %d : %s() : --- Unlocked the mutex\r\n", __LINE__, __FUNCTION__ );

    return CY_RSLT_SUCCESS;
}

/*****************************************************************/
/* These are helper functions for TLS wrapper to communicate with
 * MBEDOS TCPIP C++ APIs.
 */
cy_rslt_t cy_tls_generic_start_tls_with_ciphers( cy_tls_context_t* tls_context, void* referee, cy_tls_certificate_verification_t verification )
{
    /* Dummy implementation of cy_tls_generic_start_tls_with_ciphers */
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tls_init_context( cy_tls_context_t* tls_context, cy_tls_identity_t* identity, char* peer_cn )
{
    /* Dummy implementation of cy_tls_init_context */
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tls_deinit_context( cy_tls_context_t* tls_context )
{
    /* Dummy implementation of cy_tls_deinit_context */
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tls_set_context_root_ca_certificates( cy_tls_context_t* context, const char* trusted_ca_certificates, const uint32_t cert_length )
{
    /* Dummy implementation of cy_tls_set_context_root_ca_certificates */
    return CY_RSLT_SUCCESS;
}

int cy_tcp_recv( cy_tcp_socket_t* tcp_socket, char* buffer, int length )
{
    /* Dummy implementation of cy_tcp_recv */
    return 0;
}

int cy_tcp_send( cy_tcp_socket_t* tcp_socket, char* buffer, int length )
{
    /* Dummy implementation of cy_tcp_send */
    return 0;
}
/*****************************************************************/

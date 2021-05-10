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
 *  Implements TCP server APIs
 *
 */

#include "cy_result_mw.h"
#include "cy_tcpip_port.h"
#include "cy_log.h"
#include "stdio.h"
#include "mbed.h"
#include "ssl.h"
#include "nsapi_types.h"

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
    /* Dummy implementation for Mbed port layer. */
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tcp_server_network_deinit( void )
{
    /* Dummy implementation for Mbed port layer. */
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tcp_server_start( cy_tcp_server_t* server, cy_network_interface_t* network_interface, uint16_t port, uint16_t max_sockets, cy_server_type_t type)
{
    cy_rslt_t         result = CY_RSLT_SUCCESS;
    char              interface_name[NSAPI_INTERFACE_NAME_MAX_SIZE];
    char*             interface_name_out;
    NetworkInterface  *interface;

    if( server == NULL || network_interface == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_server_start" );
        return CY_RSLT_TCPIP_ERROR;
    }

    interface = (NetworkInterface*) network_interface->object;

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " %d : %s() : --- Init mutex\r\n", __LINE__, __FUNCTION__ );
    result = cy_rtos_init_mutex( &server->mutex );
    if ( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, " Error in mutex init : %d : %s() \r\n", (int) result, __FUNCTION__ );
        return result;
    }

    /* Intialize tcp server socket list */
    cy_linked_list_init( &server->socket_list );

    TCPSocket* server_socket = new TCPSocket();

    server_socket->set_blocking( false );

    result = server_socket->open( interface );
    if ( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, " Error in opening socket result : %d : %s() \r\n", (int) result, __FUNCTION__ );
        delete server_socket;
        cy_rtos_deinit_mutex( &server->mutex );
        return CY_RSLT_TCPIP_ERROR_SOCKET_OPEN;
    }

    interface_name_out = interface->get_interface_name( interface_name );
    if ( interface_name_out == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, " Error in getting interface name \r\n" );
        result = CY_RSLT_TCPIP_ERROR;
        goto cleanup;
    }

    result = server_socket->setsockopt( NSAPI_SOCKET, NSAPI_BIND_TO_DEVICE, interface_name_out, strlen(interface_name_out) );
    if(result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, " Error in setting socket options : %d : %s() \r\n", (int) result, __FUNCTION__ );
        result = CY_RSLT_TCPIP_ERROR_SOCKET_OPTIONS;
        goto cleanup;
    }

    result = server_socket->bind( port );
    if ( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, " Error in socket bind result : %d : %s() \r\n", (int) result, __FUNCTION__ );
        result = CY_RSLT_TCPIP_ERROR_SOCKET_BIND;
        goto cleanup;
    }

    result = server_socket->listen( max_sockets );
    if ( result != CY_RSLT_SUCCESS )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, " Error in socket listen result : %d : %s() \r\n", (int) result, __FUNCTION__ );
        result = CY_RSLT_TCPIP_ERROR_SOCKET_LISTEN;
        goto cleanup;
    }

    server->type                 = type;
    server->server_socket.socket = server_socket;

    /* Initialize socket list to store client connection socket pointer */
    server->max_tcp_connections = max_sockets;

    return result;

cleanup:
    server_socket->close();
    delete server_socket;
    cy_rtos_deinit_mutex( &server->mutex );

    return result;
}

cy_rslt_t cy_tcp_server_accept( cy_tcp_server_t* server, cy_tcp_socket_t** accepted_socket )
{
    cy_tcp_socket_t* client_socket          = NULL;
    TCPSocket* socket                       = NULL;
    TCPSocket* server_socket                = NULL;
    cy_tls_context_t* context               = NULL;
    SocketAddress client_addr;
    cy_rslt_t result                        = CY_RSLT_SUCCESS;
    nsapi_error_t socket_accept_error;

    if( server == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_server_accept" );
        return CY_RSLT_TCPIP_ERROR;
    }
    server_socket = (TCPSocket*) server->server_socket.socket;

    /* Check for maximum clients, If maximum client accepted then don't accept new client connection */
    if ( server->active_tcp_connections >= server->max_tcp_connections )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_INFO, "Maximum accepted socket are [%ld], No more connection can be accepted \r\n", (long)server->max_tcp_connections );
        result = CY_RSLT_TCPIP_ERROR_NO_MORE_SOCKET;
        server->listen_backlog_exhausted = true;
        goto error_cleanup;
    }

    /* Allocate memory to accept client socket */
    *accepted_socket = (cy_tcp_socket_t*) malloc( sizeof(cy_tcp_socket_t) );
    if ( *accepted_socket == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, " Error in allocating memory : %s() \r\n", __FUNCTION__ );
        result = CY_RSLT_TCPIP_ERROR_NO_MEMORY;
        goto error_cleanup;
    }

    client_socket = *accepted_socket;

    memset( client_socket, 0, sizeof(cy_tcp_socket_t) );

    client_socket->socket = (TCPSocket*) server_socket->accept( &socket_accept_error );
    if( socket_accept_error != NSAPI_ERROR_OK)
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " Error in socket accept : %d : %s() \r\n", socket_accept_error, __FUNCTION__ );
        result = CY_RSLT_TCPIP_ERROR_SOCKET_ACCEPT;
        goto error_cleanup;
    }

    socket = (TCPSocket*) client_socket->socket;
    socket->set_blocking( false );

    if( server->type == CY_HTTP_SERVER_TYPE_SECURE )
    {
        context = (cy_tls_context_t*) malloc( sizeof(cy_tls_context_t) );
        if ( context == NULL )
        {
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "Failed to allocate memory for TLS context : %s() \r\n", __FUNCTION__ );
            result = CY_RSLT_TCPIP_ERROR_TLS_OPERATION;
            goto error_cleanup;
        }

        memset( context, 0, sizeof(cy_tls_context_t) );
        client_socket->context = context;

        result = cy_tls_init_context( context, (cy_tls_identity_t*)server->identity, NULL );
        if ( result != 0 )
        {
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "Failed to initialize TLS context result : %ld : %s() \r\n", (long)result, __FUNCTION__ );
            result = CY_RSLT_TCPIP_ERROR_TLS_OPERATION;
            goto error_cleanup;
        }

        result = cy_tls_generic_start_tls_with_ciphers( client_socket->context, client_socket, TLS_CERT_VERIFICATION_REQUIRED );
        if ( result != 0 )
        {
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, "Failed to start cipher result : %ld : %s() \r\n", (long)result, __FUNCTION__ );
            result = CY_RSLT_TCPIP_ERROR_TLS_OPERATION;
            goto error_cleanup;
        }

        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_INFO, "Connection established \n" );
    }

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " %d : %s() : --- Lock the mutex\r\n", __LINE__, __FUNCTION__ );
    cy_rtos_get_mutex(&server->mutex, osWaitForever);
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " %d : %s() : --- Locked the mutex\r\n", __LINE__, __FUNCTION__ );

    cy_linked_list_set_node_data( &client_socket->socket_node, client_socket );
    cy_linked_list_insert_node_at_rear( &server->socket_list, &client_socket->socket_node );
    server->active_tcp_connections = server->active_tcp_connections + 1;

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " %d : %s() : --- Unlock the mutex\r\n", __LINE__, __FUNCTION__ );
    cy_rtos_set_mutex( &server->mutex );
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " %d : %s() : --- Unlocked the mutex\r\n", __LINE__, __FUNCTION__ );

    return CY_RSLT_SUCCESS;

error_cleanup:

    if( client_socket != NULL )
    {
        if( client_socket->socket != NULL )
        {
            TCPSocket* tcp_socket = (TCPSocket*) client_socket->socket;
            tcp_socket->close();
        }

        if( client_socket->context != NULL )
        {
            cy_tls_deinit_context( client_socket->context );
            free( client_socket->context );
            client_socket->context = NULL;
        }

        free( client_socket );
        client_socket = NULL;
    }

    return result;
}

int cy_tcp_server_recv( cy_tcp_socket_t* tcp_socket, char* buffer, int length )
{
    int len = 0;

    if( tcp_socket == NULL || tcp_socket->socket == NULL || buffer == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_server_recv" );
        return CY_RSLT_TCPIP_ERROR;
    }

    if( tcp_socket->context != NULL )
    {
        len = mbedtls_ssl_read( &tcp_socket->context->context, (unsigned char *) buffer, length );
    }
    else
    {
        TCPSocket* client_socket = (TCPSocket*) tcp_socket->socket;
        len = client_socket->recv( buffer, length );
    }

    if( len < 0 )
    {
        /* Socket is non-blocking hence NSAPI_ERROR_WOULD_BLOCK will be ignored and wait for the next event */
        if( len == NSAPI_ERROR_WOULD_BLOCK )
        {
            return CY_HTTP_SERVER_SOCKET_NO_DATA;
        }
        return CY_HTTP_SERVER_SOCKET_ERROR;
    }

    return len;
}

cy_rslt_t cy_tcp_stream_init( cy_tcp_stream_t* stream, void* socket )
{
    if( stream == NULL || socket == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_stream_init" );
        return CY_RSLT_TCPIP_ERROR;
    }
    stream->socket = (cy_tcp_socket_t*) socket;
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
    int data_to_send = data_length;
    int data_written = 0;

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

    if ( stream->socket->context != NULL )
    {
        while( data_to_send > 0 )
        {
            data_written = mbedtls_ssl_write( &stream->socket->context->context, (const unsigned char*) data, data_to_send);
            if( data_written == 0 )
            {
                return data_written;
            }
            else if( data_written < 0 )
            {
                if( data_written == NSAPI_ERROR_WOULD_BLOCK )
                    continue;
                else
                    return data_written;
            }
            else
            {
                data_to_send = data_to_send - data_written;
                data = ((char*) data + data_written);
            }
        }
    }
    else
    {
        TCPSocket*    socket = (TCPSocket*) stream->socket->socket;

        while( data_to_send > 0 )
        {
            data_written = socket->send( data, data_to_send );
            if( data_written == 0 )
            {
                return data_written;
            }
            else if( data_written < 0 )
            {
                if( data_written == NSAPI_ERROR_WOULD_BLOCK )
                      continue;
                else
                      return data_written;
            }
            else
            {
                data_to_send = data_to_send - data_written;
                data = ((char*) data + data_written);
            }
        }
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tcp_stream_flush( cy_tcp_stream_t* stream )
{
    /* This function will be needed for packet driven data approach ( which are not used currently ) */
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_register_socket_callback( cy_tcp_socket_t* socket, receive_callback rcv_callback )
{
    TCPSocket* tcp_socket;

    if( (socket == NULL) || (socket->socket == NULL) )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_register_socket_callback" );
        return CY_RSLT_TCPIP_ERROR;
    }
    tcp_socket = (TCPSocket*) socket->socket;
    tcp_socket->sigio( mbed::callback( *rcv_callback, socket ) );
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_register_connect_callback( cy_tcp_socket_t* socket, connect_callback callback)
{
    TCPSocket* tcp_socket;

    if( (socket == NULL) || (socket->socket == NULL) )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_register_connect_callback" );
        return CY_RSLT_TCPIP_ERROR;
    }
    tcp_socket = (TCPSocket*) socket->socket;
    tcp_socket->sigio( mbed::callback( *callback, socket ) );
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_register_disconnect_callback( cy_tcp_socket_t* socket, disconnect_callback callback )
{
    /* Dummy implementation of cy_register_disconnect_callback */
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_set_socket_recv_timeout( cy_tcp_socket_t* socket, uint32_t timeout )
{
    TCPSocket* tcp_socket;

    if( (socket == NULL) || (socket->socket == NULL) )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_set_socket_recv_timeout" );
        return CY_RSLT_TCPIP_ERROR;
    }
    tcp_socket = (TCPSocket*) socket->socket;
    tcp_socket->set_timeout( timeout );
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tcp_server_stop( cy_tcp_server_t* server )
{
    TCPSocket* server_socket = NULL;
    cy_linked_list_node_t* current_client_socket_node = NULL;
    cy_linked_list_node_t* current_client_socket_next_node = NULL;

    if ( server->server_socket.socket != NULL )
    {
        server_socket = (TCPSocket*) server->server_socket.socket;

        cy_linked_list_get_front_node( &server->socket_list, (cy_linked_list_node_t**) &current_client_socket_node );
        while ( current_client_socket_node != NULL )
        {
            cy_tcp_socket_t* current_client_socket = (cy_tcp_socket_t*) current_client_socket_node->data;
            current_client_socket_next_node = current_client_socket_node->next;

            cy_tcp_server_disconnect_socket( server, current_client_socket );

            current_client_socket_node = current_client_socket_next_node;
        }

        server_socket->close();
        delete server_socket;
        server_socket = NULL;

        cy_linked_list_deinit( &server->socket_list );
    }

    cy_rtos_deinit_mutex( &server->mutex );

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tcp_server_disconnect_socket( cy_tcp_server_t* server, cy_tcp_socket_t* client_socket )
{
    TCPSocket* tcp_socket = (TCPSocket*) client_socket->socket;
    TCPSocket* server_socket = (TCPSocket*) server->server_socket.socket;
    cy_tcp_socket_t* current_client_node = NULL;
    nsapi_error_t socket_accept_error;

    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " %d : %s() : --- Lock the mutex\r\n", __LINE__, __FUNCTION__ );
    cy_rtos_get_mutex( &server->mutex, osWaitForever );
    hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_DEBUG, " %d : %s() : --- Locked the mutex\r\n", __LINE__, __FUNCTION__ );

    cy_linked_list_get_front_node( &server->socket_list, (cy_linked_list_node_t**) &current_client_node );
    while ( current_client_node != NULL )
    {
        cy_tcp_socket_t* current_client_socket = (cy_tcp_socket_t*) current_client_node->socket_node.data;
        if ( current_client_socket == client_socket )
        {
            cy_linked_list_remove_node( &server->socket_list, &current_client_node->socket_node );

            if ( client_socket->context != NULL )
            {
                cy_tls_deinit_context( client_socket->context );
                free( client_socket->context );
                client_socket->context = NULL;
            }

            tcp_socket->close();
            client_socket->socket = NULL;

            free( client_socket );
            current_client_node = NULL;

            server->active_tcp_connections = server->active_tcp_connections - 1;
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
        (void)( (TCPSocket*)server_socket->accept( &socket_accept_error ) );
        if( socket_accept_error != NSAPI_ERROR_WOULD_BLOCK)
        {
            hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "accept call on listen exhaust succeeded : %d : %s() %d NSAPI_ERROR_WOULD_BLOCK\r\n", socket_accept_error, __FUNCTION__, NSAPI_ERROR_WOULD_BLOCK);
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
int cy_tcp_recv( cy_tcp_socket_t* tcp_socket, char* buffer, int length )
{
    int len = 0;
    TCPSocket* client_socket;

    if( tcp_socket == NULL || tcp_socket->socket == NULL || buffer == NULL )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_recv" );
        return -1;
    }

    client_socket = (TCPSocket*) tcp_socket->socket;
    len = client_socket->recv( buffer, length );

    return len;
}

int cy_tcp_send( cy_tcp_socket_t* tcp_socket, char* buffer, int length )
{
    TCPSocket* client_socket;

    if( (tcp_socket == NULL) || (tcp_socket->socket == NULL) )
    {
        hs_cy_log_msg( CYLF_MIDDLEWARE, CY_LOG_ERR, "\nInvalid parameter to cy_tcp_recv" );
        return -1;
    }
    client_socket = (TCPSocket*) tcp_socket->socket;
    return client_socket->send( buffer, length );
}
/*****************************************************************/

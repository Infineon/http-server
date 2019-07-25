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
 *  Implements TCP server APIs
 *
 */

#include "cy_result_mw.h"
#include "cy_tcpip.h"
#include "stdio.h"
#include "mbed.h"
#include "ssl.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define TCP_SERVER_INFO(X)    printf X

#define TCP_SERVER_DEBUG(X)   //printf X
#define TCP_SERVER_ERROR(X)   //printf X

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
cy_rslt_t cy_tcp_server_start( cy_tcp_server_t* server, cy_network_interface_t* network_interface, uint16_t port, uint16_t max_sockets, cy_server_type_t type)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    NetworkInterface* interface = (NetworkInterface*) network_interface->object;

    memset(server, 0, sizeof(cy_tcp_server_t));

    TCP_SERVER_DEBUG((" %d : %s() : --- Init mutex\r\n", __LINE__, __FUNCTION__));
    result = cy_rtos_init_mutex(&server->mutex);
    if ( result != CY_RSLT_SUCCESS )
    {
        TCP_SERVER_ERROR((" Error in mutex init : %d : %s() \r\n", (int) result, __FUNCTION__));
        return result;
    }

    /* Intialize tcp server socket list */
    cy_linked_list_init( &server->socket_list );

    TCPSocket* server_socket = new TCPSocket();

    server_socket->set_blocking(false);

    result = server_socket->open(interface);
    if ( result != CY_RSLT_SUCCESS )
    {
        TCP_SERVER_ERROR ((" Error in opening socket result : %d : %s() \r\n", (int) result, __FUNCTION__));
        cy_rtos_deinit_mutex(&server->mutex);
        return CY_RSLT_TCPIP_ERROR_SOCKET_OPEN;
    }

    result = server_socket->bind( port );
    if ( result != CY_RSLT_SUCCESS )
    {
        TCP_SERVER_ERROR((" Error in socket bind result : %d : %s() \r\n", (int) result, __FUNCTION__));
        delete server_socket;
        cy_rtos_deinit_mutex(&server->mutex);
        return CY_RSLT_TCPIP_ERROR_SOCKET_BIND;
    }

    result = server_socket->listen( max_sockets );
    if ( result != CY_RSLT_SUCCESS )
    {
        TCP_SERVER_ERROR((" Error in socket listen result : %d : %s() \r\n", (int) result, __FUNCTION__));
        delete server_socket;
        cy_rtos_deinit_mutex(&server->mutex);
        return CY_RSLT_TCPIP_ERROR_SOCKET_LISTEN;
    }

    server->type          = type;
    server->server_socket.socket = server_socket;

    /* Initialize socket list to store client connection socket pointer */
    server->max_tcp_connections = max_sockets;

    return result;
}

cy_rslt_t cy_tcp_server_accept ( cy_tcp_server_t* server, cy_tcp_socket_t** accepted_socket)
{
    cy_tcp_socket_t* client_socket          = NULL;
    TCPSocket* socket                       = NULL;
    TCPSocket* server_socket                = NULL;
    cy_tls_context_t* context               = NULL;
    SocketAddress client_addr;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    nsapi_error_t socket_accept_error;

    server_socket = (TCPSocket*) server->server_socket.socket;

    /* Check for maximum clients, If maximum client accepted then don't accept new client connection */
    if ( server->active_tcp_connections >= server->max_tcp_connections )
    {
        TCP_SERVER_INFO(("Maximum accepted socket are [%ld], No more connection can be accepted \r\n", server->max_tcp_connections));
        result = CY_RSLT_TCPIP_ERROR_NO_MORE_SOCKET;
        server->listen_backlog_exhausted = true;
        goto error_cleanup;
    }

    /* Allocate memory to accept client socket */
    *accepted_socket = (cy_tcp_socket_t*) malloc (sizeof(cy_tcp_socket_t));
    if ( *accepted_socket == NULL )
    {
        TCP_SERVER_ERROR((" Error in allocating memory : %s() \r\n", __FUNCTION__));
        result = CY_RSLT_TCPIP_ERROR_NO_MEMORY;
        goto error_cleanup;
    }

    client_socket = *accepted_socket;

    memset(client_socket, 0, sizeof(cy_tcp_socket_t));

    client_socket->socket = (TCPSocket*) server_socket->accept(&socket_accept_error);
    if( socket_accept_error != NSAPI_ERROR_OK)
    {
        TCP_SERVER_ERROR((" Error in socket accept : %d : %s() \r\n", socket_accept_error, __FUNCTION__));
        result = CY_RSLT_TCPIP_ERROR_SOCKET_ACCEPT;
        goto error_cleanup;
    }

    socket = (TCPSocket*) client_socket->socket;
    socket->set_blocking(false);

    if( server->type == CY_HTTP_SERVER_TYPE_SECURE )
    {
        context = (cy_tls_context_t*) malloc (sizeof(cy_tls_context_t));
        if ( context == NULL )
        {
            TCP_SERVER_ERROR(("Failed to allocate memory for TLS context : %s() \r\n", __FUNCTION__));
            result = CY_RSLT_TCPIP_ERROR_TLS_OPERATION;
            goto error_cleanup;
        }

        memset( context, 0, sizeof(cy_tls_context_t));

        result = cy_tls_init_context(context, server->identity, NULL);
        if ( result != 0 )
        {
            TCP_SERVER_ERROR(("Failed to initialize TLS context result : %lu : %s() \r\n", result, __FUNCTION__));
            result = CY_RSLT_TCPIP_ERROR_TLS_OPERATION;
            goto error_cleanup;
        }

        client_socket->context = context;

        result = cy_tls_generic_start_tls_with_ciphers( client_socket->context, client_socket, TLS_CERT_VERIFICATION_REQUIRED );
        if ( result != 0 )
        {
            TCP_SERVER_ERROR(("Failed to start cipher result : %lu : %s() \r\n", result, __FUNCTION__));
            result = CY_RSLT_TCPIP_ERROR_TLS_OPERATION;
            goto error_cleanup;
        }

        TCP_SERVER_INFO(("Connection established \n"));
    }

    TCP_SERVER_DEBUG((" %d : %s() : --- Lock the mutex\r\n", __LINE__, __FUNCTION__));
    cy_rtos_get_mutex(&server->mutex, osWaitForever);
    TCP_SERVER_DEBUG((" %d : %s() : --- Locked the mutex\r\n", __LINE__, __FUNCTION__));

    cy_linked_list_set_node_data( &client_socket->socket_node, client_socket );
    cy_linked_list_insert_node_at_rear( &server->socket_list, &client_socket->socket_node );
    server->active_tcp_connections = server->active_tcp_connections + 1;

    TCP_SERVER_DEBUG((" %d : %s() : --- Unlock the mutex\r\n", __LINE__, __FUNCTION__));
    cy_rtos_set_mutex(&server->mutex);
    TCP_SERVER_DEBUG((" %d : %s() : --- Unlocked the mutex\r\n", __LINE__, __FUNCTION__));

    return CY_RSLT_SUCCESS;

error_cleanup:
    if( client_socket != NULL )
    {
        free(client_socket);
        client_socket = NULL;
    }

    if( context != NULL)
    {
        free(context);
    }

    return result;
}

int cy_tcp_server_recv ( cy_tcp_socket_t* tcp_socket, char* buffer, int length)
{
    int len = 0;

    if( tcp_socket->context != NULL )
    {
        len = mbedtls_ssl_read( &tcp_socket->context->context, (unsigned char *) buffer, length);
    }
    else
    {
        TCPSocket* client_socket = (TCPSocket*) tcp_socket->socket;

        len = client_socket->recv(buffer, length);
    }

    return len;
}

cy_rslt_t cy_tcp_stream_init( cy_tcp_stream_t* stream, void* socket )
{
    stream->socket = (cy_tcp_socket_t*) socket;
   return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tcp_stream_deinit( cy_tcp_stream_t* stream )
{
    stream->socket  = NULL;
   return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tcp_stream_write( cy_tcp_stream_t* stream, const void* data, uint32_t data_length )
{
    int data_to_send = data_length;
    int data_written = 0;

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
            data_written = socket->send( data, data_to_send);
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

cy_rslt_t cy_register_socket_callback( cy_tcp_socket_t* socket, receive_callback rcv_callback)
{
    TCPSocket* tcp_socket = (TCPSocket*) socket->socket;

    tcp_socket->sigio(mbed::callback(*rcv_callback, socket));

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_register_connect_callback( cy_tcp_socket_t* socket, connect_callback callback)
{
    TCPSocket* tcp_socket = (TCPSocket*) socket->socket;

    tcp_socket->sigio(mbed::callback(*callback, socket));

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tcp_server_stop( cy_tcp_server_t* server )
{
    TCPSocket* server_socket = NULL;
    cy_linked_list_node_t* last_node = NULL;

    if ( server->server_socket.socket != NULL )
    {
        server_socket = (TCPSocket*) server->server_socket.socket;

        /* Remove all the accepted client socket from socket list */
        while( cy_linked_list_remove_node_from_rear( &server->socket_list, &last_node ) == CY_RSLT_SUCCESS )
        {
            cy_tcp_socket_t* client_socket = (cy_tcp_socket_t*)last_node->data;
            cy_tcp_server_disconnect_socket( server, client_socket );
        }

        server_socket->close();
        delete server_socket;
        server_socket = NULL;

        cy_linked_list_deinit( &server->socket_list );
    }

    cy_rtos_deinit_mutex(&server->mutex);

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_tcp_server_disconnect_socket ( cy_tcp_server_t* server, cy_tcp_socket_t* client_socket )
{
    TCPSocket* tcp_socket = (TCPSocket*) client_socket->socket;
    TCPSocket* server_socket = (TCPSocket*) server->server_socket.socket;
    cy_tcp_socket_t* current_client_node = NULL;
    nsapi_error_t socket_accept_error;

    TCP_SERVER_DEBUG((" %d : %s() : --- Lock the mutex\r\n", __LINE__, __FUNCTION__));
    cy_rtos_get_mutex(&server->mutex, osWaitForever);
    TCP_SERVER_DEBUG((" %d : %s() : --- Locked the mutex\r\n", __LINE__, __FUNCTION__));

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
                free(client_socket->context);
                client_socket->context = NULL;
            }

            tcp_socket->close();
            client_socket->socket = NULL;

            free(client_socket);
            current_client_node = NULL;

            server->active_tcp_connections = server->active_tcp_connections - 1;
        }
        else
        {
            current_client_node = (cy_tcp_socket_t*) current_client_node->socket_node.next;
        }
    }

    /* once server socket listen backlog is exhausted, need to call accept to get further signals */
    if(server->listen_backlog_exhausted)
    {
        server->listen_backlog_exhausted = false;
        (TCPSocket*)server_socket->accept(&socket_accept_error);
        if( socket_accept_error != NSAPI_ERROR_WOULD_BLOCK)
        {
            TCP_SERVER_ERROR(("accept call on listen exhaust succeeded : %d : %s() %lu NSAPI_ERROR_WOULD_BLOCK\r\n", socket_accept_error, __FUNCTION__, NSAPI_ERROR_WOULD_BLOCK));
        }
    }

    TCP_SERVER_DEBUG((" %d : %s() : --- Unlock the mutex\r\n", __LINE__, __FUNCTION__));
    cy_rtos_set_mutex(&server->mutex);
    TCP_SERVER_DEBUG((" %d : %s() : --- Unlocked the mutex\r\n", __LINE__, __FUNCTION__));

    return CY_RSLT_SUCCESS;
}


/*****************************************************************/
/* These are helper functions for TLS wrapper to communicate with
 * MBEDOS TCPIP C++ APIs.
 */
int cy_tcp_recv ( cy_tcp_socket_t* tcp_socket, char* buffer, int length)
{
    int len = 0;
    TCPSocket* client_socket = (TCPSocket*) tcp_socket->socket;

    len = client_socket->recv(buffer, length);

    return len;
}

int cy_tcp_send( cy_tcp_socket_t* tcp_socket, char* buffer, int length)
{
    TCPSocket* client_socket = (TCPSocket*) tcp_socket->socket;

    return client_socket->send(buffer, length);
}
/*****************************************************************/

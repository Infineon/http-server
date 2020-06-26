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
 *  TCP server APIs
 *
 */

#include "cy_result_mw.h"
#include "cy_nw_helper.h"
#include "cy_linked_list.h"
#include "cy_tls_port.h"
#include "cyabs_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/
#define CY_HTTP_SERVER_SOCKET_NO_DATA      ( 0 )
#define CY_HTTP_SERVER_SOCKET_ERROR        ( -1 )

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef enum
{
    CY_HTTP_SERVER_TYPE_SECURE,
    CY_HTTP_SERVER_TYPE_NON_SECURE
} cy_server_type_t;

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    cy_linked_list_node_t socket_node;
    void                  *socket;
    cy_tls_context_t      *context;
    void (*receive_cb)( void * );
    void (*disconnect_cb)( void * );
} cy_tcp_socket_t;

typedef struct
{
    cy_server_type_t      type;
    cy_tcp_socket_t       server_socket;
    cy_linked_list_t      socket_list;
    cy_tls_identity_t     *identity;
    uint8_t               *root_ca_certificate;
    uint8_t               root_ca_certificate_length;
    uint32_t              max_tcp_connections;
    uint32_t              active_tcp_connections;
    cy_mutex_t            mutex;
    bool                  listen_backlog_exhausted;
} cy_tcp_server_t;

/**
 * TCP Stream Structure
 */
typedef struct
{
    cy_tcp_socket_t *socket;
} cy_tcp_stream_t;

typedef void (*receive_callback) (void*);
typedef void (*connect_callback) (void*);
typedef void (*disconnect_callback) ( void* );

/******************************************************
 *               Static Function Declarations
 ******************************************************/
cy_rslt_t cy_tcp_server_network_init      ( void );
cy_rslt_t cy_tcp_server_network_deinit    ( void );
cy_rslt_t cy_tcp_server_start             ( cy_tcp_server_t* server, cy_network_interface_t* network_interface, uint16_t port, uint16_t max_sockets, cy_server_type_t type );
cy_rslt_t cy_tcp_server_accept            ( cy_tcp_server_t* server, cy_tcp_socket_t** client_socket );
cy_rslt_t cy_tcp_stream_init              ( cy_tcp_stream_t* stream, void* socket );
cy_rslt_t cy_tcp_stream_deinit            ( cy_tcp_stream_t* stream );
cy_rslt_t cy_tcp_stream_write             ( cy_tcp_stream_t* stream, const void* data, uint32_t data_length );
cy_rslt_t cy_tcp_stream_flush             ( cy_tcp_stream_t* stream );
cy_rslt_t cy_register_socket_callback     ( cy_tcp_socket_t* socket, receive_callback rcv_callback);
cy_rslt_t cy_register_connect_callback    ( cy_tcp_socket_t* socket, connect_callback rcv_callback);
cy_rslt_t cy_tcp_server_stop              ( cy_tcp_server_t* server );
int       cy_tcp_server_recv              ( cy_tcp_socket_t* server_socket, char* buffer, int length);
cy_rslt_t cy_tcp_server_disconnect_socket ( cy_tcp_server_t* server, cy_tcp_socket_t* client_socket );
cy_rslt_t cy_register_disconnect_callback ( cy_tcp_socket_t* socket, disconnect_callback dis_callback);
cy_rslt_t cy_set_socket_recv_timeout      ( cy_tcp_socket_t* socket, uint32_t timeout );

/* Helper functions for TLS wrapper to communicate with MBEDOS TCPIP C++ APIs. */
int cy_tcp_send( cy_tcp_socket_t* socket, char* buffer, int length);
int cy_tcp_recv( cy_tcp_socket_t* socket, char* buffer, int length);
/******************************************************
 *                 Static Variables
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif

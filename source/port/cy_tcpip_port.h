/*
 * Copyright 2025, Cypress Semiconductor Corporation (an Infineon company) or
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

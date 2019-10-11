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
 *  Implements both HTTP and HTTPS servers
 *
 */

#include <string.h>
#include <stdlib.h>
#include "nsapi_types.h"
#include "cy_HTTP_server.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define MTU_SIZE 1460

//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define HTTP_DEBUG( X )        printf X
#else
#define HTTP_DEBUG( X )
#endif

#define HTTP_ERROR( X )      //printf X

#ifndef HTTP_SERVER_CONNECT_THREAD_STACK_SIZE
#define HTTP_SERVER_CONNECT_THREAD_STACK_SIZE (6000)
#endif

#ifndef HTTP_SERVER_EVENT_THREAD_STACK_SIZE
#define HTTP_SERVER_EVENT_THREAD_STACK_SIZE   (6000)
#endif

#define HTTP_SERVER_THREAD_PRIORITY    (osPriorityNormal)
#define HTTP_SERVER_RECEIVE_TIMEOUT    (cy_NO_WAIT)

/* HTTP Tokens */
#define GET_TOKEN                      "GET "
#define POST_TOKEN                     "POST "
#define PUT_TOKEN                      "PUT "

#define HTTP_1_1_TOKEN                 " HTTP/1.1"
#define FINAL_CHUNKED_PACKET           "0\r\n\r\n"

/*
 * Request-Line =   Method    SP        Request-URI           SP       HTTP-Version      CRLFCRLF
 *              = <-3 char->  <-1 char->   <-1 char->      <-1 char->  <--8 char-->    <-4char->
 *              = 18
 */
#define MINIMUM_REQUEST_LINE_LENGTH    (18)
#define EVENT_QUEUE_DEPTH              (20)
#define CONNECT_EVENT_QUEUE_DEPTH      (10)
#define COMPARE_MATCH                  (0)
#define MAX_URL_LENGTH                 (100)

/* Most of the webserver supports request length 2KB to 8KB.
 * We may thus assume that 8KB is the maximum possible length
 * and 2KB is a more affordable length to rely on at the server side */
#define MAXIMUM_CACHED_LENGTH          (8192)

#define CY_VERIFY(x)                   {cy_rslt_t res = (cy_rslt_t)(x); if (res != CY_RSLT_SUCCESS){return res;}}

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    CY_SOCKET_ERROR_EVENT,
    CY_SOCKET_DISCONNECT_EVENT,
    CY_SOCKET_PACKET_RECEIVED_EVENT,
    CY_SERVER_STOP_EVENT,
    CY_SERVER_CONNECT_EVENT
} cy_http_server_event_t;

typedef enum
{
    CY_HTTP_HEADER_AND_DATA_FRAME_STATE,
    CY_HTTP_DATA_ONLY_FRAME_STATE,
    CY_HTTP_ERROR_STATE
} cy_http_packet_state_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    void*                  socket;
    cy_http_server_event_t event_type;
} server_event_message_t;

typedef struct
{
    cy_linked_list_node_t  node;
    cy_http_stream_t       stream;
} cy_stream_node_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static void                http_server_connect_callback         ( void* socket );
static void                http_server_receive_callback         ( void* socket );
void                       http_server_event_thread_main        ( cy_thread_arg_t arg );
void                       http_server_connect_thread_main      ( cy_thread_arg_t arg );
static cy_rslt_t           http_server_parse_receive_packet     ( cy_http_server_t* server, cy_http_stream_t* stream, char* data, uint32_t length );
cy_rslt_t                  http_server_process_url_request      ( cy_http_stream_t* stream, const cy_http_page_t* page_database, char* url, uint32_t url_length, cy_http_message_body_t* http_message_body );
uint16_t                   http_server_remove_escaped_characters( char* output, uint16_t output_length, const char* input, uint16_t input_length );
cy_packet_mime_type_t      http_server_get_mime_type            ( const char* request_data );
cy_rslt_t                  http_server_get_request_type_and_url ( char* request, uint16_t request_length, cy_http_request_type_t* type, char** url_start, uint16_t* url_length );
cy_rslt_t                  http_server_find_url_in_page_database( char* url, uint32_t length, cy_http_message_body_t* http_request, const cy_http_page_t* page_database, cy_http_page_t** page_found, cy_packet_mime_type_t* mime_type );
bool                       http_server_compare_stream_socket    ( cy_linked_list_node_t* node_to_compare, void* user_data );
static cy_rslt_t           http_internal_server_start           (cy_http_server_t* server, void* network_interface, uint16_t port, uint16_t max_sockets, const cy_http_page_t* page_database, uint32_t http_thread_stack_size, uint32_t server_connect_thread_stack_size, cy_server_type_t type, cy_http_security_info* security_info );
/******************************************************
 *                 Static Variables
 ******************************************************/

static const char* const http_mime_array[ MIME_UNSUPPORTED ] =
{
    MIME_TABLE( EXPAND_AS_MIME_TABLE )
};

static const char* const cy_http_status_codes[ ] =
{
    [CY_HTTP_200_TYPE] = HTTP_HEADER_200,
    [CY_HTTP_204_TYPE] = HTTP_HEADER_204,
    [CY_HTTP_207_TYPE] = HTTP_HEADER_207,
    [CY_HTTP_301_TYPE] = HTTP_HEADER_301,
    [CY_HTTP_400_TYPE] = HTTP_HEADER_400,
    [CY_HTTP_403_TYPE] = HTTP_HEADER_403,
    [CY_HTTP_404_TYPE] = HTTP_HEADER_404,
    [CY_HTTP_405_TYPE] = HTTP_HEADER_405,
    [CY_HTTP_406_TYPE] = HTTP_HEADER_406,
    [CY_HTTP_412_TYPE] = HTTP_HEADER_412,
    [CY_HTTP_415_TYPE] = HTTP_HEADER_406,
    [CY_HTTP_429_TYPE] = HTTP_HEADER_429,
    [CY_HTTP_444_TYPE] = HTTP_HEADER_444,
    [CY_HTTP_470_TYPE] = HTTP_HEADER_470,
    [CY_HTTP_500_TYPE] = HTTP_HEADER_500,
    [CY_HTTP_504_TYPE] = HTTP_HEADER_504
};

static char*  cached_string = NULL;
static size_t cached_length = 0;

MBED_ALIGN(8) uint8_t HTTP_server_thread_stack[HTTP_SERVER_CONNECT_THREAD_STACK_SIZE]     = {0};
MBED_ALIGN(8) uint8_t HTTP_server_event_thread_stack[HTTP_SERVER_EVENT_THREAD_STACK_SIZE] = {0};

static cy_queue_t event_queue;
static cy_queue_t connect_event_queue;

/******************************************************
 *               Function Definitions
 ******************************************************/

cy_rslt_t cy_http_server_start( cy_http_server_t* server, void* network_interface, uint16_t port, uint16_t max_sockets, cy_http_page_t* page_database, cy_server_type_t type, cy_http_security_info* security_info )
{
    if ( server == NULL || page_database == NULL )
    {
        return CY_RSLT_HTTP_SERVER_ERROR_BADARG;
    }

    memset( server, 0, sizeof( *server ) );
    return http_internal_server_start (server, network_interface, port,max_sockets,page_database, HTTP_SERVER_EVENT_THREAD_STACK_SIZE, HTTP_SERVER_CONNECT_THREAD_STACK_SIZE, type, security_info);
}

static cy_rslt_t http_internal_server_start(cy_http_server_t* server, void* network_interface, uint16_t port, uint16_t max_sockets, const cy_http_page_t* page_database, uint32_t http_thread_stack_size, uint32_t server_connect_thread_stack_size, cy_server_type_t type, cy_http_security_info* security_info )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_stream_node_t* stream_node;
    uint32_t a;

    /* Store the inputs database */
    server->page_database = page_database;

    /* Allocate space for response streams and insert them into the inactive stream list */
    cy_linked_list_init( &server->inactive_stream_list );
    server->streams = malloc( sizeof(cy_stream_node_t) * max_sockets );
    if ( server->streams == NULL )
    {
        return CY_RSLT_HTTP_SERVER_ERROR_NO_MEMORY;
    }

    memset( server->streams, 0, sizeof(cy_stream_node_t) * max_sockets );
    stream_node = (cy_stream_node_t*)server->streams;
    for ( a = 0; a < max_sockets; a++ )
    {
        cy_linked_list_set_node_data( &stream_node[a].node, (void*)&stream_node[a] );
        cy_linked_list_insert_node_at_rear( &server->inactive_stream_list, &stream_node[a].node );
    }

    /* Create linked-list for holding active response streams */
    cy_linked_list_init( &server->active_stream_list );

    result = cy_rtos_init_queue(&event_queue, EVENT_QUEUE_DEPTH, sizeof(server_event_message_t));
    if ( result != CY_RSLT_SUCCESS )
    {
        HTTP_ERROR(("Failed to initialize event queue \n"));
        result = CY_RSLT_HTTP_SERVER_ERROR_QUEUE_INIT;
        goto ERROR_QUEUE_INIT;
    }

    result = cy_rtos_init_queue(&connect_event_queue, CONNECT_EVENT_QUEUE_DEPTH, sizeof(server_event_message_t)) ;
    if ( result != CY_RSLT_SUCCESS )
    {
        HTTP_ERROR(("Failed to initialize connect queue \n"));
        result = CY_RSLT_HTTP_SERVER_ERROR_QUEUE_INIT;
        goto ERROR_QUEUE_INIT;
    }

    result = cy_rtos_init_mutex(&server->mutex);
    if ( result != CY_RSLT_SUCCESS )
    {
        HTTP_ERROR((" Error in mutex init : %d : %s() \r\n", (int) result, __FUNCTION__));
        result = CY_RSLT_HTTP_SERVER_ERROR_MUTEX_INIT;
        goto ERROR_MUTEX_INIT;
    }

    /* Create thread to process connect events */
    result = cy_rtos_create_thread(&server->connect_thread, http_server_connect_thread_main, "connect_thread", HTTP_server_thread_stack,
                                    HTTP_SERVER_CONNECT_THREAD_STACK_SIZE, (cy_thread_priority_t) HTTP_SERVER_THREAD_PRIORITY, (cy_thread_arg_t)server);
    if( result != CY_RSLT_SUCCESS )
    {
        HTTP_ERROR(("Unable to create connect thread \n"));
        result = CY_RSLT_HTTP_SERVER_ERROR_THREAD_INIT;
        goto ERROR_THREAD_INIT;
    }

    /* Create HTTP server connect thread */
    result = cy_rtos_create_thread(&server->event_thread, http_server_event_thread_main, "event_thread", HTTP_server_event_thread_stack,
                                    HTTP_SERVER_EVENT_THREAD_STACK_SIZE, (cy_thread_priority_t) HTTP_SERVER_THREAD_PRIORITY, (cy_thread_arg_t)server);
    if( result != CY_RSLT_SUCCESS )
    {
        HTTP_ERROR(("Unable to create event thread \n"));
        result = CY_RSLT_HTTP_SERVER_ERROR_THREAD_INIT;
        goto ERROR_THREAD_INIT;
    }

    result = cy_tcp_server_start( &server->tcp_server, (cy_network_interface_t*) network_interface, port, max_sockets, type );
    if ( result != CY_RSLT_SUCCESS )
    {
        HTTP_ERROR(("Error starting tcp server : %d \n", (int) result));
        result = CY_RSLT_HTTP_SERVER_ERROR_TCP_SERVER_START;
        goto ERROR_THREAD_INIT;
    }

    server->tcp_server.identity = security_info->tls_identity;

    cy_register_connect_callback(&server->tcp_server.server_socket, http_server_connect_callback);

    return result;

ERROR_THREAD_INIT:
    if ( server->connect_thread != NULL )
    {
        cy_rtos_terminate_thread( &server->connect_thread );
    }

    if ( server->event_thread != NULL )
    {
        cy_rtos_terminate_thread( &server->event_thread );
    }

ERROR_MUTEX_INIT:
    cy_rtos_deinit_mutex( &server->mutex );

ERROR_QUEUE_INIT:
    if ( connect_event_queue != NULL )
    {
        cy_rtos_deinit_queue( &connect_event_queue );
        connect_event_queue = NULL;
    }

    if ( event_queue != NULL )
    {
        cy_rtos_deinit_queue( &event_queue );
        event_queue = NULL;
    }

    free(server->streams);
    return result;

}

cy_rslt_t cy_http_server_stop( cy_http_server_t* server )
{
    server_event_message_t current_event;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if ( server == NULL )
    {
        return CY_RSLT_HTTP_SERVER_ERROR_BADARG;
    }

    current_event.event_type = CY_SERVER_STOP_EVENT;
    current_event.socket     = 0;

    HTTP_DEBUG((" %s() : ----- ### Send STOP event to connect thread\r\n", __FUNCTION__));
    result = cy_rtos_put_queue (&connect_event_queue, &current_event, 0, 0);
    if ( result != CY_RSLT_SUCCESS)
    {
        HTTP_ERROR(("cy_rtos_put_queue failed : %s() \r\n", __FUNCTION__));
        return result;
    }

    HTTP_DEBUG((" %s() : ----- ### Send STOP event to event thread\r\n", __FUNCTION__));
    result = cy_rtos_put_queue (&event_queue, &current_event, 0, 0);
    if ( result != CY_RSLT_SUCCESS)
    {
        HTTP_ERROR(("cy_rtos_put_queue failed : %s() \r\n", __FUNCTION__));
        return result;
    }

    HTTP_DEBUG((" %s() : ----- ### wait for connect thread to process stop event exit\r\n", __FUNCTION__));
    result = cy_rtos_join_thread( &server->connect_thread );
    if ( result != CY_RSLT_SUCCESS)
    {
        HTTP_ERROR(("osThreadJoin failed : %s() \r\n", __FUNCTION__));
        return result;
    }

    HTTP_DEBUG((" %s() : ----- ### wait for event thread to process stop event and exit\r\n", __FUNCTION__));
    result = cy_rtos_join_thread( &server->event_thread );
    if ( result != CY_RSLT_SUCCESS)
    {
        HTTP_ERROR(("osThreadJoin failed : %s() \r\n", __FUNCTION__));
        return result;
    }

    HTTP_DEBUG((" %s() : ----- ### DBG : Stop TCP server\r\n", __FUNCTION__));
    cy_tcp_server_stop( &server->tcp_server );

    cy_rtos_deinit_mutex(&server->mutex);

    HTTP_DEBUG((" %s() : ----- ### DBG : Message queue delete\r\n", __FUNCTION__));
    cy_rtos_deinit_queue(&event_queue);
    cy_rtos_deinit_queue(&connect_event_queue);

    HTTP_DEBUG((" %s() : ----- ### Delete stream mgmt lists\r\n", __FUNCTION__));
    cy_linked_list_deinit( &server->inactive_stream_list );
    cy_linked_list_deinit( &server->active_stream_list );
    free( server->streams );
    server->streams = NULL;

    HTTP_DEBUG((" %s() : ----- ### Stop even processed successfully\r\n", __FUNCTION__));

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_http_response_stream_enable_chunked_transfer( cy_http_response_stream_t* stream )
{
    if ( stream == NULL )
    {
        return CY_RSLT_HTTP_SERVER_ERROR_BADARG;
    }

    stream->chunked_transfer_enabled = true;
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_http_response_stream_disable_chunked_transfer( cy_http_response_stream_t* stream )
{

    if ( stream == NULL )
    {
        return CY_RSLT_HTTP_SERVER_ERROR_BADARG;
    }

    if ( stream->chunked_transfer_enabled == true )
    {
        /* Send final chunked frame */
        CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, FINAL_CHUNKED_PACKET, sizeof( FINAL_CHUNKED_PACKET ) - 1 ) );
    }

    stream->chunked_transfer_enabled = false;
    return CY_RSLT_SUCCESS;
}

static uint8_t match_string_with_wildcard_pattern( const char* string, uint32_t length, const char* pattern )
{
    uint32_t current_string_length = length;
    uint32_t temp_string_length    = 0;
    char*    current_string        = (char*)string;
    char*    current_pattern       = (char*)pattern;
    char*    temp_string           = NULL;
    char*    temp_pattern          = NULL;

    /* Iterate through string and pattern until '*' is found */
    while ( ( current_string_length != 0 ) && ( *current_pattern != '*' ) )
    {
        /* Current pattern is not equal current string and current pattern isn't a wildcard character */
        if ( ( *current_pattern != *current_string ) && ( *current_pattern != '?' ) )
        {
            return 0;
        }
        current_pattern++;
        current_string++;
        current_string_length--;
    }

    /* '*' is detected in pattern. Consume string until matching pattern is found */
    while ( current_string_length != 0 )
    {
        switch ( *current_pattern )
        {
            case '*':
                if ( *(++current_pattern) == '\0' )
                {
                    /* Last character in the pattern is '*'. Return successful */
                    return 1;
                }

                /* Store temp variables for starting another matching iteration when non-matching character is found. */
                temp_pattern       = current_pattern;
                temp_string_length = current_string_length - 1;
                temp_string        = current_string + 1;
                break;

            case '?':
                current_pattern++;
                current_string++;
                current_string_length--;
                break;

            default:
                if ( *current_pattern == *current_string )
                {
                    current_pattern++;
                    current_string++;
                    current_string_length--;
                }
                else
                {
                    current_pattern       = temp_pattern;
                    current_string        = temp_string++;
                    current_string_length = temp_string_length--;
                }
                break;
        }
    }

    while ( *current_pattern == '*' )
    {
        current_pattern++;
    }

    return ( *current_pattern == '\0' );
}

static char* strnstrn(const char *s, uint16_t s_len, const char *substr, uint16_t substr_len)
{
    for (; s_len >= substr_len; s++, s_len--)
    {
        if (strncmp(s, substr, substr_len) == 0)
        {
            return (char*)s;
        }
    }

    return NULL;
}

cy_rslt_t cy_http_response_stream_write_header( cy_http_response_stream_t* stream, cy_http_status_codes_t status_code, uint32_t content_length, cy_http_cache_t cache_type, cy_packet_mime_type_t mime_type )
{
    char data_length_string[ 15 ];

    if ( stream == NULL )
    {
        return CY_RSLT_HTTP_SERVER_ERROR_BADARG;
    }

    memset( data_length_string, 0x0, sizeof( data_length_string ) );

    /*HTTP/1.1 <status code>\r\n*/
    CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, cy_http_status_codes[ status_code ], strlen( cy_http_status_codes[ status_code ] ) ) );
    CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, CRLF, sizeof( CRLF )-1 ) );

    /* Content-Type: xx/yy\r\n */
    CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, HTTP_HEADER_CONTENT_TYPE, strlen( HTTP_HEADER_CONTENT_TYPE ) ) );
    CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, http_mime_array[ mime_type ], strlen( http_mime_array[ mime_type ] ) ) );
    CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, CRLF, strlen( CRLF ) ) );

    if ( cache_type == CY_HTTP_CACHE_DISABLED )
    {
        CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, NO_CACHE_HEADER, strlen( NO_CACHE_HEADER ) ) );
        CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, CRLF, strlen( CRLF ) ) );
    }

    if ( status_code == CY_HTTP_444_TYPE )
    {
        /* Connection: close */
        CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, HTTP_HEADER_CLOSE, strlen( HTTP_HEADER_CLOSE ) ) );
        CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, CRLF, strlen( CRLF ) ) );
    }
    else
    {
        CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, HTTP_HEADER_KEEP_ALIVE, strlen( HTTP_HEADER_KEEP_ALIVE ) ) );
        CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, CRLF, strlen( CRLF ) ) );
    }

    if ( stream->chunked_transfer_enabled == true )
    {
        /* Chunked transfer encoding */
        CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, HTTP_HEADER_CHUNKED, strlen( HTTP_HEADER_CHUNKED ) ) );
        CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, CRLF, strlen( CRLF ) ) );
    }
    else
    {
        /* for EVENT Stream content length is  Zero*/
        if ( mime_type != MIME_TYPE_TEXT_EVENT_STREAM )
        {
            /* Content-Length: xx\r\n */
            sprintf( data_length_string, "%lu", (long) content_length );
            CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, HTTP_HEADER_CONTENT_LENGTH, strlen( HTTP_HEADER_CONTENT_LENGTH ) ) );
            CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, data_length_string, strlen( data_length_string ) ) );
            CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, CRLF, strlen( CRLF ) ) );
        }
    }

    /* Closing sequence */
    CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, CRLF, strlen( CRLF ) ) );

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_http_response_stream_write( cy_http_response_stream_t* stream, const void* data, uint32_t length )
{

    if ( length == 0 || stream == NULL )
    {
        return CY_RSLT_HTTP_SERVER_ERROR_BADARG;
    }

    if ( stream->chunked_transfer_enabled == true )
    {
        char data_length_string[10];
        memset( data_length_string, 0x0, sizeof( data_length_string ) );
        sprintf( data_length_string, "%lx", (long unsigned int)length );
        CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, data_length_string, strlen( data_length_string ) ) );
        CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, CRLF, sizeof( CRLF ) - 1 ) );
    }

    CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, data, length ) );

    if ( stream->chunked_transfer_enabled == true )
    {
        CY_VERIFY( cy_tcp_stream_write( &stream->tcp_stream, CRLF, sizeof( CRLF ) - 1 ) );
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_http_response_stream_write_resource( cy_http_response_stream_t* stream, const void* resource )
{
#ifdef WICED
    cy_rslt_t result;
    const void*    data;
    uint32_t       res_size;
    uint32_t       pos = 0;
    uint32_t       max_size = resource->size;

    /* If resource size is big and memory is not available to allocate that much size, resource_get_readonly_buffer will return with RESOURCE_OUT_OF_HEAP_SPACE. Try reducing the maximum size
     * to half and try reading again till we get the data.
     */
    do
    {
        resource_result_t resource_result = resource_get_readonly_buffer ( resource, pos, max_size, &res_size, &data );
        if ( resource_result != RESOURCE_SUCCESS )
        {
            /* If resource_result is RESOURCE_OUT_OF_HEAP_SPACE, reduce the max_size to half and try to read from resource again */
            if ( resource_result == RESOURCE_OUT_OF_HEAP_SPACE )
            {
                result = resource_result;
                max_size = max_size/2;
            }
            else
            {
                return resource_result;
            }
        }
        else
        {
            result = cy_http_response_stream_write( stream, data, res_size );
            resource_free_readonly_buffer( resource, data );
            if ( result != CY_RSLT_SUCCESS )
            {
                return result;
            }
            pos += res_size;
            max_size = resource->size - res_size;
        }
    } while ( max_size > 0 );

    return result;
#endif

    return CY_RSLT_HTTP_SERVER_ERROR_UNSUPPORTED;
}

cy_rslt_t cy_http_response_stream_flush( cy_http_response_stream_t* stream )
{
    /* This function will be needed for packet driven data approach ( which are not used currently ) */
    if ( stream == NULL )
    {
        return CY_RSLT_HTTP_SERVER_ERROR_BADARG;
    }

    return cy_tcp_stream_flush( &stream->tcp_stream );
}

cy_rslt_t cy_http_response_stream_disconnect( cy_http_response_stream_t* stream )
{
    server_event_message_t current_event;
    cy_rslt_t  result;

    if ((stream == NULL) || (stream->tcp_stream.socket == NULL ))
    {
        return CY_RSLT_HTTP_SERVER_ERROR_BADARG;
    }

    HTTP_DEBUG((" L%d : %s() : ----- ### DBG : Stream = [%p]\r\n", __LINE__, __FUNCTION__, (void *)stream));

    current_event.event_type = CY_SOCKET_DISCONNECT_EVENT;
    current_event.socket     = stream->tcp_stream.socket;

    result = cy_rtos_put_queue(&event_queue, &current_event, 0, 0);
    if( result != CY_RSLT_SUCCESS)
    {
        HTTP_ERROR((" failure in pushing to queue : %d \n", (int) result));
        return result;
    }

    return result;
}

cy_rslt_t cy_http_disconnect_all_response_stream( cy_http_server_t* server )
{
    cy_stream_node_t* stream;

    cy_linked_list_get_front_node( &server->active_stream_list, (cy_linked_list_node_t**)&stream );
    while ( stream != NULL )
    {
        cy_http_response_stream_disconnect(&stream->stream.response);
        stream = (cy_stream_node_t*)stream->node.next;
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_http_response_stream_init( cy_http_response_stream_t* stream, void* socket )
{

    if ( stream == NULL )
    {
        return CY_RSLT_HTTP_SERVER_ERROR_BADARG;
    }

    memset( stream, 0, sizeof( cy_http_response_stream_t ) );

    stream->chunked_transfer_enabled = false;

    return cy_tcp_stream_init( &stream->tcp_stream, socket );

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_http_response_stream_deinit( cy_http_response_stream_t* stream )
{
    cy_rslt_t  result;

    if ((stream == NULL) || (stream->tcp_stream.socket == NULL))
    {
        return CY_RSLT_HTTP_SERVER_ERROR_BADARG;
    }

    HTTP_DEBUG((" %s() : ----- ### DBG : Stream = [0x%X]\r\n", __FUNCTION__, (unsigned int)stream));
    result = cy_tcp_stream_deinit( &stream->tcp_stream );
    return result;
}

cy_rslt_t cy_http_get_query_parameter_value( const char* url_query, const char* parameter_key, char** parameter_value, uint32_t* value_length )
{
    char* iterator = (char*)url_query;

    while ( *iterator != '\0' )
    {
        char*    current_key = iterator;
        uint32_t current_key_length;

        while( ( *iterator != '\0' ) && ( *iterator != '=' ) && ( *iterator != '&' ) )
        {
            iterator++;
        }

        current_key_length = (uint32_t)( iterator - current_key );

        if ( match_string_with_wildcard_pattern( current_key, current_key_length, parameter_key ) != 0 )
        {
            if ( *iterator == '=' )
            {
                *parameter_value = iterator + 1;
                while( *iterator != '\0' && *iterator != '&' )
                {
                    iterator++;
                }
                *value_length = (uint32_t)( iterator - *parameter_value );
            }
            else
            {
                *parameter_value = NULL;
                *value_length    = 0;
            }
            return CY_RSLT_SUCCESS;
        }
        else
        {
            iterator++;
        }
    }

    *parameter_value = NULL;
    *value_length    = 0;

    return CY_RSLT_HTTP_SERVER_ERROR_NOT_FOUND;
}

uint32_t cy_http_get_query_parameter_count( const char* url_query )
{
    char*    current_query = (char*) url_query;
    uint32_t count;

    if ( current_query == NULL )
    {
        return 0;
    }

    /* Non-NULL URL query is considered 1 parameter */
    count = 1;

    while ( *current_query != '\0' )
    {
        /* Count up everytime '&' is found */
        if ( *current_query == '&' )
        {
            count++;
        }

        current_query++;
    }

    return count;
}

cy_rslt_t cy_http_match_query_parameter( const char* url_query, const char* parameter_key, const char* parameter_value )
{
    cy_rslt_t      result;
    char*          value_found = NULL;
    uint32_t       value_length = 0;

    result = cy_http_get_query_parameter_value( url_query, parameter_key, &value_found, &value_length );
    if ( result == CY_RSLT_SUCCESS )
    {
        if ( strncmp( parameter_value, value_found, value_length ) != 0 )
        {
            result = CY_RSLT_ERROR;
        }
    }

    return result;

}

static void http_server_connect_callback( void* socket )
{
    server_event_message_t message;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    message.event_type = CY_SERVER_CONNECT_EVENT;
    message.socket = socket;

    result = cy_rtos_put_queue(&connect_event_queue, &message, 0, 0);
    if( result != CY_RSLT_SUCCESS)
    {
        HTTP_ERROR((" failure in pushing to queue : %d \n", (int) result));
        return;
    }

    return;
}

void http_server_receive_callback( void* socket )
{
    server_event_message_t message;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    message.event_type = CY_SOCKET_PACKET_RECEIVED_EVENT;
    message.socket = socket;

    result = cy_rtos_put_queue(&event_queue, &message, 0, 0);
    if( result != CY_RSLT_SUCCESS)
    {
        HTTP_ERROR((" failure in pushing to queue : %d \n", (int) result));
        return;
    }

    return;
}

static cy_rslt_t http_server_parse_receive_packet( cy_http_server_t* server, cy_http_stream_t* stream, char* data, uint32_t length )
{
    cy_rslt_t      result                        = CY_RSLT_SUCCESS;
    bool           disconnect_current_connection = false;
    char*          start_of_url                  = NULL; /* Suppress compiler warning */
    uint16_t       url_length                    = 0;    /* Suppress compiler warning */
    char*          request_string                = NULL;
    uint16_t       request_length;
    uint16_t       new_url_length;
    char*          message_data_length_string;
    char*          mime;
    char*          cached_string_to_be_freed     = NULL;

    cy_http_message_body_t http_message_body =
    {
        .data                         = NULL,
        .data_length                  = 0,
        .data_remaining               = 0,
        .is_chunked_transfer          = false,
        .mime_type                    = MIME_UNSUPPORTED,
        .request_type                 = CY_HTTP_REQUEST_UNDEFINED
    };

    request_string = data;
    request_length = length;

    /* If application registers a receive callback, call the callback before further processing */
    if ( server->receive_callback != NULL )
    {
        HTTP_DEBUG((" ####### HTTP Request Received : Payload-Len = [%d], Payload = [%.*s]\r\n", request_length, request_length, request_string));
#ifdef ENABLE_DEBUG
        {
            int i;
            for (i = 0; i < request_length; i++)
            {
                HTTP_DEBUG(("%02X ", request_string[i]));
            }
            HTTP_DEBUG(("]\r\n"));
        }
#endif

        result = server->receive_callback( &stream->response, (uint8_t**)&request_string, &request_length );
        if ( result != CY_RSLT_SUCCESS )
        {
            if ( result != CY_RSLT_HTTP_SERVER_ERROR_PARTIAL_RESULTS )
            {
                disconnect_current_connection = true;
            }
            goto exit;
        }
    }

    /* Check if this is a close request */
    if ( (char*) strnstrn( request_string, request_length, HTTP_HEADER_CLOSE, sizeof( HTTP_HEADER_CLOSE ) - 1 ) != NULL )
    {
        disconnect_current_connection = true;
    }

    /* Code allows to support if content length > MTU then send data to callback registered for particular page_found. */
    if(stream->request.page_found != NULL)
    {
        /* currently we only handle content length > MTU for RAW_DYNAMIC_URL_CONTENT and cy_DYNAMIC_CONTENT */
        if(stream->request.page_found->url_content_type == CY_RAW_DYNAMIC_URL_CONTENT || stream->request.page_found->url_content_type == CY_DYNAMIC_URL_CONTENT)
        {
            if(stream->request.data_remaining > 0)
            {
                stream->request.data_remaining = (uint16_t)(stream->request.data_remaining -  request_length);

                http_message_body.data = (uint8_t*)request_string;
                http_message_body.data_length = request_length;
                http_message_body.data_remaining = stream->request.data_remaining;
                http_message_body.mime_type = stream->request.mime_type;
                http_message_body.request_type = stream->request.request_type;

                HTTP_DEBUG((" %s() : ----- ### DBG : Invoking the URL generator to process partial data\r\n", __FUNCTION__));
                stream->request.page_found->url_content.dynamic_data.generator(stream->request.page_found->url, NULL, &stream->response, stream->request.page_found->url_content.dynamic_data.arg, &http_message_body);

                /* We got all fragmented packets of http request [as total_data_remaining is 0 ] now send the response */
                if(stream->request.data_remaining == 0)
                {
                   /* Disable chunked transfer as it was enabled previously in library itself and then flush the data */
                   if( stream->request.page_found->url_content_type == CY_DYNAMIC_URL_CONTENT )
                   {
                       cy_http_response_stream_disable_chunked_transfer( &stream->response );
                   }

                   CY_VERIFY( cy_http_response_stream_flush( &stream->response ) );
                }

                return CY_RSLT_SUCCESS;
            }
        }
    }

    if ((cached_string != NULL) || ((char*) strnstrn(request_string, request_length, CRLF_CRLF, sizeof(CRLF_CRLF) - 1) == NULL))
    {
      char* new_cached_string;
      size_t new_cached_length = cached_length + request_length;

      HTTP_DEBUG((" %s() : ----- ### DBG : Caching the request\r\n", __FUNCTION__));

      if (new_cached_length > MAXIMUM_CACHED_LENGTH) {
          HTTP_DEBUG((" %s() : ----- ### DBG : Request exceeds %d bytes\r\n", __FUNCTION__, MAXIMUM_CACHED_LENGTH));
          free(cached_string);
          cached_string = NULL;
          cached_length = 0;
          goto exit;
      }

      new_cached_string = realloc(cached_string, new_cached_length);

      if (new_cached_string == NULL)
      {
          HTTP_DEBUG((" %s() : ----- ### DBG : Not enough memory\r\n", __FUNCTION__));
          free(cached_string);
          cached_string = NULL;
          cached_length = 0;
          goto exit;
      }

      memcpy(new_cached_string + cached_length, request_string, request_length);

      cached_string = new_cached_string;
      cached_length = new_cached_length;

      if ( (char*) strnstrn(cached_string, (uint16_t)cached_length, CRLF_CRLF, sizeof(CRLF_CRLF) - 1) == NULL)
      {
          HTTP_DEBUG((" %s() : ----- ### DBG : Not found the end of request\r\n", __FUNCTION__));
          goto exit;
      }
      else
      {
          request_string = cached_string;
          request_length = (uint16_t)cached_length;

          cached_string_to_be_freed = cached_string;

          cached_string = NULL;
          cached_length = 0;
      }
    }

    /* Verify we have enough data to start processing */
    if ( request_length < MINIMUM_REQUEST_LINE_LENGTH )
    {
        result = CY_RSLT_ERROR;
        goto exit;
    }

    /* First extract the URL from the packet */
    HTTP_DEBUG((" %s() : ----- ### DBG : Extract the request type\r\n", __FUNCTION__));
    result = http_server_get_request_type_and_url( request_string, request_length, &http_message_body.request_type, &start_of_url, &url_length );
    if ( result == CY_RSLT_ERROR )
    {
        goto exit;
    }

    HTTP_DEBUG((" %s() : ----- ### DBG : request type : %d \r\n", __FUNCTION__, http_message_body.request_type));

    /* Remove escape strings from URL */
    new_url_length = http_server_remove_escaped_characters( start_of_url, url_length, start_of_url, url_length );

    /* Now extract packet payload info such as data, data length, data type and message length */
    HTTP_DEBUG((" %s() : ----- ### DBG : Extract payload\r\n", __FUNCTION__));
    http_message_body.data = (uint8_t*) strnstrn( request_string, request_length, CRLF_CRLF, sizeof( CRLF_CRLF ) - 1 );

    HTTP_DEBUG((" %s() : ----- ### DBG : payload\r\n", __FUNCTION__));
    int i;
    for (i = 0; i < request_length; i++)
    {
       HTTP_DEBUG(("%02X ", http_message_body.data[i]));
    }
    HTTP_DEBUG(("]\r\n"));

    /* This indicates start of data/end of header was not found, so exit */
    if ( http_message_body.data == NULL )
    {
        result = CY_RSLT_ERROR;
        goto exit;
    }
    else
    {
        /* Payload starts just after the header */
        http_message_body.data += strlen( CRLF_CRLF );

        /* if there is no payload after header just set data pointer to NULL */
        if ( ( (uint8_t*) ( request_string + request_length ) - http_message_body.data ) == 0 )
        {
            http_message_body.data = NULL;
            http_message_body.data_length = 0;
        }
    }

    HTTP_DEBUG((" %s() : ----- ### DBG : Extract content type\r\n", __FUNCTION__));
    mime = (char*) strnstrn( request_string, request_length, HTTP_HEADER_CONTENT_TYPE, sizeof( HTTP_HEADER_CONTENT_TYPE ) - 1 );
    if ( ( mime != NULL ) && ( mime < (char*) http_message_body.data ) )
    {
        mime += strlen( HTTP_HEADER_CONTENT_TYPE );
        http_message_body.mime_type = http_server_get_mime_type( mime );
    }
    else
    {
        http_message_body.mime_type = MIME_TYPE_ALL;
    }

    HTTP_DEBUG((" %s() : ----- ### DBG : content type : %d \r\n", __FUNCTION__, http_message_body.mime_type));

    if ( strnstrn( request_string, request_length, HTTP_HEADER_CHUNKED, sizeof( HTTP_HEADER_CHUNKED ) - 1 ) )
    {
        /* Indicate the format of this frame is chunked. Its up to the application to parse and reassemble the chunk */
        http_message_body.is_chunked_transfer = true;
        if ( http_message_body.data != NULL )
        {
            http_message_body.data_length = (uint16_t) ( (uint8_t*) ( request_string + request_length ) - http_message_body.data );
        }
    }
    else
    {
        message_data_length_string = (char*) strnstrn( request_string, request_length, HTTP_HEADER_CONTENT_LENGTH, sizeof( HTTP_HEADER_CONTENT_LENGTH ) - 1 );

        /* This case handles case where content-length : X but there is no data present in payload, in this case atleast application should be informed about correct
         * remaining data length so application can take appropriate action
         */
        if ( ( message_data_length_string != NULL ) && ( http_message_body.data == NULL ) )
        {
            message_data_length_string += ( sizeof( HTTP_HEADER_CONTENT_LENGTH ) - 1 );

            http_message_body.data_remaining = (uint16_t) ( strtol( message_data_length_string, NULL, 10 ) - http_message_body.data_length );

            stream->request.data_remaining = http_message_body.data_remaining;
        }
        else if ( ( message_data_length_string != NULL ) && ( message_data_length_string < (char*) http_message_body.data ) )
        {
            http_message_body.data_length = (uint16_t) ( (uint8_t*) ( request_string + request_length ) - http_message_body.data );

            message_data_length_string += ( sizeof( HTTP_HEADER_CONTENT_LENGTH ) - 1 );

            http_message_body.data_remaining = (uint16_t) ( strtol( message_data_length_string, NULL, 10 ) - http_message_body.data_length );

            stream->request.data_remaining = http_message_body.data_remaining;
        }
        else
        {
            http_message_body.data_length = 0;
            stream->request.data_remaining = 0;
        }
        stream->request.mime_type = http_message_body.mime_type;
        stream->request.request_type = http_message_body.request_type;
    }

    HTTP_DEBUG((" %s() : ----- ### DBG : Process the URL request\r\n", __FUNCTION__));
    result = http_server_process_url_request( stream, server->page_database, start_of_url, new_url_length, &http_message_body );

exit:
    free(cached_string_to_be_freed);

    if ( disconnect_current_connection == true )
    {
        cy_http_response_stream_disconnect( &stream->response );
    }

    return result;
}

cy_rslt_t http_server_process_url_request( cy_http_stream_t* stream, const cy_http_page_t* page_database, char* url, uint32_t url_length, cy_http_message_body_t* http_message_body )
{
    char*                    url_query_parameters = url;
    uint32_t                 query_length         = url_length;
    cy_http_page_t*          page_found           = NULL;
    cy_packet_mime_type_t    mime_type            = MIME_TYPE_ALL;
    cy_http_status_codes_t   status_code;
    cy_rslt_t                result = CY_RSLT_SUCCESS;

    url[ url_length ] = '\x00';

    while ( ( *url_query_parameters != '?' ) && ( query_length > 0 ) && ( *url_query_parameters != '\0' ) )
    {
        url_query_parameters++;
        query_length--;
    }

    if ( query_length != 0 )
    {
        url_length = url_length - query_length;
        *url_query_parameters = '\x00';
        url_query_parameters++;
    }
    else
    {
        url_query_parameters = NULL;
    }

    HTTP_DEBUG( ("Processing request for: %s, %s\r\n", url, page_database[0].url) );

    /* Find URL in server page database */
    if ( http_server_find_url_in_page_database( url, url_length, http_message_body, page_database, &page_found, &mime_type ) == CY_RSLT_SUCCESS )
    {
        stream->request.page_found = page_found;
        status_code = CY_HTTP_200_TYPE; /* OK */
    }
    else
    {
        stream->request.page_found = NULL;
        status_code = CY_HTTP_404_TYPE; /* Not Found */
    }

    if ( status_code == CY_HTTP_200_TYPE )
    {
        HTTP_DEBUG((" %s() : ----- ### DBG : URL content type = [%d]\r\n", __FUNCTION__, page_found->url_content_type));
        /* Call the content handler function to write the page content into the packet and adjust the write pointers */
        switch ( page_found->url_content_type )
        {
            case CY_DYNAMIC_URL_CONTENT:
                HTTP_DEBUG((" %s() : ----- ### DBG : CY_DYNAMIC_URL_CONTENT\r\n", __FUNCTION__));
                cy_http_response_stream_enable_chunked_transfer( &stream->response );
                cy_http_response_stream_write_header( &stream->response, status_code, CHUNKED_CONTENT_LENGTH, CY_HTTP_CACHE_DISABLED, mime_type );
                result = page_found->url_content.dynamic_data.generator( url, url_query_parameters, &stream->response, page_found->url_content.dynamic_data.arg, http_message_body );
                /* if content length is < MTU then just disable chunked transfer and flush the data */
                if(stream->request.data_remaining == 0)
                {
                    cy_http_response_stream_disable_chunked_transfer( &stream->response );
                    CY_VERIFY( cy_http_response_stream_flush( &stream->response ) );
                }
                break;

            case CY_RAW_DYNAMIC_URL_CONTENT:
                HTTP_DEBUG((" %s() : ----- ### DBG : CY_RAW_DYNAMIC_URL_CONTENT\r\n", __FUNCTION__));
                result = page_found->url_content.dynamic_data.generator( url, url_query_parameters, &stream->response, page_found->url_content.dynamic_data.arg, http_message_body );
                /* if content length is < MTU then just flush the response */
                if(stream->request.data_remaining == 0)
                {
                    CY_VERIFY( cy_http_response_stream_flush( &stream->response ) );
                }
                break;

            case CY_STATIC_URL_CONTENT:
                HTTP_DEBUG((" %s() : ----- ### DBG : CY_STATIC_URL_CONTENT\r\n", __FUNCTION__));
                cy_http_response_stream_write_header( &stream->response, status_code, page_found->url_content.static_data.length, CY_HTTP_CACHE_ENABLED, mime_type );
                CY_VERIFY( cy_http_response_stream_write( &stream->response, page_found->url_content.static_data.ptr, page_found->url_content.static_data.length ) );
                CY_VERIFY( cy_http_response_stream_flush( &stream->response ) );
                break;

            case CY_RAW_STATIC_URL_CONTENT: /* This is just a Location header */
                HTTP_DEBUG((" %s() : ----- ### DBG : CY_RAW_STATIC_URL_CONTENT\n", __FUNCTION__));
                CY_VERIFY( cy_http_response_stream_write( &stream->response, HTTP_HEADER_301, strlen( HTTP_HEADER_301 ) ) );
                CY_VERIFY( cy_http_response_stream_write( &stream->response, CRLF, strlen( CRLF ) ) );
                CY_VERIFY( cy_http_response_stream_write( &stream->response, HTTP_HEADER_LOCATION, strlen( HTTP_HEADER_LOCATION ) ) );
                CY_VERIFY( cy_http_response_stream_write( &stream->response, page_found->url_content.static_data.ptr, page_found->url_content.static_data.length ) );
                CY_VERIFY( cy_http_response_stream_write( &stream->response, CRLF, strlen( CRLF ) ) );
                CY_VERIFY( cy_http_response_stream_write( &stream->response, HTTP_HEADER_CONTENT_LENGTH, strlen( HTTP_HEADER_CONTENT_LENGTH ) ) );
                CY_VERIFY( cy_http_response_stream_write( &stream->response, "0", 1 ) );
                CY_VERIFY( cy_http_response_stream_write( &stream->response, CRLF_CRLF, strlen( CRLF_CRLF ) ) );
                CY_VERIFY( cy_http_response_stream_flush( &stream->response ) );
                break;

            case CY_RESOURCE_URL_CONTENT:
                HTTP_DEBUG((" %s() : ----- ### DBG : CY_RESOURCE_URL_CONTENT\r\n", __FUNCTION__));
                /* Fall through */
            case CY_RAW_RESOURCE_URL_CONTENT:
                HTTP_DEBUG((" %s() : ----- ### DBG : CY_RAW_RESOURCE_URL_CONTENT\r\n", __FUNCTION__));
                cy_http_response_stream_enable_chunked_transfer( &stream->response );
                cy_http_response_stream_write_header( &stream->response, status_code, CHUNKED_CONTENT_LENGTH, CY_HTTP_CACHE_DISABLED, mime_type );
                cy_http_response_stream_write_resource( &stream->response, page_found->url_content.resource_data );
                cy_http_response_stream_disable_chunked_transfer( &stream->response );
                CY_VERIFY( cy_http_response_stream_flush( &stream->response ) );
                break;

            default:
                HTTP_DEBUG((" %s() : ----- ### DBG : default\r\n", __FUNCTION__));
                break;
        }
    }
    else if ( status_code >= CY_HTTP_400_TYPE )
    {
        cy_http_response_stream_write_header( &stream->response, status_code, NO_CONTENT_LENGTH, CY_HTTP_CACHE_DISABLED, MIME_TYPE_TEXT_HTML );
        CY_VERIFY( cy_http_response_stream_flush( &stream->response ) );
    }

    return result;
}

uint16_t http_server_remove_escaped_characters( char* output, uint16_t output_length, const char* input, uint16_t input_length )
{
    uint16_t bytes_copied;
    int a;

    for ( bytes_copied = 0; ( input_length > 0 ) && ( bytes_copied != output_length ); ++bytes_copied )
    {
        if ( *input == '%' )
        {
            --input_length;
            /* If there is only % remain in encoded URL string, then just return as it is not valid */
            if ( input_length == 0 )
            {
                return 0;
            }

            ++input;

            /* Valid encoding should result as % followed by two hexadecimal digits. If encoded URL comes with %% then just return as it is invalid */
            if ( *input == '%' )
            {
                return 0;
            }
            else
            {
                *output = 0;

                /* As per RFC 3986 : percent encoding octet is encoded as character triplet, consisting of "%" followed by two hexadecimal digits. After percentage if we are not able to
                 * find two hexadecimal digits then return as it is invalid encoded URL */
                if (input_length < 2)
                {
                    return 0;
                }

                for ( a = 0; (a < 2); ++a )
                {
                    *output = (char) ( *output << 4 );
                    if ( *input >= '0' && *input <= '9' )
                    {
                        *output = (char) ( *output + *input - '0' );
                    }
                    else if ( *input >= 'a' && *input <= 'f' )
                    {
                        *output = (char) ( *output + *input - 'a' + 10 );
                    }
                    else if ( *input >= 'A' && *input <= 'F' )
                    {
                        *output = (char) ( *output + *input - 'A' + 10 );
                    }
                    else
                    {
                        return 0;
                    }
                    --input_length;
                    if ( input_length > 0 )
                    {
                        ++input;
                    }
                }
                ++output;
            }
        }
        else
        {
            /* If there is + present in encoded URL then replace with space */
            if( *input == '+' )
            {
                *output = ' ';
            }
            else
            {
                *output = *input;
            }

            --input_length;
            if ( input_length > 0 )
            {
                ++input;
                ++output;
            }
        }
    }

    return bytes_copied;
}

cy_packet_mime_type_t http_server_get_mime_type( const char* request_data )
{
    cy_packet_mime_type_t mime_type = MIME_TYPE_TLV;

    if ( request_data != NULL )
    {
        while ( mime_type < MIME_TYPE_ALL )
        {
            if ( strncmp( request_data, http_mime_array[ mime_type ], strlen( http_mime_array[ mime_type ] ) ) == COMPARE_MATCH )
            {
                break;
            }
            mime_type++;
        }
    }
    else
    {
        /* If MIME not specified, assumed all supported (according to rfc2616)*/
        mime_type = MIME_TYPE_ALL;
    }
    return mime_type;
}

cy_rslt_t http_server_get_request_type_and_url( char* request, uint16_t request_length, cy_http_request_type_t* type, char** url_start, uint16_t* url_length )
{
    char* end_of_url;

    end_of_url = (char*) strnstrn( request, request_length, HTTP_1_1_TOKEN, sizeof( HTTP_1_1_TOKEN ) - 1 );
    if ( end_of_url == NULL )
    {
        return CY_RSLT_ERROR;
    }

    if ( memcmp( request, GET_TOKEN, sizeof( GET_TOKEN ) - 1 ) == COMPARE_MATCH )
    {
        /* Get type  */
        *type = CY_HTTP_GET_REQUEST;
        *url_start = request + sizeof( GET_TOKEN ) - 1;
        *url_length = (uint16_t) ( end_of_url - *url_start );
    }
    else if ( memcmp( request, POST_TOKEN, sizeof( POST_TOKEN ) - 1 ) == COMPARE_MATCH )
    {
        *type = CY_HTTP_POST_REQUEST;
        *url_start = request + sizeof( POST_TOKEN ) - 1;
        *url_length = (uint16_t) ( end_of_url - *url_start );
    }
    else if ( memcmp( request, PUT_TOKEN, sizeof( PUT_TOKEN ) - 1 ) == COMPARE_MATCH )
    {
        *type = CY_HTTP_PUT_REQUEST;
        *url_start = request + sizeof( PUT_TOKEN ) - 1;
        *url_length = (uint16_t) ( end_of_url - *url_start );
    }
    else
    {
        *type = CY_HTTP_REQUEST_UNDEFINED;
        return CY_RSLT_ERROR;
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t http_server_find_url_in_page_database( char* url, uint32_t length, cy_http_message_body_t* http_request, const cy_http_page_t* page_database, cy_http_page_t** page_found, cy_packet_mime_type_t* mime_type )
{
    uint32_t i = 0;

    /* Search URL list to determine if request matches one of our pages, and break out when found */
    if( page_database == NULL )
    {
        return CY_RSLT_ERROR;
    }

    while ( page_database[ i ].url != NULL )
    {
        if ( match_string_with_wildcard_pattern( url, length, page_database[ i ].url ) != 0 )
        {
            *mime_type = http_server_get_mime_type( page_database[ i ].mime_type );

            if ( ( *mime_type == http_request->mime_type ) || ( http_request->mime_type == MIME_TYPE_ALL ) )
            {
                *page_found = (cy_http_page_t*)&page_database[i];
                return CY_RSLT_SUCCESS;
            }
        }
        i++;
    }

    return CY_RSLT_HTTP_SERVER_ERROR_NOT_FOUND;
}

void http_server_connect_thread_main( cy_thread_arg_t arg )
{
    cy_tcp_socket_t* client_socket;
    cy_rslt_t res = CY_RSLT_SUCCESS;
    cy_http_server_t* http_server = (cy_http_server_t*) arg;
    cy_tcp_server_t* server = (cy_tcp_server_t*) &http_server->tcp_server;
    server_event_message_t current_event;
    osStatus_t status;
    cy_stream_node_t*     stream = NULL;

    while( http_server->quit != true )
    {
        status = cy_rtos_get_queue(&connect_event_queue, &current_event, osWaitForever, 0);

        if( status != osOK)
        {
            HTTP_ERROR((" Failed in pop from the queue : %d \n", status));
            return;
        }

        if( current_event.event_type == CY_SERVER_STOP_EVENT)
        {
            cy_rtos_exit_thread();
        }

        HTTP_DEBUG((" L%d : %s() : ----- Current event = [%d]\r\n", __LINE__, __FUNCTION__, current_event.event_type));

        client_socket = NULL;
        res = cy_tcp_server_accept(server, &client_socket);
        if ( res != CY_RSLT_SUCCESS )
        {
            HTTP_ERROR((" TCP server accept failed \n"));
        }

        if( res == CY_RSLT_SUCCESS )
        {

            HTTP_DEBUG((" L%d : %s() : ----- New client connection accepted : %0x \r\n", __LINE__, __FUNCTION__, client_socket));

            /* Register receive callback for client socket */
            cy_register_socket_callback(client_socket, http_server_receive_callback);

            /* active stream list can be modified in connect thread and event thread */
            cy_rtos_get_mutex(&server->mutex, osWaitForever);

            /* Create a new stream for client socket, remove node from inactive_stream_list and add it to active_stream_list */
            cy_linked_list_remove_node_from_front( &http_server->inactive_stream_list, (cy_linked_list_node_t**)&stream );

            HTTP_DEBUG((" L%d : %s() : ----- ### DBG : Stream = [0x%X], Socket = [0x%X]\r\n", __LINE__, __FUNCTION__, (unsigned int)stream, (unsigned int)client_socket));

            cy_linked_list_insert_node_at_rear( &http_server->active_stream_list, &stream->node );
            cy_http_response_stream_init( &stream->stream.response, client_socket );
            memset( &stream->stream.request, 0, sizeof( stream->stream.request ) );

            /* NOTE: There is a need to push one receive event as data packets are received in LWIP socket callback ( lwipstack.cpp - Socket_callback
             * function ) layer before application calls cy_tcp_server_accept (in TCP socket accept call lwip_arena will be updated with the client
             * socket) hence lwip_arena is not updated and LWIP callback doesn't call the data callback in application. Need more investigation on
             * LWIP MBEDOS wrapper to fix the issue. But for now, this fix is adequate for GET and POST requests.
             */
            http_server_receive_callback( client_socket );

            cy_rtos_set_mutex(&server->mutex);

        }
    }
}

void http_server_event_thread_main( cy_thread_arg_t arg )
{
    cy_http_server_t* http_server = (cy_http_server_t*) arg;
    cy_stream_node_t*     stream = NULL;
    server_event_message_t current_event;
    char buffer[MTU_SIZE];
    osStatus_t        status;
    int received_length = 0;
    cy_tcp_socket_t* client_socket;

    while( http_server->quit != true )
    {
        status = cy_rtos_get_queue (&event_queue, &current_event, osWaitForever, 0);

        if( status != osOK)
        {
            HTTP_ERROR((" Failed in pop from the queue : %d \n", status));
            return;
        }

        HTTP_DEBUG((" L%d : %s() : ----- Current event = [%d]\r\n", __LINE__, __FUNCTION__, current_event.event_type));
        switch ( current_event.event_type )
        {
            case CY_SOCKET_DISCONNECT_EVENT:
            {
                cy_stream_node_t* stream;
                cy_tcp_socket_t*  tcp_socket = NULL;

                tcp_socket = current_event.socket;
                if ( tcp_socket != NULL )
                {

                    /* Search in active stream whether stream for this socket is available. If available, removed it */
                    HTTP_DEBUG ((" ### DBG : Search Stream to be removed\r\n"));

                    if ( cy_linked_list_find_node( &http_server->active_stream_list, http_server_compare_stream_socket, (void*)tcp_socket->socket, (cy_linked_list_node_t**)&stream ) == CY_RSLT_SUCCESS )
                    {
                        HTTP_DEBUG((" %s() : ----- ### DBG : Found Stream to be removed = [0x%X], Socket = [0x%X]\r\n", __FUNCTION__, (unsigned int)stream, (unsigned int)current_event.socket));
                        HTTP_DEBUG((" L%d : %s() : ----- Disconnect event for Stream = [0x%X]\r\n", __LINE__, __FUNCTION__, (unsigned int)&stream->stream.response));
                        /* If application registers a disconnection callback, call the callback before further processing */
                        if( http_server->disconnect_callback != NULL )
                        {
                            http_server->disconnect_callback( &stream->stream.response );
                        }

                        cy_rtos_get_mutex(&http_server->mutex, osWaitForever);

                        cy_linked_list_remove_node( &http_server->active_stream_list, &stream->node );
                        cy_linked_list_insert_node_at_rear( &http_server->inactive_stream_list, &stream->node );

                        cy_rtos_set_mutex(&http_server->mutex);

                        cy_http_response_stream_deinit( &stream->stream.response );

                        cy_tcp_server_disconnect_socket( &http_server->tcp_server, tcp_socket );

                        HTTP_DEBUG((" L%d : %s() : Disconnect event processed\r\n", __LINE__, __FUNCTION__));
                     }
                }

                break;
            }
            case CY_SERVER_STOP_EVENT:
            {
                cy_stream_node_t* stream;

                http_server->quit = true;

                cy_rtos_get_mutex(&http_server->mutex, osWaitForever);

                /* Deinit all response stream */
                cy_linked_list_get_front_node( &http_server->active_stream_list, (cy_linked_list_node_t**)&stream );
                while ( stream != NULL )
                {
                    cy_linked_list_remove_node( &http_server->active_stream_list, &stream->node );
                    HTTP_DEBUG((" %s() : ----- De-Init Stream = [0x%X]\r\n", __FUNCTION__, (unsigned int)&stream->stream.response));
                    cy_http_response_stream_deinit( &stream->stream.response );
                    cy_linked_list_get_front_node( &http_server->active_stream_list, (cy_linked_list_node_t**)&stream );
                }

                cy_rtos_set_mutex(&http_server->mutex);

                HTTP_DEBUG((" ### DBG : Stop event processed\r\n"));
                break;
            }
            case CY_SOCKET_PACKET_RECEIVED_EVENT:
            {
                client_socket = current_event.socket;

                if ( cy_linked_list_find_node( &http_server->active_stream_list, http_server_compare_stream_socket, (void*)client_socket->socket, (cy_linked_list_node_t**)&stream ) != CY_RSLT_SUCCESS )
                {
                    /* Stream is not available, it means that client already disconnected to server but these are stale events */
                    break;
                }
                else
                {
                    do
                    {

                        received_length = cy_tcp_server_recv( client_socket, buffer, MTU_SIZE);

                        HTTP_DEBUG((" Received [%d] bytes \r\n", received_length));

                        /* Socket is non-blocking hence NSAPI_ERROR_WOULD_BLOCK will be ignored and wait for the next event */
                        if( received_length == NSAPI_ERROR_WOULD_BLOCK )
                        {
                            HTTP_DEBUG((" L%d : %s() : ----- No data is available to read \r\n", __LINE__, __FUNCTION__));
                            break;
                        }

                        /* If recv returns any error or 0, then consider it as a disconnection and disconnect the socket */
                        if( received_length <= 0 )
                        {
                            cy_stream_node_t* stream;

                            /* Search in active stream whether stream for this socket is available. If available, removed it */
                            HTTP_DEBUG((" ### DBG : Search Stream to be removed\r\n"));
                            if ( cy_linked_list_find_node( &http_server->active_stream_list, http_server_compare_stream_socket, (void*)client_socket->socket, (cy_linked_list_node_t**)&stream ) == CY_RSLT_SUCCESS )
                            {
                                HTTP_DEBUG((" %s() : ----- ### DBG : Found Stream to be removed = [0x%X], Socket = [0x%X]\r\n", __FUNCTION__, (unsigned int)stream, (unsigned int)current_event.socket));
                                HTTP_DEBUG((" L%d : %s() : ----- Disconnect event for Stream = [0x%X]\r\n", __LINE__, __FUNCTION__, (unsigned int)&stream->stream.response));
                                /* If application registers a disconnection callback, call the callback before further processing */
                                if( http_server->disconnect_callback != NULL )
                                {
                                    http_server->disconnect_callback( &stream->stream.response );
                                }

                                cy_rtos_get_mutex(&http_server->mutex, osWaitForever);

                                cy_linked_list_remove_node( &http_server->active_stream_list, &stream->node );
                                cy_linked_list_insert_node_at_rear( &http_server->inactive_stream_list, &stream->node );

                                cy_rtos_set_mutex(&http_server->mutex);

                                cy_http_response_stream_deinit( &stream->stream.response );
                            }
                            else
                            {
                                /* Check for NULL sockets in the active stream list and remove them */
                                uint32_t        active_stream_cnt;
                                cy_rslt_t  find_null_socket = CY_RSLT_ERROR;

                                HTTP_DEBUG((" L%d : %s() : ----- Check and remove invalid streams\r\n", __LINE__, __FUNCTION__));
                                for (active_stream_cnt = http_server->active_stream_list.count; active_stream_cnt>0; active_stream_cnt-- )
                                {
                                    find_null_socket = cy_linked_list_find_node(&http_server->active_stream_list, http_server_compare_stream_socket, (void*)NULL, (cy_linked_list_node_t**)&stream );
                                    /* if all active stream has valid socket, stop search */
                                    if ( find_null_socket != CY_RSLT_SUCCESS )
                                    {
                                        break;
                                    }

                                    /* If application registers a disconnection callback, call the callback before further processing */
                                    HTTP_DEBUG((" L%d : %s() : ----- Disconnect event for Stream = [0x%X]\r\n", __LINE__, __FUNCTION__, (unsigned int)&stream->stream.response));
                                    if( http_server->disconnect_callback != NULL )
                                    {
                                        http_server->disconnect_callback( &stream->stream.response );
                                    }

                                    cy_rtos_get_mutex(&http_server->mutex, osWaitForever);

                                    /* active stream list must have valid socket */
                                    /* re-claim from active stream if socket is null */
                                    HTTP_DEBUG(("found null socket in active list, active.cnt=%ld, inactive.cnt=%ld\r\n",
                                            http_server->active_stream_list.count,
                                            http_server->inactive_stream_list.count));
                                    cy_linked_list_remove_node( &http_server->active_stream_list, &stream->node );
                                    cy_linked_list_insert_node_at_rear( &http_server->inactive_stream_list, &stream->node );

                                    cy_rtos_set_mutex(&http_server->mutex);
                                }
                            }

                            cy_tcp_server_disconnect_socket( &http_server->tcp_server, client_socket );
                            HTTP_DEBUG((" L%d : %s() : Disconnect event processed\r\n", __LINE__, __FUNCTION__));
                        }
                        else
                        {
                            /* Process packet */
                            HTTP_DEBUG((" L%d : %s() : Parse the HTTP packet\r\n", __LINE__, __FUNCTION__));
                            http_server_parse_receive_packet( http_server, &stream->stream, buffer, received_length);
                        }

                    } while( received_length > 0 );
                }
                break;
            }
            default:
            {
                HTTP_DEBUG((" L%d : %s() : Unhandled event type [%d]\r\n", __LINE__, __FUNCTION__, current_event.event_type));
                break;
            }
        }
    }

    HTTP_DEBUG((" %s() : ----- ### Exited from event thread \r\n", __FUNCTION__));

    cy_rtos_exit_thread();
}

bool http_server_compare_stream_socket( cy_linked_list_node_t* node_to_compare, void* user_data )
{
    cy_tcp_socket_t*  socket = (cy_tcp_socket_t*) user_data;
    cy_stream_node_t* stream = (cy_stream_node_t*)node_to_compare;

    if ( stream->stream.response.tcp_stream.socket->socket == socket )
    {
        return true;
    }
    else
    {
        return false;
    }
}

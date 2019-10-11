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

/**
 * @file
 *
 * API for the HTTP / HTTPS Web server
 *
 * Web pages and other resources are provided via an array which gets passed as an argument when starting the web server.
 * The array is constructed using the START_OF_HTTP_PAGE_DATABASE() and END_OF_HTTP_PAGE_DATABASE() macros, and optionally the ROOT_HTTP_PAGE_REDIRECT() macro
 * Below is an example of a list of web pages (taken from one of the demo apps)
 *
 * START_OF_HTTP_PAGE_DATABASE(web_pages)
 *     ROOT_HTTP_PAGE_REDIRECT("/apps/temp_control/main.html"),
 *     { "/apps/temp_control/main.html",    "text/html",                CY_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_apps_DIR_temp_control_DIR_main_html, },
 *     { "/temp_report.html",               "text/html",                CY_DYNAMIC_URL_CONTENT,   .url_content.dynamic_data   = {process_temperature_update, 0 }, },
 *     { "/temp_up",                        "text/html",                CY_DYNAMIC_URL_CONTENT,   .url_content.dynamic_data   = {process_temperature_up, 0 }, },
 *     { "/temp_down",                      "text/html",                CY_DYNAMIC_URL_CONTENT,   .url_content.dynamic_data   = {process_temperature_down, 0 }, },
 *     { "/images/favicon.ico",             "image/vnd.microsoft.icon", CY_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_images_DIR_favicon_ico, },
 *     { "/scripts/general_ajax_script.js", "application/javascript",   CY_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_scripts_DIR_general_ajax_script_js, },
 *     { "/images/cypresslogo.png",         "image/png",                CY_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_images_DIR_cypresslogo_png, },
 *     { "/images/cypressogo_line.png",     "image/png",                CY_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_images_DIR_cypresslogo_line_png, },
 *     { "/styles/buttons.css",             "text/css",                 CY_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_styles_DIR_buttons_css, },
 * END_OF_HTTP_PAGE_DATABASE();
 */

#pragma once

#include <stdbool.h>
#include "stdio.h"
#include "cy_tcpip.h"
#include "cmsis_os2.h"
#include "rtx_os.h"
#include "mbed_toolchain.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

#define EXPAND_AS_ENUMERATION(a,b)   a,
#define EXPAND_AS_MIME_TABLE(a,b)    b,

/**
 * @addtogroup http_server_defines
 *
 * HTTP server preprocessor directives such as results and error codes
 *
 * Cypress middleware APIs return results of type cy_rslt_t and comprise of three parts:
 * - module base
 * - type
 * - error code
 *
 * \par Result Format
 *
   \verbatim
              Module base         Type    Library specific error code
      +---------------------------+------+------------------------------+
      |CY_RSLT_MODULE_HTTP_SERVER | 0x2  |           Error Code         |
      +---------------------------+------+------------------------------+
                14-bits            2-bits            16-bits

   Refer to the macro section of this document for library specific error codes.
   \endverbatim
 *
 * The data structure cy_rslt_t is part of cy_result.h in MBED OS PSoC6 target platform, located in <mbed-os/targets/TARGET_Cypress/TARGET_PSOC6/psoc6csp/core_lib/include>
 *
 * Module base: This base is derived from CY_RSLT_MODULE_MIDDLEWARE_BASE (defined in cy_result.h) and is an offset of the CY_RSLT_MODULE_MIDDLEWARE_BASE
 *              The details of the offset and the middleware base are defined in cy_result_mw.h, that is part of [Github connectivity-utilities] (https://github.com/cypresssemiconductorco/connectivity-utilities)
 *              For instance, HTTP server uses CY_RSLT_MODULE_HTTP_SERVER as the module base
 *
 * Type: This type is defined in cy_result.h and can be one of CY_RSLT_TYPE_FATAL, CY_RSLT_TYPE_ERROR, CY_RSLT_TYPE_WARNING or CY_RSLT_TYPE_INFO. AWS library error codes are of type CY_RSLT_TYPE_ERROR
 *
 * Library specific error code: These error codes are library specific and defined in macro section
 *
 * Helper macros used for creating the library specific result are provided as part of cy_result.h
 *
 *  @{
 */
/**
 * Results returned by HTTP server library
 */
#define CY_RSLT_MODULE_HTTP_SERVER_ERR_CODE_START       (0)

/** HTTP server error code base */        
#define CY_RSLT_HTTP_SERVER_ERR_BASE                    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_HTTP_SERVER, CY_RSLT_MODULE_HTTP_SERVER_ERR_CODE_START)

/** Out of memory */
#define CY_RSLT_HTTP_SERVER_ERROR_NO_MEMORY             ((cy_rslt_t)(CY_RSLT_HTTP_SERVER_ERR_BASE + 1))
/** Error initializing thread */
#define CY_RSLT_HTTP_SERVER_ERROR_THREAD_INIT           ((cy_rslt_t)(CY_RSLT_HTTP_SERVER_ERR_BASE + 2))
/** Error initializing queue */
#define CY_RSLT_HTTP_SERVER_ERROR_QUEUE_INIT            ((cy_rslt_t)(CY_RSLT_HTTP_SERVER_ERR_BASE + 3))
/** Error initializing mutex */
#define CY_RSLT_HTTP_SERVER_ERROR_MUTEX_INIT            ((cy_rslt_t)(CY_RSLT_HTTP_SERVER_ERR_BASE + 4))
/** Failed to start TCP server */
#define CY_RSLT_HTTP_SERVER_ERROR_TCP_SERVER_START      ((cy_rslt_t)(CY_RSLT_HTTP_SERVER_ERR_BASE + 5))
/** Feature not supported */        
#define CY_RSLT_HTTP_SERVER_ERROR_UNSUPPORTED           ((cy_rslt_t)(CY_RSLT_HTTP_SERVER_ERR_BASE + 6))
/** Bad argument/parameter */        
#define CY_RSLT_HTTP_SERVER_ERROR_BADARG                ((cy_rslt_t)(CY_RSLT_HTTP_SERVER_ERR_BASE + 7))
/** Resource not found */        
#define CY_RSLT_HTTP_SERVER_ERROR_NOT_FOUND             ((cy_rslt_t)(CY_RSLT_HTTP_SERVER_ERR_BASE + 8))
/** Partially processed. Returned by application's receive callback */        
#define CY_RSLT_HTTP_SERVER_ERROR_PARTIAL_RESULTS       ((cy_rslt_t)(CY_RSLT_HTTP_SERVER_ERR_BASE + 9))
/** Exceeded maximum number of resources */
#define CY_RSLT_HTTP_SERVER_PAGE_DATABASE_FULL          ((cy_rslt_t)(CY_RSLT_HTTP_SERVER_ERR_BASE + 10))
/** HTTP server generic error */        
#define CY_RSLT_ERROR                                   ((cy_rslt_t)(CY_RSLT_HTTP_SERVER_ERR_BASE + 11))
/**
 * @}
 */

/******************************************************
 *                    Constants
 ******************************************************/

#define HTTP_404 \
    "HTTP/1.1 404 Not Found\r\n" \
    "Content-Type: text/html\r\n\r\n" \
    "<!doctype html>\n" \
    "<html><head><title>404 - CY Web Server</title></head><body>\n" \
    "<h1>Address not found on CY Web Server</h1>\n" \
    "<p><a href=\"/\">Return to home page</a></p>\n" \
    "</body>\n</html>\n"

#define MIME_TABLE( ENTRY ) \
    ENTRY( MIME_TYPE_TLV = 0 ,                "application/x-tlv8"               ) \
    ENTRY( MIME_TYPE_APPLE_BINARY_PLIST,      "application/x-apple-binary-plist" ) \
    ENTRY( MIME_TYPE_APPLE_PROXY_AUTOCONFIG,  "application/x-ns-proxy-autoconfig") \
    ENTRY( MIME_TYPE_BINARY_DATA,             "application/octet-stream"         ) \
    ENTRY( MIME_TYPE_JAVASCRIPT,              "application/javascript"           ) \
    ENTRY( MIME_TYPE_JSON,                    "application/json"                 ) \
    ENTRY( MIME_TYPE_HAP_JSON,                "application/hap+json"             ) \
    ENTRY( MIME_TYPE_HAP_PAIRING,             "application/pairing+tlv8"         ) \
    ENTRY( MIME_TYPE_HAP_VERIFY,              "application/hap+verify"           ) \
    ENTRY( MIME_TYPE_TEXT_HTML,               "text/html"                        ) \
    ENTRY( MIME_TYPE_TEXT_PLAIN,              "text/plain"                       ) \
    ENTRY( MIME_TYPE_TEXT_EVENT_STREAM,       "text/event-stream"                ) \
    ENTRY( MIME_TYPE_TEXT_CSS,                "text/css"                         ) \
    ENTRY( MIME_TYPE_IMAGE_PNG,               "image/png"                        ) \
    ENTRY( MIME_TYPE_IMAGE_GIF,               "image/gif"                        ) \
    ENTRY( MIME_TYPE_IMAGE_MICROSOFT,         "image/vnd.microsoft.icon"         ) \
    ENTRY( MIME_TYPE_ALL,                     "*/*"                              ) /* This must always be the last mimne*/

/**
 * A string with the address which iOS searches during the captive-portal part of a Wi-Fi attachment.
 */
#define IOS_CAPTIVE_PORTAL_ADDRESS        "/library/test/success.html"

#define DEFAULT_URL_PROCESSOR_STACK_SIZE  5000

#define NO_CONTENT_LENGTH                 0
#define CHUNKED_CONTENT_LENGTH            NO_CONTENT_LENGTH

#define HTTP_HEADER_200                   "HTTP/1.1 200 OK"
#define HTTP_HEADER_204                   "HTTP/1.1 204 No Content"
#define HTTP_HEADER_207                   "HTTP/1.1 207 Multi-Status"
#define HTTP_HEADER_301                   "HTTP/1.1 301"
#define HTTP_HEADER_400                   "HTTP/1.1 400 Bad Request"
#define HTTP_HEADER_403                   "HTTP/1.1 403"
#define HTTP_HEADER_404                   "HTTP/1.1 404 Not Found"
#define HTTP_HEADER_405                   "HTTP/1.1 405 Method Not Allowed"
#define HTTP_HEADER_406                   "HTTP/1.1 406 Not Acceptable"
#define HTTP_HEADER_412                   "HTTP/1.1 412 Precondition Failed"
#define HTTP_HEADER_429                   "HTTP/1.1 429 Too Many Requests"
#define HTTP_HEADER_444                   "HTTP/1.1 444"
#define HTTP_HEADER_470                   "HTTP/1.1 470 Connection Authorization Required"
#define HTTP_HEADER_500                   "HTTP/1.1 500 Internal Server Error"
#define HTTP_HEADER_504                   "HTTP/1.1 504 Not Able to Connect"
#define HTTP_HEADER_CONTENT_LENGTH        "Content-Length: "
#define HTTP_HEADER_CONTENT_TYPE          "Content-Type: "
#define HTTP_HEADER_CHUNKED               "Transfer-Encoding: chunked"
#define HTTP_HEADER_LOCATION              "Location: "
#define HTTP_HEADER_ACCEPT                "Accept: "
#define HTTP_HEADER_KEEP_ALIVE            "Connection: Keep-Alive"
#define HTTP_HEADER_CLOSE                 "Connection: close"
#define NO_CACHE_HEADER                   "Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0\r\n"\
                                          "Pragma: no-cache"
#define CRLF                              "\r\n"
#define CRLF_CRLF                         "\r\n\r\n"
#define LFLF                              "\n\n"
#define EVENT_STREAM_DATA                 "data: "

/******************************************************
 *                   Enumerations
 ******************************************************/
/**
 * HTTP request type
 */
typedef enum
{
    CY_HTTP_GET_REQUEST,       /**< HTTP server GET request type */
    CY_HTTP_POST_REQUEST,      /**< HTTP server POST request type */
    CY_HTTP_PUT_REQUEST,       /**< HTTP server PUT request type*/
    CY_HTTP_REQUEST_UNDEFINED  /**< HTTP server undefined request type */
} cy_http_request_type_t;

/**
 * HTTP cache
 */
typedef enum
{
    CY_HTTP_CACHE_DISABLED, /**< Do not cache previously fetched resources */
    CY_HTTP_CACHE_ENABLED   /**< Allow caching of previously fetched resources  */
} cy_http_cache_t;

/**
 * HTTP MIME type
 */
typedef enum
{
    MIME_TABLE( EXPAND_AS_ENUMERATION )  /**< HTTP MIME type */
    MIME_UNSUPPORTED                     /**< HTTP unsupported MIME type */
} cy_packet_mime_type_t;

/**
 * HTTP status code
 */
typedef enum
{
    CY_HTTP_200_TYPE,
    CY_HTTP_204_TYPE,
    CY_HTTP_207_TYPE,
    CY_HTTP_301_TYPE,
    CY_HTTP_400_TYPE,
    CY_HTTP_403_TYPE,
    CY_HTTP_404_TYPE,
    CY_HTTP_405_TYPE,
    CY_HTTP_406_TYPE,
    CY_HTTP_412_TYPE,
    CY_HTTP_415_TYPE,
    CY_HTTP_429_TYPE,
    CY_HTTP_444_TYPE,
    CY_HTTP_470_TYPE,
    CY_HTTP_500_TYPE,
    CY_HTTP_504_TYPE
} cy_http_status_codes_t;

/**
 * HTTP server resource type
 */
typedef enum
{
    CY_STATIC_URL_CONTENT,                 /**< Page is constant data in memory addressable area */
    CY_DYNAMIC_URL_CONTENT,                /**< Page is dynamically generated by a @ref url_processor_t type function */
    CY_RESOURCE_URL_CONTENT,               /**< Page data is proivded by a cypress Resource which may reside off-chip */
    CY_RAW_STATIC_URL_CONTENT,             /**< Same as @ref CY_STATIC_URL_CONTENT but HTTP header must be supplied as part of the content */
    CY_RAW_DYNAMIC_URL_CONTENT,            /**< Same as @ref CY_DYNAMIC_URL_CONTENT but HTTP header must be supplied as part of the content */
    CY_RAW_RESOURCE_URL_CONTENT            /**< Same as @ref CY_RESOURCE_URL_CONTENT but HTTP header must be supplied as part of the content */
} cy_url_resource_type;
/**
 * @}
 */

/**
 * @addtogroup http_server_struct
 *
 * HTTP server data structures and type definitions
 *
 *  @{
 */
/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
/**
 * HTTP message structure that gets passed to dynamic URL processor functions
 */
typedef struct
{
    const uint8_t*            data;                         /**< packet data in message body      */
    uint16_t                  data_length;                  /**< data length in current packet    */
    uint32_t                  data_remaining;               /**< data yet to be consumed          */
    bool                      is_chunked_transfer;          /**< chunked data format              */
    cy_packet_mime_type_t     mime_type;                    /**< mime type                        */
    cy_http_request_type_t    request_type;                 /**< request type                     */
} cy_http_message_body_t;

/**
 * @}
 */

typedef struct cy_http_page_s cy_http_page_t;

/**
 * HTTP server request info sent as part of the request callback
 */
typedef struct
{
   cy_http_page_t*           page_found;      /**< Pointer to the found page/resource */
   uint32_t                  data_remaining;  /**< Number of bytes remaining to be sent to the application */
   cy_packet_mime_type_t     mime_type;       /**< Mime type of the request */
   cy_http_request_type_t    request_type;    /**< Request type */
} cy_http_request_info_t;

/**
 * @addtogroup http_server_struct
 *
 * @{
 */
/**
 * Workspace structure for HTTP server stream
 * Users should not access these values - they are provided here only
 * to provide the compiler with datatype size information allowing static declarations
 */
typedef struct
{
    cy_tcp_stream_t tcp_stream;                /**< TCP stream handle */
    bool            chunked_transfer_enabled;  /**< Flag to indicate if chunked transfer is enabled */
} cy_http_response_stream_t;
/**
 * @}
 */

/**
 * HTTP server request/response stream info
 */
typedef struct
{
    cy_http_response_stream_t response;  /**< HTTP server response */
    cy_http_request_info_t    request;   /**< HTTP server request */
} cy_http_stream_t;

/**
 * HTTP server receive data callback
 */
typedef cy_rslt_t (*cy_http_server_receive_callback_t)( cy_http_response_stream_t* stream, uint8_t** data, uint16_t* data_length );

/**
 * HTTP server disconnect socket callback
 */
typedef cy_rslt_t (*cy_http_server_disconnect_callback_t)( cy_http_response_stream_t* stream );

/**
 * Prototype for URL processor functions
 */
typedef int32_t (*url_processor_t)(  const char* url_path, const char* url_query_string, cy_http_response_stream_t* stream, void* arg, cy_http_message_body_t* http_data );

/**
 * HTTP page list structure
 * Request with content length more than MTU size is handled for RAW_DYNAMIC_URL_CONTENT and CY_DYNAMIC_CONTENT type for now.
 */
struct cy_http_page_s
{
    char*                url;                  /**< String containing the path part of the URL of this page/file */
    char*                mime_type;            /**< String containing the MIME type of this page/file */
    cy_url_resource_type url_content_type;     /**< The page type - this selects which part of the url_content union will be used - also see above */
    union
    {
        struct                                 
        {
            url_processor_t generator;         /**< The function which will handle requests for this page */
            void*           arg;               /**< An argument to be passed to the generator function    */
        } dynamic_data;                        /**< Used for CY_DYNAMIC_URL_CONTENT and CY_RAW_DYNAMIC_URL_CONTENT */
        struct                                 
        {
            const void*     ptr;               /**< A pointer to the data for the page/file */
            uint32_t        length;            /**< The length in bytes of the page/file */
        } static_data;                         /**< Used for CY_STATIC_URL_CONTENT and CY_RAW_STATIC_URL_CONTENT */
        const void*         resource_data;     /**< A cypress Resource containing the page/file - Used for CY_RESOURCE_URL_CONTENT and CY_RAW_RESOURCE_URL_CONTENT */
    } url_content;                             /**< Static/Dynamic URL content */
};

/**
 * HTTP server secure TCP credentials
 */
typedef struct
{
    cy_tls_identity_t* tls_identity;    /**< TLS identity */
    uint8_t*           root_ca;         /**< Root certificate */
    uint16_t           root_ca_length;  /**< Root certificate length */
} cy_http_security_info;

/**
 * Workspace structure for HTTP server
 * Users should not access these values - they are provided here only
 * to provide the compiler with datatype size information allowing static declarations
 */
typedef struct
{
    cy_network_interface_t                 network_interface;     /**< MBED network interface */
    cy_tcp_server_t    	                   tcp_server;            /**< MBED TCP server socket handle */
    cy_thread_t                            event_thread;          /**< HTTP server data event thread */
    cy_thread_t                            connect_thread;        /**< HTTP server connection request thread */
    cy_mutex_t                             mutex;                 /**< Mutex for critical section */
    volatile bool                          quit;                  /**< Internal quit flag to stop HTTP server */
    const cy_http_page_t*                  page_database;         /**< Handle to the page/resource database */
    uint8_t*                               streams;               /**< Pointer to allocated streams for the max connections */
    cy_linked_list_t                       active_stream_list;    /**< List of active streams */
    cy_linked_list_t                       inactive_stream_list;  /**< List of inactive streams */
    cy_http_server_receive_callback_t      receive_callback;      /**< Internal TCP socket receive callback */
    cy_http_server_disconnect_callback_t   disconnect_callback;   /**< Internal TCP disconnect callback */
} cy_http_server_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/**
 * Start a HTTP server daemon (web server)
 *
 * The web server implements HTTP1.1 using a non-blocking architecture which allows
 * multiple sockets to be served simultaneously.
 * Web pages and other files can be served either dynamically from a function or
 * from static data in memory or internal/external flash resources
 *
 * @param[in] server                  Structure workspace that will be used for this HTTP server instance - allocated by caller
 * @param[in] network_interface       Pointer to the network interface
 * @param[in] port                    TCP port number on which to listen - usually port 80 for normal HTTP
 * @param[in] max_sockets             Maximum number of sockets to be served simultaneously
 * @param[in] page_database           A list of web pages / files that will be served by the HTTP server. See @ref cy_http_page_t for details and snippet apps for examples
 * @param[in] type                    type of server whether secure or non-secure
 * @param[in] identity                For non-secure server, identity will be NULL or ignored if passed. for secure server identity has server certificate, key and rootCA certificate
 *
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_server_start( cy_http_server_t* server, void* network_interface, uint16_t port, uint16_t max_sockets, cy_http_page_t* page_database, cy_server_type_t type, cy_http_security_info* security_info );

/**
 *  Stop a HTTP server daemon (web server)
 *
 * @param[in] server   The structure workspace that was used with @ref cy_http_server_start
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_server_stop( cy_http_server_t* server );

/**
 * Start a HTTPS server daemon (secure web server)
 *
 * This is identical to @ref cy_http_server_start except that it uses TLS to provide
 * a secure HTTPS link
 *
 * @param[in] server                   Structure workspace that will be used for this HTTP server instance - allocated by caller.
 * @param[in] network_interface        Pointer to the network interface
 * @param[in] port                     TCP port number on which to listen - usually port 80 for normal HTTP
 * @param[in] max_sockets              Maximum number of sockets to be served simultaneously
 * @param[in] page_database            A list of web pages / files that will be served by the HTTP server. See @ref cy_http_page_t for details and snippet apps for examples
 * @param[in] identity                 Certificate & private key of server
 * @param[in] url_processor_stack_size Thread stack size
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_https_server_start( cy_http_server_t* server, void* network_interface, uint16_t port, uint16_t max_sockets, const cy_http_page_t* page_database, void* identity, uint32_t url_processor_stack_size );

/**
 *  Stop a HTTPS server daemon (web server)
 *
 * @param[in] server   The structure workspace that was used with @ref cy_https_server_start
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_https_server_stop( cy_http_server_t* server );

/**
 * Register HTTP server callback(s)
 *
 * @param[in] server              : HTTP server
 * @param[in] receive_callback    : Callback function that will be called when a packet is received by the server
 * @param[in] disconnect_callback : Callback function that will be called when a disconnection event is received by the server
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_server_register_callbacks( cy_http_server_t* server, cy_http_server_receive_callback_t receive_callback, cy_http_server_disconnect_callback_t disconnect_callback );

/**
 * Deregister HTTP server callback(s)
 *
 * @param[in] server : HTTP server
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_server_deregister_callbacks( cy_http_server_t* server );

/**
 * Queue a disconnect request to the HTTP server
 *
 * @param[in] stream : stream to disconnect
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_response_stream_disconnect( cy_http_response_stream_t* stream );

/**
 * Disconnect all HTTP stream in a server
 *
 * @param[in] server   The structure workspace that was used with @ref cy_http_server_start
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_disconnect_all_response_stream( cy_http_server_t* server );

/**
 * Initialise HTTP server stream
 *
 * @param[in] stream : HTTP server stream
 * @param[in] socket : TCP socket for the stream to use
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_response_stream_init( cy_http_response_stream_t* stream, void* socket );

/**
 * Deinitialise HTTP server stream
 *
 * @param[in] stream : HTTP server stream
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_response_stream_deinit( cy_http_response_stream_t* stream );

/**
 * Enable chunked transfer encoding on the HTTP stream
 *
 * @param[in] stream : HTTP stream
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_response_stream_enable_chunked_transfer( cy_http_response_stream_t* stream );

/**
 * Disable chunked transfer encoding on the HTTP stream
 *
 * @param[in] stream : HTTP stream
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_response_stream_disable_chunked_transfer( cy_http_response_stream_t* stream );

/**
 * Write HTTP header to the TCP stream provided
 *
 * @param[in] stream         : HTTP stream to write the header into
 * @param[in] status_code    : HTTP status code
 * @param[in] content_length : HTTP content length to follow in bytes
 * @param[in] cache_type     : HTTP cache type (enabled or disabled)
 * @param[in] mime_type      : HTTP MIME type
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_response_stream_write_header( cy_http_response_stream_t* stream, cy_http_status_codes_t status_code, uint32_t content_length, cy_http_cache_t cache_type, cy_packet_mime_type_t mime_type );

/**
 * Write data to HTTP stream
 *
 * @param[in] stream : HTTP stream to write the data into
 * @param[in] data   : data to write
 * @param[in] length : data length in bytes
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_response_stream_write( cy_http_response_stream_t* stream, const void* data, uint32_t length );

/**
 * Write resource to HTTP stream
 *
 * @param[in] stream   : HTTP stream to write the resource into
 * @param[in] resource : Pointer to resource
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_response_stream_write_resource( cy_http_response_stream_t* stream, const void* resource );

/**
 * Flush HTTP stream
 *
 * @param[in] stream : HTTP stream to flush
 *
 * @return cy_rslt_t
 */
cy_rslt_t cy_http_response_stream_flush( cy_http_response_stream_t* stream );

/**
 * Search for a parameter (key-value pair) in a URL query string and return a pointer to the value
 *
 * @param[in]  url_query       : URL query string
 * @param[in]  parameter_key   : Key or name of the parameter to find in the URL query string
 * @param[out] parameter_value : If the parameter with the given key is found, this pointer will point to the parameter value upon return; NULL otherwise
 * @param[out] value_length    : This variable will contain the length of the parameter value upon return; 0 otherwise
 *
 * @return cy_rslt_t CY_RSLT_SUCCESS if found; CY_RSLT_NOT_FOUND if not found
 */
cy_rslt_t cy_http_get_query_parameter_value( const char* url_query, const char* parameter_key, char** parameter_value, uint32_t* value_length );

/**
 * Return the number of parameters found in the URL query string
 *
 * @param[in] url_query : URL query string
 *
 * @return parameter count
 */
uint32_t cy_http_get_query_parameter_count( const char* url_query );

/**
 * Match a URL query string contains a parameter with the given parameter key and value
 *
 * @param[in]  url_query       : URL query string
 * @param[in]  parameter_key   : NUL-terminated key or name of the parameter to find in the URL query string
 * @param[out] parameter_value : NUL-terminated value of the parameter to find in the URL query string
 *
 * @return cy_rslt_t CY_RSLT_SUCCESS if matched; CY_RSLT_NOT_FOUND if matching parameter is not found
 */
cy_rslt_t cy_http_match_query_parameter( const char* url_query, const char* parameter_key, const char* parameter_value );

#ifdef __cplusplus
} /* extern "C" */
#endif

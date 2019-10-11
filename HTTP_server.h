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

#pragma once

#include "cy_HTTP_server.h"

/**
********************************************************************************
* \mainpage Overview
********************************************************************************
* Communication functions for HTTP (Hypertext Transfer Protocol) Server
*
* HTTP functions as a request-response protocol in the client-server computing model. A web browser,
* for example, may be the client and an application running on a computer hosting a website may be the
* server. The client submits an HTTP request message to the server. The server, which provides
* resources such as HTML files and other content, or performs other functions on behalf of the client,
* returns a response message to the client. The response contains completion status information about
* the request and may also contain requested content in its message body.
*
* The Cypress HTTP server library is capable of both secure [with TLS security] and non-secure modes of connection.
* The library provides support for various RESTful HTTP methods such as GET, POST and PUT; and has support for
* various content types [e.g. HTML, Plain, JSON].
*
* The HTTP library is capable of handling content payload that is greater than MTU size for CY_RAW_DYNAMIC_URL_CONTENT
* and CY_DYNAMIC_URL_CONTENT type for now.
*
* This library has the capability of Server-Sent Events (SSE). SSE is a server push technology enabling a browser or
* any HTTP client to receive automatic updates from a server via HTTP connection.
*
* \defgroup http_server HTTP server library
* \defgroup http_server_struct HTTP server data structures
* @ingroup http_server
* \defgroup http_server_classes HTTP server base class and sub-classes
* @ingroup http_server
* \defgroup http_server_defines HTTP server preprocessor directives
* @ingroup http_server
*/
 

/******************************************************
 *                      Macros
 ******************************************************/
/* Currently maximum number of resources are configured to 10.
 * Change below macro to register more number of resources
 */
#define MAX_NUMBER_OF_RESOURCES 10

/**
 * @addtogroup http_server_struct
 *
 * HTTP server data structures and type definitions
 *
 * @{
 */
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
/** Dynamic HTTP resource info */
typedef struct cy_resource_dynamic_data_s
{
    url_processor_t resource_handler;       /**< The function that will handle requests for this page */
    void*           arg;                    /**< Argument to be passed to the generator function    */
} cy_resource_dynamic_data_t;

/** Static HTTP resource info */
typedef struct cy_resource_static_data_t
{
    const void* data;                       /**< A pointer to the data for the page/file resource */
    uint32_t    length;                     /**< The length in bytes of the page/file */
} cy_resource_static_data_t;

/** HTTP server security info */
typedef struct
{
    uint8_t*    private_key;                /**< HTTP server private key */
    uint16_t    key_length;                 /**< HTTP server private key length */
    uint8_t*    certificate;                /**< HTTP server certificate */
    uint16_t    certificate_length;         /**< HTTP server certificate length */
    uint8_t*    root_ca_certificate;        /**< Root CA certificate to verify client certificate */
    uint16_t    root_ca_certificate_length; /**< Root CA certificate length */
} cy_https_server_security_info_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *                 Static Variables
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

/**
 * @}
 */

/*****************************************************************************/
/**
 *
 * @addtogroup http_server_classes
 *
 * Base class and communication functions for HTTP server middleware library
 * This is the primary HTTP server interface for the applications.
 *
 *  @{
 */
/*****************************************************************************/
/** Defines HTTP server class */
class HTTPServer
{

public:

    /**
     * Non-Secure HTTP server constructor
     *
     * @param[in] interface          : Pointer to a structure with network interface type and object
     * @param[in] port               : Port number on which to listen - usually port 80 for normal HTTP
     * @param[in] max_connection     : Maximum number of client connections that can be accepted
     *
     */
    HTTPServer( cy_network_interface_t* interface, uint16_t port, uint16_t max_connection );

    /**
     * Secure HTTP server constructor
     *
     * @param[in] interface          : Pointer to a structure with network interface type and object
     * @param[in] port               : Port number on which to listen - usually port 443 for secure HTTP
     * @param[in] max_connection     : Maximum number of client connection that can be accepted
     * @param[in] security_info      : Security info containing certificate, private key and rootCA certificate
     *
     */
    HTTPServer( cy_network_interface_t* interface, uint16_t port, uint16_t max_connection, cy_https_server_security_info_t* security_info );

    /**
     * HTTPserver Destructor
     *
     */
    ~HTTPServer();

    /**
     * Start the HTTP server
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in @ref http_server_defines otherwise
    **/
    cy_rslt_t start();

    /**
     * Stop the HTTP server
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in @ref http_server_defines otherwise
    **/
    cy_rslt_t stop();

    /**
     * Used to register a static resource with the HTTP server.
     *
     * @param[in] uri                 :   URI of the resource. Application should reserve memory for the URI
     * @param[in] mime_type           :   MIME type of the resource. Application should reserve memory for the MIME type
     * @param[in] url_resource_type   :   Content type of the resource
     * @param[in] resource_data       :   Pointer to structure of static, dynamic or resource type
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in @ref http_server_defines otherwise
    **/
    cy_rslt_t register_resource( uint8_t* uri, uint8_t* mime_type, cy_url_resource_type url_resource_type, void* resource_data );

    /**
     * Enable chunked transfer encoding on the HTTP stream
     *
     * @param[in] stream              : HTTP stream
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in @ref http_server_defines otherwise
     */
    cy_rslt_t http_response_stream_enable_chunked_transfer( cy_http_response_stream_t* stream );

    /**
     * Disable chunked transfer encoding on the HTTP stream
     *
     * @param[in] stream              : HTTP stream
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in @ref http_server_defines otherwise
     */
    cy_rslt_t http_response_stream_disable_chunked_transfer( cy_http_response_stream_t* stream );

    /**
     * Write HTTP header to the TCP stream provided
     *
     * @param[in] stream              : HTTP stream to write the header into
     * @param[in] status_code         : HTTP status code
     * @param[in] content_length      : HTTP content length to follow in bytes
     * @param[in] cache_type          : HTTP cache type (enabled or disabled)
     * @param[in] mime_type           : HTTP MIME type
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in @ref http_server_defines otherwise
     */
    cy_rslt_t http_response_stream_write_header( cy_http_response_stream_t* stream, cy_http_status_codes_t status_code, uint32_t content_length, cy_http_cache_t cache_type, cy_packet_mime_type_t mime_type );

    /**
     * Write data to HTTP stream
     *
     * @param[in] stream              : HTTP stream to write the data into
     * @param[in] data                : Data to write
     * @param[in] length              : Data length in bytes
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in @ref http_server_defines otherwise
     */
    cy_rslt_t http_response_stream_write( cy_http_response_stream_t* stream, const void* data, uint32_t length );

    /**
     * Disconnect HTTP response stream
     *
     * @param[in] stream              : stream to disconnect
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in @ref http_server_defines otherwise
     */
    cy_rslt_t http_response_stream_disconnect( cy_http_response_stream_t* stream );

    /**
     * Disconnect all HTTP stream in a server
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in @ref http_server_defines otherwise
     */
    cy_rslt_t http_disconnect_all_response_stream();

    /**
     * Flush HTTP stream
     *
     * @param[in] stream              : HTTP stream to flush
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in @ref http_server_defines otherwise
     */
    cy_rslt_t http_response_stream_flush( cy_http_response_stream_t* stream );

    /**
     * Search for a parameter (key-value pair) in a URL query string and return a pointer to the value
     *
     * @param[in]  url_query          : URL query string
     * @param[in]  key                : Key or name of the parameter to find in the URL query string
     * @param[out] value              : If the parameter with the given key is found, this pointer will point to the parameter value upon return; NULL otherwise
     * @param[out] value_length       : This variable will contain the length of the parameter value upon return; 0 otherwise
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in @ref http_server_defines otherwise
     */
    cy_rslt_t http_get_query_parameter_value( const char* url_query, const char* key, char** value, uint32_t* value_length );

    /**
     * Return the number of parameters found in the URL query string
     *
     * @param[in] url_query           : URL query string
     *
     * @return parameter count
     */
    uint32_t http_get_query_parameter_count( const char* url_query );

    /**
     * Match a URL query string contains a parameter with the given parameter key and value
     *
     * @param[in]  url_query          : URL query string
     * @param[in]  parameter_key      : NUL-terminated key or name of the parameter to find in the URL query string
     * @param[out] parameter_value    : NUL-terminated value of the parameter to find in the URL query string
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in @ref http_server_defines otherwise
     */
    cy_rslt_t http_match_query_parameter( const char* url_query, const char* parameter_key, const char* parameter_value );

private:
   cy_https_server_security_info_t* security_credentials;
   cy_network_interface_t*          nw_interface;
   uint16_t                         port;
   uint16_t                         max_sockets;
   cy_http_server_t                 http_server;
   uint16_t                         resource_count;
   cy_http_page_t                   page_database[MAX_NUMBER_OF_RESOURCES];
   cy_tls_identity_t                identity;
   bool                             is_secure;
   cy_http_security_info            certificate_info;
};
/**
 * @}
 */

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

#pragma once

#include "cy_http_server.h"

/**
 * \defgroup group_api_cpp C++ Class Interface
 */

/******************************************************
 *                      Macros
 ******************************************************/

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

/*****************************************************************************/
/**
 *
 * @addtogroup group_api_cpp
 *
 * This section contains details about C++ base class interface provided by the library.
 *
 * @{
 */
/*****************************************************************************/
/** Definition of the HTTP server class object */
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
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in \ref http_server_defines otherwise
    **/
    cy_rslt_t start();

    /**
     * Stop the HTTP server
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in \ref http_server_defines otherwise
    **/
    cy_rslt_t stop();

    /**
     * Used to register a static resource with the HTTP server.
     *
     * @param[in] url                 :   URL of the resource. Application should reserve memory for the URL.
     * @param[in] mime_type           :   MIME type of the resource. Application should reserve memory for the MIME type.
     * @param[in] url_resource_type   :   Content type of the resource.
     * @param[in] resource_data       :   Pointer to structure of static, dynamic or resource type.
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in \ref http_server_defines otherwise.
    **/
    cy_rslt_t register_resource( uint8_t* url, uint8_t* mime_type, cy_url_resource_type url_resource_type, void* resource_data );

    /**
     * Enable chunked transfer encoding on the HTTP stream
     *
     * @param[in] stream              : HTTP stream
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in \ref http_server_defines otherwise
     */
    cy_rslt_t http_response_stream_enable_chunked_transfer( cy_http_response_stream_t* stream );

    /**
     * Disable chunked transfer encoding on the HTTP stream
     *
     * @param[in] stream              : HTTP stream
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in \ref http_server_defines otherwise
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
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in \ref http_server_defines otherwise
     */
    cy_rslt_t http_response_stream_write_header( cy_http_response_stream_t* stream, cy_http_status_codes_t status_code, uint32_t content_length, cy_http_cache_t cache_type, cy_http_mime_type_t mime_type );

    /**
     * Write data to HTTP stream
     *
     * @param[in] stream              : HTTP stream to write the data into
     * @param[in] data                : Data to write
     * @param[in] length              : Data length in bytes
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in \ref http_server_defines otherwise
     */
    cy_rslt_t http_response_stream_write( cy_http_response_stream_t* stream, const void* data, uint32_t length );

    /**
     * Disconnect HTTP response stream
     *
     * @param[in] stream              : stream to disconnect
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in \ref http_server_defines otherwise
     */
    cy_rslt_t http_response_stream_disconnect( cy_http_response_stream_t* stream );

    /**
     * Disconnect all HTTP stream in a server
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in \ref http_server_defines otherwise
     */
    cy_rslt_t http_disconnect_all_response_stream();

    /**
     * Flush HTTP stream
     *
     * @param[in] stream              : HTTP stream to flush
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in \ref http_server_defines otherwise
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
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in \ref http_server_defines otherwise
     */
    cy_rslt_t http_get_query_parameter_value( const char* url_query, const char* key, char** value, uint32_t* value_length );

    /**
     * Returns number of parameters found in the URL query string.
     *
     * @param[in] url_query           : NULL terminated URL query string.
     * @param[out] count              : Parameter count.
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes from \ref http_server_defines otherwise.
     */
    cy_rslt_t http_get_query_parameter_count( const char *url_query, uint32_t *count );

    /**
     * Match a URL query string contains a parameter with the given parameter key and value
     *
     * @param[in]  url_query          : URL query string
     * @param[in]  parameter_key      : NUL-terminated key or name of the parameter to find in the URL query string
     * @param[out] parameter_value    : NUL-terminated value of the parameter to find in the URL query string
     *
     * @return cy_rslt_t              : CY_RSLT_SUCCESS - on success, error codes in \ref http_server_defines otherwise
     */
    cy_rslt_t http_match_query_parameter( const char* url_query, const char* parameter_key, const char* parameter_value );

private:
   cy_http_server_t         http_server_obj;
};
/**
 * @}
 */

/*
 * Copyright 2022, Cypress Semiconductor Corporation (an Infineon company) or
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
 *  HTTP_server.cpp
 *  HTTP server C++ functions
 *
 */

#include "HTTP_server.hpp"
#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "cy_tls_port.h"

#define HTTP_SERVER_INFO( X )    printf X

HTTPServer::HTTPServer(cy_network_interface_t* network_interface, uint16_t port, uint16_t max_connection)
{
    HTTPServer::http_server_obj = NULL;
    cy_http_server_create( network_interface, port, max_connection, NULL, &(HTTPServer::http_server_obj) );
}

HTTPServer::HTTPServer(cy_network_interface_t* network_interface, uint16_t port, uint16_t max_connection, cy_https_server_security_info_t* security_info)
{
    HTTPServer::http_server_obj = NULL;
    cy_http_server_create( network_interface, port, max_connection, security_info, &(HTTPServer::http_server_obj) );
}

cy_rslt_t HTTPServer::start( )
{
    return cy_http_server_start( HTTPServer::http_server_obj );
}

cy_rslt_t HTTPServer::stop()
{
    return cy_http_server_stop( HTTPServer::http_server_obj );
}

cy_rslt_t HTTPServer::register_resource( uint8_t* url, uint8_t* mime_type, cy_url_resource_type url_resource_type, void* resource_data)
{
    return cy_http_server_register_resource( HTTPServer::http_server_obj, url, mime_type, url_resource_type, resource_data);
}

HTTPServer::~HTTPServer()
{
    cy_http_server_delete( HTTPServer::http_server_obj );
}

cy_rslt_t  HTTPServer::http_response_stream_write( cy_http_response_stream_t* stream, const void* data, uint32_t length )
{
    return cy_http_server_response_stream_write_payload( stream, data, length );
}

cy_rslt_t HTTPServer::http_response_stream_write_header( cy_http_response_stream_t* stream, cy_http_status_codes_t status_code, uint32_t content_length, cy_http_cache_t cache_type, cy_http_mime_type_t mime_type )
{
    return cy_http_server_response_stream_write_header(stream, status_code, content_length, cache_type, mime_type);
}

cy_rslt_t HTTPServer::http_response_stream_disable_chunked_transfer( cy_http_response_stream_t* stream )
{
    return cy_http_server_response_stream_disable_chunked_transfer(stream);
}

cy_rslt_t HTTPServer::http_response_stream_enable_chunked_transfer( cy_http_response_stream_t* stream )
{
    return cy_http_server_response_stream_enable_chunked_transfer(stream);
}

cy_rslt_t HTTPServer::http_response_stream_disconnect( cy_http_response_stream_t* stream )
{
    return cy_http_server_response_stream_disconnect( stream );
}

cy_rslt_t HTTPServer::http_disconnect_all_response_stream( )
{
    return cy_http_server_response_stream_disconnect_all( HTTPServer::http_server_obj );
}

cy_rslt_t HTTPServer::http_response_stream_flush( cy_http_response_stream_t* stream )
{
    return cy_http_server_response_stream_flush( stream );
}

cy_rslt_t HTTPServer::http_get_query_parameter_value( const char* url_query, const char* key, char** value, uint32_t* value_length )
{
    return cy_http_server_get_query_parameter_value( url_query, key, value, value_length );
}

uint32_t HTTPServer::http_get_query_parameter_count( const char* url_query, uint32_t *count )
{
    return cy_http_server_get_query_parameter_count( url_query, count );
}

cy_rslt_t HTTPServer::http_match_query_parameter( const char* url_query, const char* parameter_key, const char* parameter_value )
{
    return cy_http_server_match_query_parameter( url_query, parameter_key, parameter_value );
}

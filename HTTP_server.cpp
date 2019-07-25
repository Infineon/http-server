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
 *  HTTP_server.cpp
 *  HTTP server C++ functions
 *
 */

#include "HTTP_server.h"
#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "cy_tls_wrapper.h"

#define HTTP_SERVER_INFO( X )    printf X

HTTPServer::HTTPServer(cy_network_interface_t* network_interface, uint16_t port, uint16_t max_connection)
{
    HTTPServer::nw_interface   = network_interface;
    HTTPServer::port           = port;
    HTTPServer::max_sockets    = max_connection;
    HTTPServer::resource_count = 0;
    HTTPServer::security_credentials = NULL;
    HTTPServer::is_secure      = false;

    memset(&http_server, 0, sizeof(cy_http_server_t));
    memset(&page_database, 0, sizeof(page_database));
    memset(&identity, 0, sizeof(cy_tls_identity_t));
    memset(&certificate_info, 0, sizeof(cy_http_security_info));
}

HTTPServer::HTTPServer(cy_network_interface_t* network_interface, uint16_t port, uint16_t max_connection, cy_https_server_security_info_t* security_info)
{
    HTTPServer::nw_interface   = network_interface;
    HTTPServer::port           = port;
    HTTPServer::max_sockets    = max_connection;
    HTTPServer::resource_count = 0;
    HTTPServer::is_secure      = true;

    memset(&http_server, 0, sizeof(cy_http_server_t));
    memset(&page_database, 0, sizeof(page_database));
    memset(&identity, 0, sizeof(cy_tls_identity_t));
    memset(&certificate_info, 0, sizeof(cy_http_security_info));

    HTTPServer::security_credentials = security_info;
}

cy_rslt_t HTTPServer::start( )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_resource_static_data_t static_resource;

    static_resource.length = 0;
    static_resource.data    = NULL;

    /* Add static resource to find out the end of the resource list */
    result = HTTPServer::register_resource(0, 0, CY_STATIC_URL_CONTENT, &static_resource);
    if( result != CY_RSLT_SUCCESS)
    {
        HTTP_SERVER_INFO(("Registration of resource failed \n"));
        return result;
    }

    if ( is_secure == true )
    {
        /* Initialize server certificate & private key */
        result = cy_tls_init_identity( &identity, (const char*) security_credentials->private_key, security_credentials->key_length, security_credentials->certificate, security_credentials->certificate_length );
        if ( result != CY_RSLT_SUCCESS )
        {
            HTTP_SERVER_INFO(("TLS init identity failed : %d \n", (int) result));
            return result;
        }

        /* Initialize root CA certificate */
        if ( security_credentials->root_ca_certificate != NULL )
        {
            result = cy_tls_init_root_ca_certificates( (const char*) security_credentials->root_ca_certificate, security_credentials->root_ca_certificate_length );
            if ( result != 0 )
            {
                HTTP_SERVER_INFO(("TLS init root CA certificate failed : %d \n", (int) result));
                return result;
            }
        }

        certificate_info.tls_identity = &identity;

        /* Start secure HTTP server */
        result = cy_http_server_start(&(HTTPServer::http_server), HTTPServer::nw_interface, port, max_sockets, HTTPServer::page_database, CY_HTTP_SERVER_TYPE_SECURE, &certificate_info);
        if( result != CY_RSLT_SUCCESS )
        {
            HTTP_SERVER_INFO(("Failed to start secure HTTP server : %d \n", (int) result));
            return result;
        }
    }
    else
    {
        /* Start non-secure HTTP server */
        result = cy_http_server_start(&(HTTPServer::http_server), HTTPServer::nw_interface, port, max_sockets, HTTPServer::page_database, CY_HTTP_SERVER_TYPE_NON_SECURE, NULL);
        if( result != CY_RSLT_SUCCESS )
        {
            HTTP_SERVER_INFO(("Failed to start HTTP server : %d \n", (int) result));
            return result;
        }
    }

    return result;
}

cy_rslt_t HTTPServer::stop()
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = cy_http_server_stop(&(HTTPServer::http_server));
    if ( result != CY_RSLT_SUCCESS )
    {
        HTTP_SERVER_INFO(("Failed to stop HTTP server : %d \n", (int) result));
        return result;
    }

    if ( is_secure == true )
    {
        cy_tls_deinit_root_ca_certificates();

        /* Deinitialize TLS identity */
        result = cy_tls_deinit_identity( &identity );
        if( result != CY_RSLT_SUCCESS )
        {
            HTTP_SERVER_INFO(("TLS deinit identity failed : %d \n", (int) result));
            return result;
        }
    }

    resource_count = 0;

    return result;
}

cy_rslt_t HTTPServer::register_resource( uint8_t* uri, uint8_t* mime_type, cy_url_resource_type url_resource_type, void* resource_data)
{

    if( resource_count > ( MAX_NUMBER_OF_RESOURCES - 1 ) )
    {
        HTTP_SERVER_INFO(("Maximum number of resources configured are [%d], Please change macro in HTTP_server.h file \n", MAX_NUMBER_OF_RESOURCES));
        return CY_RSLT_HTTP_SERVER_PAGE_DATABASE_FULL;
    }

    page_database[resource_count].url_content_type                   = url_resource_type;
    page_database[resource_count].mime_type                          = (char*) mime_type;
    page_database[resource_count].url                                = (char*) uri;

    if( url_resource_type == CY_DYNAMIC_URL_CONTENT || url_resource_type == CY_RAW_DYNAMIC_URL_CONTENT )
    {
        cy_resource_dynamic_data_t* dynamic_resource = (cy_resource_dynamic_data_t*) resource_data;

        page_database[resource_count].url_content.dynamic_data.generator = dynamic_resource->resource_handler;
        page_database[resource_count].url_content.dynamic_data.arg       = dynamic_resource->arg;
    }
    else if( url_resource_type == CY_STATIC_URL_CONTENT || url_resource_type == CY_RAW_STATIC_URL_CONTENT )
    {
        cy_resource_static_data_t* static_resource = (cy_resource_static_data_t*) resource_data;

        page_database[resource_count].url_content.static_data.ptr    = static_resource->data;
        page_database[resource_count].url_content.static_data.length = static_resource->length;
    }
    else
    {
        /*Fixme : Handle case when resource is in filesystem */
    }

    resource_count++;

    return CY_RSLT_SUCCESS;
}

HTTPServer::~HTTPServer()
{
    /* Nothing to destroy */
}

cy_rslt_t  HTTPServer::http_response_stream_write( cy_http_response_stream_t* stream, const void* data, uint32_t length )
{
    return cy_http_response_stream_write( stream, data, length );
}

cy_rslt_t HTTPServer::http_response_stream_write_header( cy_http_response_stream_t* stream, cy_http_status_codes_t status_code, uint32_t content_length, cy_http_cache_t cache_type, cy_packet_mime_type_t mime_type )
{
    return cy_http_response_stream_write_header(stream, status_code, content_length, cache_type, mime_type);
}

cy_rslt_t HTTPServer::http_response_stream_disable_chunked_transfer( cy_http_response_stream_t* stream )
{
    return cy_http_response_stream_disable_chunked_transfer(stream);
}

cy_rslt_t HTTPServer::http_response_stream_enable_chunked_transfer( cy_http_response_stream_t* stream )
{
    return cy_http_response_stream_enable_chunked_transfer(stream);
}

cy_rslt_t HTTPServer::http_response_stream_disconnect( cy_http_response_stream_t* stream )
{
    return cy_http_response_stream_disconnect( stream );
}

cy_rslt_t HTTPServer::http_disconnect_all_response_stream( )
{
    return cy_http_disconnect_all_response_stream( &(HTTPServer::http_server) );
}

cy_rslt_t HTTPServer::http_response_stream_flush( cy_http_response_stream_t* stream )
{
    return cy_http_response_stream_flush( stream );
}

cy_rslt_t HTTPServer::http_get_query_parameter_value( const char* url_query, const char* key, char** value, uint32_t* value_length )
{
    return cy_http_get_query_parameter_value( url_query, key, value, value_length );
}

uint32_t HTTPServer::http_get_query_parameter_count( const char* url_query )
{
    return http_get_query_parameter_count( url_query );
}

cy_rslt_t HTTPServer::http_match_query_parameter( const char* url_query, const char* parameter_key, const char* parameter_value )
{
    return cy_http_match_query_parameter( url_query, parameter_key, parameter_value );
}

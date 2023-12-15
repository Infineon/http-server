# HTTP Server Library

## Introduction
This library provides the HTTP Server implementation that can work on Infineon MCUs with Wi-Fi connectivity.
It supports RESTful methods such as GET, PUT, and POST for the client to communicate with this HTTP Server library.

## Features
* Supports Wi-Fi and Ethernet connections.
* Supports HTTP/1.1 protocol version.
* Secure [with TLS security] and non-secure modes of connection.
* Supports RESTful HTTP methods: GET, PUT, and POST.
* Handles various resource content types such as HTML, Plain, and JSON.
* Capable of handling content payload greater than the MTU size using the Content-Length HTTP header. This feature is supported only for `CY_RAW_DYNAMIC_URL_CONTENT` and `CY_DYNAMIC_URL_CONTENT` content types.
* Supports chunked encoding for GET and POST methods.
  **Note:** For a POST request, chunked encoding is supported only for the data that is less than a single MTU; Content-Length headers are recommended for larger data.
* Supports Server-Sent Events (SSE). SSE is a server push technology, enabling an HTTP client (for example, a browser or any device running an HTTP client) to receive automatic updates from the HTTP server via the HTTP connection.

## Supported Platforms

### ModusToolbox&trade;
* [PSoC&trade; 6 Wi-Fi Bluetooth&reg; prototyping kit (CY8CPROTO-062-4343W)](https://www.infineon.com/cms/en/product/evaluation-boards/cy8cproto-062-4343w/)
* [PSoC&trade; 62S2 Wi-Fi Bluetooth&reg; pioneer kit (CY8CKIT-062S2-43012)](https://www.infineon.com/cms/en/product/evaluation-boards/cy8ckit-062s2-43012/)
* [PSoC&trade; 62S2 evaluation kit (CY8CEVAL-062S2-LAI-4373M2)](https://www.infineon.com/cms/en/product/evaluation-boards/cy8ceval-062s2/)
* [PSoC&trade; 6 WiFi-BT Pioneer Kit (CY8CKIT-062-WiFi-BT)](https://www.infineon.com/cms/en/product/evaluation-boards/cy8ckit-062-wifi-bt/)
* [CYW954907AEVAL1F evaluation kit (CYW954907AEVAL1F)](https://www.infineon.com/cms/en/product/evaluation-boards/cyw954907aeval1f/)
* [CYW943907AEVAL1F Evaluation Kit (CYW943907AEVAL1F)](https://www.infineon.com/cms/en/product/evaluation-boards/cyw943907aeval1f/)
* [PSoC&trade; 62S2 evaluation kit (CY8CEVAL-062S2-MUR-43439M2)](https://www.infineon.com/cms/en/product/evaluation-boards/cy8ceval-062s2/)
* [XMC7200D-E272K8384 kit (KIT-XMC72-EVK)](https://www.infineon.com/cms/en/product/evaluation-boards/kit_xmc72_evk/)
* [XMC7200D-E272K8384 kit (KIT_XMC72_EVK_MUR_43439M2)](https://www.infineon.com/cms/en/product/evaluation-boards/kit_xmc72_evk/)

## Supported Frameworks
This middleware library supports ModusToolbox&trade; framework:
* ModusToolbox&trade; environment: In this environment the HTTP Server Library uses the [abstraction-rtos](https://github.com/Infineon/abstraction-rtos) library that provides the RTOS abstraction API and uses the [secure-sockets](https://github.com/Infineon/secure-sockets) library for implementing socket functions.

## Quick Start
This library is supported on ModusToolbox&trade; framework. The section below provides information on how to build the library in those framework.

### ModusToolbox&trade;
- To use http-server library with Wi-Fi kits on FreeRTOS, lwIP, and Mbed TLS combination, the application should pull [http-server](https://github.com/Infineon/http-server) library and [wifi-core-freertos-lwip-mbedtls](https://github.com/Infineon/wifi-core-freertos-lwip-mbedtls) library which will internally pull secure-sockets, wifi-connection-manager, FreeRTOS, lwIP, Mbed TLS and other dependent modules.
To pull wifi-core-freertos-lwip-mbedtls and http-server libraries create the following *.mtb* files in deps folder.
   - *wifi-core-freertos-lwip-mbedtls.mtb:*
      `https://github.com/Infineon/wifi-core-freertos-lwip-mbedtls#latest-v1.X#$$ASSET_REPO$$/wifi-core-freertos-lwip-mbedtls/latest-v1.X`
	  
	  **Note:** To use TLS version 1.3, please upgrade wifi-core-freertos-lwip-mbedtls to latest-v2.X (It is supported on all the platforms except [PSoC&trade; 64S0S2 Wi-Fi Bluetooth&reg; pioneer kit (CY8CKIT-064S0S2-4343W)](https://www.cypress.com/documentation/development-kitsboards/psoc-64-standard-secure-aws-wi-fi-bt-pioneer-kit-cy8ckit))
	  
   - *http-server.mtb:*
      `https://github.com/Infineon/http-server#latest-v2.X#$$ASSET_REPO$$/http-server/latest-v2.X`

- To use http-server library with Ethernet kits on FreeRTOS, lwIP, and Mbed TLS combination, the application should pull [http-server](https://github.com/Infineon/http-server) library and [ethernet-core-freertos-lwip-mbedtls](https://github.com/Infineon/ethernet-core-freertos-lwip-mbedtls) library which will internally pull secure-sockets, ethernet-connection-manager, FreeRTOS, lwIP, Mbed TLS and other dependent modules.
To pull ethernet-core-freertos-lwip-mbedtls and http-server libraries create the following *.mtb* files in deps folder.
   - *ethernet-core-freertos-lwip-mbedtls.mtb:*
      `https://github.com/Infineon/ethernet-core-freertos-lwip-mbedtls#latest-v1.X#$$ASSET_REPO$$/ethernet-core-freertos-lwip-mbedtls/latest-v1.X`
	  
	  **Note:** To use TLS version 1.3, please upgrade ethernet-core-freertos-lwip-mbedtls to latest-v2.X
	  
   - *http-server.mtb:*
      `https://github.com/Infineon/http-server#latest-v2.X#$$ASSET_REPO$$/http-server/latest-v2.X`

- A set of pre-defined configuration files have been bundled with the wifi-core-freertos-lwip-mbedtls library for FreeRTOS, lwIP, and mbed TLS. Review the configuration and make the required adjustments. See the "Quick Start" section in [README.md](https://github.com/Infineon/wifi-core-freertos-lwip-mbedtls/blob/master/README.md).
-  If the application is using bundle library then the configuration files are in the bundle library. For example if the application is using Wi-Fi core freertos lwip mbedtls bundle library, the configuration files are in wifi-core-freertos-lwip-mbedtls/configs folder. Similarly if the application is using Ethernet Core FreeRTOS lwIP mbedtls library, the configuration files are in ethernet-core-freertos-lwip-mbedtls/configs folder.
- Define following COMPONENTS in the application's makefile for the HTTP Server Library.
  ```
    COMPONENTS=FREERTOS MBEDTLS LWIP SECURE_SOCKETS
  ```
- HTTP Server Library disables all the debug log messages by default. To enable log messages, the application must perform the following:
  - Add `ENABLE_HTTP_SERVER_LOGS` macro to the *DEFINES* in the code example's Makefile. The Makefile entry would look like as follows:
     ```
       DEFINES+=ENABLE_HTTP_SERVER_LOGS
     ```
  - Call the `cy_log_init()` function provided by the *cy-log* module. cy-log is part of the *connectivity-utilities* library. See [connectivity-utilities library API documentation](https://Infineon.github.io/connectivity-utilities/api_reference_manual/html/group__logging__utils.html) for cy-log details.
- Define the following macro in application's makefile to configure the maximum number of HTTP server resources to 'N':
  ```
    DEFINES+=MAX_NUMBER_OF_HTTP_SERVER_RESOURCES=<N>
  ```

## Additional Information
* [HTTP Server RELEASE.md](./RELEASE.md)
* [HTTP Server API Reference Guide](https://Infineon.github.io/http-server/api_reference_manual/html/index.html)
* [HTTP Server Library Version](./version.xml)


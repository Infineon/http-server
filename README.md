# HTTP Server Library

## Introduction
This library provides the HTTP Server implementation that can work on the PSoC 6 MCU platforms with Wi-Fi connectivity. 
It supports RESTful methods such as GET, PUT, and POST for the client to communicate with this HTTP Server library.
 
## Features
* Secure [with TLS security] and non-secure modes of connection.
* Supports RESTful HTTP methods: GET, PUT, and POST.
* Handles various resource content types such as HTML, Plain, and JSON.
* Capable of handling content payload greater than the MTU size using the Content-Length HTTP header. This feature is supported only for `CY_RAW_DYNAMIC_URL_CONTENT` and `CY_DYNAMIC_URL_CONTENT` content types.
* Supports chunked encoding for GET and POST methods.   
  **Note:** For a POST request, chunked encoding is supported only for the data that is less than a single MTU; Content-Length headers are recommended for larger data.
* Supports Server-Sent Events (SSE). SSE is a server push technology, enabling an HTTP client (for example, a browser or any device running an HTTP client) to receive automatic updates from the HTTP server via the HTTP connection.

## Supported Platforms
### AnyCloud
* [PSoC 6 WiFi-BT Prototyping Kit (CY8CPROTO-062-4343W)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wi-fi-bt-prototyping-kit-cy8cproto-062-4343w)
* [PSoC 62S2 Wi-Fi BT Pioneer Kit (CY8CKIT-062S2-43012)](https://www.cypress.com/documentation/development-kitsboards/psoc-62s2-wi-fi-bt-pioneer-kit-cy8ckit-062s2-43012)

### Mbed OS
* [PSoC 6 WiFi-BT Prototyping Kit (CY8CPROTO-062-4343W)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wi-fi-bt-prototyping-kit-cy8cproto-062-4343w)
* [PSoC 6 WiFi-BT Pioneer Kit (CY8CKIT-062-WiFi-BT)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wifi-bt-pioneer-kit-cy8ckit-062-wifi-bt)
* [PSoC 62S2 Wi-Fi BT Pioneer Kit (CY8CKIT-062S2-43012)](https://www.cypress.com/documentation/development-kitsboards/psoc-62s2-wi-fi-bt-pioneer-kit-cy8ckit-062s2-43012)

## Supported Frameworks
This middleware library supports the following frameworks:
* AnyCloud Framework: AnyCloud is a FreeRTOS-based solution. HTTP Server Library uses the [abstraction-rtos](https://github.com/cypresssemiconductorco/abstraction-rtos) library that provides the RTOS abstraction API and uses the [secure-sockets](https://github.com/cypresssemiconductorco/secure-sockets) library for implementing socket functions.
* Mbed Framework: Mbed framework is a Mbed OS-based solution. HTTP Server Library uses the [abstraction-rtos](https://github.com/cypresssemiconductorco/abstraction-rtos) library that provides RTOS abstraction API and uses the Mbed socket API for implementing socket functions.

## Dependencies
This section provides the list of dependent libraries required for this middleware library to work on AnyCloud and Arm Mbed OS IoT frameworks.

### AnyCloud
  * [Wi-Fi Middleware Core](https://github.com/cypresssemiconductorco/wifi-mw-core)

### Mbed OS
  * [Arm Mbed OS 6.2.0](https://os.mbed.com/mbed-os/releases)
  * [Connectivity Utilities Library](https://github.com/cypresssemiconductorco/connectivity-utilities/releases/tag/latest-v3.X)

## Quick Start
This library is supported on both AnyCloud and Mbed OS frameworks. The section below provides information on how to build the library in those framework.

### AnyCloud
- A set of pre-defined configuration files have been bundled with the wifi-mw-core library for FreeRTOS, lwIP, and mbed TLS. Review the configuration and make the required adjustments. See the "Quick Start" section in [README.md](https://github.com/cypresssemiconductorco/wifi-mw-core/blob/master/README.md).
- Define following COMPONENTS in the application's makefile for the HTTP Server Library. For additional information, see the "Quick Start" section in [README.md](https://github.com/cypresssemiconductorco/wifi-mw-core/blob/master/README.md).
  ```
    COMPONENTS=FREERTOS MBEDTLS LWIP SECURE_SOCKETS
  ```
- HTTP Server Library disables all the debug log messages by default. To enable log messages, the application must perform the following:   
  - Add `ENABLE_HTTP_SERVER_LOGS` macro to the *DEFINES* in the code example's Makefile. The Makefile entry would look like as follows:
     ```
       DEFINES+=ENABLE_HTTP_SERVER_LOGS
     ```
  - Call the `cy_log_init()` function provided by the *cy-log* module. cy-log is part of the *connectivity-utilities* library. See [connectivity-utilities library API documentation](https://cypresssemiconductorco.github.io/connectivity-utilities/api_reference_manual/html/group__logging__utils.html) for cy-log details. 
- Define the following macro in application's makefile to configure the maximum number of HTTP server resources to 'N':
  ```
    DEFINES+=MAX_NUMBER_OF_HTTP_SERVER_RESOURCES=<N>
  ```

### Mbed OS
- Add the .lib file(s) for dependent libraries.
  - Create a folder named `deps`.
  - Create a file with name mbed-os.lib and add the following line to this file:
    ```
    https://github.com/ARMmbed/mbed-os/#a2ada74770f043aff3e61e29d164a8e78274fcd4
    ```
  - Create a file with name connectivity-utilities.lib and add the following line to this file:
    ```
    https://github.com/cypresssemiconductorco/connectivity-utilities/#<commit-SHA-for-latest-release-v3.X>
    ```
    - Replace `<commit-SHA-for-latest-release-v3.X>` in the above line with commit SHA of the latest-v3.X tag available in the [GitHub repository](https://github.com/cypresssemiconductorco/connectivity-utilities/releases/tag/latest-v3.X).
      -  Example: For tag `release-v3.0.1`
         ```
         https://github.com/cypresssemiconductorco/connectivity-utilities/#68bd1bc9883a0ab424eb6daf1e726f0aba2c54a6
         ```
- HTTP Server Library disables all the debug log messages by default. To enable log messages, the application must perform the following:   
  - Add `ENABLE_HTTP_SERVER_LOGS` macro to the *DEFINES* in the code example's JSON file. The JSON file entry would look like as follows:
     ```
       "macros": ["ENABLE_HTTP_SERVER_LOGS"],
       "target.components_add": ["MBED"],
     ```
  - Call the `cy_log_init()` function provided by the *cy-log* module. cy-log is part of the *connectivity-utilities* library. See [connectivity-utilities library API documentation](https://cypresssemiconductorco.github.io/connectivity-utilities/api_reference_manual/html/group__logging__utils.html) for cy-log details.
- Define the following macro in the application JSON file to configure the maximum number of HTTP server resources to 'N':
  ```
    "macros": ["MAX_NUMBER_OF_HTTP_SERVER_RESOURCES=<N>"],
  ```

## Additional Information
* [HTTP Server RELEASE.md](./RELEASE.md)
* [HTTP Server API Reference Guide](https://cypresssemiconductorco.github.io/http-server/api_reference_manual/html/index.html)
* [HTTP Server Library Version](./version.txt)


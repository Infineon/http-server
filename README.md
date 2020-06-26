# **HTTP Server Library**

## **Introduction**
This library provides the HTTP Server implementation that can work on the PSoC 6 MCU platforms with Wi-Fi connectivity. 
It supports RESTful methods such as GET, PUT, and POST for the client to communicate with this HTTP Server library.
 
## **Features** 
* Secure [with TLS security] and non-secure modes of connection.
* Supports RESTful HTTP methods: GET, PUT, and POST.
* Handles various resource content types such as HTML, Plain, and JSON.
* Capable of handling content payload greater than the MTU size using the Content-Length HTTP header. This feature is supported only for `CY_RAW_DYNAMIC_URL_CONTENT` and `CY_DYNAMIC_URL_CONTENT` content types
* Supports chunked encoding for GET and POST methods. 

  **Note:** For a POST request, chunked encoding is supported only for the data that is less than a single MTU; Content-Length headers are recommended for larger data.
* Supports Server-Sent Events (SSE). SSE is a server push technology, enabling an HTTP client (for example, a browser or any device running an HTTP client) to receive automatic updates from the HTTP server via the HTTP connection.

## **Supported Platforms**
### AnyCloud
* [PSoC 6 WiFi-BT Prototyping Kit (CY8CPROTO-062-4343W)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wi-fi-bt-prototyping-kit-cy8cproto-062-4343w)
* [PSoC 62S2 Wi-Fi BT Pioneer Kit (CY8CKIT-062S2-43012)](https://www.cypress.com/documentation/development-kitsboards/psoc-62s2-wi-fi-bt-pioneer-kit-cy8ckit-062s2-43012)

### Mbed OS
* [PSoC 6 WiFi-BT Prototyping Kit (CY8CPROTO-062-4343W)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wi-fi-bt-prototyping-kit-cy8cproto-062-4343w)
* [PSoC 6 WiFi-BT Pioneer Kit (CY8CKIT-062-WiFi-BT)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wifi-bt-pioneer-kit-cy8ckit-062-wifi-bt)
* [PSoC 62S2 Wi-Fi BT Pioneer Kit (CY8CKIT-062S2-43012)](https://www.cypress.com/documentation/development-kitsboards/psoc-62s2-wi-fi-bt-pioneer-kit-cy8ckit-062s2-43012)

## **Supported Frameworks**
This middleware library supports the following frameworks:
* AnyCloud Framework : AnyCloud is a FreeRTOS-based solution. HTTP server library uses the [abstraction-rtos](https://github.com/cypresssemiconductorco/abstraction-rtos) library that provides the RTOS abstraction API and uses the [secure-sockets](https://github.com/cypresssemiconductorco/secure-sockets) library for implementing socket functions.
* Mbed framework: Mbed framework is a Mbed OS-based solution. HTTP server library uses the [abstraction-rtos](https://github.com/cypresssemiconductorco/abstraction-rtos) library that provides RTOS abstraction API and uses the Mbed socket API for implementing socket functions.

## **Dependencies**
This section provides the list of dependent libraries required for this middleware library to work on AnyCloud and Arm Mbed OS IoT frameworks.

### AnyCloud
  * [Wi-Fi Middleware Core](https://github.com/cypresssemiconductorco/wifi-mw-core)

### Mbed OS
  * [Arm Mbed OS Stack version 5.15.3](https://os.mbed.com/mbed-os/releases)
  * [Connectivity Utilities Library](https://github.com/cypresssemiconductorco/connectivity-utilities)

## **Quick Start**
This library is supported on both AnyCloud and Mbed OS frameworks. The section below provides information on how to build the library in those framework.

### AnyCloud
- A set of pre-defined configuration files have been bundled with the wifi-mw-core library for FreeRTOS, lwIP, and mbed TLS. Review the configuration and make the required adjustments. See the "Quick Start" section in [README.md](https://github.com/cypresssemiconductorco/wifi-mw-core/blob/master/README.md).

- Define a set of COMPONENTS in the application's makefile for the HTTP Server Library. See the "Quick Start" section in [README.md](https://github.com/cypresssemiconductorco/wifi-mw-core/blob/master/README.md).

- HTTP Server Library disables all the debug log messages by default. To enable log messages, the application must perform the following:
  -# Add `ENABLE_HTTP_SERVER_LOGS` macro to the *DEFINES* in the code example's Makefile. The Makefile entry would look like as follows:
     \code
       DEFINES+=ENABLE_HTTP_SERVER_LOGS
     \endcode
  -# Call the `cy_log_init()` function provided by the *cy-log* module. cy-log is part of the *connectivity-utilities* library. See [connectivity-utilities library API documentation](https://cypresssemiconductorco.github.io/connectivity-utilities/api_reference_manual/html/group__logging__utils.html) for cy-log details.
- Define the following macro in application's makefile to configure the maximum number of HTTP server resources:
  ```
  MAX_NUMBER_OF_HTTP_SERVER_RESOURCES
  ```

### Mbed OS
- HTTP Server Library disables all the debug log messages by default. To enable log messages, the application must perform the following:
  -# Add `ENABLE_HTTP_SERVER_LOGS` macro to the *DEFINES* in the code example's JSON file. The JSON file entry would look like as follows:
     \code
       "macros": ["ENABLE_HTTP_SERVER_LOGS"],
     \endcode
  -# Call the `cy_log_init()` function provided by the *cy-log* module. cy-log is part of the *connectivity-utilities* library. See [connectivity-utilities library API documentation](https://cypresssemiconductorco.github.io/connectivity-utilities/api_reference_manual/html/group__logging__utils.html) for cy-log details.

- Define the following macro in the application JSON file to configure the maximum number of HTTP server resources:
  ```
  MAX_NUMBER_OF_HTTP_SERVER_RESOURCES
  ```

## **Additional Information**
* [HTTP server RELEASE.md](./RELEASE.md)
* [HTTP server API reference guide](https://cypresssemiconductorco.github.io/http-server/api_reference_manual/html/index.html)
* [HTTP server library version](./version.txt)

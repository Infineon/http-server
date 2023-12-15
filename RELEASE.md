# HTTP Server Library

## What's Included?
Refer to the [README.md](./README.md) for a complete description of the HTTP server library.

## Known Issues
| Problem | Workaround |
| ------- | ---------- |
| Sometimes, during PUT and POST requests from clients, lwip may return Netconn Receive events with data length more than the HTTP_SERVER_MTU_SIZE (1460 bytes). In such cases, server might not receive the complete data sent by the client. | Increase the 'HTTP_SERVER_MTU_SIZE' by adding it as a Makefile DEFINE. Makefile entry example: DEFINES+=HTTP_SERVER_MTU_SIZE='2812'  |
| If the HTTP server is stopped while connected to certain clients (such as some browsers), then a duration of approximately 2-3 minutes needs to elapse (TCP wait time) prior to re-starting the HTTP server again (or it could result in socket bind to fail). | None |
| On memory constrained devices (such as CY8CKIT_062_WIFI_BT), there could be a limit on the max number of simultaneous secure connections. | None |
| IAR 9.30 toolchain throws build errors on Debug mode, if application explicitly includes iar_dlmalloc.h file | Add '--advance-heap' to LDFLAGS in application Makefile. |

## Changelog

### v3.0.0
* Supports TLS version 1.3
* Supports MBEDTLS upgrade 3.4.0
* Removed support for mbed OS
* Added support for CY8CEVAL-062S2-CYW43022CUB kit

### v2.4.0
* General bug fixes
* Added support for KIT_XMC72_EVK_MUR_43439M2 kit

### v2.3.0
* Added support for KIT_XMC72_EVK kit
* Minor bug fixes

### v2.2.2
* Updated FreeRTOS specific code to make it generic.
* Documentation updates.

### v2.2.1
* Added support for CM0P core
* Minor Documentation Updates

### v2.2.0
* Added support for CY8CEVAL-062S2-MUR-43439M2 kit

### v2.1.0
* Added support for CYW943907AEVAL1F and CYW954907AEVAL1F kits

### v2.0.1
* Minor documentation changes to add support for CY8CEVAL-062S2-LAI-4373M2 kit.

### v2.0.0
* Updated library to enable/disable RootCA validation based on the user input.
* Introduced deps folder for AnyCloud build.
* ARMC6 build support added for AnyCloud build.
* Integrated with v3.X version of wifi-mw-core library.

### v1.1.1
* Updates to support mbed-os 6.2 version

### v1.1.0
* Introduced C APIs for AnyCloud framework.
* Updated the API reference guide with code snippets for AnyCloud.
* This version of library can work on both AnyCloud and Mbed OS frameworks.

### v1.0.1
* Provision to configure the stack size of the library.
* Added API reference guide
* Tested with Arm® mbed OS 5.14.0 

### v1.0.0
* This is the first version of HTTP server library.

## Supported Software and Tools
The current version of the library was validated for compatibility with the following Software and Tools:

| Software and Tools                                        | Version |
| :---                                                      | :----:  |
| ModusToolbox&trade; Software Environment                  | 3.1     |
| ModusToolbox&trade; Device Configurator                   | 4.10    |
| GCC Compiler                                              | 11.3.1  |
| IAR Compiler (only for ModusToolbox&trade;)               | 9.30    |
| Arm Compiler 6                                            | 6.16    |

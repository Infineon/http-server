# HTTP Server Library

## What's Included?
Refer to the [README.md](./README.md) for a complete description of the HTTP server library.

## Known Issues
| Problem | Workaround |
| ------- | ---------- |
| If the HTTP server is stopped while connected to certain clients (such as some browsers), then a duration of approximately 2-3 minutes needs to elapse (TCP wait time) prior to re-starting the HTTP server again (or it could result in socket bind to fail). | None |
| On memory constrained devices (such as CY8CKIT_062_WIFI_BT), there could be a limit on the max number of simultaneous secure connections. | None |

## Changelog

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
| ModusToolbox&trade; Software Environment                  | 2.4     |
| - ModusToolbox&trade; Device Configurator                 | 3.10    |
| - ModusToolbox&trade; CapSense Configurator / Tuner tools | 4.0     |
| PSoC 6 Peripheral Driver Library (PDL)                    | 2.3.0   |
| GCC Compiler                                              | 10.3.1  |
| IAR Compiler (only for ModusToolbox&trade;)               | 8.32    |
| Arm Compiler 6                                            | 6.14    |
| MBED OS                                                   | 6.2.0   |

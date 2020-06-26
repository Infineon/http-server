# HTTP Server Library

### What's Included?
Refer to the [README.md](./README.md) for a complete description of the HTTP server library.

## Known Issues
| Problem | Workaround |
| ------- | ---------- |
| The implementation of newlib from GCC will leak ~1.4kb of heap memory per task/thread that uses stdio functions (i.e. printf, snprintf, etc.) | By default,  log messages are disabled in the HTTP server library. Refer to the [README.md](./README.md) for enabling log messages. It is recommended to enable log  messages, only for debugging purposes |

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

### Known Issues
- If the HTTP server is stopped, while connected to certain clients (such as some browsers), then a duration of approximately 2-3 minutes needs to elapse (TCP wait time) prior to re-starting the HTTP server again (or it could result in socket bind to fail).
- On memory constrained devices (such as CY8CKIT_062_WIFI_BT), there could be a limit on the max number of simultaneous secure connections.

### Supported Software and Tools
This version of the library was validated for compatibility with the following Software and Tools:

| Software and Tools                                      | Version |
| :---                                                    | :----:  |
| ModusToolbox Software Environment                       | 2.1     |
| - ModusToolbox Device Configurator                      | 2.1     |
| - ModusToolbox CSD Personality in Device Configurator   | 2.0     |
| - ModusToolbox CapSense Configurator / Tuner tools      | 3.0     |
| PSoC 6 Peripheral Driver Library (PDL)                  | 1.5.1   |
| GCC Compiler                                            | 9.2.1   |
| IAR Compiler                                            | 8.32    |
| Arm Compiler 6                                          | 6.13    |
| MBED OS                                                 | 5.15.3  |

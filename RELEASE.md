# Cypress HTTP Server Library

### What's Included?
Refer to the [README.md](./README.md) for a complete description of the HTTP server library.

### v1.0.1
* Provision to configure the stack size of the library.
* Added API reference guide
* Tested with ARMmbed OS 5.14.0 

### v1.0.0
* This is the first version of HTTP server library.

### Known Issues
- If the HTTP server is stopped, while connected to certain clients (such as some browsers), then a duration of approximately 2-3 minutes needs to elapse (TCP wait time) prior to re-starting the HTTP server again (or it could result in socket bind to fail).
- On memory constrained devices (such as CY8CKIT_062_WIFI_BT), there could be a limit on the max number of simultaneous secure connections.

### Supported Software and Tools
This version of the bluetooth gateway Middleware was validated for compatibility with the following Software and Tools:

| Software and Tools                                      | Version |
| :---                                                    | :----:  |
| GCC Compiler                                            | 7.2.1   |
| IAR Compiler                                            | 8.32    |
| ARM Compiler 6                                          | 6.11    |
| MBED OS                                                 | >5.13.4 |

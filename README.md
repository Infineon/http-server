# Cypress HTTP Server Library

### Introduction
Communication functions for HTTP (Hypertext Transfer Protocol) Server

HTTP functions as a request-response protocol in the client-server computing model. A web browser,
for example, may be the client and an application running on a computer hosting a website may be the
server. The client submits an HTTP request message to the server. The server, which provides
resources such as HTML files and other content, or performs other functions on behalf of the client,
returns a response message to the client. The response contains completion status information about
the request and may also contain requested content in its message body.
  
### Features 
Following are the features supported by this library:
- Capable of  secure [with TLS security] and non-secure modes of connection.
- Provides support for various RESTful HTTP methods such as GET and POST.
- Handles various resource content types such as HTML, Plain, JSON, etc.
- Capable of handling content payload greater than MTU size using content length HTTP header.
- Supports chunked encoding for  GET and POST methods. 
  Note: For POST request, chunked encoding is supported only for data that is less than a single MTU; 
  content length headers are recommended for larger data.

### **Supported Platforms**
This middleware library and it's features are supported on following Cypress platforms:
* [PSoC6 WiFi-BT Prototyping Kit (CY8CPROTO-062-4343W)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wi-fi-bt-prototyping-kit-cy8cproto-062-4343w)
* [PSoC6 WiFi-BT Pioneer Kit (CY8CKIT-062-WiFi-BT)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wifi-bt-pioneer-kit-cy8ckit-062-wifi-bt)
* CY8CKIT-062S2-43012

### Dependencies
This section provides the list of dependency libraries required for this middleware library to work.
* [ARM mbed-os stack version 5.13.4 and above](https://os.mbed.com/mbed-os/releases)
* [Cypress Connectivity Utilities Library](https://github.com/cypresssemiconductorco/connectivity-utilities)

### Additional Information
* [HTTP server RELEASE.md](./RELEASE.md)
* [HTTP server library version](./version.txt)

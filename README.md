# HTTP server library

Communication functions for HTTP (Hypertext Transfer Protocol) Server

HTTP functions as a request-response protocol in the client-server computing model. A web browser,
for example, may be the client and an application running on a computer hosting a website may be the
server. The client submits an HTTP request message to the server. The server, which provides
resources such as HTML files and other content, or performs other functions on behalf of the client,
returns a response message to the client. The response contains completion status information about
the request and may also contain requested content in its message body.

This HTTP server library is capable of both secure [with TLS security] and
non-secure modes of connection. The library also provides support for various RESTful HTTP methods
such as GET and POST; and has support for various content types [e.g. HTML, Plain, JSON].
The HTTP library is capable of handling content payload that is greater than MTU size using content 
length HTTP header. Chunked encoding is supported for both GET and POST operations (for POST request,
chunked encoding is supported only for data that is less than a single MTU; content length headers are
recommended for larger data).


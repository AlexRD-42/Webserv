
A connection will always either read from a client or write to a client

1) Reading from a client implies reading a HTTP header and/or body
	- Reading a HTTP body is only relevant for POST or CGI

2) Writing to a client implies writing a HTTP header and/or body
	- A write to client will always be 


When a HTTP body is present and valid, it will either:
- Be copied to a write buffer		(regular response)
- Be processed to a write buffer 	(when it is a chunked response)

When it is CGI: 
	a) The data from the write buffer will be streamed to the CGI's input
	b) Server will read from CGI's output and wait for a header (*)
	c) Once a header is received, the HTTP response is formed

*) Violates the always reads or writes to client rule:
Solutions to this are:
	- Force reading until a header is found
		Should be ok given that the headers from the CGI should be small and its local communication
	- Fork twice, one to handle CGI IO, and the other for execve
		First fork will have parallel and unrestricted access to IO, which might be a problem

Steps:

Buffer A B C D

1) Read from client into buffer A until header is formed
2) Validate the header

CGI:
3) Fork, exec CGI, set up pipeline
4) Wait for CGI Header
5) Generate HTTP response header and stream CGI output to client

CGI:
3) Generate HTTP response header
4) If its a GET, stream server file to client


If its CGI or POST:
	If it not chunked, send direct
	else parsecpy
else
	ignore body


If its 

Pipeline:

Client fd -> Buffer A -> 
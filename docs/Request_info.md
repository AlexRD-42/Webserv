/*

// Before execve'ing a CGI, the environment variables must be set with the query strings
// TODO: Create a sendfile method for buffer class

Method Parsing:
  0) Read from client into the read buffer until a HTTP header is complete
  	a) Information is streamed, parsed line by line, and validated
  	b) The read buffer may contain spill information after the header
  	c) For GET and DELETE requests, HTTP body is ignored (unless its CGI)

  For a GET Request:
  	1) Read from server files to the write buffer
  	2) Write from write buffer to the client

  For a POST Request (Regular):
    1) Read from client to the read buffer
		a) Validate path first (permission checks, ensure file path stays inside, etc)
  	2) Write from read buffer to server files

  For a POST Request (Chunked):
    1) Read from client to the read buffer
	2) Process the information and append to the write buffer
  	3) Write from write buffer to server files

  For a DELETE Request
	1) Delete file
		a) Validate path first (permission checks, ensure file path stays inside, etc)

  For a CGI Request:
	1) Create a pipe, dup CGI's input to pipe's read end and its output to the server
	2) Initial write target becomes the CGI's write end
	3) Read information written from the CGI to the ? buffer
	4) Proceed with writing to the original target
	
  A generic form is:
	1) Read and process from INPUT_FD to X buffer
	2) Write from X buffer to OUTPUT_FD

  A CGI form is:
	1) Read and process from INPUT_FD to X buffer
    2) Write from X buffer to CGI_WRITE_END
	3) Read from CGI_READ_END to Y buffer
	4) Write from Y buffer to OUTPUT_FD

  Memory model:
  	4x Buffers of 8kb each (let's call them ABCD)
	Each buffer is guaranteed to hold at least 8000 bytes, because each connection is going to be aligned to 32kb max usage

			[READ BUFFER    ] [WRITE BUFFER     ]
			[READ_A] [READ_B] [WRITE_A] [WRITE_B]

	GET/DEL	[HEADER] [BODY            ] [WRITE_A]
	GET/DEL	[HEADER] [BODY            ] [WRITE_A]	CHUNKED
	GET		[HEADER] [BODY       ] [WRITE_AB    ]	CGI
	GET		[HEADER] [BODY  ] [WRITE_A] [WRITE_B]	CGI - CHUNKED

	POST	[HEADER] [BODY       ] [WRITE_AB    ]
	POST	[HEADER] [BODY       ] [WRITE_AB    ]	CHUNKED
	POST	[HEADER] [BODY       ] [WRITE_AB    ]	CGI
	POST	[HEADER] [BODY  ] [WRITE_A] [WRITE_B]	CGI - CHUNKED

	* When there is no chunked encoding, write directly from the buffer responsible for client read

  READ_A:	Holds the header information, but also compacts within itself the target + cookies
  READ_B:	Holds the body information
  WRITE_A:	Holds the processed information to be uploaded (or sent to CGI)
  WRITE_B:	Holds the CGI output
*/

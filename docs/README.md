# webserv
An HTTP/1.1 server written in C++

## Description

## Instructions

```bash
make
./build/webserv config/penguin.conf
```

## Glossary
OOB Padding: 			Will not permanently modify
Clobberable padding:	Will permanently modify

POST/PRE refer to the location of the padding. [PRE] [DATA] [POST]

1) Prepending QUERY_STRING= in cgi setup depends on the buffer being pre-clobberable-padded with 8 bytes. QUERY_STRING= is 13 bytes
(PRE 8 Clobberable padding)
2) Most find algorithms depend on the buffer being padded with at least 4 bytes to insert a sentinel like \r\n\r\n
(POST 8 bytes OOB padding because it is restored)
3) Match algorithms require 24 bytes OOB padding
(POST 24 bytes OOB padding)

Buffer has 8 clobberable bytes before data and 8 after it. The three size counters provide another 24 readable bytes after data, for 32 bytes of physical trailing storage. Sentinel writes use only the first 8 bytes and restore them; the counters must never be clobbered

## Memory Layout
A single connection uses 16kb of space, of which 64 bytes is used by metadata, and the rest by buffers
A connection pool holds 4096 connections, totalling 64MB + ~1KB of metadata

Each virtual server takes up roughly 784 bytes of memory, plus a fixed storage space for its configs (a budget of 64KB)
This means that for 64 virtual servers, it uses 64 * (784 + 64KB) =  50KB + 4MB = ~4MB. Configured error pages also consume this shared budget.

Additionally, default error pages and status strings are cached in memory, occupying roughly 10kb of space

There are two arenas:
	* Alpha	(64MB): Memory space occupied by the connection pool;
	* Beta	(4MB): Fixed storage space budget for virtual server configs and error pages

## Parsing Invariants
For temporary things that aren't going to be used by the program later like tokens and unprocessed configs, arena alpha is used. This ensures no additional allocations or frees are necessary, since whatever was going to be used by connections is considered garbage memory before initialization.

#### General
* Comments are stripped from parsing
* The config file read is allocated with at least 64 bytes padding

#### Limits
* Each server block is at maximum MAX_SERVER_BLOCK_SIZE (64KB)
* Each location block is at maximum MAX_LOCATION_BLOCK_SIZE (32KB)
* Number of locations is at maximum MAX_LOCATION_COUNT (32767 locations)
* Each error page cached is at maximum MAX_ERROR_PAGE_SIZE (HTTP_BUFFERSIZE - 512B)

#### Location Invariants
* All stored location strings are null terminated and 0 <= length <= MAX_PATH_SIZE
* 0 length strings still point to empty data
* There are no duplicates of any kind
* CGI blocks are length-prefixed binary records. Each record stores two u16 lengths, extension bytes without a terminator, and interpreter bytes with a terminator. The interpreter length includes its terminator
* A configured redirect status is a supported 3xx status
* There is at least one allowed method
* A server root span always exists
* A server root and a location root never end with a "/"
* An upload store always ends with a "/"
* An index always starts with "/"
* A URI always starts with a "/"

#### Defaults
* If server root does not exist, it becomes ""
* If location root does not exist, it becomes server root
* If upload store does not exist, upload store becomes root with a trailing "/" (or "/" when root is empty)
* If index does not exist, it becomes "/index.html"
* If no methods are specified, it becomes GET only
* If no client_max_body_size is specified, it becomes LONG_MAX

#### Valid syntax:
* For body sizes, you can define G, M and K for GB, MB and KB respectively.
If not specified, bytes are assumed instead
Example: client_max_body_size 20M;

* CGI blocks are defined per location.
Example: 
	cgi {
		.extension = /absolute_path_to_interpreter;
		.py = /bin/python3;
	}

## Conventions adopted
1) Size type variables will always take a maximum size of LONG_MAX, even for unsigned types. 
This is done to avoid overflows and always have error sentinels.
LONG_MAX is a ridiculously large number anyhow, any real constraint should realistically be much smaller
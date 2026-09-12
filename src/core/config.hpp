#pragma once

// Server configuration
#define HTTP_METADATA_SIZE (64)
#define HTTP_BUFFERSIZE (8192 - HTTP_METADATA_SIZE / 2)
#define MAX_VIRTUAL_SERVERS (64)	// TODO: This isn't really configurable yet
#define HTTP_TIMEOUT 60

#define MAX_SERVER_BLOCK_SIZE UINT16_MAX
#define MAX_LOCATION_BLOCK_SIZE INT16_MAX
#define MAX_LOCATION_COUNT INT16_MAX

#define CONFIG_POOL_SIZE (MAX_SERVER_BLOCK_SIZE * MAX_VIRTUAL_SERVERS)

// Server parameters
#define HTTP_INDEX_NAME_LENGTH 52
#define HTTP_DIRENT_MAX_SIZE (3 * 255 + HTTP_INDEX_NAME_LENGTH * 6 + 100)
#define HTTP_MAX_ERROR_PAGE_SIZE (HTTP_BUFFERSIZE - 512)

// Kernel configurations
#define MAX_FILE_SIZE (1ul << 48)	// 256 TiB
#define MAX_PATH_SIZE (4096ul)
#ifdef PIPE_BUF
	#if PIPE_BUF > 4096
		#define ATOMIC_IOSIZE 4096
	#else
		#define ATOMIC_IOSIZE PIPE_BUF
	#endif
#else
	#ifdef _POSIX_PIPE_BUF
		#define ATOMIC_IOSIZE _POSIX_PIPE_BUF
	#else
		#define ATOMIC_IOSIZE 512
	#endif
#endif

#pragma once
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>

#include "core.hpp"
#include "pure_functions.hpp"
#include "webserv.hpp"
#include "Span.hpp"
#include "Status.hpp"
#include "Array.hpp"
#include "Environment.hpp"

struct Location {
	Span16	uri;
	Span16	root;
	Span16	index;
	Span16	uploadStore;
	Span16	cgiBlock;
	Span16	redirectTarget;
	Status	redirectStatus;
	u8		methods;
	bool	autoindex;

	Span extract(const Span16 &span) {
		Span result = {(char*)this + span.index, span.size};
		ASSERT(result.ptr[result.size] == '\0', "Location span is not null terminated");
		return result;
	}

	Span get_uri()				{ return extract(uri); }
	Span get_root() 			{ return extract(root); }
	Span get_index()			{ return extract(index); }
	Span get_upload_store()		{ return extract(uploadStore); }
	Span get_cgi_block()		{ return extract(cgiBlock); }
	Span get_redirect_target()	{ return extract(redirectTarget); }
};

class VirtualServer {
public:
	Span			serverRoot;
	Span			errorPages[Status::errorPageCount];
	Span			host;
	ArrayView<Location>	locations;
	usize			port;
	usize			maxBodySize;
	void*			gameState;
	int 			listenFd;

	VirtualServer()
		: serverRoot(Span::create("")), host(), locations(), port(SIZE_MAX),
		maxBodySize(SIZE_MAX), gameState(NULL), listenFd(-1) {
		MEMSET_INLINE(errorPages, 0, sizeof(errorPages));
	}

	~VirtualServer() {
		clear();
	}

	int clear() {
		if (listenFd != -1) {
			close(listenFd);
			listenFd = -1;
		}
		gameState = NULL;
		return 1;
	}

	void init() {
		if (listenFd != -1)
			clear();

		ASSERT(port >= 1 && port <= 65535, "Invalid virtual server port");

		listenFd = socket(AF_INET, SOCK_STREAM, 0);
		if (listenFd == -1)
			PERR_EXIT(clear(), "Error: Failed to create listening socket");
		if (fn::set_stream_mode(listenFd))
			PERR_EXIT(clear(), "Error: Failed to configure listening socket");

		int reuse = 1;
		if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
			PERR_EXIT(clear(), "Error: Failed to configure listening socket");

		sockaddr_in address = {};
		address.sin_family = AF_INET;
		address.sin_port = htons((u16) port);
		if (LITCMP(host.ptr, "localhost") == 0)
			address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		else if (inet_pton(AF_INET, host.ptr, &address.sin_addr) != 1)
			PERR_EXIT(clear(), "Error: Failed to resolve virtual server host");
		if (bind(listenFd, (sockaddr*) &address, sizeof(address)) == -1)
			PERR_EXIT(clear(), "Error: Failed to bind listening socket");
		if (listen(listenFd, SOMAXCONN) == -1)
			PERR_EXIT(clear(), "Error: Failed to listen on socket");
	}
};

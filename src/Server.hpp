#pragma once
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "core.hpp"
#include "webserv.hpp"
#include "Epoll.hpp"
#include "ConnectionPool.hpp"
#include "VirtualServer.hpp"
#include "Parser.hpp"
#include "Clock.hpp"
#include "Environment.hpp"

volatile sig_atomic_t gRunning = 1;
extern "C" void handle_sigint(int) {
    gRunning = 0;
}

__attribute__((constructor))
void init(int argc, char** argv, char** envp) {
	(void) argc, (void)argv, (void) envp;

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		PERR_EXIT(1, "Error: Failed to configure SIGPIPE handling");
	if (signal(SIGINT, handle_sigint) == SIG_ERR)
		PERR_EXIT(1, "Error: Failed to configure SIGINT handling");
	Clock::init();
	Environment::init(envp);
}

#define SERVER_INL(ret_type) ret_type inline Server::

struct Server {
	ConnectionPool connections;
	u8 storage[CONFIG_POOL_SIZE];
	Arena alpha, beta;

	VirtualServer servers[MAX_VIRTUAL_SERVERS];
	Parser parser;
	Epoll epoll;

	void init(const char* filePath) {
		alpha = {(u8*)&connections, 0, sizeof(connections)};
		beta = {storage, 0, sizeof(storage)};
		parser.init(filePath, servers, alpha, beta);
		connections.reset();
		epoll.init();
		if (epoll.fd == -1)
			PERR_EXIT(clear(), "Error: Failed to create epoll");

		for (usize index = 0; index < parser.serverCount; index++)
			servers[index].init();
	}

	void run() {
		struct epoll_event* event;

		for (u32 serverIndex = 0; serverIndex < parser.serverCount; serverIndex++) {
			if (epoll.add(servers[serverIndex].listenFd, EPOLLIN, UINT32_MAX, serverIndex))
				PERR_EXIT(clear(), "Error: Failed to add listening socket event");
		}
		PRINT_LN(1, "Webserv configuration parsed, server is now running (CTRL-C to end)");
		while (gRunning == true) {
			const usize eventCount = epoll.wait(1000);	// REVIEW
			if (eventCount == SIZE_MAX) {
				if (errno == EINTR)
					continue;
				PERR_EXIT(clear(), "Error: epoll_wait failed");
			}

			Clock::update();
			for (usize eventIndex = 0; eventIndex < eventCount; eventIndex++) {
				event = epoll.get_event(eventIndex);
				if ((u32)event->data.u64 == UINT32_MAX)
					server_event(event->data.u64);
				else
					connection_event(event->data.u64);
			}
			check_timeouts();
		}
		clear();
		PRINT_LN(1, "\nWebserv process has been terminated");
		_exit(0);
	}

	void reap_children() {
		usize elementIndex;

		for (usize i = 0; i < connections.blockCount; i++) {
			Bitmap bitmap = connections.map.del[i];
			const usize outerIndex = 64 * i;

			while ((elementIndex = bitmap.pop_first_set()) != WORD_BITS) {
				const usize linearIndex = outerIndex + elementIndex;
				Connection& connection = connections.connections[linearIndex];

				if (connection.processId != -1) {
					const pid_t result = waitpid(connection.processId, NULL, WNOHANG);
					if (result == 0 || (result == -1 && errno == EINTR))
						continue;
				}

				connections.map.del[i].bitclr(elementIndex);
				connections.free_slot(linearIndex);
			}
		}
	}

	void check_timeouts() {
		time_t timeNow = Clock::update();
		usize elementIndex;

		for (usize i = 0; i < connections.blockCount; i++) {
			Bitmap bitmap = connections.map.element[i];
			bitmap.value &= ~connections.map.del[i];
			usize outerIndex = 64 * i;
			while ((elementIndex = bitmap.pop_first_set()) != WORD_BITS) {
				usize linearIndex = outerIndex + elementIndex;
				if (timeNow - connections.connections[linearIndex].startTime > HTTP_TIMEOUT)
					connections.mark_for_deletion(linearIndex);
			}
		}
		reap_children();
	}

	int clear() {
		epoll.clear();
		connections.clear();
		for (usize index = 0; index < parser.serverCount; index++)
			servers[index].clear();
		parser.serverCount = 0;
		alpha.clear();
		beta.clear();
		return 1;
	}

	// Handling
	void server_event(u64 key);
	void connection_event(u64 key);
	void add_connection(u32 serverIndex);
	void remove_connection(u32 connectionIndex);
};

#include "Server_dispatch.ipp"

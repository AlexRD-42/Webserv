#pragma once
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "core.hpp"
#include "pure_functions.hpp"

struct Epoll {
	static const usize maxEvents = 64;

	i32 fd;
	usize index;
	struct epoll_event eventList[maxEvents];

	void init() {
		index = 0;
		fd = epoll_create1(EPOLL_CLOEXEC);
		if (fd == -1)
			return ;
	}

	void clear() {
		fd = fn::close_noerr(fd);
		MEMSET_INLINE(eventList, 0, sizeof(eventList));
	}

	struct epoll_event* get_event(usize srcIndex) {
		index = srcIndex;
		return eventList + index;
	}

	usize wait(int timeout) {
		return (usize)epoll_wait(fd, eventList, maxEvents, timeout);
	}

	bool add(i32 targetFd, u32 events, u32 connectionIndex, u32 serverIndex) {
		eventList[index].events = events;
		eventList[index].data.u64 = ((u64)serverIndex << 32) | (u64)connectionIndex;
		return epoll_ctl(fd, EPOLL_CTL_ADD, targetFd, eventList + index) == -1;
	}

	bool remove(i32 targetFd) {
		return epoll_ctl(fd, EPOLL_CTL_DEL, targetFd, NULL) == -1;
	}

	bool modify(i32 targetFd, u8 newState, u8 &curState) {
		if (newState == curState)
			return false;
		struct epoll_event newEvent = eventList[index];
		newEvent.events = (u32) newState;
		if (epoll_ctl(fd, EPOLL_CTL_MOD, targetFd, &newEvent) == -1)
			return true;
		curState = newState;
		return false;
	}

	bool request_read() {
		bool canRead = !!(eventList[index].events & EPOLLIN);
		eventList[index].events &= ~(u32)EPOLLIN;
		return canRead;
	}

	bool request_write() {
		bool canWrite = !!(eventList[index].events & EPOLLOUT);
		eventList[index].events &= ~(u32)EPOLLOUT;
		return canWrite;
	}

	bool is_error() {
		return !!(eventList[index].events & (EPOLLERR | EPOLLHUP));
	}
};

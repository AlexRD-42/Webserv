#pragma once
#include "Connection.hpp"

/*
	Attempts to open the target path given by client
	If it's a directory, routes to get_directory_setup
	If that fails, routes to get_autoindex_setup (if appropriate/allowed)
*/
CONNECTION_INL
(isize) get_setup(Epoll& epoll) {
	Buffer64 pathBuffer = {};
	append_target_path(pathBuffer);
	if (epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;
	struct stat st;
	readFd = fn::open_with_info(AT_FDCWD, &st, pathBuffer, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (readFd == -1)
		return flush_setup_close(epoll, s_get_status());
	if (S_ISDIR(st.st_mode))
		return get_directory_setup(epoll, pathBuffer);
	if (fn::validate_file(readFd, &st) == -1)
		return flush_setup_close(epoll, Status::i500);
	bodySize = (usize)st.st_size;
	contentType = fn::match_mime(pathBuffer.get_span());
	activate_streaming(Mode::GET);
	build_header(Status::i200);
	return upload_file(epoll);
}

// TODO: The pathbuffer append can go away once unified buffer for get is working
CONNECTION_INL
(isize) get_redirect_setup(Epoll& epoll, Buffer64& pathBuffer) {
	char* target = pathBuffer.append(req.target);
	pathBuffer.append("/");
	if (req.query.size != 0) {
		pathBuffer.append("?");
		pathBuffer.append(req.query);
	}
	const usize targetSize = (usize)(pathBuffer.wptr() - target);
	options &= ~(u16)Options::KEEP_ALIVE;
	activate_streaming(Mode::FLUSH);
	sendBuffer.append("HTTP/1.1 301 Moved Permanently\r\nLocation: ");
	sendBuffer.append(target, targetSize);
	sendBuffer.append("\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
	return flush_setup(epoll);
}

CONNECTION_INL
(isize) get_directory_setup(Epoll& epoll, Buffer64& pathBuffer) {
	if (req.target.ptr[req.target.size - 1] != '/')
		return get_redirect_setup(epoll, pathBuffer);
	const Span index = req.location->get_index();	// Index span will either be index.html or the one supplied by the config
	struct stat st;
	int indexFd = fn::open_with_info(readFd, &st, index.ptr, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (indexFd == -1) {
		if (errno != ENOENT && errno != ENOTDIR)
			return flush_setup_close(epoll, s_get_status());
		if (req.location->autoindex == false)
			return flush_setup_close(epoll, Status::i403);
		return get_autoindex_setup(epoll, pathBuffer);
	}
	fn::close_noerr(readFd);
	readFd = indexFd;
	if (fn::validate_file(readFd, &st) == -1)
		return flush_setup_close(epoll, Status::i500);
	contentType = fn::match_mime(index);
	bodySize = (usize)st.st_size;
	activate_streaming(Mode::GET);
	build_header(Status::i200);
	return upload_file(epoll);
}

CONNECTION_INL
(isize) get_autoindex_setup(Epoll& epoll, Buffer64& pathBuffer) {
	contentType = Mime::HTML;
	options &= ~(u16)Options::KEEP_ALIVE;
	// Its unfortunate that we have to append then copy again, but compaction might destroy target
	// TODO: Might not be needed if autoindex doesn't transform buffers
	char* targetClean = pathBuffer.append_html(req.target.ptr, req.target.size);
	usize targetCleanSize = (usize)(pathBuffer.wptr() - targetClean);
	const usize fixedSize = sizeof(HTTP_INDEX_HEADER HTTP_INDEX_MIDDLE HTTP_INDEX_TAIL);
	if (fixedSize + targetCleanSize * 2 > sizeof(sendBuffer.data))
		return flush_setup_close(epoll, Status::i414);
	activate_streaming(Mode::AUTOINDEX);
	recvBuffer.clear();	// Reuse receive storage for directory records, response closes the connection
	sendBuffer.append(HTTP_INDEX_HEADER);
	sendBuffer.append(targetClean, targetCleanSize);
	sendBuffer.append(HTTP_INDEX_MIDDLE);
	sendBuffer.append(targetClean, targetCleanSize);
	sendBuffer.append(HTTP_INDEX_TAIL);
	return upload_directory(epoll);
}

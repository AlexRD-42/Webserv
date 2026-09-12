#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) get_setup(Epoll &epoll) {
	Buffer64 pathBuffer = {};
	append_target_path(pathBuffer);

	if (epoll.modify(clientFd, EPOLLOUT, epollState))
		return -1;

	struct stat st;
	if (stat(pathBuffer, &st) == -1)
		return flush_setup_close(epoll, s_get_status());
	if (S_ISDIR(st.st_mode))
		return get_directory_setup(epoll, pathBuffer);
	if (!S_ISREG(st.st_mode))
		return flush_setup_close(epoll, Status::i500);
	readFd = open(pathBuffer, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	if (readFd == -1)
		return flush_setup_close(epoll, s_get_status());
	bodySize = (usize)st.st_size;
	contentType = fn::match_mime(pathBuffer.get_span());
	activate_streaming(Mode::GET);
	build_header(Status::i200);
	return upload_file(epoll);
}

CONNECTION_INL
(isize) get_directory_setup(Epoll &epoll, Buffer64 &pathBuffer) {
	const usize directoryLength = pathBuffer.writePos;
	const Span index = req.location->get_index();
	const bool hasSlash = pathBuffer.data[pathBuffer.writePos - 1] == '/';
	pathBuffer.append(index.ptr + hasSlash, index.size - hasSlash);
	*pathBuffer = 0;
	readFd = open(pathBuffer, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	if (readFd >= 0) {
		struct stat st;
		if (fstat(readFd, &st) == -1)
			return flush_setup_close(epoll, s_get_status());
		if (!S_ISREG(st.st_mode))
			return flush_setup_close(epoll, Status::i500);
		contentType = fn::match_mime(pathBuffer.get_span());
		bodySize = (usize)st.st_size;
		activate_streaming(Mode::GET);
		build_header(Status::i200);
		return upload_file(epoll);
	}
	if (errno != ENOENT && errno != ENOTDIR)
		return flush_setup_close(epoll, s_get_status());	// REVIEW
	pathBuffer.writePos = directoryLength;
	*pathBuffer = 0;
	if (req.location->autoindex == false)
		return flush_setup_close(epoll, Status::i403);
	const usize targetSize = fn::html_encoded_size(req.target.ptr, req.target.size);
	const usize fixedSize = sizeof(HTTP_INDEX_HEADER) + sizeof(HTTP_INDEX_MIDDLE) + sizeof(HTTP_INDEX_TAIL) - 3;
	const usize headerSize = fixedSize + targetSize * 2;
	if (headerSize > sizeof(sendBuffer.data))
		return flush_setup_close(epoll, Status::i414);
	readFd = open(pathBuffer, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (readFd == -1)
		return flush_setup_close(epoll, s_get_status());
	contentType = Mime::HTML;
	options &= ~(u16)Options::KEEP_ALIVE;
	// Its unfortunate that we have to append then copy again, but compaction might destroy target 
	char* targetClean = pathBuffer.append_html(req.target.ptr, req.target.size);
	usize targetCleanSize = (usize)(pathBuffer.wptr() - targetClean);

	activate_streaming(Mode::AUTOINDEX);
	recvBuffer.clear();	// Reuse receive storage for directory records; this response closes the connection
	sendBuffer.append(HTTP_INDEX_HEADER);
	sendBuffer.append(targetClean, targetCleanSize);
	sendBuffer.append(HTTP_INDEX_MIDDLE);
	sendBuffer.append(targetClean, targetCleanSize);
	sendBuffer.append(HTTP_INDEX_TAIL);
	return upload_directory(epoll);
}

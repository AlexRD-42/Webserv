#pragma once
#include "Connection.hpp"

// Can only call this once all the req variables have been used
CONNECTION_INL
(void) activate_streaming(Mode::e_http_mode nextMode) {
	ASSERT(mode <= Mode::PARSE, "Parsing buffer was not active");
	ASSERT(nextMode > Mode::PARSE, "Invalid streaming mode");
	ASSERT(parseBuffer.size() <= sizeof(recvBuffer.data), "Buffered request tail exceeded receive buffer");
	if (parseBuffer.writePos > sizeof(recvBuffer.data))
		parseBuffer.compact();
	mode = nextMode;
	recvBuffer.init(parseBuffer.writePos, parseBuffer.readPos, parseBuffer.scanPos);
	sendBuffer.clear();
}

CONNECTION_INL
(void) activate_parsing() {
	ASSERT(mode == Mode::FLUSH, "Streaming buffer was not active");
	ASSERT(sendBuffer.size() == 0, "Response was not fully flushed");
	recvBuffer.compact();
	mode = Mode::PARSE_FIRST;
	parseBuffer.init(recvBuffer.writePos, recvBuffer.readPos, recvBuffer.scanPos);
	req.clear();
}

CONNECTION_INL
(char*) append_target_path(Buffer64 &buffer) {
	const Span root = req.location->get_root();
	const Span uri = req.location->get_uri();

	char* fullPath = buffer.append(root);
	const Span suffix = {req.target.ptr + uri.size, req.target.size - uri.size};
	if (suffix.size != 0) {
		if (suffix.ptr[0] != '/')
			buffer.append("/");
		buffer.append(suffix);
	}
	else if (root.size == 0)
		buffer.append("/");
	*buffer = 0;
	return fullPath;
}

static inline
Status::Code s_get_status() {
	Status::Code code;
	const int error = errno;

	if (error == ENOENT || error == ENOTDIR)
		code = Status::i404;
	else if (error == EACCES || error == EPERM || error == EROFS)
		code = Status::i403;
	else if (error == EEXIST || error == ENOTEMPTY || error == EBUSY)
		code = Status::i409;
	else if (error == ENAMETOOLONG)
		code = Status::i414;
	else if (error == ENOSPC || error == EDQUOT)
		code = Status::i507;
	else if (error == EMFILE || error == ENFILE || error == ENOMEM)
		code = Status::i503;
	else
		code = Status::i500;
	errno = 0;
	return code;
}

CONNECTION_INL
(isize) init(int fd, VirtualServer* serverConfig) {
	clientFd = fd;
	cfg = serverConfig;
	readFd = -1;
	writeFd = -1;
	processId = -1;
	epollState = EPOLLIN;
	options = 0;
	contentType = Mime::OCTET_STREAM;
	bodySize = 0;
	chunkSize = 0;
	mode = Mode::PARSE_FIRST;
	parseBuffer.clear();
	req.clear();
	startTime = Clock::time_elapsed();
	return 1;
}

CONNECTION_INL
(isize) end_connection() {
	clear();
	if (clientFd >= 0)
		close(clientFd);
	clientFd = -1;
	if (processId != -1)
		kill(processId, SIGKILL);
	return -1;
}

CONNECTION_INL
(void) clear() {
	if (readFd >= 0)
		close(readFd);
	if (writeFd >= 0)
		close(writeFd);
	readFd = -1;
	writeFd = -1;
}

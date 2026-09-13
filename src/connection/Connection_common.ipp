#pragma once
#include "Connection.hpp"

// Can only call this once all the req variables have been used
CONNECTION_INL
(void) activate_streaming(Mode nextMode) {
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
	const int error = errno;
	errno = 0;

	if (error == ENOENT || error == ENOTDIR)
		return Status::i404;
	if (error == EACCES || error == EPERM || error == EROFS || error == EISDIR)
		return Status::i403;
	if (error == EEXIST || error == ENOTEMPTY || error == EBUSY)
		return Status::i409;
	if (error == ENAMETOOLONG)
		return Status::i414;
	if (error == ENOSPC || error == EDQUOT)
		return Status::i507;
	if (error == EMFILE || error == ENFILE || error == ENOMEM)
		return Status::i503;
	return Status::i500;
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
(isize) clear() {
	readFd = fn::close_noerr(readFd);
	writeFd = fn::close_noerr(writeFd);
	clientFd = fn::close_noerr(clientFd);
	if (processId != -1)
		kill(processId, SIGKILL);
	return -1;
}

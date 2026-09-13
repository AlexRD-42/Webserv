#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) write_to_client(Epoll& epoll) {
	if (epoll.request_write())
		return sendBuffer.write(clientFd, ATOMIC_IOSIZE);
	return 0;
}

CONNECTION_INL
(isize) read_from_client(Epoll& epoll) {
	if (!epoll.request_read())
		return 0;
	const isize bytesRead = recvBuffer.read_compact(clientFd, ATOMIC_IOSIZE);
	if (bytesRead == -2)
		return flush_setup_close(epoll, Status::i413);
	return bytesRead == 0 ? -1 : bytesRead;
}

CONNECTION_INL
(Status::Code) write_chunked() {
	HTTP_Buffer tmpBuffer = {};
	Status::Code code = recvBuffer.dechunk(tmpBuffer, chunkSize, bodySize);
	if (code >= Status::i400)
		return code;

	const usize bytesToWrite = tmpBuffer.size();
	if (bytesToWrite == 0)
		return code;
	if (tmpBuffer.write_all(writeFd, bytesToWrite) != (isize)bytesToWrite)
		return Status::i500;
	return code;
}

CONNECTION_INL
(isize) flush(Epoll& epoll) {
	isize bytesWritten = write_to_client(epoll);
	if (sendBuffer.size() > 0)
		return bytesWritten;
	if (options & Options::KEEP_ALIVE)
		return parse_setup(epoll);
	return -1;	// Close the connection
}

CONNECTION_INL
(isize) download_file_fixed(Epoll& epoll) {
	if (read_from_client(epoll) < 0)
		return -1;
	isize bytesWritten = 0;
	if (bodySize != 0 && recvBuffer.size() != 0) {
		bytesWritten = recvBuffer.write_all(writeFd, bodySize);
		if (bytesWritten < 0)
			return flush_setup_close(epoll, Status::i500);
		bodySize -= (usize)bytesWritten;
	}
	if (bodySize == 0) {
		build_header(Status::i201);
		return flush_setup(epoll);
	}
	return 0;
}

CONNECTION_INL
(isize) download_file_chunked(Epoll& epoll) {
	if (read_from_client(epoll) < 0)
		return -1;
	Status::Code code = write_chunked();
	if (code >= Status::i400)
		return flush_setup_close(epoll, code);
	if (code == Status::ok) {
		bodySize = 0;
		build_header(Status::i201);
		return flush_setup(epoll);
	}
	return 0;
}

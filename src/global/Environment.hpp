#pragma once
#include <unistd.h>
#include "core.hpp"

// Needed for cookies and queries
struct Environment {
	Environment() = delete;
	static const usize envSize = 1024;
	static const usize minElements = 64;

	stinl char* envp[envSize] = {};
	stinl char** optr = envp;
	stinl char** writePtr = envp;

	static void append(char* ptr) {
		ASSERT(writePtr < envp + envSize - 1, "Environment buffer overflow");
		*writePtr++ = ptr;
		*writePtr = NULL;
	}

	static void reset() {
		writePtr = optr;
		*optr = NULL;
	}

	static void init(char* const* envpSrc) {
		optr = envp;
		char** endPtr = envp + envSize - minElements;
		while (optr < endPtr && *envpSrc != NULL)
			*optr++ = *envpSrc++;
		*optr = NULL;
		writePtr = optr;
	}
};

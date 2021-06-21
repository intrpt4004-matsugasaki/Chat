#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern const int ERROR_MESSAGE_OUTSET_LENGTH;
extern const char ERROR_MESSAGE_OUTSET[];
extern const char ERROR_MESSAGE_INDENT[];

typedef struct Error {
	bool _error;
	char *_Message;

	bool (* IsError)(struct Error *);
	void (* SetMessage)(struct Error *, const char *message);
	char * (* GetMessage)(struct Error *);
	void (* ExitIfError)(struct Error *);

	void (* Delete)(struct Error *);
} Error;

extern Error * NewError(const bool error);

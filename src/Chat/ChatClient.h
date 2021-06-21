#pragma once

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>

#include "../Error/Error.h"
#include "ChatData.h"

typedef struct {
	Error *E;
	Message Message;
} Return_GetMessage;

typedef struct ChatClient {
	char *_Host;
	int _Port;
	int _Socket;
	fd_set _FDState;

	char *_Handle;

	Error * (* Login)(struct ChatClient *, const char *handle);
	char * (* GetHandle)(struct ChatClient *);
	Error * (* Write)(struct ChatClient *, const char *message);
	bool (* ConfirmUpdate)(struct ChatClient *);
	Return_GetMessage * (* GetMessage)(struct ChatClient *);
	Error * (* Logout)(struct ChatClient *);

	void (* Delete)(struct ChatClient *);
} ChatClient;

typedef struct {
	Error *E;
	ChatClient *ChatClient;
} Return_NewChatClient;

extern Return_NewChatClient * NewChatClient(const char *host, const int port);

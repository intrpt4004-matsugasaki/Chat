#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>

#include "../Error/Error.h"
#include "ChatData.h"

typedef struct ChatServer {
	int _Port;

	Error * (* Start)(struct ChatServer *);
} ChatServer;

typedef struct {
	Error *E;
	ChatServer *ChatServer;
} Return_NewChatServer;

extern Return_NewChatServer * NewChatServer(const int port);

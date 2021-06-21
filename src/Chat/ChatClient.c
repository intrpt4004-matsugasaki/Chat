#include "ChatClient.h"

/**
 * 入場処理を行う函数
 */
static Error * Login(ChatClient *self, const char *handle) {
	Error *err;

	/* ハンドル名の設定 */
	self->_Handle = (char *)(malloc(sizeof(char) * (strlen(handle) + 1)));
		err = NewError(self->_Handle == NULL);
		if (err->IsError(err)) { err->SetMessage(err, "malloc failed."); return err; }
	strcpy(self->_Handle, handle);

	/* 接続の確立 */
	// 名前解決
	struct hostent *hent = gethostbyname(self->_Host);
		err = NewError(hent == NULL);
		if (err->IsError(err)) { err->SetMessage(err, "gethostbyname failed."); return err; }

	// 接続情報構成
	struct sockaddr_in addr_in;
	memset(&addr_in, 0, sizeof(addr_in));

	addr_in.sin_family	= AF_INET;
	addr_in.sin_port	= htons(self->_Port);
	memcpy((struct in_addr *)(&addr_in.sin_addr), hent->h_addr_list[0], hent->h_length);

	// ソケット作成
	self->_Socket = socket(PF_INET, SOCK_STREAM, 0);
		err = NewError(self->_Socket == -1);
		if (err->IsError(err)) { err->SetMessage(err, "socket failed."); return err; }

	// 接続
	int result = connect(self->_Socket, (struct sockaddr *)(&addr_in), sizeof(addr_in));
		err = NewError(result == -1);
		if (err->IsError(err)) { err->SetMessage(err, "connect failed."); return err; }

	/* ログイン */
	char str[DATA_MAX_SIZE];

	// "Login.Request/{handle_name}" を送信
	sprintf(str, "Login.Request/%s%s", self->_Handle, CRLF);
	result = send(self->_Socket, str, strlen(str), 0);
		err = NewError(result == -1);
		if (err->IsError(err)) { err->SetMessage(err, "send failed."); return err; }

	// "Login.Succeeded" または "LoginFailed/{message}" を受信
	memset(str, '\0', sizeof(str));

	result = recv(self->_Socket, str, DATA_MAX_SIZE - 1, 0);
		err = NewError(result == -1);
		if (err->IsError(err)) { err->SetMessage(err, "recv failed."); return err; }
	str[strlen(str)] = '\0';

	// ログイン失敗検査
	if (strstr(str, "Login.Failed/") != NULL) {
		/* ログイン失敗 */
		char str_1[DATA_MAX_SIZE];
		sprintf(str_1, "入場失敗\n");
		sprintf(str_1 + strlen(str_1), "%s%s", ERROR_MESSAGE_INDENT, str + strlen("Login.Failed/"));
		str_1[strlen(str_1) - 1] = '\0'; // 末尾LF削除
		str_1[strlen(str_1) - 1] = '\0'; // 末尾CR削除

		err = NewError(true);
		if (err->IsError(err)) { err->SetMessage(err, str_1); return err; }
	}

	/* ソケットの監視を開始 */
	FD_SET(self->_Socket, &self->_FDState);

	return NewError(false);
}

/**
 * ハンドル名を返す函数
 */
static char * GetHandle(ChatClient *self) {
	return self->_Handle;
}

/**
 * 書込処理を行う函数
 */
static Error * Write(ChatClient *self, const char *message) {
	Error *err;

	/* 書き込み */
	char str[DATA_MAX_SIZE];
	int result;

	// "Write.Request/{message}" を送信
	sprintf(str, "Write.Request/%s%s", message, CRLF);
	result = send(self->_Socket, str, strlen(str), 0);
		err = NewError(result == -1);
		if (err->IsError(err)) { err->SetMessage(err, "send failed."); return err; }

	// 応答受信
	memset(str, '\0', sizeof(str));

	result = recv(self->_Socket, str, DATA_MAX_SIZE - 1, 0);
		err = NewError(result == -1);
		if (err->IsError(err)) { err->SetMessage(err, "recv failed."); return err; }
	str[strlen(str) + 1] = '\0';

	// 書き込み失敗検査
	if (strstr(str, "Write.Failed/") != NULL) {
		char str_1[DATA_MAX_SIZE];
		sprintf(str_1, "書き込み失敗\n");
		sprintf(str_1 + strlen(str_1), "%s%s", ERROR_MESSAGE_INDENT, str + strlen("Write.Failed/"));
		str_1[strlen(str_1) - 1] = '\0'; // 末尾LF削除
		str_1[strlen(str_1) - 1] = '\0'; // 末尾CR削除

		err = NewError(true);
		if (err->IsError(err)) { err->SetMessage(err, str_1); return err; }
	}

	return NewError(false);
}

/**
 * 更新確認を行う函数
 */
static bool ConfirmUpdate(ChatClient *self) { // TODO: 偶に反映されない?
	fd_set tmp = self->_FDState;
	select(self->_Socket + 1, &tmp, NULL, NULL, &(struct timeval){ .tv_sec = 0, .tv_usec = 0 });

	return FD_ISSET(self->_Socket, &tmp);
}

/**
 * メッセージ取得を行う函数
 */
static Return_GetMessage * GetMessage(ChatClient *self) { // TODO: 全般的に受信に使える汎用性のある函数に書き変える
	Error *err;

	if (!self->ConfirmUpdate(self)) return &(Return_GetMessage){ NewError(false) };

	/* 新規メッセージ受信 */
	char str[DATA_MAX_SIZE];
	int result;

	// 応答受信
	result = recv(self->_Socket, str, DATA_MAX_SIZE - 1, 0);
		err = NewError(result == -1);
		if (err->IsError(err)) { err->SetMessage(err, "recv failed."); return &(Return_GetMessage){ err }; }

		/* 切断検知 */
		err = NewError(result == 0);
		if (err->IsError(err)) { err->SetMessage(err, "接続が切断されました"); return &(Return_GetMessage){ err }; }

	str[strlen(str) + 1] = '\0';

	// 応答解析 "Message/{handle_name}/{message}"
	char str_1[DATA_MAX_SIZE];
	strcpy(str_1, str);
	strtok(str_1, "/");
	char *handle = strtok(NULL, "/");
		err = NewError(handle == NULL);
		if (err->IsError(err)) { err->SetMessage(err, "handle strtok failed."); return &(Return_GetMessage){ err }; }

	char str_2[DATA_MAX_SIZE];
	strcpy(str_2, str);
	strtok(str_2, "/");
	strtok(NULL, "/");
	char *message = strtok(NULL, "/");
		err = NewError(message == NULL);
		if (err->IsError(err)) { err->SetMessage(err, "message strtok failed."); return &(Return_GetMessage){ err }; }
	strtok(message, CRLF);

	// 返り値の構成
	Message msg;
	msg.Handle = (char *)(malloc(sizeof(char) * (strlen(handle) + 1)));
		err = NewError(msg.Handle == NULL);
		if (err->IsError(err)) { err->SetMessage(err, "malloc failed."); return &(Return_GetMessage){ err }; }
	strcpy(msg.Handle, handle);

	msg.Message = (char *)(malloc(sizeof(char) * (strlen(message) + 1)));
		err = NewError(msg.Message == NULL);
		if (err->IsError(err)) { err->SetMessage(err, "malloc failed."); return &(Return_GetMessage){ err }; }
	strcpy(msg.Message, message);

	return &(Return_GetMessage){ NewError(false), msg };
}

/**
 * 退出処理を行う函数
 */
static Error * Logout(ChatClient * self) {
	Error *err;

	/* ログアウト */
	char *str;
	int result;

	// Logout.Request を送信
	str = (char *)(malloc(sizeof(char) * DATA_MAX_SIZE));
	sprintf(str, "Logout.Request%s", CRLF);
	result = send(self->_Socket, str, strlen(str), 0);
		err = NewError(result == -1);
		if (err->IsError(err)) { err->SetMessage(err, "send failed."); return err; }

	// 応答受信
	memset(str, '\0', sizeof(str));

	result = recv(self->_Socket, str, DATA_MAX_SIZE - 1, 0);
		err = NewError(result == -1);
		if (err->IsError(err)) { err->SetMessage(err, "recv failed."); return err; }
	str[strlen(str) + 1] = '\0';

	// ログアウト失敗検査
	if (strstr(str, "Logout.Failed/") != NULL) {
		/* 失敗の場合 */
		char str_1[DATA_MAX_SIZE];
		sprintf(str_1, "退出失敗\n");
		sprintf(str_1 + strlen(str_1), "%s%s", ERROR_MESSAGE_INDENT, str + strlen("Logout.Failed/"));
		str_1[strlen(str_1) - 1] = '\0'; // 末尾LF削除
		str_1[strlen(str_1) - 1] = '\0'; // 末尾CR削除

		err = NewError(true);
		if (err->IsError(err)) { err->SetMessage(err, str_1); return err; }
	}

	// サーバと切断
	close(self->_Socket);

	return NewError(false);
}

/**
 * ChatClientの解体を行う函数
 */
static void Delete(ChatClient *self) {
	free(self->_Handle);
	free(self->_Host);
	free(self);
}

/**
 * ChatClientの構築を行う函数
 */
extern Return_NewChatClient * NewChatClient(const char *host, const int port) {
	Error *err;

	ChatClient *cli = (ChatClient *)(malloc(sizeof(ChatClient)));
		err = NewError(cli == NULL);
		if (err->IsError(err)) { err->SetMessage(err, "malloc failed."); return &(Return_NewChatClient){ err }; }

	cli->Login = Login;
	cli->GetHandle = GetHandle;
	cli->Write = Write;
	cli->ConfirmUpdate = ConfirmUpdate;
	cli->GetMessage = GetMessage;
	cli->Logout = Logout;

	cli->Delete = Delete;


	cli->_Host = (char *)(malloc(sizeof(char) * strlen(host)));
		err = NewError(cli == NULL);
		if (err->IsError(err)) { err->SetMessage(err, "malloc failed."); return &(Return_NewChatClient){ err }; }
	strcpy(cli->_Host, host);

	cli->_Port = port;

	FD_ZERO(&cli->_FDState);

	return &(Return_NewChatClient){ NewError(false), cli };
}

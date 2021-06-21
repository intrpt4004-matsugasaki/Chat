#include <stdio.h>
#include <signal.h>

#include "Error/Error.h"

#include "Chat/ChatData.h"
#include "Chat/ChatClient.h"
#include "Chat/ChatServer.h"

typedef union {
	ChatClient *ChatClient;
	ChatServer *ChatServer;
	Message Message;
} Value;

typedef struct {
	Error *E;
	Value V;
} Return;

/**
 * 返り値を取り出す函数（エラー値が附されている場合は強制終了）
 */
Value RetrieveOrExitIfError(Return ret) {
	ret.E->ExitIfError(ret.E);
	return ret.V;
}

/**
 * プログラムのエントリーポイントとなる函数
 *
 * シェル処理を行う
 */
int main(const int argc, char* argv[]) {
	// ./Main -s 10000
	// ./Main -c localhost 10000 fusianasan

	if (argc != 3 && argc != 5) {
		fprintf(stderr,"<使用法>\n", argv[0]);
		fprintf(stderr, "チャットサーバを建てる場合:\n");
		fprintf(stderr, "  %s -s [ポート番号]\n", argv[0]);
		fprintf(stderr, "チャットサーバに繋ぐ場合:\n");
		fprintf(stderr, "  %s -c [ホスト名] [ポート番号] [ハンドル名] \n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (argc == 3 && strcmp("-s", argv[1]) ) {
		fprintf(stderr,"[エラー] 不正なコマンド\n");
		exit(EXIT_FAILURE);
	}

	if ( argc == 5 && strcmp("-c", argv[1]) ) {
		fprintf(stderr,"[エラー] 不正なコマンド\n");
		exit(EXIT_FAILURE);
	}

	if (!strcmp("-s", argv[1])) {
		ChatServer *svr = RetrieveOrExitIfError( *(Return *)(NewChatServer(atoi(argv[2]))) ).ChatServer;
		Error *err = svr->Start(svr);
		err->ExitIfError(err);
	} else if (!strcmp("-c", argv[1])) {
		/* チャット */
		ChatClient *cli = RetrieveOrExitIfError( *(Return *)(NewChatClient(argv[2], atoi(argv[3]))) ).ChatClient;

		printf("%s[%s として入場要求中...]%s\n", "\x1b[32m", argv[4], "\x1b[39m"); fflush(stdout);
		Error *err = cli->Login(cli, argv[4]);
		err->ExitIfError(err);

		printf("\n%s[情報] 終了するには /exit と打鍵してください%s\n", "\x1b[33m", "\x1b[39m"); fflush(stdout);
		printf("------------------------------------------------------------\n"); fflush(stdout);

		fd_set state;
		FD_ZERO(&state);
		FD_SET(0, &state);

		for (;;) {
			fd_set tmp = state;
			select(1, &tmp, NULL, NULL, &(struct timeval){ .tv_sec = 0, .tv_usec = 0 });

			if (cli->ConfirmUpdate(cli)) {
				/* 更新がある場合 */
				Message msg = RetrieveOrExitIfError( *(Return *)(cli->GetMessage(cli)) ).Message;
				if (!strcmp(msg.Handle, cli->GetHandle(cli))) {
					// 自分の書き込み
					printf("%s[%s(自分)] %s%s\n", "\x1b[31m", msg.Handle, msg.Message, "\x1b[39m"); fflush(stdout);
				} else {
					// 他人の書き込み
					printf("%s[%s] %s%s\n", "\x1b[34m", msg.Handle, msg.Message, "\x1b[39m"); fflush(stdout);
				}
			}

			if (FD_ISSET(0, &tmp)) {
				/* 標準入力がある場合 */
				char str[DATA_MAX_SIZE];
				fgets(str, DATA_MAX_SIZE, stdin);
				str[strlen(str)-1] = '\0'; // 改行文字を消す

				if (!strcmp(str, "/exit")) {
					/* 退出 */
					err = cli->Logout(cli);
					err->ExitIfError(err);

					return EXIT_SUCCESS;
				} else {
					err = cli->Write(cli, str);
					err->ExitIfError(err);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}

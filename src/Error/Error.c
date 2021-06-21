#include "Error.h"

const char ERROR_MESSAGE_OUTSET[] = "[Error]: ";
const char ERROR_MESSAGE_INDENT[] = "         ";

/**
 * 引数がNULLを指す場合に強制終了する函数（malloc呼出後に呼び出される想定）
 */
static void ExitIfMallocIsFailed(void *pointer) {
	if (pointer == NULL) {
		fprintf(stderr, "%s malloc failed.", ERROR_MESSAGE_OUTSET);
		exit(EXIT_FAILURE);
	}
}

/**
 * エラーの有無を返す函数
 */
static bool IsError(Error *self) {
	return self->_error;
}

/**
 * エラーメッセージを設定する函数
 */
static void SetMessage(Error *self, const char *message) {
	self->_Message = (char *)(malloc(sizeof(char) * (strlen(message) + 1)));
		ExitIfMallocIsFailed(self->_Message);
	strcpy(self->_Message, message);
}

/**
 * エラーメッセージを返す函数
 */
static char * GetMessage(Error *self) {
	return self->_Message;
}

/**
 * エラーの場合に強制終了する函数
 */
static void ExitIfError(Error * self) {
	if (!self->IsError(self)) return;

	fprintf(stderr, "%s%s%s%s\n", "\x1b[31m", ERROR_MESSAGE_OUTSET, self->GetMessage(self), "\x1b[39m");

	self->Delete(self);

	exit(EXIT_FAILURE);
}

/**
 * Errorの解体を行う函数
 */
static void Delete(Error *self) {
	if (self->_Message != NULL) free(self->_Message);
	free(self);
}

/**
 * Errorの構築を行う函数
 */
extern Error * NewError(const bool error) {
	Error *err = (Error *)(malloc(sizeof(Error)));
		ExitIfMallocIsFailed(err);

	err->IsError = IsError;
	err->SetMessage = SetMessage;
	err->GetMessage = GetMessage;
	err->ExitIfError = ExitIfError;

	err->Delete = Delete;


	err->_error = error;

	return err;
}

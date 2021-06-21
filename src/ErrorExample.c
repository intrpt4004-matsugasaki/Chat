#include "Error/Error.h"

/**
 * Error.aの使用例
 */
// gcc ErrorExample.c Error.a -o ErrorExample
// ./ErrorExample
void main() {
	Error *err = NewError(false);
	if (err->IsError(err)) {
		printf("エラー\n");
	} else {
		printf("非エラー\n");
	}
	err->Delete(err);


	err = NewError(true);
	if (err->IsError(err)) {
		printf("エラー\n");
		err->SetMessage(err, "かくかくしかじか");
		printf("%s\n", err->GetMessage(err)); // SetMessageしていなければ、GetMessageでSIGSEGV
	} else {
		printf("非エラー\n");
	}
	err->Delete(err);


	err = NewError(false);
	err->ExitIfError(err); // 通過
	err->Delete(err);


	err = NewError(true);
	err->SetMessage(err, "ここに理由");
	err->ExitIfError(err); // 引っ掛かって終了
	err->Delete(err);
}

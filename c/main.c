#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pg.h"

int main() {
    printf("Hello, World!\n");
    //HMODULE h = LoadLibrary("pg.dll");
    //char *dest = GetStr("hello");
    char str[6] = {'H','e','l','l','o','\0'};
    char *returnSn = SecretAESBase64Md532(str);
    printf("%s\n",returnSn);
    free(returnSn);
    return 0;
}


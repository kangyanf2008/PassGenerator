#include <iostream>
#include <cstring>
using namespace std;

#include "pg.h"

int main() {
    char sn[6] = {'H','e','l','l','o','\0'};
    char *returnSn = SecretAESBase64Md532(sn);

    std::cout << returnSn << std::endl;
    delete (returnSn);
    return 0;
}
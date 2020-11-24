#ifndef _pwKey
#define _pwKey
#define DEFAULT_PRODUCT_CODE 4300
#define MUL_FIRM_CODE 103391

/**
读取拓展数据
*/
int getExtDataKey(unsigned long ProductCode, unsigned long mulFirmCode, unsigned char data[1024], unsigned int *ilen);
/*
读取拓展加密数据
*/
int getCmCrypt2DataKey(unsigned long ProductCode, unsigned long mulFirmCode, unsigned char data[1024], unsigned int *ilen);

#endif
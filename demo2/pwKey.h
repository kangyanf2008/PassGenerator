#ifndef _pwKey
#define _pwKey
#define DEFAULT_PRODUCT_CODE 4300
#define MUL_FIRM_CODE 103391
/**
读取拓展数据
*/
int getExtDataKey(unsigned long ProductCode, unsigned long mulFirmCode, unsigned char data[1024], unsigned int *ilen);
/*
读取隐密数据
*/
int getSecretDataKey(unsigned long ProductCode, unsigned long mulFirmCode, unsigned char data[1024], unsigned int *ilen);
/*
读取隐藏数据
*/
int getHiddenDataKey(unsigned long ProductCode, unsigned long mulFirmCode, unsigned char data[1024], unsigned int *ilen);

#endif
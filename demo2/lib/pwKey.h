#ifndef _pwKey
#define _pwKey
#define DEFAULT_PRODUCT_CODE 4300
#define MUL_FIRM_CODE 103391
int GetData(unsigned long ProductCode, unsigned long mulFirmCode, unsigned char data[1024], unsigned int *ilen);

#endif
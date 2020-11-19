#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CodeMeter.h"
#include "pwKey.h"


//16进制密码
unsigned char PioEncryptionKey[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


//客户端解密数据所用的秘钥
unsigned char pabPiodkDest[CM_BLOCK_SIZE];

int ErrorHandler(char *line, int ExitCode, HCMSysEntry hcmEntry)
{
	int ret = 0;
	char acErrText[256];

	switch (CmGetLastErrorCode())
	{
	case CMERROR_NO_ERROR:
		return ret;
	case CMERROR_BOX_NOT_FOUND:
		fprintf(stderr, "%s: 没有发现对应CmContainer。\n", line);
		ret = -1;
		break;
	case CMERROR_ENTRY_NOT_FOUND:
		fprintf(stderr, "%s: 无法找到对应单元。\n", line);
		ret = -2;
		break;
	case CMERROR_INVALID_HANDLE:
		fprintf(stderr, "%s: 句柄无效!CmDongle已移除？\n", line);
		ret = -3;
		break;
	case CMERROR_NO_DATA_AVAILABLE:
		fprintf(stderr, "%s: 未找到任何产品项目。\n", line);
		ret = -4;
		break;
	default:
		CmGetLastErrorText(CM_GLET_ERRORTEXT, acErrText, sizeof(acErrText));
		fprintf(stderr, "%s: 发生其他错误: \"%s\"\n", line, acErrText);
		ret = -5;
		break;
	}

	// 针对该错误，请尝试关闭句柄。
	if (NULL != hcmEntry)
		CmRelease(hcmEntry);
	//exit(ExitCode);
	return ret;
}


int xDump(unsigned char *data, int dataLen){
	for (int i = 0; i < dataLen; i++)
	{
		//result[i] = data[i];
		printf("%02.2X", data[i]);
		if (i < dataLen - 1)
		if ((i & 15) != 15)
			printf(", ");
		else
			printf("\n");
		else
			printf("\n");
	}
	return 0;
}


/*
读取加密数据
*/
int getSecretDataKey(unsigned long ProductCode, unsigned long mulFirmCode, unsigned char data[1024], unsigned int *ilen){
	
	HCMSysEntry hcmDataEntry;
	CMENTRYDATA cmEntry;
	CMSECUREDATA cmSecureData;
	CMACCESS cmacc;
	int iRes;
	unsigned int keyLen;

	memset(&cmacc, 0, sizeof(cmacc));
	cmacc.mflCtrl = CM_ACCESS_NOUSERLIMIT;

	//如果code为空，则使用默认
	if (mulFirmCode <= 0) {
		cmacc.mulFirmCode = MUL_FIRM_CODE;
	}
	else {
		cmacc.mulFirmCode = mulFirmCode;
	}

	//如果产品码未传，则使用默认的
	if (ProductCode <= 0) {
		cmacc.mulProductCode = DEFAULT_PRODUCT_CODE;
	} else {
		cmacc.mulProductCode = ProductCode;
	}

	// 访问CmDongle。
	hcmDataEntry = CmAccess(CM_ACCESS_LOCAL, &cmacc);

	// 处理任何错误。
	// 很可能没有找到带有合适公司码和产品码的CmDongle
	if (NULL == hcmDataEntry){
		return ErrorHandler("CmAccess", 1, hcmDataEntry);
	}

	// 返回值包含一个字节数目 
	// 数据在返回加密在CMENTRYDATA::mabData。
	// 通常此处会有一些产品信息。



	memset(&cmEntry, 0, sizeof(cmEntry));
	memset(&cmSecureData, 0, sizeof(cmSecureData));

	cmSecureData.mcmBaseCrypt.mflCtrl |= CM_CRYPT_FIRMKEY;
	cmSecureData.mcmBaseCrypt.mflCtrl |= CM_CRYPT_AES;
	cmSecureData.mcmBaseCrypt.mulEncryptionCode = 2468;
	cmSecureData.mcmBaseCrypt.mulEncryptionCode |= CM_CRYPT_UCCHECK;
	cmSecureData.mcmBaseCrypt.mulEncryptionCode |= CM_CRYPT_ATCHECK;
	cmSecureData.mcmBaseCrypt.mulEncryptionCode |= CM_CRYPT_ETCHECK;
	cmSecureData.mcmBaseCrypt.mulEncryptionCode |= CM_CRYPT_SAUNLIMITED;
	memcpy(cmSecureData.mabPioEncryptionKey, PioEncryptionKey, CM_BLOCK_SIZE);
	cmSecureData.musPioType |= CM_GF_HIDDENDATA;
	cmSecureData.musExtType = 0;

	keyLen = CmGetSecureData(hcmDataEntry, &cmSecureData, &cmEntry);
	

	// 处理任何错误。
	// 句柄已经打开，此处不应再有其他句柄。
	if (keyLen == 0){
		return ErrorHandler("CmGetSecureData (data)", 3, hcmDataEntry);
	}
	printf("Product Item隐密数据。长度： %d\n", keyLen);
	xDump(cmEntry.mabData, keyLen);

	//解密数据
	iRes = CmDecryptPioData(cmEntry.mabData, cmEntry.mcbData, pabPiodkDest, CM_BLOCK_SIZE);
	if (0 == iRes) {
		printf("\tCmDecryptPioData Failed, Cannot Decrypt the data!\n");
		return -2;
	} else {
		printf("\tCmDecryptPioData OK!\n");
	}//end if(0 == iRes)

	
	printf("\tHidden data read out:\n\t");
	for (unsigned int iLenIndex = 0; iLenIndex < cmEntry.mcbData; iLenIndex++)
	{
		printf("%X ", cmEntry.mabData[iLenIndex]);
	}//end for(unsigned int iLenIndex=0; iLenIndex<cmEntry.mcbData; iLenIndex++)
	
	*ilen = cmEntry.mcbData;
	memcpy(data, cmEntry.mabData, cmEntry.mcbData);

	//清除所有信息。
	CmRelease(hcmDataEntry);
	
	return 0;
}

/*
读取隐藏数据
*/
int getHiddenDataKey(unsigned long ProductCode, unsigned long mulFirmCode, unsigned char data[1024], unsigned int *ilen){

	unsigned int res, size, i;
	HCMSysEntry hcmEntry;
	CMACCESS cmAcc;

	memset(&cmAcc, 0, sizeof(cmAcc));
	cmAcc.mflCtrl = CM_ACCESS_NOUSERLIMIT;

	//如果code为空，则使用默认
	if (mulFirmCode <= 0) {
		cmAcc.mulFirmCode = MUL_FIRM_CODE;
	}
	else {
		cmAcc.mulFirmCode = mulFirmCode;
	}

	//如果产品码未传，则使用默认的
	if (ProductCode <= 0) {
		cmAcc.mulProductCode = DEFAULT_PRODUCT_CODE;
	}
	else {
		cmAcc.mulProductCode = ProductCode;
	}


	// 访问CmDongle。
	hcmEntry = CmAccess(CM_ACCESS_LOCAL_LAN, &cmAcc);

	// 处理任何错误。
	// 很可能没有找到带有合适公司码和产品码的CmDongle
	if (NULL == hcmEntry){
		return ErrorHandler("CmAccess", 1, hcmEntry);
	}

	// 现在重新获得用户数据。
	// 首先获得所需缓冲的大小。
	size = CmGetInfo(hcmEntry, CM_GEI_ENTRYDATA, NULL, 0);

	// 处理任何错误。
	// 当我们希望返回实际所需长度时
	// 通常情况下无需数据CMERROR_BUFFER_OVERFLOW。
	if (CmGetLastErrorCode() != CMERROR_BUFFER_OVERFLOW) {
		return ErrorHandler("CmGetInfo (size)", 2, hcmEntry);
	}


	if (size)
	{
		// 通常此处会有一些产品信息。
		CMENTRYDATA *pCmBoxEntry;
		pCmBoxEntry = (CMENTRYDATA *)malloc(size);
		res = CmGetInfo(hcmEntry, CM_GEI_ENTRYDATA, pCmBoxEntry, size);

		// 处理任何错误。
		// 句柄已经打开，此处不应再有其他句柄。
		if (!res){
			return ErrorHandler("CmGetInfo (data)", 3, hcmEntry);
		}

		for (i = 0; i < res / sizeof(CMENTRYDATA); i++)
		{
			int type = pCmBoxEntry[i].mflCtrl & 0x0ffff;
			int extType = pCmBoxEntry[i].mflCtrl >> 16;
			int len = pCmBoxEntry[i].mcbData;
			switch (pCmBoxEntry[i].mflCtrl & 0x0ffff)
			{
			case CM_GF_TEXT:
				//printf("Product Item文本信息。长度： %d\n", len);
				//wprintf(L"\"%s\"\n", pCmBoxEntry[i].mabData);
				break;
			case CM_GF_USERDATA:
				//printf("Product Item用户数据。长度： %d\n", len);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_PROTDATA:
				//printf("Product Item保护数据。长度： %d\n", len);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_EXTPROTDATA:
				//printf("Product Item扩展保护数据。长度： %d, 类型： %d\n", len, extType);
				//xDump(pCmBoxEntry[i].mabData, len);
				//创建空间存放返回结果
				//*ilen = len;
				//memcpy(data, pCmBoxEntry[i].mabData, len);
				//free(pCmBoxEntry);//释放内存
				//CmRelease(hcmEntry);
				//return 0;
				break;
			case CM_GF_HIDDENDATA:
				//printf("Product Item隐藏数据。长度: %d, 类型： %d\n", len, extType);
				//xDump(pCmBoxEntry[i].mabData, len);
				*ilen = len;
				memcpy(data, pCmBoxEntry[i].mabData, len);
				free(pCmBoxEntry);//释放内存
				CmRelease(hcmEntry);
				return 0;
				break;
			case CM_GF_SECRETDATA:
				//printf("Product Item隐密数据。长度： %d, 类型： %d\n", len, extType);
				//xDump(pCmBoxEntry[i].mabData, len);
				//*ilen = len;
				//memcpy(data, pCmBoxEntry[i].mabData, len);
				//free(pCmBoxEntry);//释放内存
				//CmRelease(hcmEntry);
				//return 0;
				break;
			case CM_GF_USAGEPERIOD:
				//printf("发现Product Item使用期限。长度： %d\n", len);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			default:
				//printf("发现未知Product Item数据。类型= %d\n", type);
				break;
			}
		}
	}
	else {
		// Size == 0 ==>没有发现Product Item选项。
		printf("未找到任何Product Item!\n");
		return -6;
	}

	// 清除所有信息。
	CmRelease(hcmEntry);

	return 0;
}


/*
读取拓展数据
*/
int getExtDataKey(unsigned long ProductCode, unsigned long mulFirmCode, unsigned char data[1024], unsigned int *ilen){
	unsigned int res, size, i;
	HCMSysEntry hcmEntry;
	CMACCESS cmAcc;

	memset(&cmAcc, 0, sizeof(cmAcc));
	cmAcc.mflCtrl = CM_ACCESS_NOUSERLIMIT;

	//如果code为空，则使用默认
	if (mulFirmCode <= 0) {
		cmAcc.mulFirmCode = MUL_FIRM_CODE;
	}
	else {
		cmAcc.mulFirmCode = mulFirmCode;
	}

	//如果产品码未传，则使用默认的
	if (ProductCode <= 0) {
		cmAcc.mulProductCode = DEFAULT_PRODUCT_CODE;
	}
	else {
		cmAcc.mulProductCode = ProductCode;
	}


	// 访问CmDongle。
	hcmEntry = CmAccess(CM_ACCESS_LOCAL_LAN, &cmAcc);

	// 处理任何错误。
	// 很可能没有找到带有合适公司码和产品码的CmDongle
	if (NULL == hcmEntry){
		return ErrorHandler("CmAccess", 1, hcmEntry);
	}

	// 现在重新获得用户数据。
	// 首先获得所需缓冲的大小。
	size = CmGetInfo(hcmEntry, CM_GEI_ENTRYDATA, NULL, 0);

	// 处理任何错误。
	// 当我们希望返回实际所需长度时
	// 通常情况下无需数据CMERROR_BUFFER_OVERFLOW。
	if (CmGetLastErrorCode() != CMERROR_BUFFER_OVERFLOW) {
		return ErrorHandler("CmGetInfo (size)", 2, hcmEntry);
	}


	if (size)
	{
		// 通常此处会有一些产品信息。
		CMENTRYDATA *pCmBoxEntry;
		pCmBoxEntry = (CMENTRYDATA *)malloc(size);
		res = CmGetInfo(hcmEntry, CM_GEI_ENTRYDATA, pCmBoxEntry, size);

		// 处理任何错误。
		// 句柄已经打开，此处不应再有其他句柄。
		if (!res){
			return ErrorHandler("CmGetInfo (data)", 3, hcmEntry);
		}

		for (i = 0; i < res / sizeof(CMENTRYDATA); i++)
		{
			int type = pCmBoxEntry[i].mflCtrl & 0x0ffff;
			int extType = pCmBoxEntry[i].mflCtrl >> 16;
			int len = pCmBoxEntry[i].mcbData;
			switch (pCmBoxEntry[i].mflCtrl & 0x0ffff)
			{
			case CM_GF_TEXT:
				//printf("Product Item文本信息。长度： %d\n", len);
				//wprintf(L"\"%s\"\n", pCmBoxEntry[i].mabData);
				break;
			case CM_GF_USERDATA:
				//printf("Product Item用户数据。长度： %d\n", len);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_PROTDATA:
				//printf("Product Item保护数据。长度： %d\n", len);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_EXTPROTDATA:
				//printf("Product Item扩展保护数据。长度： %d, 类型： %d\n", len, extType);
				//xDump(pCmBoxEntry[i].mabData, len);
				//创建空间存放返回结果
				*ilen = len;
				memcpy(data, pCmBoxEntry[i].mabData, len);
				//free(pCmBoxEntry);//释放内存
				//CmRelease(hcmEntry);
				//return 0;
				break;
			case CM_GF_HIDDENDATA:
				//printf("Product Item隐藏数据。长度: %d, 类型： %d\n", len, extType);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_SECRETDATA:
				//printf("Product Item隐密数据。长度： %d, 类型： %d\n", len, extType);
				//xDump(pCmBoxEntry[i].mabData, len);
				//*ilen = len;
				//memcpy(data, pCmBoxEntry[i].mabData, len);
				//free(pCmBoxEntry);//释放内存
				//CmRelease(hcmEntry);
				//return 0;
				break;
			case CM_GF_USAGEPERIOD:
				//printf("发现Product Item使用期限。长度： %d\n", len);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			default:
				//printf("发现未知Product Item数据。类型= %d\n", type);
				break;
			}
		}
	}
	else {
		// Size == 0 ==>没有发现Product Item选项。
		printf("未找到任何Product Item!\n");
		return -6;
	}

	// 清除所有信息。
	CmRelease(hcmEntry);

	return 0;
}
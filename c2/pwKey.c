#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CodeMeter.h"
#include "pwKey.h"


//16��������
unsigned char PioEncryptionKey[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


//�ͻ��˽����������õ���Կ
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
		fprintf(stderr, "%s: û�з��ֶ�ӦCmContainer��\n", line);
		ret = -1;
		break;
	case CMERROR_ENTRY_NOT_FOUND:
		fprintf(stderr, "%s: �޷��ҵ���Ӧ��Ԫ��\n", line);
		ret = -2;
		break;
	case CMERROR_INVALID_HANDLE:
		fprintf(stderr, "%s: �����Ч!CmDongle���Ƴ���\n", line);
		ret = -3;
		break;
	case CMERROR_NO_DATA_AVAILABLE:
		fprintf(stderr, "%s: δ�ҵ��κβ�Ʒ��Ŀ��\n", line);
		ret = -4;
		break;
	default:
		CmGetLastErrorText(CM_GLET_ERRORTEXT, acErrText, sizeof(acErrText));
		fprintf(stderr, "%s: ������������: \"%s\"\n", line, acErrText);
		ret = -5;
		break;
	}

	// ��Ըô����볢�Թرվ����
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
��ȡ��������
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

	//���codeΪ�գ���ʹ��Ĭ��
	if (mulFirmCode <= 0) {
		cmacc.mulFirmCode = MUL_FIRM_CODE;
	}
	else {
		cmacc.mulFirmCode = mulFirmCode;
	}

	//�����Ʒ��δ������ʹ��Ĭ�ϵ�
	if (ProductCode <= 0) {
		cmacc.mulProductCode = DEFAULT_PRODUCT_CODE;
	} else {
		cmacc.mulProductCode = ProductCode;
	}

	// ����CmDongle��
	hcmDataEntry = CmAccess(CM_ACCESS_LOCAL, &cmacc);

	// �����κδ���
	// �ܿ���û���ҵ����к��ʹ�˾��Ͳ�Ʒ���CmDongle
	if (NULL == hcmDataEntry){
		return ErrorHandler("CmAccess", 1, hcmDataEntry);
	}

	// ����ֵ����һ���ֽ���Ŀ 
	// �����ڷ��ؼ�����CMENTRYDATA::mabData��
	// ͨ���˴�����һЩ��Ʒ��Ϣ��



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
	

	// �����κδ���
	// ����Ѿ��򿪣��˴���Ӧ�������������
	if (keyLen == 0){
		return ErrorHandler("CmGetSecureData (data)", 3, hcmDataEntry);
	}
	printf("Product Item�������ݡ����ȣ� %d\n", keyLen);
	xDump(cmEntry.mabData, keyLen);

	//��������
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

	//���������Ϣ��
	CmRelease(hcmDataEntry);
	
	return 0;
}

/*
��ȡ��������
*/
int getHiddenDataKey(unsigned long ProductCode, unsigned long mulFirmCode, unsigned char data[1024], unsigned int *ilen){

	unsigned int res, size, i;
	HCMSysEntry hcmEntry;
	CMACCESS cmAcc;

	memset(&cmAcc, 0, sizeof(cmAcc));
	cmAcc.mflCtrl = CM_ACCESS_NOUSERLIMIT;

	//���codeΪ�գ���ʹ��Ĭ��
	if (mulFirmCode <= 0) {
		cmAcc.mulFirmCode = MUL_FIRM_CODE;
	}
	else {
		cmAcc.mulFirmCode = mulFirmCode;
	}

	//�����Ʒ��δ������ʹ��Ĭ�ϵ�
	if (ProductCode <= 0) {
		cmAcc.mulProductCode = DEFAULT_PRODUCT_CODE;
	}
	else {
		cmAcc.mulProductCode = ProductCode;
	}


	// ����CmDongle��
	hcmEntry = CmAccess(CM_ACCESS_LOCAL_LAN, &cmAcc);

	// �����κδ���
	// �ܿ���û���ҵ����к��ʹ�˾��Ͳ�Ʒ���CmDongle
	if (NULL == hcmEntry){
		return ErrorHandler("CmAccess", 1, hcmEntry);
	}

	// �������»���û����ݡ�
	// ���Ȼ�����軺��Ĵ�С��
	size = CmGetInfo(hcmEntry, CM_GEI_ENTRYDATA, NULL, 0);

	// �����κδ���
	// ������ϣ������ʵ�����賤��ʱ
	// ͨ���������������CMERROR_BUFFER_OVERFLOW��
	if (CmGetLastErrorCode() != CMERROR_BUFFER_OVERFLOW) {
		return ErrorHandler("CmGetInfo (size)", 2, hcmEntry);
	}


	if (size)
	{
		// ͨ���˴�����һЩ��Ʒ��Ϣ��
		CMENTRYDATA *pCmBoxEntry;
		pCmBoxEntry = (CMENTRYDATA *)malloc(size);
		res = CmGetInfo(hcmEntry, CM_GEI_ENTRYDATA, pCmBoxEntry, size);

		// �����κδ���
		// ����Ѿ��򿪣��˴���Ӧ�������������
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
				//printf("Product Item�ı���Ϣ�����ȣ� %d\n", len);
				//wprintf(L"\"%s\"\n", pCmBoxEntry[i].mabData);
				break;
			case CM_GF_USERDATA:
				//printf("Product Item�û����ݡ����ȣ� %d\n", len);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_PROTDATA:
				//printf("Product Item�������ݡ����ȣ� %d\n", len);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_EXTPROTDATA:
				//printf("Product Item��չ�������ݡ����ȣ� %d, ���ͣ� %d\n", len, extType);
				//xDump(pCmBoxEntry[i].mabData, len);
				//�����ռ��ŷ��ؽ��
				//*ilen = len;
				//memcpy(data, pCmBoxEntry[i].mabData, len);
				//free(pCmBoxEntry);//�ͷ��ڴ�
				//CmRelease(hcmEntry);
				//return 0;
				break;
			case CM_GF_HIDDENDATA:
				//printf("Product Item�������ݡ�����: %d, ���ͣ� %d\n", len, extType);
				//xDump(pCmBoxEntry[i].mabData, len);
				*ilen = len;
				memcpy(data, pCmBoxEntry[i].mabData, len);
				free(pCmBoxEntry);//�ͷ��ڴ�
				CmRelease(hcmEntry);
				return 0;
				break;
			case CM_GF_SECRETDATA:
				//printf("Product Item�������ݡ����ȣ� %d, ���ͣ� %d\n", len, extType);
				//xDump(pCmBoxEntry[i].mabData, len);
				//*ilen = len;
				//memcpy(data, pCmBoxEntry[i].mabData, len);
				//free(pCmBoxEntry);//�ͷ��ڴ�
				//CmRelease(hcmEntry);
				//return 0;
				break;
			case CM_GF_USAGEPERIOD:
				//printf("����Product Itemʹ�����ޡ����ȣ� %d\n", len);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			default:
				//printf("����δ֪Product Item���ݡ�����= %d\n", type);
				break;
			}
		}
	}
	else {
		// Size == 0 ==>û�з���Product Itemѡ�
		printf("δ�ҵ��κ�Product Item!\n");
		return -6;
	}

	// ���������Ϣ��
	CmRelease(hcmEntry);

	return 0;
}


/*
��ȡ��չ����
*/
int getExtDataKey(unsigned long ProductCode, unsigned long mulFirmCode, unsigned char data[1024], unsigned int *ilen){
	unsigned int res, size, i;
	HCMSysEntry hcmEntry;
	CMACCESS cmAcc;

	memset(&cmAcc, 0, sizeof(cmAcc));
	cmAcc.mflCtrl = CM_ACCESS_NOUSERLIMIT;

	//���codeΪ�գ���ʹ��Ĭ��
	if (mulFirmCode <= 0) {
		cmAcc.mulFirmCode = MUL_FIRM_CODE;
	}
	else {
		cmAcc.mulFirmCode = mulFirmCode;
	}

	//�����Ʒ��δ������ʹ��Ĭ�ϵ�
	if (ProductCode <= 0) {
		cmAcc.mulProductCode = DEFAULT_PRODUCT_CODE;
	}
	else {
		cmAcc.mulProductCode = ProductCode;
	}


	// ����CmDongle��
	hcmEntry = CmAccess(CM_ACCESS_LOCAL_LAN, &cmAcc);

	// �����κδ���
	// �ܿ���û���ҵ����к��ʹ�˾��Ͳ�Ʒ���CmDongle
	if (NULL == hcmEntry){
		return ErrorHandler("CmAccess", 1, hcmEntry);
	}

	// �������»���û����ݡ�
	// ���Ȼ�����軺��Ĵ�С��
	size = CmGetInfo(hcmEntry, CM_GEI_ENTRYDATA, NULL, 0);

	// �����κδ���
	// ������ϣ������ʵ�����賤��ʱ
	// ͨ���������������CMERROR_BUFFER_OVERFLOW��
	if (CmGetLastErrorCode() != CMERROR_BUFFER_OVERFLOW) {
		return ErrorHandler("CmGetInfo (size)", 2, hcmEntry);
	}


	if (size)
	{
		// ͨ���˴�����һЩ��Ʒ��Ϣ��
		CMENTRYDATA *pCmBoxEntry;
		pCmBoxEntry = (CMENTRYDATA *)malloc(size);
		res = CmGetInfo(hcmEntry, CM_GEI_ENTRYDATA, pCmBoxEntry, size);

		// �����κδ���
		// ����Ѿ��򿪣��˴���Ӧ�������������
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
				//printf("Product Item�ı���Ϣ�����ȣ� %d\n", len);
				//wprintf(L"\"%s\"\n", pCmBoxEntry[i].mabData);
				break;
			case CM_GF_USERDATA:
				//printf("Product Item�û����ݡ����ȣ� %d\n", len);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_PROTDATA:
				//printf("Product Item�������ݡ����ȣ� %d\n", len);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_EXTPROTDATA:
				//printf("Product Item��չ�������ݡ����ȣ� %d, ���ͣ� %d\n", len, extType);
				//xDump(pCmBoxEntry[i].mabData, len);
				//�����ռ��ŷ��ؽ��
				*ilen = len;
				memcpy(data, pCmBoxEntry[i].mabData, len);
				//free(pCmBoxEntry);//�ͷ��ڴ�
				//CmRelease(hcmEntry);
				//return 0;
				break;
			case CM_GF_HIDDENDATA:
				//printf("Product Item�������ݡ�����: %d, ���ͣ� %d\n", len, extType);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_SECRETDATA:
				//printf("Product Item�������ݡ����ȣ� %d, ���ͣ� %d\n", len, extType);
				//xDump(pCmBoxEntry[i].mabData, len);
				//*ilen = len;
				//memcpy(data, pCmBoxEntry[i].mabData, len);
				//free(pCmBoxEntry);//�ͷ��ڴ�
				//CmRelease(hcmEntry);
				//return 0;
				break;
			case CM_GF_USAGEPERIOD:
				//printf("����Product Itemʹ�����ޡ����ȣ� %d\n", len);
				//xDump(pCmBoxEntry[i].mabData, len);
				break;
			default:
				//printf("����δ֪Product Item���ݡ�����= %d\n", type);
				break;
			}
		}
	}
	else {
		// Size == 0 ==>û�з���Product Itemѡ�
		printf("δ�ҵ��κ�Product Item!\n");
		return -6;
	}

	// ���������Ϣ��
	CmRelease(hcmEntry);

	return 0;
}
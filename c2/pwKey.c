#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CodeMeter.h"
#include "pwKey.h"



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
��ȡ�������� CM_CRYPT_AES_ENC_CBC
*/
int getCmCrypt2DataKey(unsigned long ProductCode, unsigned long mulFirmCode, unsigned char data[1024], unsigned int *ilen){
	unsigned int res, size, i;
	HCMSysEntry hcmse;
	CMACCESS2 cmAcc;

	memset(&cmAcc, 0, sizeof(cmAcc));
	cmAcc.mflCtrl |= CM_ACCESS_NOUSERLIMIT;

	//��ʼ������
	unsigned char initkey[] = { 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
		0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61 };
	CMCRYPT2 cmCrypt;

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

	//����оƬ���
	hcmse = CmAccess2(CM_ACCESS_LOCAL_LAN, &cmAcc);


	// �����κδ���
	// �ܿ���û���ҵ����к��ʹ�˾��Ͳ�Ʒ���CmDongle
	if (NULL == hcmse){
		return ErrorHandler("CmAccess2", 1, hcmse);
	}

	// �������»���û����ݡ�
	// ���Ȼ�����軺��Ĵ�С��
	size = CmGetInfo(hcmse, CM_GEI_ENTRYDATA, NULL, 0);

	// �����κδ���
	// ������ϣ������ʵ�����賤��ʱ
	// ͨ���������������CMERROR_BUFFER_OVERFLOW��
	if (CmGetLastErrorCode() != CMERROR_BUFFER_OVERFLOW) {
		return ErrorHandler("CmGetInfo (size)", 2, hcmse);
	}


	if (size)
	{
		// ͨ���˴�����һЩ��Ʒ��Ϣ��
		CMENTRYDATA *pCmBoxEntry;
		pCmBoxEntry = (CMENTRYDATA *)malloc(size);
		res = CmGetInfo(hcmse, CM_GEI_ENTRYDATA, pCmBoxEntry, size);

		// �����κδ���
		// ����Ѿ��򿪣��˴���Ӧ�������������
		if (!res){
			return ErrorHandler("CmGetInfo (data)", 3, hcmse);
		}

		for (i = 0; i < res / sizeof(CMENTRYDATA); i++)	{
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
		
				memset(&cmCrypt, 0, sizeof(cmCrypt));
				cmCrypt.mcmBaseCrypt.mflCtrl |= CM_CRYPT_FIRMKEY;
				cmCrypt.mcmBaseCrypt.mflCtrl |= CM_CRYPT_AES;
				cmCrypt.mcmBaseCrypt.mulEncryptionCodeOptions |= CM_CRYPT_UCCHECK;
				cmCrypt.mcmBaseCrypt.mulEncryptionCodeOptions |= CM_CRYPT_ATCHECK;
				cmCrypt.mcmBaseCrypt.mulEncryptionCodeOptions |= CM_CRYPT_ETCHECK;
				cmCrypt.mcmBaseCrypt.mulEncryptionCodeOptions |= CM_CRYPT_SAUNLIMITED;
				memcpy(cmCrypt.mabInitKey, initkey, CM_BLOCK_SIZE);

				CmCrypt2(hcmse, CM_CRYPT_AES_DEC_CBC, &cmCrypt, pCmBoxEntry[i].mabData, len);
				// �����κδ���
				// ����Ѿ��򿪣��˴���Ӧ�������������
				/*
				if (!res){
					return ErrorHandler("CmCrypt2 (data)", 3, hcmse);
				}
				*/
				
				*ilen = len;
				memcpy(data, pCmBoxEntry[i].mabData, len);

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
	} else {
		// Size == 0 ==>û�з���Product Itemѡ�
		printf("δ�ҵ��κ�Product Item!\n");
		return -6;
	}

	
	//���������Ϣ��
	CmRelease(hcmse);
	
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
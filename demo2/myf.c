#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include"myf.h"
#include"CodeMeter.h"



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



int GetData(unsigned long ProductCode, unsigned char data[512], unsigned int *ilen)
{
	HCMSysEntry hcmEntry;
	CMACCESS cmAcc;

	memset(&cmAcc, 0, sizeof(cmAcc));
	cmAcc.mflCtrl = CM_ACCESS_NOUSERLIMIT;
	cmAcc.mulFirmCode = 103391;
	if (ProductCode <= 0) {
		cmAcc.mulProductCode = DEFAULTPRODUCTCODE;
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
	int size = CmGetInfo(hcmEntry, CM_GEI_ENTRYDATA, NULL, 0);

	// 处理任何错误。
	// 当我们希望返回实际所需长度时
	// 通常情况下无需数据CMERROR_BUFFER_OVERFLOW。
	if (CmGetLastErrorCode() != CMERROR_BUFFER_OVERFLOW)
	{
		return ErrorHandler("CmGetInfo (size)", 2, hcmEntry);
	}
	if (size){
		// 通常此处会有一些产品信息。
		CMENTRYDATA *pCmBoxEntry;
		pCmBoxEntry = (CMENTRYDATA *)malloc(size);
		unsigned int res = CmGetInfo(hcmEntry, CM_GEI_ENTRYDATA, pCmBoxEntry, size);

		// 处理任何错误。
		// 句柄已经打开，此处不应再有其他句柄。
		if (!res){
			return ErrorHandler("CmGetInfo (data)", 3, hcmEntry);
		}

		for (int i = 0; i < res/sizeof(CMENTRYDATA); i++)
		{
			int type = pCmBoxEntry[i].mflCtrl & 0x0ffff;
			int extType = pCmBoxEntry[i].mflCtrl >> 16;
			int len = pCmBoxEntry[i].mcbData;
			switch (pCmBoxEntry[i].mflCtrl & 0x0ffff)
			{
			/*
			case CM_GF_TEXT:
				printf("Product Item文本信息。长度： %d\n", len);
				wprintf(L"\"%s\"\n", pCmBoxEntry[i].mabData);
				break;
			case CM_GF_USERDATA:
				printf("Product Item用户数据。长度： %d\n", len);
				xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_PROTDATA:
				printf("Product Item保护数据。长度： %d\n", len);
				xDump(pCmBoxEntry[i].mabData, len);
				break;
			*/
			case CM_GF_EXTPROTDATA:
				printf("Product Item扩展保护数据。长度： %d, 类型： %d\n", len, extType);
				*ilen = len;
				memcpy(data, pCmBoxEntry[i].mabData, len);
				//break;
				free(pCmBoxEntry);
				goto outofloop;
			/*
			case CM_GF_HIDDENDATA:
				printf("Product Item隐藏数据。长度: %d, 类型： %d\n", len, extType);
				xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_SECRETDATA:
				printf("Product Item隐密数据。长度： %d, 类型： %d\n", len, extType);
				xDump(pCmBoxEntry[i].mabData, len);
				break;
			case CM_GF_USAGEPERIOD:
				printf("发现Product Item使用期限。长度： %d\n", len);
				xDump(pCmBoxEntry[i].mabData, len);
				break;
			*/
			default:
				printf("发现未知Product Item数据。类型= %d\n", type);
				break;
			}
		}
	}
	else
	{
		// Size == 0 ==>没有发现Product Item选项。
		printf("未找到任何Product Item!\n");
		return -6;
	}

outofloop:
	// 清除所有信息。
	CmRelease(hcmEntry);

	return 0;
}



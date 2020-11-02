package encryption

import (
	"config"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"
	"utils"
)

/**
1 拼接 {key}_{sn.txt}
2 AES/ECB/PKCS5
3 md5 32
*/
func SecretAESBase64Md532(sn string) string {

	//取出密钥
	srcKey, err := utils.Base64Decode(config.KEY)
	if err != nil {
		panic(err.Error())
		os.Exit(-1)
	}

	//拼接字符串{key}_{sn.txt}
	srcChar := string(srcKey) + config.SEPARATE_CHAR + sn

	//对 {key}_{sn.txt} 进行AES/ECB/PKCS5 普通base64编码加密 区别于urlsafe base64
	crypted := utils.AesEncrypt(srcChar, string(srcKey))

	//生成md5 32位字符串
	md5String := utils.Md532(crypted)

	return md5String
}

/**
1 拼接 {key}_{sn.txt}
2 AES/ECB/PKCS5
3 md5 32
return 20字符串
*/
//export SecretAESBase64Md532Len20
func SecretAESBase64Md532Len20(sn string) string {

	//取出密钥
	srcKey, err := utils.Base64Decode(config.KEY)
	if err != nil {
		panic(err.Error())
		os.Exit(-1)
	}

	//拼接字符串{key}_{sn.txt}
	srcChar := string(srcKey) + config.SEPARATE_CHAR + sn

	//对 {key}_{sn.txt} 进行AES/ECB/PKCS5 普通base64编码加密 区别于urlsafe base64
	crypted := utils.AesEncrypt(srcChar, string(srcKey))

	//生成md5 32位字符串
	md5String := utils.Md532(crypted)

	return md5String[0:config.SecretLen]
}

func BatchCreateSnPWBySnFile(snFilePath string) error {
	sns, err := utils.ReadSn(snFilePath)
	if err != nil {
		return err
	}
	if len(sns) == 0 {
		emsg := "sn.txt file is empty" + snFilePath
		fmt.Println(emsg)
		return errors.New(emsg)
	}
	for idx, v := range sns {
		//fdsfds	fdsfdsfdsfdsfdsf格式
		tem := strings.Trim(v, " ")
		//计算SN对应的密码【AES/ECB/PKCS5 进行AES/ECB/PKCS5 md5 截取20位字符串】
		key := SecretAESBase64Md532Len20(tem)
		//进行AES/ECB/PKCS5 进行AES/ECB/PKCS5 普通base64编码加密
		crypted := utils.AesEncrypt(key, config.ServerKEY)
		sns[idx] = fmt.Sprintf("%s\t%s\n", tem, crypted)
	}
	//计算需要写新文件路径
	baseDir := filepath.Dir(snFilePath)
	targetFile := utils.FormatTime(time.Now()) + ".txt"
	return utils.WriteFileByBuffer(filepath.Join(baseDir, targetFile), sns)
}

package main

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -L. -lpwKey -lWibuCm64
#include "pwKey.h"
*/
import "C"
import (
	"config"
	"encryption"
	"flag"
	"fmt"
	"os"
	"unsafe"
)

func main() {
	var sn string
	var snFile string
	flag.StringVar(&sn, "sn", "", "")
	flag.StringVar(&snFile, "snFile", "", "")
	flag.Parse()

	if config.GetConfig() == nil {
		//加载版本号配置文件
		err := config.LoadConfigAndSetDefault()
		if err != nil {
			fmt.Println("【error】" + err.Error())
			os.Exit(-1)
		}

		//使用非默认密钥key
		if !config.GetConfig().ConfigParam.UseDefkey {
			var data [1024]uint8
			var dataLen uint
			code := C.getCmCrypt2DataKey(C.ulong(0), C.ulong(0), (*C.uchar)(unsafe.Pointer(&data)), (*C.uint)(unsafe.Pointer(&dataLen)))
			if code != 0 {
				fmt.Printf("read key error code=%d \n", code)
				os.Exit(int(code))
			}
			config.KEY = string(data[0:dataLen])
		}
	}
	//判断sn文件是否为空
	if snFile != "" {
		err := encryption.BatchCreateSnPWBySnFile(snFile)
		if err != nil {
			fmt.Println(err)
			os.Exit(-1)
		}
		os.Exit(0)
	}

	if sn == "" {
		fmt.Println("【error】 sn is empty")
		os.Exit(-1)
	} else {
		//生成20位密码字符串
		fmt.Println(encryption.SecretAESBase64Md532Len17(sn))
		os.Exit(0)
	}

}

/*func SecretAESBase64Md532(sn string) string {
	currentPath, _ :=os.Getwd()
	filePath := filepath.Join(currentPath, utils.FormatTime(time.Now())+".txt")
	f, _ := os.Create(filePath) //创建文件
	defer f.Close()
	w := bufio.NewWriter(f) //创建新的 Writer 对象
	w.WriteString(sn)
	w.Flush()
	//加载版本号配置文件
	err := config.LoadConfigAndSetDefault()
	if err != nil {
		fmt.Println("【error】"+err.Error())
		os.Exit(-1)
	}
	return encryption.SecretAESBase64Md532Len17(sn)
}
*/
//export SecretAESBase64Md532
func SecretAESBase64Md532(src *C.char) *C.char {
	sn := C.GoString(src)
	if sn == "" {
		fmt.Println("【error】r sn is null")
		return C.CString("")
	}

	if config.GetConfig() == nil {
		//加载版本号配置文件
		err := config.LoadConfigAndSetDefault()
		if err != nil {
			fmt.Println("【error】" + err.Error())
			os.Exit(-1)
		}

		//使用非默认密钥key
		if !config.GetConfig().ConfigParam.UseDefkey {
			var data [1024]uint8
			var dataLen uint
			code := C.getCmCrypt2DataKey(C.ulong(0), C.ulong(0), (*C.uchar)(unsafe.Pointer(&data)), (*C.uint)(unsafe.Pointer(&dataLen)))
			if code != 0 {
				fmt.Printf("read key error code=%d \n", code)
				os.Exit(int(code))
			}
			config.KEY = string(data[0:dataLen])
		}
	}
	return C.CString(encryption.SecretAESBase64Md532Len17(sn))
}

package main

import (
	"encryption"
	"flag"
	"fmt"
	"os"
)

func main() {
	var sn string
	var snFile string
	flag.StringVar(&sn, "sn", "", "")
	flag.StringVar(&snFile, "snFile", "", "")
	flag.Parse()

	if snFile != "" {
		err := encryption.BatchCreateSnPWBySnFile(snFile)
		if err != nil {
			os.Exit(-1)
		}
		os.Exit(0)
	}

	if sn ==  "" {
		os.Exit(-1)
	} else {
		//生成20位密码字符串
		fmt.Println(encryption.SecretAESBase64Md532Len20(sn))
		os.Exit(0)
	}

}

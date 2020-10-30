package utils

import (
	"bufio"
	"config"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
)


//读取到file中，再利用ioutil将file直接读取到[]byte中
func ReadSn(filePath string) ([]string, error){
	f, err := os.Open(filePath)
	if err != nil {
		return nil, errors.New("read file fail"+err.Error())
	}
	defer f.Close()

	fd, err := ioutil.ReadAll(f)
	if err != nil {
		return nil, errors.New("read to fd fail"+err.Error())
	}

	sn  := make([]string, 0, 3)
	if len(fd) > 2 {
		snBuffer :=make([]byte, 128)
		var i int = -1
		for _, v := range fd {
			if v == config.LF || v == config.CR {
				if i >= 0 {
					sn = append(sn, string(snBuffer[0:i+1]))
					i = -1
				}
				continue
			}
			i++
			snBuffer[i]= v
		}
		if i >= 0 {
			sn = append(sn, string(snBuffer[0:i+1]))
		}
	}
	return sn, nil

}

//写入文件
func WriteFile(filePath string, fileContent string) error {
	err := ioutil.WriteFile(filePath, []byte(fileContent), 0666)
	if err != nil {
		return errors.New("write fail" + err.Error())
	}
	return nil
}

//写入文件通过buffer
func WriteFileByBuffer(filePath string, fileContent []string) error {
	f, err3 := os.Create(filePath) //创建文件
	if err3 != nil{
		return errors.New("create file fail,filepath=" + filePath + err3.Error())
	}
	defer f.Close()

	w := bufio.NewWriter(f) //创建新的 Writer 对象
	i :=0
	if len(fileContent) > 0 {
		for _, v := range fileContent {
			n4, err3 := w.WriteString(v)
			if err3 != nil {
				return errors.New("create file fail, filepath="+ filePath + err3.Error())
			}
			i+=n4
			w.Flush()
		}
	}
	fmt.Printf("写入 %d 个字节n", i)
	return nil
}
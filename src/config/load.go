package config

import (
	"errors"
	"fmt"
	"github.com/BurntSushi/toml"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

func LoadConfigAndSetDefault() error {
	currentPath, err := os.Getwd()
	if err != nil {
		fmt.Println("【error】get current path error:" + err.Error())
		return err
	}
	filePath := filepath.Join(currentPath, "config.toml")
	if _, err := toml.DecodeFile(filePath, &config); err != nil {
		fmt.Println("【error】load config.toml error:" + err.Error())
		return errors.New("【error】load config.toml error:" + err.Error())
	}
	if config.Version.VersionCode == "" {
		fmt.Println("【error】config.toml conf error,less VersionCode config")
		return errors.New("【error】config.toml conf error,less VersionCode config")
	}
	//密码根据。进行字符分隔 v.255.255
	versionStrArray := strings.Split(config.Version.VersionCode, VersionSplit)
	if len(versionStrArray) != 3 {
		fmt.Println("【error】config.toml VersionCode config error[" + config.Version.VersionCode + "]")
		return errors.New("【error】config.toml VersionCode config error")
	}

	//版本第一个字段
	vArray := []byte(versionStrArray[0])
	if len(vArray) == 0 {
		fmt.Println("【error】config.toml VersionCode field config error [v=" + versionStrArray[0] + "]")
		return errors.New("【error】config.toml VersionCode field config error[v=" + versionStrArray[0] + "]")
	}

	//版本第二个字段
	field2 := versionStrArray[1]
	v1Int, err := strconv.Atoi(field2)
	if err != nil {
		fmt.Println("【error】config.toml VersionCode field config error [v1=" + field2 + "]err=" + err.Error())
		return errors.New("【error】config.toml VersionCode field config error" + "[v1=" + field2 + "]err=" + err.Error())
	}
	if v1Int > 255 {
		fmt.Println("【error】config.toml VersionCode field config error [v1=" + field2 + "] over 255")
		return errors.New("【error】config.toml VersionCode field config error" + "[v1=" + field2 + "]over 255")
	}

	//版本第三个字段
	field3 := versionStrArray[2]
	v2Int, err2 := strconv.Atoi(field3)
	if err2 != nil {
		fmt.Println("【error】config.toml VersionCode field config error [V2=" + field3 + "]err=" + err2.Error())
		return errors.New("【error】config.toml VersionCode field config error" + "[V2=" + field3 + "]err=" + err2.Error())
	}
	if v2Int > 255 {
		fmt.Println("【error】config.toml VersionCode field config error [V2=" + field3 + "] over 255")
		return errors.New("【error】config.toml VersionCode field config error" + "[V2=" + field3 + "]over 255")
	}

	//解析版本号
	config.V = vArray[0]
	config.V1 = uint8(v1Int)
	config.V2 = uint8(v2Int)

	return nil
}

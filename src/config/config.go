package config

var (
	config *Config
)

type VersionStruct struct {
	VersionCode string `toml:"VersionCode"`
}

type Config struct {
	Version VersionStruct `toml:"version"` //v.255.255
	V       byte          //版本号第一个字节
	V1      uint8         //版本号第二个字节
	V2      uint8         //版本号第三个字节

}

func GetConfig() *Config {
	return config
}
//返回3字节版本字符串
func GetVersionByte() string {
	return string([]byte{config.V, config.V1,config.V2})
}
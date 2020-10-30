package utils

import "encoding/base64"

//base64编码
func Base64Encode(src []byte) string {
	return base64.StdEncoding.EncodeToString(src)
}
//base64解码
func Base64Decode(src string) ([]byte, error) {
	return base64.StdEncoding.DecodeString(src)
}
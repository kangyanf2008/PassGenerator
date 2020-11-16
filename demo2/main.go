package main
/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -L. -lpwKey -lWibuCm64
#include "pwKey.h"
*/
import "C"
import (
	"fmt"
	"os"
	"unsafe"
)

func main() {
	var data [1024]uint8
	var dataLen uint
	code := C.getKey(C.ulong(0), C.ulong(0), (*C.uchar)(unsafe.Pointer(&data)), (*C.uint)(unsafe.Pointer(&dataLen)))
	if code != 0 {
		fmt.Printf("read key error code=%d \n", code)
		os.Exit(int(code))
	}
	fmt.Print(int(code))
	fmt.Printf("%s, %d, %d\n============", data[0:dataLen], dataLen, code)

	//defer C.free(key)
	os.Exit(0)
}

package main
/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -L. -ldemo
#include "demo.h"
*/
import "C"
import "fmt"

func main() {
	fmt.Println(C.sum(1,2))
	fmt.Println("111111")
}

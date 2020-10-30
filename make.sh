#!/bin/bash -x

if [ ! -f "$0" ]; then
    echo 'make must be run within its container folder' 1>&2
    exit 1
fi

if [ "$1" == "clean" ]; then
        rm bin/* pkg/* utility/pkg/* -rf
        exit
fi

CURDIR=`pwd`
export GOPATH="$CURDIR"
export GOBIN=${GOPATH}/bin
author=${USER}
date=`date "+%Y-%m-%d_%H:%I:%S"`

goversion=`go version`

ldflags="-X web-api/logic._AUTHOR_=$author -X web-api/logic._COMPILETIME_=\"$date\""
echo ldflags ${ldflags}

echo "formating code..."
#gofmt -w src/

export GOPROXY=https://goproxy.cn
export GO111MODULE=on

# 以下命令可以使用go get golang.org/x/tools/cmd/goimports获取
#${GOROOT}/bin/goimports -w=true src/

cd ./src/
#${GOROOT}/bin/go install -v -ldflags "$ldflags"  main/main.go
#go install -v -ldflags "$ldflags"  main/main.go
CGO_ENABLED=0 GOOS=linux go build -v -ldflags "$ldflags" -a -installsuffix cgo -o pg main/main.go
mv  pg ../bin/
if [ $? == 0 ]; then
	echo "build success"
else
	echo "build error"
fi

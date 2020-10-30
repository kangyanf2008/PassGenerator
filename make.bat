::编译linux版本
cd src
set GOOS=linux
set GOARCH=amd64
set GOHOSTOS=linux
set CGO_ENABLED=0

go.exe build -o  ../bin/pg main.go
cd ../
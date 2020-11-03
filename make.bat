::编译linux版本
::set CUR_DIR=%~dp0
::set GOPATH=%CUR_DIR%
set GOOS=linux
set GOARCH=amd64
set GOHOSTOS=linux
set CGO_ENABLED=0
echo %GOPATH%

cd ./src/main
::go.exe build -o  ../bin/pg main.go

go.exe build -o  ../bin/pg

cd ../../
::编译window版本
set CUR_DIR=%~dp0
set GOPATH=%CUR_DIR%

set CGO_ENABLED=1
set GOOS=windows
set GOARCH=amd64

cd src
go.exe build -p 4 -x -v -o  ../bin/pg.exe main.go
cd ../
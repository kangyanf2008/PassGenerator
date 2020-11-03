::编译window版本
set CUR_DIR=%~dp0
set GOPATH=%CUR_DIR%

cd src
go.exe build -p 4 -v -o  ../bin/pg.exe main.go
cd ../
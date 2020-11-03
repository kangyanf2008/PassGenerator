::编译linux版本
set CUR_DIR=%~dp0
set GOPATH=%CUR_DIR%
cd src
::生成动态库
go build -p 4 -buildmode=c-shared -o ../lib/pg.dll .
::生成静太库
go build -p 4 -buildmode=c-archive -o ../lib/pg.lib .
cd ../
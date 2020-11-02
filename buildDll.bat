::编译linux版本
cd src
::生成动态库
::go build -buildmode=c-shared -o pg.dll .
::生成静太库
go build -buildmode=c-archive -o pg.dll .
cd ../
::编译linux版本
set CUR_DIR=%~dp0
set GOPATH=%CUR_DIR%
set CGO_ENABLED=1

cd src
::生成静太库
go build -p 4 -buildmode=c-archive -o ../lib/pg.a .
::生成动态库
::go build -p 4 -buildmode=c-shared -o ../lib/pg.dll .
::生成静太库
::go build -p 4 -buildmode=c-archive -o ../lib/pg.lib .
cd ../lib/

gcc pg.def pg.a -shared -lwinmm -lWs2_32 -lpwKey -lWibuCm64  -L./ -o pg.dll -Wl,--out-implib,pg.dll.a

lib /def:pg.def /name:pg.dll /out:pg.lib /MACHINE:X86
cd ..
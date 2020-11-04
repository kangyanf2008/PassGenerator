::编译linux版本
::set CUR_DIR=%~dp0
::set GOPATH=%CUR_DIR%

SET CGO_ENABLED=0
SET GOOS=linux
SET GOARCH=amd64
echo %GOPATH%

cd src
::go.exe build -o  ../bin/pg main.go
go.exe build -p 4 -x -o  ../bin/pg main.go

cd ..
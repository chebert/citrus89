echo "int main(int argc, char **argv) {" $(cat -) "}" | gcc -ansi -pedantic --std=c89 -x c - -include .snippet.h -o snippet.exe
./snippet.exe
rm snippet.exe

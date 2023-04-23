.PHONY: all test clean

#PATH=$PATH:yournewpath
#PATH=~/xiao/sqlite3/install

 
all:
#	gcc -shared -fPIC -o libcli.so myDns.c

client:
	@gcc -shared -fPIC -o libcli.so client.c get_time.c ds18b20.c packet.c socket.c timer.c sql.c 
	@gcc ./client.c -o cli.out -I ./ -I ../sqlite3/install/include -lcli -L ./ -lsqlite3 -L ../sqlite3/install/lib/
	@export LD_LIBRARY_PATH=/home/iot23/xiao/one_Temp/base
#	@./cli.out -i 127.0.0.1 -p 7799 -t2
	@./cli.out -t2

server:
	@gcc -shared -fPIC -o libser.so server.c get_time.c socket.c timer.c sql.c
	@gcc ./server.c -o ser.out -I ./ -I ..sqlite3/install/include -lser -L ./ -lsqlite3 -L ../sqlite3/install/lib/
#	./ser.out -i0.0.0.0 -p 6666
	@./ser.out -p 7799 

linkList:
	@gcc -shared -fPIC -o liblist.so myLinkList.c
	@gcc ./myLinkList.c -o list.out -I ./ -llist -L ./
	./list.out

sql:
	@gcc -shared -fPIC -o libsql.so sql.c
	@gcc ./sql.c -o sql.out -I ./ -I ..sqlite3/install/include -lsql -L ./ -lsqlite3 -L ..sq/lite3/install/lib/
	./sql.out

uninstall:
	@rm -f *.out *.so *.db


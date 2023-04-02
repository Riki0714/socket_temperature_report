.PHONY: all test clean

#PATH=$PATH:yournewpath
#PATH=~/xiao/sqlite3/install

 
all:
#	gcc -shared -fPIC -o libcli.so myDns.c

client:
	@gcc -shared -fPIC -o libcli.so myDns.c client.c getTime.c get_temp.c mySocket.c myTimer.c mySql.c myLinkList_temp.c
	@gcc ./client.c -o cli.out -I ./ -I ..sqlite3/install/include -lcli -L ./ -lsqlite3 -L ..sqlite3/install/lib/
#	@gcc ./client.c -o cli.out -I ./ -lcli -L ./
	./cli.out -i 127.0.0.1 -p 7777 -t2

server:
	@gcc -shared -fPIC -o libser.so myDns.c server.c getTime.c get_temp.c mySocket.c myTimer.c mySql.c
	@gcc ./server.c -o ser.out -I ./ -I ..sqlite3/install/include -lser -L ./ -lsqlite3 -L ..sqlite3/install/lib/
#	./ser.out -i0.0.0.0 -p 6666
	./ser.out -p 7777 

linkList:
	@gcc -shared -fPIC -o liblist.so myLinkList.c
	@gcc ./myLinkList.c -o list.out -I ./ -llist -L ./
	./list.out

sql:
	@gcc -shared -fPIC -o libsql.so mySql.c
	@gcc ./mySql.c -o sql.out -I ./ -I ..sqlite3/install/include -lsql -L ./ -lsqlite3 -L ..sqlite3/install/lib/
	./sql.out

uninstall:
	@rm -f *.out *.so *.db


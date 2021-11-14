dist/server: src/server.c src/server_lib.c src/server_client.c src/server_sqlite3.c
		gcc -g $^ -o $@ -lpthread -lsqlite3 -I./include

java:
		cd javaclient && make

clean:
		rm -f dist/* javaclient/dist/*

initdb:
		sqlite3 db/data.sqlite3 < sql/init.sql
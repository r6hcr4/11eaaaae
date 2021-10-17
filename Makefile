dist/server: src/server.c src/server_lib.c src/server_client.c
		gcc -g $^ -o $@ -lpthread -I./include

java:
		cd javaclient && make

clean:
		rm -f dist/* javaclient/dist/*
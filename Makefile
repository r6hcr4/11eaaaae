dist/server: src/server.c src/server_lib.c
		gcc -g $^ -o $@ -lpthread -I./include

java:
		cd javaclient && make

clean:
		rm -f dist/* javaclient/dist/*
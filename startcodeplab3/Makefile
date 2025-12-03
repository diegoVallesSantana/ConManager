all: test_server sensor_node

run: runserver

clean:
	rm test_server
	rm sensor_node

test_server: test_server.c lib/tcpsock.c
	gcc -g test_server.c lib/tcpsock.c -o test_server -pthread

sensor_node: sensor_node.c lib/tcpsock.c
	gcc -g sensor_node.c lib/tcpsock.c -o sensor_node

runserver: test_server
	./test_server 5678 3

runclient1: sensor_node
	./sensor_node 1 2 127.0.0.1 5678

runclient2: sensor_node
	./sensor_node 2 5 127.0.0.1 5678

runclient3: sensor_node
	./sensor_node 3 2 127.0.0.1 5678

runclient4: sensor_node
	./sensor_node 4 5 127.0.0.1 5678

runclient5: sensor_node
	./sensor_node 5 3 127.0.0.1 5678

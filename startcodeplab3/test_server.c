/**
 * \author {AUTHOR}
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include "config.h"
#include "lib/tcpsock.h"

/**
 * Implements a sequential test server (only one connection at the same time)
 */

static int conn_counter = 0;                // number of clients that have been fully served
static int dec_counter = 0;                 //TODO : Still problems limiting the number of clients, it doesn't respect MAX_CONN
static int MAX_CONN = 0;                  // maximum number of clients to serve

static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void* client_handler(void* arg) {
    tcpsock_t* client = (tcpsock_t*) arg;
    sensor_data_t data;
    int bytes, result;

    do {
        // read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *) &data.id, &bytes);
        if (result != TCP_NO_ERROR) break;

        // read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *) &data.value, &bytes);
        if (result != TCP_NO_ERROR) break;

        // read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *) &data.ts, &bytes);
        if ((result == TCP_NO_ERROR) && bytes) {
            printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n",
                   data.id, data.value, (long int) data.ts);
        }
    } while (result == TCP_NO_ERROR);

    if (result == TCP_CONNECTION_CLOSED)
        printf("Peer has closed connection\n");
    else
        printf("Error occured on connection to peer\n");

    tcp_close(&client);

    // update global counter in a thread-safe way
    pthread_mutex_lock(&counter_mutex);
    conn_counter++;
    pthread_mutex_unlock(&counter_mutex);

    return NULL;
}

int main(int argc, char *argv[]) {
    tcpsock_t *server, *client;
    int PORT;

    if (argc < 3) {
        printf("Please provide the right arguments: first the port, then the max nb of clients\n");
        return -1;
    }

    PORT = atoi(argv[1]);
    MAX_CONN = atoi(argv[2]);

    printf("Test server is started\n");
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);

    while (1) {
        // check if we already served enough clients
        pthread_mutex_lock(&counter_mutex);
        int done = (dec_counter >= MAX_CONN);
        pthread_mutex_unlock(&counter_mutex);

        if (done) break;

        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) {
            printf("Error while waiting for incoming connection\n");
            break;
        }

        printf("Incoming client connection\n");

        pthread_t tid;
        int rc = pthread_create(&tid, NULL, client_handler, (void*) client);
        if (rc != 0) {
            printf("Error creating thread, closing client\n");
            tcp_close(&client);
        } else {
            pthread_detach(tid);
        }
    }

    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");

    return 0;
}





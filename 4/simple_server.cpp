#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>

#include <pthread.h>
#include "http_server.hh"

#include <vector>
#include <queue>
#include<csignal>

queue<int> CONN_QUEUE;
#define THREAD_NUM 50
#define QUEUE_SIZE 1000
pthread_t threads[THREAD_NUM];
// Set to 1 if SIGINT intercepted
int kill_var = 0;
// Condition variables
pthread_cond_t cv_producer = PTHREAD_COND_INITIALIZER;
pthread_cond_t cv_consumer = PTHREAD_COND_INITIALIZER;
// mutexes
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Check if Kill_var has been changed
//Signal Handler for SIGINT
void sig_int_handler(int signal){
    // Wake all possible sleeping threads
    kill_var = 1;
    pthread_cond_broadcast(&cv_consumer);
    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(threads[i],NULL);
    }
    exit(0);
    
}

void *start_function(void *);

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    // Creating thread pool
    
    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_create(&threads[i], NULL, start_function, NULL);
    }
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    /* create socket */

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* fill in port number to listen on. IP address can be anything (INADDR_ANY)
     */

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* bind socket to this port number on this machine */

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    /* listen for incoming connection requests */
    listen(sockfd, 1000);
    clilen = sizeof(cli_addr);
    // total accepts
    // unsigned long long accepted = 0;

    while (1)
    {
        /* accept a new request, create a newsockfd */

        // check if queue size < QUEUE_SIZE
        // if yes then add sockfd and send a signal via CV-Consumer
        // else go to sleep till some queue slot is empty, i.e. receive signal via CV_Producer

        // Accept request
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        // Queue is the CS, push newsockfd in Queue if slot available
        // critical section starts

        // Count accepted requests
        // accepted++;
        //Test check done
        pthread_mutex_lock(&lock);
        // changed here too
        // because other threads might get pre-epmted
        while (CONN_QUEUE.size() >= QUEUE_SIZE)
            pthread_cond_wait(&cv_producer, &lock);
        CONN_QUEUE.push(newsockfd);
        // Signal to CV_Consumer
        pthread_cond_signal(&cv_consumer);
        pthread_mutex_unlock(&lock);
        // Critical section done
    }
    close(sockfd);
    return 0;
}

void *start_function(void *)
{
    char buffer[1024];
    int n, m;
    int clisockfd;
    
    signal(SIGINT, sig_int_handler);
    while (1)
    {
        // Possibly a while loop starts here
        // acquire lock on CONN_QUEUE, check empty to sleep else pop() from it
        // rest goes as given below
        // release lock after getting a SOCKFD
        // one cond var for thread pool
        // one for server

        /* read message from client */
        // critical section starts

        pthread_mutex_lock(&lock);
        // making change here
        // because double check required to access QUEUE
        while (CONN_QUEUE.empty() || kill_var == 1)
        {
            if(kill_var == 1)
            {
                pthread_mutex_unlock(&lock);
                pthread_exit(0);
            }

            pthread_cond_wait(&cv_consumer, &lock);
        }
        clisockfd = CONN_QUEUE.front();
        CONN_QUEUE.pop();

        // generate some signal for main thread to know that a worker is now available

        pthread_cond_signal(&cv_producer);
        pthread_mutex_unlock(&lock);
        // Critical section done

        // Code same as week3 from here
        bzero(buffer, 1024);
        n = read(clisockfd, buffer, 511);
        if (n < 0)
            error("ERROR reading from socket");
        // printf("Here is the message: %s", buffer);
        if (n == 0)
        {
            close(clisockfd);
            continue;
        }
        /* send reply to client */
        // Here response will be generated

        HTTP_Response *incoming = handle_request(string(buffer));
        string result;
        result = incoming->get_string();
        // clear the response memory 
        delete incoming;

        // Server sends the response to the client
        m = write(clisockfd, result.c_str(), result.length());
        if (m < 0)
            error("ERROR writing to socket");
        // now that request has been served we'll close the socket
        close(clisockfd);
        
    }
    // after sending response we loop back to see if some new request is available
    return NULL;
}

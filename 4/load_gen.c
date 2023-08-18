/* run using: ./load_gen localhost <server port> <number of concurrent users>
   <think time (in s)> <test duration (in s)> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>

#include <pthread.h>
#include <sys/time.h>

int time_up;
FILE *log_file;

// user info struct
struct user_info
{
    // user id
    int id;

    // socket info
    int portno;
    char *hostname;
    float think_time;

    // user metrics
    int total_count;
    float total_rtt;
};

// error handling function
void error(char *msg)
{
    perror(msg);
    exit(0);
}

// time diff in seconds
float time_diff(struct timeval *t2, struct timeval *t1)
{
    return (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1e6;
}

// user thread function
void *user_function(void *arg)
{
    /* get user info */
    struct user_info *info = (struct user_info *)arg;

    int sockfd, n;
    char buffer[1024];
    struct timeval start, end;

    struct sockaddr_in serv_addr;
    struct hostent *server;
    // while(time_up == 1);

    while (1)
    {
        /* start timer */
        gettimeofday(&start, NULL);

        /* TODO: create socket */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            error("ERROR opening socket");

        /* TODO: set server attrs */
        server = gethostbyname(info->hostname);
        if (server == NULL)
        {
            fprintf(stderr, "ERROR, no such host\n");
            exit(0);
        }
        bzero((char *)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
              server->h_length);
        serv_addr.sin_port = htons(info->portno);

        /* TODO: connect to server */

        // if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        //     error("ERROR connecting");
        while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            error("ERROR connecting");

        /* TODO: send message to server */

        bzero(buffer, 1024);
        char server_port[10] = {};
        sprintf(server_port, "%d",info->portno);
        char to_send[] = "GET /apart3/index.html HTTP/1.1 \nHost: ";
        strcat(to_send,info->hostname);
        strcat(to_send,":");
        strcat(to_send,server_port);
        // strcpy(buffer,to_send);
        //printf("%d\n",info->portno);
        // printf("%s\n",to_send);
        
        n = write(sockfd, to_send, strlen(to_send));
        if (n < 0)
            error("ERROR writing to socket");
        bzero(buffer, 1024);

        /* TODO: read reply from server */

        n = read(sockfd, buffer, 1024);
        if (n < 0)
            error("ERROR reading from socket");

        //printf("%s",buffer);
        /* TODO: close socket */
        close(sockfd);

        /* end timer */
        gettimeofday(&end, NULL);

        /* if time up, break */
        if (time_up)
            break;
            

        /* TODO: update user metrics */
        info->total_count += 1;
        info->total_rtt += time_diff(&end, &start);

        /* TODO: sleep for think time */
        usleep(info->think_time);
    }

    /* exit thread */
    fprintf(log_file, "User #%d finished\n", info->id);
    fflush(log_file);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int user_count, portno, test_duration;
    float think_time;
    char *hostname;

    if (argc != 6)
    {
        fprintf(stderr,
                "Usage: %s <hostname> <server port> <number of concurrent users> "
                "<think time (in s)> <test duration (in s)>\n",
                argv[0]);
        exit(0);
    }

    hostname = argv[1];
    portno = atoi(argv[2]);
    user_count = atoi(argv[3]);
    think_time = atof(argv[4])*1e6;
    test_duration = atoi(argv[5]);

    printf("Hostname: %s\n", hostname);
    printf("Port: %d\n", portno);
    printf("User Count: %d\n", user_count);
    printf("Think Time: %f s\n", think_time/1e6);
    printf("Test Duration: %d s\n", test_duration);

    /* open log file */
    log_file = fopen("load_gen.log", "w");

    pthread_t threads[user_count];
    struct user_info info[user_count];
    struct timeval start, end;

    /* start timer */
    // gettimeofday(&start, NULL);
    // time_up = 1;

    for (int i = 0; i < user_count; ++i)
    {
        /* TODO: initialize user info */
        info[i].id = i;
        info[i].portno = portno;
        info[i].hostname = hostname;
        info[i].think_time = think_time;
        info[i].total_count = 0;
        info[i].total_rtt = 0.0;

        /* TODO: create user thread */
        pthread_create(&threads[i], NULL, user_function, (void *)&info[i]);

        fprintf(log_file, "Created thread %d\n", i);
    }
    /* start timer */
    gettimeofday(&start, NULL);
    time_up = 0;

    /* TODO: wait for test duration */
    sleep(test_duration);

    fprintf(log_file, "Woke up\n");

    /* end timer */
    time_up = 1;
    gettimeofday(&end, NULL);

    /* TODO: wait for all threads to finish */
    for (int i = 0; i< user_count ; i++)
    {
        pthread_join(threads[i],NULL);
    }
    float time_dif = time_diff(&end, &start);

    /* TODO: print results */
    unsigned long long total_load = 0;
    double total_time = 0.0;
    for (int i = 0; i< user_count ; i++)
    {
        total_load += info[i].total_count;
        total_time += info[i].total_rtt;
    }
    double tp = 1.0*total_load/time_dif;
    // printf("Total Served : %llu Total RTT : %lf Time diff: %lf",total_load,total_time, time_dif);
    printf("User Count : %d\nThroughput : %lf\nResponse Time : %lf",user_count,tp,1.0*total_time/total_load);
    /* close log file */
    fclose(log_file);

    return 0;
}

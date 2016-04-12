/*
 * ASSIGNMENT 3 for CS3305
 * Jonathan Lam, 250754703 (jlam323)
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <pthread.h>
#include <semaphore.h>

#define MAX_MSG_SIZE 1024
#define MAX_CONNECTIONS 10				//max number of connections in the queue
#define N 5								//max number of threads active at one time

int connfd_array[MAX_CONNECTIONS];		//array containing the connection ID's
int first, empty;						//indicates the first connection in line and the first empty spot in the queue



/*
 * This function receives a number from the client, multiplies it by 10,
 * and returns the new number. Modified from the source code provided by
 * the instructor.
 */

void multiplyNum(int connfd)
{
	int numbytes;
	char recvBuff[MAX_MSG_SIZE], sendBuff[MAX_MSG_SIZE];
	time_t ticks;
	int num, newnum;
	int rv;

	memset(recvBuff, '0', sizeof(recvBuff));
	memset(sendBuff, '0', sizeof(sendBuff));

	/*receive data from the client*/
	numbytes = recv(connfd, recvBuff, sizeof(recvBuff) - 1, 0);
	if (numbytes == -1) {
		perror("recv");
		exit(1);
	}
	recvBuff[numbytes] = '\0';

	/*Receive the number */
	sscanf(recvBuff, "%d", &num);

	newnum = num * 10;

	snprintf(sendBuff, sizeof(sendBuff), "%d", newnum);
	send(connfd, sendBuff, strlen(sendBuff), 0);

	close(connfd);
}



/*
 * This function is the starting function for each of the 5 threads.
 */
void *threadStart(void *arg)
{
	int connfd;

	//loop for processing connections and also sleeping
	while (1)
	{
		//if there are threads available and connections waiting
		while (first != empty)
		{
			//get the connection ID to be processed
			connfd = connfd_array[first];

			//check if end of the array is reached and mark connection as processed
			if (first < 19)
				first++;
			else
				first = 0;

			//call function to connect to client
			multiplyNum(connfd);
		}

		sleep(1);
	}
}



/*
 * Main method of the server. Modified from the source coded provided by
 * the instructor.
 */
int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 
    int rv;
	int count;

	//array of thread ID's
	pthread_t threads;

    /*Command line argument: port number*/
    if(argc != 2)
    {
        printf("\n Usage: %s port \n",argv[0]);
        return 1;
    }

    /*Socket creation and binding*/
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd <  0) {
      perror("Error in socket creation");
      exit(1);
    }
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1])); 

    rv = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    if (rv <  0) {
      perror("Error in binding");
      exit(1);
    }

	//initialize the connections array
	first = 0;
	empty = 1;
	for (count = 0; count < 20; count++)
		connfd_array[count] = 0;

    listen(listenfd, 10); 

	//listen for the first connection
	connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
	
	//loop to create the pool of N threads
	for (count = 0; count < N; count++)
	{
		//create thread
		pthread_create(&threads, NULL, threadStart, NULL);
	}


    //loop for server to run
    while(1)
    {
		//listen for the connection
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

		//check if the array is full
		if (first == empty)
			printf("Connections array is full");
		//if not full, put the connection into the array queue
		else
		{
			//record the connection ID and move the pointer
			connfd_array[first] = connfd;
			//check if end of the array is reached
			if (empty < 19)
				empty++;
			//go back to the beginning
			else
				empty = 0;
		}

        sleep(1);
     }
}

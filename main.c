//
// Created by verka on 29. 12. 2021.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "pthread.h"

typedef struct dataClient{
    char * login;
    int socket;
} DATAC;

DATAC poleKlientov[100];
int * pocet = 0;

void * komunikacia(void * data){
    DATAC * datac = data;
    int n;
    char buffer[256];
    int newsockfd = datac->socket;
    char login[100];
    bzero(login,100);

    recv(newsockfd, login, 100, 0);

    n = read(newsockfd, login, 99);
    if (n < 0)
    {
        perror("Error reading from socket");
        return 4;
    }
    printf("Here is the login: %s\n", login);



    datac->login = strcpy(datac->login, login);
    poleKlientov[*pocet] = *datac;
    (*pocet)++;
    printf("Som tu");




    char contact[100];

    while(1){
        recv(newsockfd, contact, 100, 0);
        n = read(newsockfd, contact, 100);
        if (n > 0)
        {
            printf("Som tu\n");

            break;
        }
        printf("Som tu\n");
    }

    for (int i = 0; i < pocet; ++i) {
        if(poleKlientov[i].login == contact)
        {
            bzero(buffer,256);
            n = read(newsockfd, buffer, 255);
            if (n < 0)
            {
                perror("Error reading from socket");
                return 4;
            }
            printf("Here is the message: %s\n", buffer);

            const char* msg = "I got your message";
            n = write(poleKlientov[i].socket, msg, strlen(msg)+1);
            if (n < 0)
            {
                perror("Error writing to socket");
                return 5;
            }

        }
    }



}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    char buffer[256];

    if (argc < 2)
    {
        fprintf(stderr,"usage %s port\n", argv[0]);
        return 1;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error binding socket address");
        return 2;
    }

    listen(sockfd, 5);
    cli_len = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
    if (newsockfd < 0)
    {
        perror("ERROR on accept");
        return 3;
    }

    DATAC client;
    client.socket = newsockfd;
    pthread_t vlakno;
    pthread_create(&vlakno, NULL, komunikacia, &client);
    pthread_join(vlakno, NULL);
}
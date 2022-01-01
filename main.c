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
    int id;
} DATAC;

DATAC poleKlientov[100];
int pocet = 0;

void * komunikacia(void * data){

    DATAC * datac = data;
    int skonci = 0;
    while(skonci == 0){
        int n = 0;
        char buffer[256];
        int newsockfd = datac->socket;

        poleKlientov[pocet] = *datac;
        (pocet)++;

        char contact[100];



            n = read(newsockfd, contact, 99);
            if (n < 0)
            {
                perror("Error reading from socket");
            }
            if(strcmp(contact,"exit") == 0){
                skonci = 1;
                break;
            }
            sprintf(buffer,"Here is the contact: %s\n", contact);
            printf("%s", buffer);


        n = 0;
            int nasielSA = 0;
            for (int i = 0; i < (pocet); ++i) {
                if (strcmp(poleKlientov[i].login, contact) != 0) {
                    nasielSA = 1;
                    bzero(buffer, 256);
                    n = read(newsockfd, buffer, 255);
                    if (n < 0) {
                        perror("Error reading from socket");
                        return NULL;
                    }
                    printf("Here is the message: %s\n", buffer);

                    const char *msg = "I got your message";
                    n = write((poleKlientov[i]).socket, msg, strlen(msg) + 1);
                    if (n < 0) {
                        perror("Error writing to socket");
                        return NULL;
                    }



            }
            printf("%d", nasielSA);
            n = nasielSA;
        }
        printf("%d",skonci);
    }
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd,n;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;

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
    char buffer[256];
    bzero(buffer,256);
    char login[100];
    bzero(login,100);
    n = read(newsockfd, login, 99);
    if (n < 0)
    {
        perror("Error reading from socket");
    }
    strcpy((client.login),login);
    sprintf(buffer,"Here is the login: %s\n", (client.login));
    printf("%s", buffer);

    pthread_t vlakno;

    pthread_create(&vlakno, NULL, &komunikacia, &client);
    pthread_join(vlakno, NULL);

}
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
#include <string.h>

typedef struct dataClient {
    char *login;
    int socket;
    int id;
} DATAC;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

DATAC *poleKlientov[100];
static int pocet;

void trim(char *string, int dlzka) {
    int i;
    for (i = 0; i < dlzka; i++) { // trim \n
        if (string[i] == '\n') {
            string[i] = '\0';
            break;
        }

    }

}

void pridatKlienta(DATAC *client){
    pthread_mutex_lock(&mutex);

    for(int i=0; i < 100; ++i){
        if(!poleKlientov[i]){
            (poleKlientov[i]) = client;
            break;
        }
    }


    pthread_mutex_unlock(&mutex);
}

void *komunikacia(void *data) {
    DATAC *datac = data;
    int n = 0;
    int newsockfd = datac->socket;

    char login[100];
    char buffer[256];

    bzero(login, 100);
    n = read(newsockfd, login, 99);
    if (n < 0) {
        perror("Error reading from socket");
    }
    trim(login, 100);
    strcpy((datac->login), login);
    sprintf(buffer, "Here is the login: %s\n", (datac->login));
    printf("%s", buffer);
    pocet++;
    printf("Počet prihlásených: %d \n",pocet);

    for(int i=0; i < 100; ++i){
        if((poleKlientov[i])){
            printf("Klient: %s",(*poleKlientov[i]).login);
        }
    }

    while (1) {
        pthread_mutex_lock(&mutex);
        char contact[100];
        n = read(newsockfd, contact, 99);
        if (n < 0) {
            perror("Error reading from socket");
        }
        if (strcmp(contact, "exit") == 0) {
            break;
        }


        trim(contact, 100);
        sprintf(buffer, "Here is the contact: %s\n", contact);
        printf("%s", buffer);

        bzero(buffer, 256);
        n = 0;

        int nasielSA = 0;
        for (int i = 0; i < pocet; ++i) {
            printf("%d. %s %s\n", i, (*poleKlientov[i]).login, contact);
            if (strcmp((*poleKlientov[i]).login, contact) == 0) {
                nasielSA = 1;
                bzero(buffer, 256);
                n = read(newsockfd, buffer, 255);
                if (n < 0) {
                    perror("Error reading from socket");
                    return NULL;
                }
                printf("Here is the message: %s\n", buffer);

                n = write((*poleKlientov[i]).socket, buffer, strlen(buffer));
                if (n < 0) {
                    perror("Error writing to socket");
                    return NULL;
                }

            }

        }
        bzero(buffer, 256);

        if (nasielSA == 0) {
            bzero(buffer, 256);
            n = read(newsockfd, buffer, 255);
            if (n < 0) {
                perror("Error reading from socket");
                return NULL;
            }
            printf("Message: %s\n", buffer);


        }
        bzero(buffer, 256);
        pthread_mutex_unlock(&mutex);

    }
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, n;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr, "usage %s port\n", argv[0]);
        return 1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding socket address");
        return 2;
    }

    listen(sockfd, 5);
    int pocetKlientov = 0;

    while (1) {
        cli_len = sizeof(cli_addr);

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            return 3;
        }

        DATAC client;
        pocetKlientov++;
        client.socket = newsockfd;
        char buffer[256];
        bzero(buffer, 256);


        pthread_t vlakno;
        pridatKlienta(&client);
        pthread_create(&vlakno, NULL, &komunikacia, &client);
    }
}
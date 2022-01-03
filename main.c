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
    char login[100];
    int socket;
    int id;
} DATAC;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

DATAC *poleKlientov[100] = {NULL};
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

void pridatKlienta(DATAC *client) {
    pthread_mutex_lock(&mutex);

    for (int i = 0; i < 100; ++i) {
        if (!poleKlientov[i]) {
            (poleKlientov[i]) = client;
            break;
        }
    }

    for (int i = 0; i < pocet; ++i) {
        printf("%s \n", poleKlientov[i]->login);
    }
    pthread_mutex_unlock(&mutex);
}

void *komunikacia(void *data) {
    printf("SOM TU 6 \n");

    DATAC *datac = (DATAC *) data;
    int n = 0;

    int newsockfd = (*datac).socket;

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

    pridatKlienta(datac);

    printf("Počet prihlásených: %d \n", pocet);

    while (1) {

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


        for (int i = 0; i < pocet; ++i) {
            printf("%d. %s \n", i, (*poleKlientov[i]).login);
        }


        int nasielSA = 0;
        for (int i = 0; i < pocet; ++i) {
            printf("%d. %s %s\n", i, poleKlientov[i]->login, contact);
            if (strcmp(poleKlientov[i]->login, contact) == 0) {
                nasielSA = 1;
                bzero(buffer, 256);
                n = read(newsockfd, buffer, 255);
                if (n < 0) {
                    perror("Error reading from socket");
                    return NULL;
                }
                printf("Here is the message: %s\n", buffer);

                n = write(poleKlientov[i]->socket, buffer, strlen(buffer));
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
    }
}
void hlavneMenu(DATAC *data);

void prihlasenie(DATAC *data) {
    int n;
    char login[100];
    char buffer[256];
    char password[100];


    while (1) {
        bzero(login, 100);
        bzero(buffer, 256);


        n = read(data->socket, login, 99);
        if (n < 0) {
            perror("Error reading from socket");
        }
        trim(login, 100);
        strcpy((data->login), login);
        sprintf(buffer, "Here is the login: %s\n", (data->login));

        bzero(password, 100);
        n = read(data->socket, password, 99);
        if (n < 0) {
            perror("Error reading from socket");
        }
        trim(password, 100);

        FILE *subor;

        subor = fopen("loginy.txt", "r");
        if (subor == NULL) {
            fputs("Error at opening File!", stderr);
            exit(1);
        }
        printf("Súbor otvorený \n");

        int index = 0;
        char line[256];
        int nasloSa = 0;
        int pocetRiadkov = 0;
        while (fscanf(subor, "%s", line) != EOF) {
            pocetRiadkov++;
            if (pocetRiadkov % 2 == 1) {
                printf("%s \n", line);
                if (strcmp(line, login) == 0) {
                    printf("Username is already used.\n");
                    const char *msg = "Username is already used.";
                    nasloSa = 1;
                    index = pocetRiadkov + 1;


                }
            }
        }

        fclose(subor);
        if (nasloSa == 1) {

            subor = fopen("loginy.txt", "r");
            if (subor == NULL) {
                fputs("Error at opening File!", stderr);
                exit(1);
            }
            printf("Súbor otvorený po 2hykrat\n");


            nasloSa = 0;
            pocetRiadkov = 0;
            int spravneHeslo = 0;
            while (fscanf(subor, "%s", line) != EOF) {
                pocetRiadkov++;
                if (pocetRiadkov == index) {
                    if (strcmp(password, line) == 0) {
                        printf("Uspesne prihlasenie\n");
                        spravneHeslo = 1;
                        break;
                    }
                }
            }
            if (spravneHeslo == 1) {
                n = write(data->socket, &spravneHeslo, sizeof(spravneHeslo));
                if (n < 0) {
                    perror("Error writing to socket");
                    return;
                }
                break;
            }


        }
        else{
            int spravneHeslo = 0;
            n = write(data->socket, &spravneHeslo, sizeof(spravneHeslo));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }

        }
    }
    hlavneMenu(data);


}

void registration(DATAC *data) {
    int n;
    char login[100];
    char buffer[256];
    char password[100];

    while (1) {
        bzero(login, 100);
        bzero(buffer, 256);
        n = read(data->socket, login, 99);
        if (n < 0) {
            perror("Error reading from socket");
        }
        trim(login, 100);
        FILE *subor;

        subor = fopen("loginy.txt", "r");
        if (subor == NULL) {
            fputs("Error at opening File!", stderr);
            exit(1);
        }
        printf("Súbor otvorený \n");


        char line[256];
        int nasloSa = 0;
        int pocetRiadkov = 0;
        while (fscanf(subor, "%s", line) != EOF) {
            pocetRiadkov++;
            if (pocetRiadkov % 2 == 1) {
                printf("%s \n", line);
                if (strcmp(line, login) == 0) {
                    printf("Username is already used.\n");
                    const char *msg = "Username is already used.";
                    nasloSa = 1;

                }
            }
        }
        fclose(subor);
        n = write(data->socket, &nasloSa, sizeof(nasloSa));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }
        if (nasloSa == 0) {
            bzero(password, 100);
            n = read(data->socket, password, 99);
            if (n < 0) {
                perror("Error reading from socket");
            }
            trim(password, 100);
            printf("Zadané heslo %s \n",password);
            FILE *subor;

            subor = fopen("loginy.txt", "a");
            if (subor == NULL) {
                fputs("Error at opening File!", stderr);
                exit(1);
            }
            fprintf(subor, "%s\n", login);
            fprintf(subor, "%s\n", password);

            fclose(subor);
            break;
        }

    }

    printf("Login zapísaný \n");
    hlavneMenu(data);
}

void hlavneMenu(DATAC *data) {
    int n;
    int poziadavka;
    n = read(data->socket, &poziadavka, sizeof(poziadavka));
    if (n < 0) {
        perror("Error reading from socket");
        return;
    }
    printf("%d \n", poziadavka);
    if (poziadavka == 1) {
        registration(data);
    }
    else if(poziadavka == 2){
        prihlasenie(data);
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

        DATAC *client = (DATAC *) malloc(sizeof(DATAC));
        client->socket = newsockfd;
        pthread_t vlakno;

        hlavneMenu(client);
        //pthread_create(&vlakno, NULL, &komunikacia, (void *) client);
    }
}
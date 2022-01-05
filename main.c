
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

void vlozitDoSuboru(char *riadok, char *nazovSuboru) {
    char slovo[100];
    sprintf(slovo, "%s", riadok);
    FILE *subor;
    subor = fopen(nazovSuboru, "a");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }
    fprintf(subor, "%s\n", slovo);

    fclose(subor);
}

void odstranitZoSuboru(int riadok, char *nazovSuboru) {
    FILE *subor;
    FILE *novysubor;
    subor = fopen(nazovSuboru, "r");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }
    novysubor = fopen("nahradny.txt", "w");
    if (novysubor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }

    int pocetRiadkov = 0;
    char string[256];
    while (!feof(subor)) {
        strcpy(string, "\0");
        fgets(string, 256, subor);
        if (!feof(subor)) {
            pocetRiadkov++;
            if (pocetRiadkov != riadok) {
                fprintf(novysubor, "%s", string);
            }
        }
    }
    fclose(subor);
    fclose(novysubor);
    remove(nazovSuboru);
    rename("nahradny.txt", nazovSuboru);
}

int indexSlovaVSubore(char *slovo, char *nazovSuboru) {
    FILE *subor;

    subor = fopen(nazovSuboru, "r");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }
    int index = -1;
    char line[256];
    int nasloSa = 0;
    int pocetRiadkov = 0;
    while (fscanf(subor, "%s", line) != EOF) {
        pocetRiadkov++;
        if (strcmp(line, slovo) == 0) {
            nasloSa = 1;
            index = pocetRiadkov;
            break;
        }
    }

    fclose(subor);
    return index;
}

int rovnaSaRiadku(char *slovo, int riadok, char *nazovSuboru) {
    FILE *subor;

    subor = fopen(nazovSuboru, "r");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }
    int vysledok = 0;
    char line[256];
    int pocetRiadkov = 0;
    while (fscanf(subor, "%s", line) != EOF) {
        pocetRiadkov++;
        if (pocetRiadkov == riadok) {
            if (strcmp(line, slovo) == 0) {
                vysledok = 1;
            }
        }
    }

    fclose(subor);
    return vysledok;
}

void odoberKlienta(DATAC *client) {
    pthread_mutex_lock(&mutex);

    for (int i = 0; i < 100; ++i) {
        if (poleKlientov[i] && (poleKlientov[i]) == client) {
            poleKlientov[i] = NULL;
            break;
        }
    }

    pthread_mutex_unlock(&mutex);
}

void pridatKlienta(DATAC *client) {
    pthread_mutex_lock(&mutex);

    for (int i = 0; i < 100; ++i) {
        if (!poleKlientov[i]) {
            (poleKlientov[i]) = client;
            break;
        }
    }

    pthread_mutex_unlock(&mutex);
}

void *komunikacia(void *data) {
    printf("SOM TU 6 \n");

    DATAC *datac = (DATAC *) data;
    int n = 0;

    int newsockfd = (*datac).socket;

    char buffer[256];

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

void hlavneMenu(DATAC *datas);

void *prihlasenie(void *datas) {
    int n;
    char login[100];
    char buffer[256];
    char password[100];

    DATAC *data = (DATAC *) datas;
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
        int nasloSa = 0;
        int indexProfilu = indexSlovaVSubore(login, "loginy.txt");
        if ( indexProfilu != -1) {
            nasloSa = 1;
        }
        if (nasloSa == 1) {
            int spravneHeslo = rovnaSaRiadku(password,indexProfilu+1,"loginy.txt");

            if (spravneHeslo == 1) {
                n = write(data->socket, &spravneHeslo, sizeof(spravneHeslo));
                if (n < 0) {
                    perror("Error writing to socket");
                    return NULL;
                }
                break;
            }
        } else {
            int spravneHeslo = 0;
            n = write(data->socket, &spravneHeslo, sizeof(spravneHeslo));
            if (n < 0) {
                perror("Error writing to socket");
                return NULL;
            }
        }
    }
    pocet++;
    pridatKlienta(data);

    hlavneMenu(data);

}

void *registration(void *datas) {
    int n;
    char login[100];
    char buffer[256];
    char password[100];
    DATAC *data = (DATAC *) datas;
    while (1) {
        bzero(login, 100);
        bzero(buffer, 256);
        n = read(data->socket, login, 99);
        if (n < 0) {
            perror("Error reading from socket");
        }
        trim(login, 100);
        int nasloSa = 0;
        if(indexSlovaVSubore(login,"loginy.txt") != -1){
            nasloSa = 1;
        }

        n = write(data->socket, &nasloSa, sizeof(nasloSa));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
        if (nasloSa == 0) {
            bzero(password, 100);
            n = read(data->socket, password, 99);
            if (n < 0) {
                perror("Error reading from socket");
            }
            trim(password, 100);
            printf("Zadané heslo %s \n", password);
            vlozitDoSuboru(login,"loginy.txt");
            vlozitDoSuboru(password,"loginy.txt");
            break;
        }
    }
    printf("Login zapísaný \n");
    hlavneMenu(data);
}

void *odhlasenie(void *datas) {
    DATAC *data = (DATAC *) datas;
    pocet--;
    odoberKlienta(data);
    printf("Odhlásenie %s \n", data->login);
    hlavneMenu(data);
}

void *zrusitUcet(void *datas) {
    DATAC *data = (DATAC *) datas;
    int n;
    char buffer[256];
    char password[100];

    while (1) {
        bzero(password, 100);
        n = read(data->socket, password, 99);
        if (n < 0) {
            perror("Error reading from socket");
        }
        trim(password, 100);
        printf("Zadané heslo %s \n", password);
        bzero(buffer, 256);
        FILE *subor, *novysubor;
        subor = fopen("loginy.txt", "r");

        if (subor == NULL) {
            fputs("Error at opening File!", stderr);
            exit(1);
        }


        printf("Súbory otvorene \n");


        char line[256];
        int pocetRiadkov = 0;
        int spravneHeslo = 0;
        int index = 0;
        while (fscanf(subor, "%s", line) != EOF) {
            pocetRiadkov++;
            if (pocetRiadkov % 2 == 1) {
                printf("%s \n", line);
                if (strcmp(line, password) == 0) {
                    printf("Správne heslo.\n");
                    spravneHeslo = 1;
                    index = pocetRiadkov;
                }
            }

        }
        fclose(subor);

        if (spravneHeslo == 1) {

            printf("Správne heslo == 1.\n");
            if (spravneHeslo == 1) {
                printf("Správne heslo == 1 - podmienka platna.\n");

                subor = fopen("loginy.txt", "r");
                if (subor == NULL) {
                    fputs("Error at opening File!", stderr);
                    exit(1);
                }

                printf("pred otvorenim novysubor - temporary.\n");

                novysubor = fopen("nahradny.txt", "w");
                if (novysubor == NULL) {
                    fputs("Error at opening File!", stderr);
                    exit(1);
                }

                pocetRiadkov = 0;
                char string[256];
                printf("prepisovanie suboru do temporary");
                while (!feof(subor)) {
                    strcpy(string, "\0");
                    fgets(string, 256, subor);
                    if (!feof(subor)) {
                        pocetRiadkov++;
                        if (pocetRiadkov != index) {
                            fprintf(novysubor, "%s", string);
                        }
                    }
                }
                printf("koniec prepisovania suboru do temporary");
                fclose(subor);
                fclose(novysubor);
                remove("loginy.txt");
                rename("nahradny.txt", "loginy.txt");

                printf("premenovane subory");
                n = write(data->socket, &spravneHeslo, sizeof(spravneHeslo));
                if (n < 0) {
                    perror("Error writing to socket");
                    return NULL;
                }
                break;
            }
            printf("Zlé heslo\n");
        }

        pocet--;
        odoberKlienta(data);
        hlavneMenu(data);
    }
}

void *pridajPriatelov(void *datas) {
    DATAC *data = (DATAC *) datas;
    int n;
    char buffer[256];
    char contact[100];

    bzero(contact, 100);
    n = read(data->socket, contact, 99);
    if (n < 0) {
        perror("Error reading from socket");
    }
    trim(contact, 100);
    printf("Zadaný kontakt %s \n", contact);
    bzero(buffer, 256);

    int nasielSa = 0;
    for (int i = 0; i < pocet; ++i) {
        if (strcmp(poleKlientov[i]->login, contact) == 0) {
            nasielSa = 1;
            bzero(buffer, 256);
            sprintf(buffer, "Používateľ %s vám poslal žiadosť o priateľstvo. \n", contact);

            n = write(poleKlientov[i]->socket, buffer, strlen(buffer));
            if (n < 0) {
                perror("Error writing to socket");
                return NULL;
            }
        }
    }
    bzero(buffer, 256);

    n = write(data->socket, &nasielSa, sizeof(nasielSa));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }

    if (nasielSa == 0) {
        FILE *subor;

        subor = fopen("loginy.txt", "a");
        if (subor == NULL) {
            fputs("Error at opening File!", stderr);
            exit(1);
        }
        fprintf(subor, "%s\n", contact);
        fprintf(subor, "%s\n", data->login);

        fclose(subor);
    }

    hlavneMenu(data);

}

void *odoberPriatelov(void *datas) {
    DATAC *data = (DATAC *) datas;
    int n;
    char buffer[256];
    char contact[100];

    bzero(contact, 100);
    n = read(data->socket, contact, 99);
    if (n < 0) {
        perror("Error reading from socket");
    }
    trim(contact, 100);
    printf("Zadaný kontakt %s \n", contact);
    bzero(buffer, 256);

    int nasielSa = 0;
    for (int i = 0; i < pocet; ++i) {
        if (strcmp(poleKlientov[i]->login, contact) == 0) {
            nasielSa = 1;
            bzero(buffer, 256);
            sprintf(buffer, "Používateľ %s vás odoberal z priateľov. \n", contact);

            n = write(poleKlientov[i]->socket, buffer, strlen(buffer));
            if (n < 0) {
                perror("Error writing to socket");
                return NULL;
            }
        }
    }
    bzero(buffer, 256);

    n = write(data->socket, &nasielSa, sizeof(nasielSa));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }

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
        pthread_t vlakno_registracia;
        pthread_create(&vlakno_registracia, NULL, &registration, (void *) data);
    } else if (poziadavka == 2) {
        pthread_t vlakno_prihlasenie;
        pthread_create(&vlakno_prihlasenie, NULL, &prihlasenie, (void *) data);
    } else if (poziadavka == 3) {
        pthread_t vlakno;
        pthread_create(&vlakno, NULL, &komunikacia, (void *) data);
    } else if (poziadavka == 4) {
        pthread_t vlakno_odhlasenie;
        pthread_create(&vlakno_odhlasenie, NULL, &odhlasenie, (void *) data);
    } else if (poziadavka == 5) {
        pthread_t vlakno_zrusenie;
        pthread_create(&vlakno_zrusenie, NULL, &zrusitUcet, (void *) data);
    } else if (poziadavka == 6) {
        pthread_t vlakno_pridanie;
        pthread_create(&vlakno_pridanie, NULL, &pridajPriatelov, (void *) data);
    } else if (poziadavka == 7) {
        pthread_t vlakno_odobrania;
        pthread_create(&vlakno_odobrania, NULL, &odoberPriatelov, (void *) data);
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


        hlavneMenu(client);

    }
}

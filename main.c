
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
    char skupina[10][100];
} DATAC;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

DATAC *poleKlientov[100] = {NULL};
static int pocet;

void *konverzaciaSkupina();

void hlavneMenu(DATAC *data);

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

int vlozitDoSuboruData(char *riadok, int n, char *nazovSuboru) {
    char buffer[1025];
    sprintf(buffer, "%s", riadok);
    FILE *subor;
    subor = fopen(nazovSuboru, "a");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        return -1;
    }
    fwrite(buffer, 1, n, subor);

    fclose(subor);
    return 1;
}

void *vytvorSkupKonverzaciu(void *data) {
    printf("uzivatel vytvara skup. konverzaciu \n");
    DATAC *datac = (DATAC *) data;
    int skonci = 0;
    int n;
    char contact[100];
    int index = 0;


    while (skonci == 0) {
        printf("zacina while skonci == 0");
        n = read(datac->socket, contact, 99);
        printf("%s", contact);
        if (n < 0) {
            perror("Error reading from socket");
        }
        trim(contact, 100);
        if (strcmp(contact, "exit") == 0) {
            skonci = 1;
            break;
        }

        int nasielSA = 0;
        printf("zacina for");
        for (int i = 0; i < pocet; ++i) {
            if (strcmp(poleKlientov[i]->login, contact) == 0) {
                nasielSA = 1;
                vlozitDoSuboru(contact, "skupina.txt");
                printf("udaj sa ulozil do suboru");
            }
        }

        //strcpy(poleKlientov[i]->skupina[index], datac->login);
        //strcpy(datac->skupina[index], contact);
        //index++;

        printf("konci for");

        n = write(datac->socket, &nasielSA, sizeof(nasielSA));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
    }
    pthread_t vlakno123;
    pthread_create(&vlakno123, NULL, &konverzaciaSkupina, NULL);
}

void *konverzaciaSkupina() {
    printf("pred break");
    int pocetRiadkov = 0;
    char line[256];

    FILE *subor;
    subor = fopen("skupina.txt", "r");
    while (fscanf(subor, "%s", line) != EOF) {
        pocetRiadkov++;
    }
    fclose(subor);

    printf("pred vkladanim");
    char array[pocetRiadkov][256];
    pocetRiadkov = 0;
    subor = fopen("skupina.txt", "r");
    while (fscanf(subor, "%s", line) != EOF) {
        strcpy(array[pocetRiadkov], line);
        pocetRiadkov++;
    }

    fclose(subor);

    printf("%s", array[0]);


}

int odstranitZoSuboru(int riadok, char *nazovSuboru) {
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

void *posliSubor(void *data) {
    printf("SOM V POSLI SUBOR - SERVER \n");

    DATAC *datac = (DATAC *) data;
    int n = 0;
    char buffer[256];
    int newsockfd = (*datac).socket;

    char contact[100];
    trim(contact, 100);
    sprintf(buffer, "Here is the contact: %s\n", contact);
    printf("%s", buffer);

    bzero(buffer, 256);
    n = 0;


    int nasielSA = 0;
    int read_size = 0;
    int sent_size = 0;
    char buff[20] = {0};
    int j = 0;

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
            printf("Here is the file: %s\n", buffer);

            FILE *subor;
            subor = fopen(buffer, "w");
            if (subor == NULL) {
                fputs("Error at opening File!", stderr);
                exit(1);
            }


            while ((read_size = fread(buffer, 1, 20, subor)) > 0) {
                sent_size = send(poleKlientov[i]->socket, buff, read_size, 0);
                fprintf(stderr, "%d th sent_size  %d\n", j,
                        sent_size); //Just printing how many bytes have been sent in every iteration.
                if (read_size != 20) {
                    fprintf(stderr, "%dth read... read_size is not 20 and it is %d\n", i,
                            read_size); //printing the last few remaining bytes when the last read from the file might not have exact 20 bytes
                }
                j++;
            }

            //send(poleKlientov[i]->socket,status,strlen(status)+1, 0 );

        }

    }


}

void *nacitajPolePriatelov(void *datas) {
    DATAC *data = (DATAC *) datas;
    int n;
    char buffer[256];
    char contact[100];
    FILE *subor;
    subor = fopen("friends.txt", "r");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }
    bzero(contact, 100);
    char meno1[100];
    char meno2[100];
    char line[100];
    int pocetRiadkov = 0;
    while (fscanf(subor, "%s", line) != EOF) {
        pocetRiadkov++;
        if (pocetRiadkov % 2 == 1) {
            strcpy(meno1, line);
        } else {
            strcpy(meno2, line);
            if (strcmp(meno1, data->login) == 0) {
                strcpy(contact, meno2);
                n = write(data->socket, contact, strlen(contact));
                if (n < 0) {
                    perror("Error writing to socket");
                    return NULL;
                }
                printf("poslaneˇ\n");

            }
            if (strcmp(meno2, data->login) == 0) {
                strcpy(contact, meno1);
                n = write(data->socket, contact, strlen(contact));
                if (n < 0) {
                    perror("Error writing to socket");
                    return NULL;
                }
                printf("poslaneˇ\n");

            }
            printf("%s\n", contact);
            int koniec = 0;
            n = write(data->socket, &koniec, sizeof(koniec));
            if (n < 0) {
                perror("Error writing to socket");
                return NULL;
            }
        }
    }
    fclose(subor);
    int koniec = 1;
    n = write(data->socket, &koniec, sizeof(koniec));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }
    printf("Priatelia načítaní. \n");
    hlavneMenu(data);
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
        if (indexProfilu != -1) {
            nasloSa = 1;
        }
        if (nasloSa == 1) {
            int spravneHeslo = rovnaSaRiadku(password, indexProfilu + 1, "loginy.txt");

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
        if (indexSlovaVSubore(login, "loginy.txt") != -1) {
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
            vlozitDoSuboru(login, "loginy.txt");
            vlozitDoSuboru(password, "loginy.txt");
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


    bzero(password, 100);
    n = read(data->socket, password, 99);
    if (n < 0) {
        perror("Error reading from socket");
    }
    trim(password, 100);
    printf("Zadané heslo %s \n", password);
    int spravneHeslo = 0;
    int indexLoginu = indexSlovaVSubore(data->login, "loginy.txt");

    if (rovnaSaRiadku(password, indexLoginu + 1, "loginy.txt") == 1) {
        spravneHeslo = 1;
    }
    printf("spravne heslo %d \n", spravneHeslo);
    n = write(data->socket, &spravneHeslo, sizeof(spravneHeslo));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }

    if (spravneHeslo == 1) {
        printf("Správne heslo == 1.\n");
        int a = odstranitZoSuboru(indexLoginu, "loginy.txt");
        int b = odstranitZoSuboru(indexLoginu, "loginy.txt");

        if (a == 1 && b == 1) {
            printf("Všetko ok vymazane.");
        }
    } else {
        printf("Zlé heslo\n");
        hlavneMenu(data);
    }
    pocet--;
    //odoberKlienta(data);
    hlavneMenu(data);

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
    if (indexSlovaVSubore(contact, "loginy.txt") != -1) {
        nasielSa = 1;
    }

    printf("Som tu, %d \n", nasielSa);
    n = write(data->socket, &nasielSa, sizeof(nasielSa));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }

    if (nasielSa == 1) {
        vlozitDoSuboru(contact, "requests.txt");
        vlozitDoSuboru(data->login, "requests.txt");

    }

    hlavneMenu(data);

}

void *odoberPriatelov(void *datas) {
    DATAC *data = (DATAC *) datas;
    int n;
    char contact[100];
    printf("Cakkam \n");

    bzero(contact, 100);
    n = read(data->socket, contact, 99);
    if (n < 0) {
        perror("Error reading from socket");
    }
    trim(contact, 100);
    printf("Zadaný kontakt %s \n", contact);

    FILE *subor;
    subor = fopen("friends.txt", "r");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }
    char meno1[100];
    int index1;
    char meno2[100];
    char line[100];
    int pocetRiadkov = 0;
    while (fscanf(subor, "%s", line) != EOF) {
        pocetRiadkov++;
        printf("Som na riadku %d \n", pocetRiadkov);
        if (pocetRiadkov % 2 == 1) {
            printf("Nepárny \n");
            strcpy(meno1, line);
        } else {
            printf("Párny, bude sa porovnávať \n");
            strcpy(meno2, line);

            printf("Porovnávam zadané %s, %s a zo suboru %s,%s. \n", data->login, contact, meno1, meno2);
            if (strcmp(meno1, data->login) == 0 && strcmp(meno2, contact) == 0) {
                index1 = pocetRiadkov;
                printf("Som na riadku %d a index -1 je %d\n", pocetRiadkov, index1);
                break;
            }
            if (strcmp(meno2, data->login) == 0 && strcmp(meno1, contact) == 0) {
                index1 = pocetRiadkov - 1;
                printf("Som na riadku %d a index je index \n", pocetRiadkov);
                break;
            }
        }
    }
    fclose(subor);
    printf("index %d \n", index1);
    odstranitZoSuboru(index1, "friends.txt");
    odstranitZoSuboru(index1, "friends.txt");
    hlavneMenu(data);

}

/*void *odosliData(void * datas){
    DATAC *data = (DATAC *) datas;
    char nazovSuboru[100];
    bzero(nazovSuboru, 100);
    int n;

    FILE *subor;
    subor = fopen("suborcopy.txt", "rb");
    char buffer[256] = {0};
    FILE *fp;

    int counter = 0;
    fp = fopen(nazovSuboru, "r");
    while (fgetc(fp) != EOF)
        counter++;
    printf("there are %d letters", counter);
    fclose(fp);

    n=write(data->socket,&counter,sizeof (counter));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }
    printf("som tu \n");
    if (subor != NULL) {
        while ((n = fread(buffer, 1, sizeof(buffer), subor)) >= counter) {
            send(data->socket, buffer, n, 0);
            printf("%s %d \n", buffer, n);
        }
    }

    fclose(subor);
    hlavneMenu();
}*/

void *prijmiData(void *datas) {
    for (int i = 0; i < pocet; ++i) {
        printf("%s \n",poleKlientov[i]->login);
    }

    DATAC *data = (DATAC *) datas;
    int n;
    char contact[100];
    char buffer[256];
    bzero(buffer, 256);

    int counter;
    n = read(data->socket, &counter, sizeof(counter));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }
    printf("%d", counter);
    int tot = 0;

    while (tot < counter) {
        (n = recv(data->socket, buffer, 1024, 0));
        tot += n;
        printf("%s %d \n", buffer, n);
        int a = vlozitDoSuboruData(buffer, n, "suborcopy.txt");
        if (a != 1) {
            printf("Chyba");
        }
    }

    printf("Received byte: %d\n", tot);
    if (n < 0) {
        perror("Receiving");
    }
    bzero(contact, 100);
    n = recv(data->socket, contact, 99, 0);
    if (n < 0) {
        perror("Error reading from socket");
    }
    trim(contact,100);
    printf("%s", contact);

    int nasielSA = 0;
    for (int i = 0; i < pocet; ++i) {
        if (strcmp(poleKlientov[i]->login, contact) == 0) {
            nasielSA = 1;
            printf("Nasiel sa klient\n");
            char nazovSuboru[100];
            bzero(nazovSuboru, 100);
            int n;
            char buff[256] = {0};
            FILE *fp;
            printf("1\n");
            int count = 0;
            fp = fopen("suborcopy.txt", "r");
            while (fgetc(fp) != EOF)
                count++;
            printf("there are %d letters", count);
            fclose(fp);
            printf("1\n");
            FILE *subor;
            subor = fopen("suborcopy.txt", "rb");
            n=write(poleKlientov[i]->socket,&count,sizeof (count));
            if (n < 0) {
                perror("Error writing to socket");
                return NULL;
            }
            printf("som tu \n");
            if (subor != NULL) {
                while ((n = fread(buff, 1, sizeof(buff), subor)) >= count) {
                    send(poleKlientov[i]->socket, buff, n, 0);
                    printf("%s %d \n", buff, n);
                }
            }

            fclose(subor);
        }
    }
    n = write(data->socket, &nasielSA, sizeof(nasielSA));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }
    printf("Som tu");

    remove("suborcopy.txt");
    printf("Som tu");
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
    } else if (poziadavka == 9) {
        pthread_t skupinovka;
        pthread_create(&skupinovka, NULL, &vytvorSkupKonverzaciu, (void *) data);
    } else if (poziadavka == 10) {
        pthread_t vlakno_data;
        pthread_create(&vlakno_data, NULL, &prijmiData, (void *) data);
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

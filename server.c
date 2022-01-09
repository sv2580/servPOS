
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
    int pocetLudiVSkupine;
} DATAC;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

DATAC *poleKlientov[100] = {NULL};
static int pocet;


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

void *spravySkupinovaKonv(void *data) {
    DATAC *datac = (DATAC *) data;

    printf("Užívateľ %s a jeho skupina \n", datac->login);
    for (int i = 0; i < datac->pocetLudiVSkupine; ++i) {
        printf("%s: \n", datac->skupina[i]);

    }
    int n = 0;
    int newsockfd = (*datac).socket;
    char buffer[500];

    while (1) {
        bzero(buffer, 499);
        for (int i = 0; i < datac->pocetLudiVSkupine - 1; ++i) {
            for (int j = 0; j < pocet; ++j) {
                if (strcmp(poleKlientov[j]->login, datac->skupina[i]) == 0) {
                    bzero(buffer, 499);
                    n = read(newsockfd, buffer, 499);
                    if (n < 0) {
                        perror("Error reading from socket");
                        return NULL;
                    }
                    printf("Here is the message: %s\n", buffer);

                    n = write(poleKlientov[j]->socket, buffer, strlen(buffer));
                    if (n < 0) {
                        perror("Error writing to socket");
                        return NULL;
                    }
                }
            }
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
    DATAC *pole[10];

    while (skonci == 0) {
        bzero(contact, 100);
        n = read(datac->socket, contact, 99);
        printf("%s \n", contact);
        if (n < 0) {
            perror("Error reading from socket");
        }
        trim(contact, 100);
        if (strcmp(contact, "exit") == 0) {
            skonci = 1;
            break;
        }

        printf("%s \n", contact);
        int nasielSA = 0;
        for (int i = 0; i < pocet; ++i) {
            if (strcmp(poleKlientov[i]->login, contact) == 0) {
                nasielSA = 1;
                pole[index] = poleKlientov[i];
                printf("Na index %d je %s \n", index, pole[index]->login);
                vlozitDoSuboru(contact, "skupina.txt");
                break;
            }
        }
        printf("používateľ %d - sa našiel\n", nasielSA);


        n = write(datac->socket, &nasielSA, sizeof(nasielSA));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
        index++;
    }
    pole[index] = datac;
    printf("Na index %d je %s \n", index, pole[index]->login);

    vlozitDoSuboru(datac->login, "skupina.txt");
    FILE *subor;

    subor = fopen("skupina.txt", "r");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }
    char line[256];
    int pocetRiadkov = 0;
    while (fscanf(subor, "%s", line) != EOF) {
        pocetRiadkov++;
    }

    fclose(subor);
    //printf("počet riadkov %d \n", pocetRiadkov);

    for (int i = 0; i < pocetRiadkov; ++i) {
        FILE *file;

        file = fopen("skupina.txt", "r");
        //printf("Súbor otvorený \n");

        if (file == NULL) {
            fputs("Error at opening File!", stderr);
            exit(1);
        }
        char slovo[256];
        n = 0;
        int id = 0;
        while (fscanf(file, "%s", slovo) != EOF) {
            if (strcmp(slovo, pole[i]->login) != 0) {
                strcpy(pole[i]->skupina[id], slovo);
                printf("Niečo %s \n", pole[i]->skupina[id]);
                id++;
            }
            n++;
        }
        fclose(file);

        printf("Login: %s \n", pole[i]->login);
        printf("Skupina: \n");
        for (int j = 0; j < pocetRiadkov - 1; ++j) {
            printf("%s \n", pole[i]->skupina[j]);
        }

        pole[i]->pocetLudiVSkupine = pocetRiadkov;
    }
    remove("skupina.txt");
    hlavneMenu(datac);
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
    int index = -1;

    subor = fopen(nazovSuboru, "r");
    if (subor == NULL) {
        subor = fopen(nazovSuboru, "a");
        return index;
    }
    char line[256];
    int pocetRiadkov = 0;
    while (fscanf(subor, "%s", line) != EOF) {
        pocetRiadkov++;
        if (strcmp(line, slovo) == 0) {
            index = pocetRiadkov;
            break;
        }
    }

    fclose(subor);
    return index;
}

int pocetVyskytov(char *slovo, char *nazovSuboru) {
    FILE *subor;

    subor = fopen(nazovSuboru, "r");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }
    int index = 0;
    char line[256];
    while (fscanf(subor, "%s", line) != EOF) {
        if (strcmp(line, slovo) == 0) {
            index++;
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

    DATAC *datac = (DATAC *) data;
    int n = 0;

    int newsockfd = (*datac).socket;

    char buffer[256];

    printf("Počet prihlásených: %d \n", pocet);

    while (1) {

        char contact[100];
        bzero(contact, 99);
        n = read(newsockfd, contact, 99);
        if (n < 0) {
            perror("Error reading from socket");
        }
        if (strcmp(contact, "exit") == 0) {
            hlavneMenu(data);
            return NULL;
        }

        trim(contact, 100);
        sprintf(buffer, "Here is the contact: %s\n", contact);
        printf("%s", buffer);

        bzero(buffer, 256);
        n = 0;


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
                if (strcmp(contact, "exit") == 0) {
                    hlavneMenu(data);
                    return NULL;
                }

                printf("Here is the message: %s\n", buffer);

                n = write(poleKlientov[i]->socket, buffer, strlen(buffer));
                if (n < 0) {
                    perror("Error writing to socket");
                    return NULL;
                }
                break;
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
    hlavneMenu(data);
}

void *sifrovaneSpravy(void *data) {
    DATAC *datac = (DATAC *) data;
    int n = 0;
    int newsockfd = (*datac).socket;
    char buffer[256];


    char contact[100] = {};
    n = read(newsockfd, contact, 99);
    if (n < 0) {
        perror("Error reading from socket");
    }

    trim(contact, 100);
    int nasielSa = 1;
    if (pocetVyskytov(contact, "loginy.txt") == 0) {
        nasielSa = 0;
    }
    n = write(newsockfd, &nasielSa, sizeof(nasielSa));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }
    sprintf(buffer, "Contact: %s\n", contact);
    printf("%s", buffer);
    bzero(buffer, 256);
    n = 0;

    n = read(newsockfd, buffer, 255);
    if (n < 0) {
        perror("Error reading from socket");
        return NULL;
    }

    int posun;
    n = read(newsockfd, &posun, sizeof(posun));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }
    char sifra[256];
    printf("Message: %s\n", buffer);
    for (int i = 0; i < strlen(buffer); ++i) {
        char znak = buffer[i];
        char zasifrovanyZnak = 'a' + ((znak - 'a') + posun) % ('z' - 'a');
        sifra[i] = zasifrovanyZnak;
        printf("Vlakno zasifrovalo %d znak %c na znak %c\n", znak, i + 1, zasifrovanyZnak);
    }
    vlozitDoSuboru(datac->login, "sifrovane.txt");
    vlozitDoSuboru(contact, "sifrovane.txt");
    char slovo[100];
    FILE *subor;
    subor = fopen("sifrovane.txt", "a");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }
    fprintf(subor, "%d\n", posun);

    fclose(subor);
    vlozitDoSuboru(sifra, "sifrovane.txt");

    hlavneMenu(datac);
}

void *desifrovanieSpravy(void *data) {
    DATAC *datac = (DATAC *) data;
    int n = 0;
    int newsockfd = (*datac).socket;
    int p = indexSlovaVSubore(datac->login, "sifrovane.txt");
    printf("%d ", p);

    n = write(newsockfd, &p, sizeof(p));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }
    if (p != -1 && p != 1) {
        printf("p = %d \n", p);
        char line[256];
        int posun;


        FILE *subor;
        subor = fopen("sifrovane.txt", "r");
        if (subor == NULL) {
            fputs("Error at opening File!", stderr);
            exit(1);
        }


        int pocetRiadkov = 0;
        while (fscanf(subor, "%s", line) != EOF) {

            pocetRiadkov++;
            if (pocetRiadkov == (p - 1)) {
                n = write(newsockfd, line, strlen(line));
                if (n < 0) {
                    perror("Error writing to socket");
                    return NULL;
                }
            }
            if (pocetRiadkov == (p + 1)) {
                posun = atoi(line);
                n = write(newsockfd, &posun, sizeof(posun));
                if (n < 0) {
                    perror("Error writing to socket");
                    return NULL;
                }
            }
            if (pocetRiadkov == (p + 2)) {
                break;
            }
        }
        printf("Zašifrované slovo: %s \n", line);
        n = write(newsockfd, line, strlen(line));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }

        fclose(subor);
        odstranitZoSuboru(p - 1, "sifrovane.txt");
        odstranitZoSuboru(p - 1, "sifrovane.txt");
        odstranitZoSuboru(p - 1, "sifrovane.txt");
        odstranitZoSuboru(p - 1, "sifrovane.txt");
    }
    hlavneMenu(datac);
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
        if (strcmp(login, "exit") == 0) {
            hlavneMenu(data);
            return NULL;
        }
        strcpy((data->login), login);
        sprintf(buffer, "Here is the login: %s\n", (data->login));

        bzero(password, 100);
        n = read(data->socket, password, 99);
        if (n < 0) {
            perror("Error reading from socket");
        }
        trim(password, 100);
        if (strcmp(login, "exit") == 0) {
            hlavneMenu(data);
            return NULL;
        }
        int nasloSa = 0;
        int indexProfilu = indexSlovaVSubore(login, "loginy.txt");
        int indexHesla = rovnaSaRiadku(password, (indexProfilu + 1), "loginy.txt");
        if (indexProfilu != -1 && indexHesla == 1) {
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
    char buff[256] = {0};

    FILE *fp;
    int count = 0;
    fp = fopen("friends.txt", "r");
    while (fgetc(fp) != EOF)
        count++;

    if (fp == NULL || count == 0) {
        n = write(data->socket, &count, sizeof(count));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
        hlavneMenu(data);
        return NULL;
    }

    fclose(fp);
    n = write(data->socket, &count, sizeof(count));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }
    if (count == 0) {
    }
    FILE *subor;
    subor = fopen("friends.txt", "rb");


    if (subor != NULL) {
        while ((n = fread(buff, 1, sizeof(buff), subor)) >= count) {
            send(data->socket, buff, n, 0);
            printf("%s %d \n", buff, n);
        }
    }

    fclose(subor);
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
        if (strcmp(login, "exit") == 0) {
            hlavneMenu(data);
            return NULL;
        }
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
            if (strcmp(password, "exit") == 0) {
                hlavneMenu(data);
                return NULL;
            }
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
    if (strcmp(password, "exit") == 0) {
        hlavneMenu(data);
        return NULL;
    }
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
    odoberKlienta(data);
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
    trim(contact, 100);
    if (strcmp(contact, "exit") == 0) {
        hlavneMenu(data);
        return NULL;
    }
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

void *pozriZiadosti(void *datas) {
    DATAC *data = (DATAC *) datas;
    int n, nasielSa = 1;
    char buffer[256];
    char contact[100];

    bzero(contact, 100);
    int index = indexSlovaVSubore(data->login, "requests.txt");
    if (pocetVyskytov(data->login, "requests.txt") == 0) {
        nasielSa = 0;
    }
    printf("Som tu, %d \n", nasielSa);
    n = write(data->socket, &nasielSa, sizeof(nasielSa));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }

    while (nasielSa == 1) {
        FILE *subor;
        subor = fopen("requests.txt", "r");
        if (subor == NULL) {
            fputs("Error at opening File!", stderr);
            exit(1);
        }
        int vysledok = 0;
        char line[256];
        int pocetRiadkov = 0;
        while (fscanf(subor, "%s", line) != EOF) {
            pocetRiadkov++;
            if (pocetRiadkov == index + 1) {
                strcpy(contact, line);
                n = write(data->socket, contact, strlen(contact));
                if (n < 0) {
                    perror("Error writing to socket");
                    return NULL;
                }
                break;
            }
        }

        fclose(subor);
        int poziadavka;
        n = read(data->socket, &poziadavka, sizeof(poziadavka));
        if (n < 0) {
            perror("Error reading from socket");
            return NULL;
        }
        if (poziadavka) {
            vlozitDoSuboru(contact, "friends.txt");
            vlozitDoSuboru(data->login, "friends.txt");
        }
        odstranitZoSuboru(index, "requests.txt");
        odstranitZoSuboru(index, "requests.txt");
        bzero(contact, 100);
        index = indexSlovaVSubore(data->login, "requests.txt");
        if (index % 2 == 0 || index == -1) {
            nasielSa = 0;
        } else {
            nasielSa = 1;
        }
        n = write(data->socket, &nasielSa, sizeof(nasielSa));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
    }
    hlavneMenu(data);

}

void *odoberPriatelov(void *datas) {
    DATAC *data = (DATAC *) datas;
    int n;
    char contact[100];
    bzero(contact, 100);
    n = read(data->socket, contact, 99);
    if (n < 0) {
        perror("Error reading from socket");
    }
    trim(contact, 100);
    if (strcmp(contact, "exit") == 0) {
        hlavneMenu(data);
        return NULL;
    }
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
        if (pocetRiadkov % 2 == 1) {
            strcpy(meno1, line);
        } else {
            strcpy(meno2, line);

            if (strcmp(meno1, data->login) == 0 && strcmp(meno2, contact) == 0) {
                index1 = pocetRiadkov;
                break;
            }
            if (strcmp(meno2, data->login) == 0 && strcmp(meno1, contact) == 0) {
                index1 = pocetRiadkov - 1;
                break;
            }
        }
    }
    fclose(subor);
    odstranitZoSuboru(index1, "friends.txt");
    odstranitZoSuboru(index1, "friends.txt");
    hlavneMenu(data);

}

void *prijmiData(void *datas) {
    for (int i = 0; i < pocet; ++i) {
        printf("%s \n", poleKlientov[i]->login);
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
    while(1) {
        bzero(contact, 100);
        n = recv(data->socket, contact, 99, 0);
        if (n < 0) {
            perror("Error reading from socket");
        }
        trim(contact, 100);
        if (strcmp(contact, "exit") == 0) {
            break;
        }
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
                n = write(poleKlientov[i]->socket, &count, sizeof(count));
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
    }
    remove("suborcopy.txt");
    printf("Som tu");
    hlavneMenu(data);
}

void *spravySPriatelom(void *datas) {
    DATAC *datac = (DATAC *) datas;
    int n = 0;
    int newsockfd = (*datac).socket;
    char buffer[256];
    char contact[100];
    if (pocetVyskytov(datac->login, "friends.txt") == 0) {
        hlavneMenu(datac);
        return NULL;
    }
    n = read(newsockfd, contact, 99);
    if (n < 0) {
        perror("Error reading from socket");
    }
    if (strcmp(contact, "exit") == 0) {
        hlavneMenu(datac);
        return NULL;
    }
    trim(contact, 100);

    char nazovSuboru[200];
    if (strcmp(contact, datac->login) > 0) {
        sprintf(nazovSuboru, "%s%s.txt", contact, datac->login);
        printf("Názov súboru: %s \n", nazovSuboru);
    } else {
        sprintf(nazovSuboru, "%s%s.txt", datac->login, contact);
        printf("Názov súboru: %s \n", nazovSuboru);
    }

    FILE *subor;
    int count = 0;

    subor = fopen(nazovSuboru, "r");
    if (subor == NULL) {
        printf("Súbor %s neexistuje, vytváram \n", nazovSuboru);
        n = write(datac->socket, &count, sizeof(count));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
    } else {
        fclose(subor);
        printf("Súbor %s existuje \n", nazovSuboru);

        FILE *fp;
        fp = fopen(nazovSuboru, "r");
        while (fgetc(fp) != EOF)
            count++;

        fclose(fp);
        n = write(datac->socket, &count, sizeof(count));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
        char buff[256];
        if (fp != NULL && count != 0) {
            subor = fopen(nazovSuboru, "rb");
            if (subor != NULL) {
                while ((n = fread(buff, 1, sizeof(buff), subor)) >= count) {
                    send(datac->socket, buff, n, 0);
                    printf("%s %d \n", buff, n);
                }
            }
            fclose(subor);
        }
    }
    while (1) {
        bzero(buffer, 256);
        char sprava[500];
        bzero(sprava, 500);
        n = read(newsockfd, buffer, 255);
        if (n < 0) {
            perror("Error reading from socket");
            return NULL;
        }
        trim(buffer, 256);
        if (strcmp(buffer, "exit") == 0) {
            break;
        }
        sprintf(sprava, "%s: %s", datac->login, buffer);
        printf("%s - zapísané \n", sprava);
        vlozitDoSuboru(sprava, nazovSuboru);
        bzero(buffer, 256);
    }
    hlavneMenu(datac);
}

void hlavneMenu(DATAC *data) {

    printf("Som v hlavnom menu \n");

    int poziadavka;
    int n = read(data->socket, &poziadavka, sizeof(poziadavka));
    if (n < 0) {
        perror("Error reading from socket");
        return;
    }
    printf("%d \n", poziadavka);
    if (poziadavka == 0) {
        close(data->socket);
        free(data);
    } else if (poziadavka == 1) {
        pthread_t vlakno_registracia;
        pthread_create(&vlakno_registracia, NULL, &registration, (void *) data);
        pthread_join(vlakno_registracia, NULL);
    } else if (poziadavka == 2) {
        pthread_t vlakno_prihlasenie;
        pthread_create(&vlakno_prihlasenie, NULL, &prihlasenie, (void *) data);
        pthread_join(vlakno_prihlasenie, NULL);
    } else if (poziadavka == 5) {
        pthread_t vlakno;
        pthread_create(&vlakno, NULL, &komunikacia, (void *) data);
        pthread_join(vlakno, NULL);
    } else if (poziadavka == 3) {
        pthread_t vlakno_odhlasenie;
        pthread_create(&vlakno_odhlasenie, NULL, &odhlasenie, (void *) data);
        pthread_join(vlakno_odhlasenie, NULL);
    } else if (poziadavka == 4) {
        pthread_t vlakno_zrusenie;
        pthread_create(&vlakno_zrusenie, NULL, &zrusitUcet, (void *) data);
        pthread_join(vlakno_zrusenie, NULL);
    } else if (poziadavka == 13) {
        pthread_t vlakno_pridanie;
        pthread_create(&vlakno_pridanie, NULL, &pridajPriatelov, (void *) data);
        pthread_join(vlakno_pridanie, NULL);
    } else if (poziadavka == 15) {
        pthread_t vlakno_odobrania;
        pthread_create(&vlakno_odobrania, NULL, &odoberPriatelov, (void *) data);
        pthread_join(vlakno_odobrania, NULL);
    } else if (poziadavka == 14) {
        pthread_t vlakno_ziadosti;
        pthread_create(&vlakno_ziadosti, NULL, &pozriZiadosti, (void *) data);
        pthread_join(vlakno_ziadosti, NULL);
    } else if (poziadavka == 7) {
        pthread_t skupinovka;
        pthread_create(&skupinovka, NULL, &vytvorSkupKonverzaciu, (void *) data);
        pthread_join(skupinovka, NULL);
    } else if (poziadavka == 9) {
        pthread_t vlakno_data;
        pthread_create(&vlakno_data, NULL, &prijmiData, (void *) data);
        pthread_join(vlakno_data, NULL);
    } else if (poziadavka == 8) {
        pthread_t vlakno_skupina;
        pthread_create(&vlakno_skupina, NULL, &spravySkupinovaKonv, (void *) data);
        pthread_join(vlakno_skupina, NULL);
    } else if (poziadavka == 11) {
        pthread_t vlakno_sifrovanie;
        pthread_create(&vlakno_sifrovanie, NULL, &sifrovaneSpravy, (void *) data);
        pthread_join(vlakno_sifrovanie, NULL);
    } else if (poziadavka == 12) {
        pthread_t vlakno_desifrovanie;
        pthread_create(&vlakno_desifrovanie, NULL, &desifrovanieSpravy, (void *) data);
        pthread_join(vlakno_desifrovanie, NULL);
    } else if (poziadavka == 6) {
        pthread_t vlakno_spravySPriatelom;
        pthread_create(&vlakno_spravySPriatelom, NULL, &spravySPriatelom, (void *) data);
        pthread_join(vlakno_spravySPriatelom, NULL);
    } else {
        hlavneMenu(data);
    }
}

int mainServer(int argc, char *argv[]) {
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
        client->pocetLudiVSkupine = 0;


        hlavneMenu(client);

    }
    pthread_mutex_destroy(&mutex);
    close(sockfd);
}

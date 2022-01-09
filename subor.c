//
// Created by verka on 9. 1. 2022.
//
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "pthread.h"


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
    return 1;
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
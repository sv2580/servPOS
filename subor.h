//
// Created by verka on 9. 1. 2022.
//


#ifndef SERVER_SUBOR_H
#define SERVER_SUBOR_H

void vlozitDoSuboru(char *riadok, char *nazovSuboru);

int vlozitDoSuboruData(char *riadok, int n, char *nazovSuboru);

int odstranitZoSuboru(int riadok, char *nazovSuboru);

int indexSlovaVSubore(char *slovo, char *nazovSuboru) ;

int pocetVyskytov(char *slovo, char *nazovSuboru);

int rovnaSaRiadku(char *slovo, int riadok, char *nazovSuboru);


#endif //SERVER_SUBOR_H

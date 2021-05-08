#include <stdio.h>
#include <nucleo.h>

FILE *arquivo;

void far processo1() {
  int i = 0;
  while(i < 10000) {
    fprintf(arquivo, "1");
    i++;
  }

  fprintf(arquivo, " Processo 1 terminando ");
  terminar_Processo();
}

void far processo2() {
  int i = 0;
  while(i < 10000) {
    fprintf(arquivo, "2");
    i++;
  }

  fprintf(arquivo, " Processo 2 terminando ");
  terminar_Processo();
}

void far processo3() {
  int i = 0;
  while(i < 10000) {
    fprintf(arquivo, "3");
    i++;
  }

  fprintf(arquivo, " Processo 3 terminando ");
  terminar_Processo();
}

void far processo4() {
  int i = 0;
  while(i < 10000) {
    fprintf(arquivo, "4");
    i++;
  }

  fprintf(arquivo, " Processo 4 terminando ");
  terminar_Processo();
}

main() {
  criar_Processo("P1", processo1, 4);
  criar_Processo("P2", processo2, 2);
  criar_Processo("P3", processo3, 2);
  criar_Processo("P4", processo4, 0);

  arquivo = fopen("saida.txt", "w");

  dispara_sistema();
}

#include <stdio.h>
#include <nucleo.h>

FILE *arquivo;

/* Processo 1 - Envia uma mensagem ao processo 2 e aguarda o recebimento de uma mensagem do processo 3
      Esse processo também envia uma mensagem ao processo 4, que a receberá seletivamente */

void far processo1() {
  int i = 0, codigo_erro;
  char msg[30], emissor[30];

  /* Simulando erro ao enviar uma mensagem a um processo inexistente. O código de erro retornado pelo envia deve ser '1' */
  codigo_erro = envia("P10", "mensagem enviada a um processo inexistente");
  if(codigo_erro != 1) {
    fprintf(arquivo, "Erro esperado mas nao obtido ao tentar enviar uma mensagem a um processo inexistente\n");
  }

  while(i < 100) {
    sprintf(msg, "%d", i);
    fprintf(arquivo, "Processo 1 enviando para P2:  %s\n", msg);
    envia("P2", msg);

    sprintf(msg, "%d", i * 10);
    fprintf(arquivo, "Processo 1 enviando para P4:  %s\n", msg);
    envia("P4", msg);

    recebe(msg, emissor);
    fprintf(arquivo, "Processo 1 recebeu de %s: %s\n", emissor, msg);
    i++;
  }

  terminar_Processo();
}

/* Processo 2 - Recebe uma mensagem do processo 1 e a envia ao processo 3 */
void far processo2() {
  int i = 0;
  char msg[30], emissor[30];

  while(i < 100) {
    recebe(msg, emissor);
    fprintf(arquivo,"Processo 2 recebeu de %s: %s\n", emissor, msg);

    fprintf(arquivo, "Processo 2 enviando para P3: %s\n", msg);
    envia("P3", msg);
    i++;
  }

  terminar_Processo();
}

/* Processo 3 - Recebe uma mensagem do processo 2 e a envia ao processo 1 */
void far processo3() {
  int i = 0;
  char msg[30], emissor[30];

  while(i < 100) {
    recebe(msg, emissor);
    fprintf(arquivo, "Processo 3 recebeu de %s: %s\n", emissor, msg);

    fprintf(arquivo, "Processo 3 enviando para P1: %s\n", msg);
    envia("P1", msg);
    i++;
  }

  terminar_Processo();
}

/* Processo 4 - Recebe seletivamente mensagens enviadas pelo processo 1 (e apenas as enviadas por ele) */
void far processo4() {
  int i = 0;
  char msg[30], emissor[30];

  while(i < 100) {
    recebe_seletivo("P1", msg, emissor);
    fprintf(arquivo, "Processo 4 recebeu de %s: %s\n", emissor, msg);
    i++;
  }

  terminar_Processo();
}

/* Processo 5 - Tenta enviar mensagens ao processo 4, que nunca serão recebidas pois o processo 4 recebe mensagens apenas do processo 1
    Como consequẽncia, esse processo permanecerá bloqueado */
void far processo5() {
  int i = 0;
  char msg[30], emissor[30];

  while(i < 100) {
    envia("P4", "essa mensagem nao sera recebida");
    i++;
  }

  terminar_Processo();
}

/* Processo 6 - Envia mensagens ao processo 9 */
void far processo6() {
  int i = 0, codigo_erro = -1;
  char msg[30], emissor[30];

  while(i < 100) {
    sprintf(msg, "%d", i + 500);
    fprintf(arquivo, "Processo 6 enviando para P9:  %s\n", msg);

    codigo_erro = envia("P9", msg);

    /* Tenta enviar até que exista espaço na fila */
    while(codigo_erro != 0) {
      codigo_erro = envia("P9", msg);
    }

    i++;
  }

  terminar_Processo();
}

/* Processo 7 - Envia mensagens ao processo 9 */
void far processo7() {
  int i = 0, codigo_erro = -1;
  char msg[30], emissor[30];

  while(i < 100) {
    sprintf(msg, "%d", i + 500);
    fprintf(arquivo, "Processo 7 enviando para P9:  %s\n", msg);

    codigo_erro = envia("P9", msg);

    /* Tenta enviar até que exista espaço na fila */
    while(codigo_erro != 0) {
      codigo_erro = envia("P9", msg);
    }

    i++;
  }

  terminar_Processo();
}

/* Processo 8 - Envia mensagens ao processo 9 */
void far processo8() {
  int i = 0, codigo_erro = -1;
  char msg[30], emissor[30];

  while(i < 100) {
    sprintf(msg, "%d", i + 500);
    fprintf(arquivo, "Processo 8 enviando para P9:  %s\n", msg);
    
    codigo_erro = envia("P9", msg);

    /* Tenta enviar até que exista espaço na fila */
    while(codigo_erro != 0) {
      codigo_erro = envia("P9", msg);
    }

    i++;
  }

  terminar_Processo();
}

/* Processo 9 - Recebe as mensagens enviadas pelos processos 6, 7 e 8 */
void far processo9() {
  int i = 0;
  char msg[30], emissor[30];

  while(i < 300) {
    recebe( msg, emissor);
    fprintf(arquivo, "Processo 9 recebeu de %s: %s\n", emissor, msg);
    i++;
  }

  terminar_Processo();
}

main() {
  criar_Processo("P1", processo1, 10);
  criar_Processo("P2", processo2, 10);
  criar_Processo("P3", processo3, 10);
  criar_Processo("P4", processo4, 10);
  criar_Processo("P5", processo5, 10);
  criar_Processo("P6", processo6, 10);
  criar_Processo("P7", processo7, 10);
  criar_Processo("P8", processo8, 10);
  criar_Processo("P9", processo9, 10);

  arquivo = fopen("saida.txt", "w");

  dispara_sistema();
}

#include <system.h>

typedef struct desc_p {
  char nome[35];
  enum {ativo, terminado} estado;
  PTR_DESC contexto;
  struct desc_p *prox_desc;
} DESCRITOR_PROC;

typedef DESCRITOR_PROC *PTR_DESC_PROC;

typedef struct {
  int s;
  PTR_DESC_PROC Q;
} semaforo;

extern void far criar_Processo(char nome_proc[35], void far(*end_proc)(), int max_fila);
extern void far dispara_sistema();
extern void far terminar_Processo();
extern void far inicia_semaforo(semaforo *sem, int n);
extern void far P(semaforo *sem);
extern void far V(semaforo *sem);
extern int far envia(char *nome_destino, char *msg);
extern void far recebe(char *msg, char *p_emissor);
extern void far recebe_seletivo(char *nome_processo, char *msg, char *p_emissor);

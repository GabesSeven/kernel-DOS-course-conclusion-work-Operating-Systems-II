#include <system.h>

typedef struct desc_p {
  char nome[35];
  enum {ativo, terminado} estado;
  PTR_DESC contexto;
  struct desc_p *prox_desc;
} DESCRITOR_PROC;

typedef DESCRITOR_PROC *PTR_DESC_PROC;

extern void far criar_Processo(char nome_proc[35], void far(*end_proc)(), int prioridade);
extern void far dispara_sistema();
extern void far terminar_Processo();

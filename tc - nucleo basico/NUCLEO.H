#include<system.h>
#include<stdio.h>
typedef struct desc_p{
        char nomep[35];
        enum{ativo, terminado} estado;
        PTR_DESC contexto;
        struct desc_p *prox_desc;
}DESCRITOR_PROC;
typedef DESCRITOR_PROC *PTR_DESC_PROC;
extern void far volta_dos();
extern PTR_DESC_PROC far procura_prox_ativo();
extern void far criar_processo(void far (*end_proc)(), char nome_proc[]);
extern void far escalador();
extern void far dispara_sistema();
extern void far termina_processo();
extern void far termina_processo1();



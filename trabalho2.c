#include <stdio.h>
#include <unistd.h>

#include <stdlib.h>

#include <string.h>

// Tamanhos configuráveis
#define MEMORY_LENGHT 10 
#define QUADRO_LENGHT 2
#define MAX_PROC_LENGHT 4

// memória o uso do char foi devido a ele representar um byte
char memory[MEMORY_LENGHT];

// struct que representa uma página livre de meória lista encadeada
typedef struct Page {
    struct Page *next;
    int address;
} Page;

// struct que representa um processo com a tabela sendo um array
typedef struct Proc 
{
    int proc_id;
    int lenght;
    int logic_address_table[];
} Proc;

// O número máximo de processos é o tamanho da memória / pelo tamanho do quadro logo é usado um array
Proc *proccesses[MEMORY_LENGHT/QUADRO_LENGHT];

// ponteiros da lista encadeada
Page *last_page = NULL;
Page *first_page = NULL;


// método que cria uma página livre e a aloca no fim da lista encadeada
void push(int address){
    Page* page = malloc(sizeof(Page));
    page->address = address;
    if(!last_page){
        last_page = page;
        first_page = page;
        return;
    }
    last_page->next = page;
    last_page = page;
}

// método que retira a primeira página da lista encadeada
Page * pop(){
    Page * aux = first_page;
    if (first_page && first_page->next){
        first_page = first_page->next;
    }else {
        first_page = NULL;
        last_page = NULL;
    }
    if(aux){
        aux->next = NULL;
    }
    return aux;
}

// método que faz a visão de uma tabela de páginas passeado em um id de processo
void view_page_table(int proc_id){
    Proc *proccess = NULL;
    for (int i = 0; i < MEMORY_LENGHT/QUADRO_LENGHT; i++){ // procuro um processo com esse id no array de processos
        if(proccesses[i] && proccesses[i]->proc_id == proc_id){
            proccess = proccesses[i];
            break;
        }
    }
    if (!proccess){
        printf("Não foi achado processo com proc_id: %d\n", proc_id);
        return;
    }
    printf("\nProcesso %d\nTamanho: %d bytes\n", proccess->proc_id, proccess->lenght);
    printf("Tabela de Páginas\n");
    for (int i =0; i < proccess->lenght/QUADRO_LENGHT; i++){ // printa iterando pelo tamanho do processo/pelo quadro
        printf("[%d] : [%d]\n", i , proccess->logic_address_table[i]);
    }
}

void show_pages(){ // mostra todas as páginas da memória
    printf("Páginas de memória:\n");
    Page *page = first_page;
    int i;
    if (page){
        printf("(livres)\n");
        do { // mostra todas as páginas livres
            printf("[%d] : [%d]\n      [%d]\n", page->address , memory[page->address], memory[page->address + 1]);
            page = page->next;
        }while (page);
    }
    printf("(ocupadas)\n");
    for (i=0; i < MEMORY_LENGHT/QUADRO_LENGHT; i++){
        if(proccesses[i]){// mostra todas as páginas ocupadas iterando pela array de processos
            for (int aux =0; aux < proccesses[i]->lenght/QUADRO_LENGHT; aux++){
                printf("[%d] : [%d]\n      [%d]\n", proccesses[i]->logic_address_table[aux] , memory[proccesses[i]->logic_address_table[aux]], memory[proccesses[i]->logic_address_table[aux] + 1]);
            }
        }
    }
    
}

void create_proccess(int proc_id, int lenght){// Cria um processo e uma tabela de memória para ele
    if (lenght > MAX_PROC_LENGHT){// Checa se o tamanho do processo não é menor que o tamanho minimo
        printf("proccesso de tamanho %d maior que o tamanho máximo de proccesso permitido: %d\n", lenght, MAX_PROC_LENGHT);
        return;
    }
    Proc * proc  = malloc(sizeof(Proc));
    proc->lenght = lenght;
    proc->proc_id = proc_id;
    for (int i =0; i < proc->lenght/QUADRO_LENGHT; i++){ // iterando pelo numero de páginas necessárias pega as páginas e tenta alocar o endereço para a tabela lógica do processo
        Page* page  = pop();
        if(!page){ // se não tem páginas suficientes recria as páginas de processo que foram tomadas
            printf("Falha ao alocar páginas de memória páginas insuficientes para o proccesso\n");
            for (int aux=0; aux<i; aux++){
                push(proc->logic_address_table[aux]);
            }
            free(proc);
            return;
        }
        proc->logic_address_table[i] = page->address;
    }
    for (int i = 0; i < MEMORY_LENGHT/QUADRO_LENGHT; i++){
        if(proccesses[i] == NULL){ // insere o processo no array de processos
            proccesses[i] = proc;
            break;
        }
    }
}

void free_proccess(int proc_id){ // destroi um processo pegando as informações da tabela lógica e recriando as páginas que foram alocadas para este processo 
    Proc *proccess = NULL;
    for (int i = 0; i < MEMORY_LENGHT/QUADRO_LENGHT; i++){
        if(proccesses[i] && proccesses[i]->proc_id == proc_id){
            proccess = proccesses[i];
            proccesses[i] = NULL;
            break;
        }
    }
    if (!proccess){
        printf("Não foi achado processo com proc_id: %d\n", proc_id);
        return;
    }
    for (int i =0; i < proccess->lenght/QUADRO_LENGHT; i++){
        push(proccess->logic_address_table[i]);
    }

    free(proccess);
    return;
}

int main(){
    for (int i =0; i< MEMORY_LENGHT; i +=QUADRO_LENGHT){ // Criação das páginas de memória baseado nos tamanhos de quadro
        Page* page = malloc(sizeof(Page));
        page->address = i;
        if (i==0){
            first_page = page;
        }
        if (last_page) {
            last_page->next = page;
        }
        last_page = page;
    }
    show_pages();
    view_page_table(4);
    create_proccess(1, 4);
    view_page_table(1);
    show_pages();
    create_proccess(2, 2);
    view_page_table(2);
    show_pages();
    free_proccess(1);
    show_pages();
    create_proccess(3, 6);
    create_proccess(3, 4);
    view_page_table(3);
    create_proccess(4, 4);
    view_page_table(4);
    show_pages();
    create_proccess(5, 2);
    free_proccess(3);
    show_pages();
    create_proccess(5, 2);
    view_page_table(5);
    show_pages();
}
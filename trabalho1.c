#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <stdlib.h>

#define NUM_THREADS_VOLUNTARIO 4 
#define NUM_THREADS_CLIENTE 3 

typedef struct Roupa
{
    int codigo;
    char *modelo;
    float preco;
    char *tamanho;
    struct Roupa *next;
} Roupa;

typedef struct LinkedList{
    int tamanho;
    Roupa *head;
}LinkedList;

int codigo = 300;
pthread_mutex_t mutex_codigo;

Roupa generate_roupa(){
    codigo +=1;
    char *modelos[] = {"camiseta", "bermuda", "calca", "moletom", "roupa intima", "meias"};
    float preco = (float)((rand() % 100) + 10) + (float)(rand() % 100) / 100;
    char *tamanhos[] = {"PP", "P", "M", "G", "GG"};
    int int_tamanho = rand() % 5;    
    int int_modelo = rand() % 6;
    Roupa roupa = {codigo, modelos[int_modelo], preco, tamanhos[int_tamanho], NULL};
    return roupa;
}

LinkedList roupas_venda = {0 , NULL};
pthread_mutex_t mutex_venda; 

LinkedList roupas_reparo = {0 , NULL};
pthread_mutex_t mutex_reparo;

void append_to_list(LinkedList * list, Roupa * roupa){
    if (!list->head){
        list->head = roupa;
        list->tamanho = 0;
    } else {
        Roupa * pointer = list->head;
        while (pointer->next) {
            pointer = pointer->next;
        }
        pointer->next = roupa;
    }
    list->tamanho +=1;
}

Roupa * retrive_from_position(LinkedList * list, int position){
    int i = 0;
    if (position == 0) 
    {
        Roupa *old_head = list->head;
        list->head = old_head->next;
        list->tamanho -= 1;
        return old_head;
    }
    Roupa* current = list->head;
    Roupa* temp_node = NULL;
    for (i = 0; i < position-1; i++) {
        if (current->next == NULL) {
            return NULL;
        }
        current = current->next;
    }

    temp_node = current->next;
    current->next = temp_node->next;
    list->tamanho -= 1;
    return temp_node;
    
}



void *thread_cliente(void *threadid){
    long tid = (int) threadid;
    printf("Thread %d started\n", tid);
    while (1)
    {
        sleep((rand() % 5 )+3);
        int action = rand() % 2;
        switch (action)
        {
            case 0:
            {
                printf("%d c1\n", tid);
                Roupa roupa;

                pthread_mutex_lock(&mutex_codigo);
                roupa = generate_roupa();
                pthread_mutex_unlock(&mutex_codigo);
                pthread_mutex_lock(&mutex_reparo);
                append_to_list(&roupas_reparo, &roupa);
                pthread_mutex_unlock(&mutex_reparo);
                printf("Cliente %d doa roupa : %d\n", tid, roupa.codigo);
                break;
            }
            case 1:
            {
                printf("%d c2\n", tid);
                pthread_mutex_lock(&mutex_venda);
                if (roupas_venda.tamanho != 0) {
                    Roupa * roupa;
                    roupa = retrive_from_position(&roupas_venda, rand() % roupas_venda.tamanho);
                    printf("Cliente %d compra roupa: %d\n", tid, roupa->codigo);
                }
                pthread_mutex_unlock(&mutex_venda);
            }
        }
    }
    
    pthread_exit(NULL);
}



void *thread_voluntario(void *threadid){
    long tid = (int) threadid;
    printf("Thread %d started\n", tid);
    while (1)
    {
        sleep((rand() % 5 )+3);
        int action = rand() % 3;
        switch (action)
        {
            case 0:
            {
                printf("%d v1\n", tid);
                pthread_mutex_lock(&mutex_codigo);
                Roupa roupa;
                roupa = generate_roupa();
                pthread_mutex_unlock(&mutex_codigo);
                pthread_mutex_lock(&mutex_venda);
                append_to_list(&roupas_venda, &roupa);
                pthread_mutex_unlock(&mutex_venda);
                printf("Voluntario %d doa roupa nova: %d\n", tid, roupa.codigo);
                break;
            }
            case 1:
            {
                printf("%d v2\n", tid);
                pthread_mutex_lock(&mutex_venda);
                if (roupas_venda.tamanho != 0) {
                    Roupa * roupa;
                    roupa = retrive_from_position(&roupas_venda, 0);
                    printf("Voluntario %d removeu roupa: %d\n", tid, roupa->codigo);
                }
                pthread_mutex_unlock(&mutex_venda);
            }
            case 2:
            {
                // Esse daqui ta quebrando os mutex 
            //     printf("v3\n");
            //     pthread_mutex_lock(&mutex_reparo);
            //     if (roupas_reparo.tamanho != 0) {
            //         Roupa * roupa;
            //         roupa = retrive_from_position(&roupas_reparo, rand() %roupas_reparo.tamanho);           
            //         pthread_mutex_lock(&mutex_venda);
            //         append_to_list(&roupas_venda, roupa);
            //         pthread_mutex_unlock(&mutex_venda);
            //         printf("Voluntario %d moveu roupa para venda: %d\n", tid, roupa->codigo);
            //     }
            //     pthread_mutex_unlock(&mutex_reparo);
            }
        }
    }
    
    pthread_exit(NULL);
}



int main(){
    pthread_t threads_voluntario[NUM_THREADS_VOLUNTARIO];
    pthread_t threads_cliente[NUM_THREADS_CLIENTE];
    int t;
    int rc;
    pthread_mutex_init(&mutex_venda, NULL);
    pthread_mutex_init(&mutex_reparo, NULL);
    pthread_mutex_init(&mutex_codigo, NULL);
    for(t=0; t<NUM_THREADS_VOLUNTARIO; t++){

        rc = pthread_create(&threads_voluntario[t], NULL, thread_voluntario, (void *)t+1);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    for(t=0; t<NUM_THREADS_CLIENTE; t++){

        rc = pthread_create(&threads_cliente[t], NULL, thread_cliente, (void *)t +1);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    for(t=0; t<NUM_THREADS_VOLUNTARIO; t++){
        pthread_join(threads_voluntario[t], NULL);
    }
    for(t=0; t<NUM_THREADS_CLIENTE; t++){
        pthread_join(threads_cliente[t], NULL);
    }

    pthread_mutex_destroy(&mutex_venda);
    pthread_mutex_destroy(&mutex_reparo);
    pthread_mutex_destroy(&mutex_codigo);

}

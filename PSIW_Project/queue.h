#ifndef PS_QUEUE_H
#define PS_QUEUE_H

// ==============================================
//
//  Version 1.1, 2026-01-19
//
// ==============================================

#include <pthread.h>

struct TQueue {
    char **messages; // tablica wskaznikow wiadomosci
    int *ref_counts; // ile watkow musi jeszcz odczytac
    int capacity; // max pojemnosc
    int current_count; // liczba wiadomosci w kolejce
    int head; // index zapisu
    int tail; // index odczytu
    int total_produced; // lacznie wyprodukowanych wiadomosci

    pthread_t *sub_threads; // id watkow subskrybujacych
    int *sub_read_index; // index nastepnej wiadomosci dla watku (zachowane dla kompatybilnosci)
    int *sub_read_total; // ile dany subskrybent juz przeczytal lacznie
    int sub_count; // liczba subskrybentow
    int sub_capacity; // rozmiar tablic subskrybentow

    pthread_mutex_t mutex;
    pthread_cond_t cond_space; // czeka producent bo jest full wiadomosci
    pthread_cond_t cond_msg; // czeka konsument bo nie ma wiadomosci
};
typedef struct TQueue TQueue;

TQueue* createQueue(int size);

void destroyQueue(TQueue *queue);

void subscribe(TQueue *queue, pthread_t thread);

void unsubscribe(TQueue *queue, pthread_t thread);

void addMsg(TQueue *queue, char *msg);

char* getMsg(TQueue *queue, pthread_t thread);

int getAvailable(TQueue *queue, pthread_t thread);

int removeMsg(TQueue *queue, char *msg);

void setSize(TQueue *queue, int size);

#endif // PS_QUEUE_H

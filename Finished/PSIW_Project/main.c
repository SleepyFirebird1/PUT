#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "queue.h"

TQueue* q;

void* thread_1(void* arg) {
    (void)arg;

    // faza 1
    printf("T1 - Faza 1  - addMsg(m1)\n");
    addMsg(q, strdup("m1"));
    
    // faza 3
    sleep(2); 
    printf("T1 - Faza 3  - addMsg(m2)\n");
    addMsg(q, strdup("m2"));
    
    // faza 8
    sleep(5); 
    printf("T1 - Faza 8  - addMsg(m3)\n");
    addMsg(q, strdup("m3"));
    
    // faza 10
    sleep(2);
    printf("T1 - Faza 10 - setSize(2)\n");
    setSize(q, 2);

    // faza 11
    sleep(1);
    printf("T1 - Faza 11 - addMsg(m4)\n");
    addMsg(q, strdup("m4"));

    // faza 13
    sleep(2);
    printf("T1 - Faza 13 - addMsg(m5)\n");
    addMsg(q, strdup("m5"));

    // faza 14
    sleep(1);
    printf("T1 - Faza 14 - addMsg(m6) - blokada\n");
    addMsg(q, strdup("m6"));
    printf("T1 - Faza 17 - addMsg(m6) - koniec blokady\n");

    // faza 19
    sleep(2);
    printf("T1 - Faza 19 - addMsg(m7)\n");
    addMsg(q, strdup("m7"));
    printf("T1 - Faza 21 - addMsg(m7) - koniec blokady\n");

    return NULL;
}

void* thread_2(void* arg) {
    (void)arg;
    pthread_t self = pthread_self();

    // faza 2
    sleep(1);
    printf("T2 - Faza 2  - subskrypcja\n");
    subscribe(q, self);
    
    // faza 4
    sleep(2); 
    int avail = getAvailable(q, self);
    printf("T2 - Faza 4  - getAvailable: %d - oczekiwana: 1\n", avail);

    // faza 6
    sleep(2);
    printf("T2 - Faza 6  - getMsg (m2)\n");
    char* m = getMsg(q, self); 
    if(m) {
        printf("T2 - Faza 6  - odebrano: %s\n", m);
        free(m);
    }

    // faza 7/9
    printf("T2 - Faza 7  - getMsg - blokada\n");
    m = getMsg(q, self);
    if(m) {
        printf("T2 - Faza 9  - getMsg - koniec blokady odebrano: %s\n", m);
        free(m);
    }

    // faza 12
    sleep(4);
    avail = getAvailable(q, self);
    printf("T2 - Faza 12 - getAvailable: %d - oczekiwana: 1\n", avail);

    // faza 14
    sleep(3);
    avail = getAvailable(q, self);
    printf("T2 - Faza 14 - getAvailable: %d - oczekiwana: 2\n", avail);

    // faza 17
    sleep(3);
    printf("T2 - Faza 17 - getMsg (m4)\n");
    m = getMsg(q, self);
    if(m) {
        printf("T2 - Faza 17 - odebrano: %s\n", m);
        free(m);
    }

    // faza 20
    sleep(3);
    avail = getAvailable(q, self);
    printf("T2 - Faza 20 - getAvailable: %d - oczekiwana: 2\n", avail);
    
    // faza 21
    sleep(1);
    printf("T2 - Faza 21 - getMsg (m5)\n");
    m = getMsg(q, self);
    if(m) {
        printf("T2 - Faza 21 - odebrano: %s\n", m);
        free(m);
    }

    // faza 22
    sleep(1);
    printf("T2 - Faza 22 - getMsg (m6)\n");
    m = getMsg(q, self);
    if(m) {
        printf("T2 - Faza 22 - odebrano: %s\n", m);
        free(m);
    }

    // faza 23
    sleep(1);
    printf("T2 - Faza 23 - getMsg (m7)\n");
    m = getMsg(q, self);
    if(m) {
        printf("T2 - Faza 23 - odebrano: %s\n", m);
        free(m);
    }
    
    return NULL;
}

void* thread_3(void* arg) {
    (void)arg;
    pthread_t self = pthread_self();
    
    // faza 4
    sleep(3); 
    printf("T3 - Faza 4  - subskrypcja\n");
    subscribe(q, self);
    
    // faza 5
    sleep(1);
    printf("T3 - Faza 5  - getMsg blokada\n");
    char* m = getMsg(q, self);
    if(m) {
        printf("T3 - Faza 9  - getMsg - koniec blokady odebrano: %s\n", m);
        free(m);
    }

    // faza 12
    sleep(4);
    int avail = getAvailable(q, self);
    printf("T3 - Faza 12 - getAvailable: %d - oczekiwana: 1\n", avail);

    // faza 14
    sleep(3);
    avail = getAvailable(q, self);
    printf("T3 - Faza 14 - getAvailable: %d - oczekiwana: 2\n", avail);

    // faza 16
    sleep(2);
    printf("T3 - Faza 16 - getMsg (m4)\n");
    m = getMsg(q, self);
    if(m) {
        printf("T3 - Faza 16 - odebrano: %s\n", m);
        free(m);
    }

    // faza 18
    sleep(2);
    printf("T3 - Faza 18 - koniec subskrypcji\n");
    unsubscribe(q, self);

    return NULL;
}

void* thread_4(void* arg) {
    (void)arg;
    pthread_t self = pthread_self();

    // faza 12
    sleep(12);
    printf("T4 - Faza 12 - subskrypcja\n");
    subscribe(q, self);

    // faza 14
    sleep(3);
    int avail = getAvailable(q, self);
    printf("T4 - Faza 14 - getAvailable: %d - oczekiwana: 1\n", avail);

    // faza 15
    sleep(1);
    printf("T4 - Faza 15 - koniec subskrypcji\n");
    unsubscribe(q, self);

    return NULL;
}

int main(void) {
    printf("START\n");
    
    q = createQueue(5);
    
    pthread_t t1, t2, t3, t4;
    pthread_create(&t1, NULL, thread_1, NULL); 
    pthread_create(&t2, NULL, thread_2, NULL);
    pthread_create(&t3, NULL, thread_3, NULL);
    pthread_create(&t4, NULL, thread_4, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    
    printf("KONIEC\n");
    printf("\nTest usunięcia wiadomości\n");

    destroyQueue(q);
    q = createQueue(5);

    pthread_t self = pthread_self();
    subscribe(q, self);
    
    char* msgA = strdup("MSG_A");
    char* msgB = strdup("MSG_B");
    
    printf("Dodaję dwie wiadomości\n");
    addMsg(q, msgA);
    printf("msg: MSG_A\n");
    addMsg(q, msgB);
    printf("msg: MSG_B\n");
    
    printf("Usunięcie wiadomości\n");
    int res = removeMsg(q, msgA); 
    
    if (res == 1) printf("removeMsg: sukces\n");
    else printf("removeMsg: porazka\n");
    
    printf("Odczyt - oczekiwana: MSG_B\n");
    char* odebrana = getMsg(q, self);
    
    if (odebrana && strcmp(odebrana, "MSG_B") == 0) {
        printf("Otrzymano '%s'\n", odebrana);
        free(odebrana);
    } else {
        if (odebrana == NULL) {
             printf("Otrzymano NULL\n");
        } else {
             printf("Otrzymano '%s'\n", odebrana);
             free(odebrana);
        }
    }
    destroyQueue(q);
    printf("KONIEC\n");
    return 0;
}

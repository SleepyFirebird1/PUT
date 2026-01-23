#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

TQueue* createQueue(int size) {
    TQueue *queue = (TQueue*)malloc(sizeof(TQueue));
    if (queue == NULL) {
        perror("Błąd alokacji pamięci");
        exit(1);
    }

    queue->capacity = size;
    queue->current_count = 0;
    queue->head = 0;
    queue->tail = 0;

    queue->messages = (char**)calloc(size, sizeof(char*));
    if (queue->messages == NULL) {
        free(queue);
        perror("Błąd alokacji pamięci");
        exit(1);
    }

    queue->ref_counts = (int*)calloc(size, sizeof(int));
    if (queue->ref_counts == NULL) {
        free(queue->messages);
        free(queue);
        perror("Błąd alokacji pamięci");
        exit(1);
    }

    queue->sub_count = 0;
    queue->sub_capacity = 10;
    queue->sub_threads = (pthread_t*)malloc(sizeof(pthread_t) * queue->sub_capacity);
    queue->sub_read_index = (int*)malloc(sizeof(int) * queue->sub_capacity);

    if (pthread_mutex_init(&queue->mutex, NULL) != 0 ||
        pthread_cond_init(&queue->cond_space, NULL) != 0 ||
        pthread_cond_init(&queue->cond_msg, NULL) != 0) {
        free(queue->messages);
        free(queue->ref_counts);
        free(queue->sub_threads);
        free(queue->sub_read_index);
        free(queue);
        perror("Błąd alokacji pamięci");
        exit(1);
    }

    return queue;
}

void destroyQueue(TQueue *queue) {
    if (queue == NULL) return;
    pthread_cond_destroy(&queue->cond_space);
    pthread_cond_destroy(&queue->cond_msg);
    for (int i = 0; i < queue->capacity; i++) {
        if (queue->messages[i] != NULL) {
            free(queue->messages[i]);
        }
    }
    free(queue->messages);
    free(queue->ref_counts);
    free(queue->sub_threads);
    free(queue->sub_read_index);
    pthread_mutex_destroy(&queue->mutex);
    free(queue);

}

void subscribe(TQueue *queue, pthread_t thread) {
    pthread_mutex_lock(&queue->mutex);
    for (int i = 0; i < queue->sub_count; i++) {
        if (pthread_equal(queue->sub_threads[i], thread)) {
            pthread_mutex_unlock(&queue->mutex);
            return;
        }
    }
    if (queue->sub_count >= queue->sub_capacity) {
        int new_sub_capacity = queue->sub_capacity * 2;

        pthread_t *new_sub_threads = (pthread_t*)realloc(queue->sub_threads, sizeof(pthread_t) * new_sub_capacity);
        int *new_sub_read_index = (int*)realloc(queue->sub_read_index, sizeof(int) * new_sub_capacity);

        if (new_sub_threads == NULL || new_sub_read_index == NULL) {
            pthread_mutex_unlock(&queue->mutex);
            return;
        }

        queue->sub_threads = new_sub_threads;
        queue->sub_read_index = new_sub_read_index;
        queue->sub_capacity = new_sub_capacity;
    }
    queue->sub_threads[queue->sub_count] = thread;
    queue->sub_read_index[queue->sub_count] = queue->head;
    queue->sub_count++;
    pthread_mutex_unlock(&queue->mutex);
}

void unsubscribe(TQueue *queue, pthread_t thread) {
    pthread_mutex_lock(&queue->mutex);
    for (int i = 0; i < queue->sub_count; i++){
        if (pthread_equal(queue->sub_threads[i], thread)) {

            int curr = queue->sub_read_index[i];
            // problem
            int messages_to_unread = (queue->head - curr + queue->capacity) % queue->capacity;
            if (messages_to_unread == 0 && queue->current_count == queue->capacity) {
                messages_to_unread = queue->capacity;
            }

            for (int k = 0; k < messages_to_unread; k++) {
                if (queue->ref_counts[curr] > 0) {
                    queue->ref_counts[curr]--;
                }
                curr = (curr + 1) % queue->capacity;
            }

            while (queue->current_count > 0 && queue->ref_counts[queue->tail] == 0) {
                if (queue->messages[queue->tail]) {
                    free(queue->messages[queue->tail]);
                    queue->messages[queue->tail] = NULL;
                }
                queue->tail = (queue->tail + 1) % queue->capacity;
                queue->current_count--;
                pthread_cond_signal(&queue->cond_space);
            }

            for (int j = i; j < queue->sub_count - 1; j++) {
                queue->sub_threads[j] = queue->sub_threads[j + 1];
                queue->sub_read_index[j] = queue->sub_read_index[j + 1];
            }
            queue->sub_count--;
            break;
        }
    }
    pthread_mutex_unlock(&queue->mutex);
}

void addMsg(TQueue *queue, char *msg){
    pthread_mutex_lock(&queue->mutex);

    while(queue->current_count>=queue->capacity) {
        pthread_cond_wait(&queue->cond_space, &queue->mutex);
    }
    if (queue->sub_count == 0) {
        pthread_mutex_unlock(&queue->mutex);
        free(msg);
        return;
    }
    queue->messages[queue->head] = msg;
    queue->ref_counts[queue->head] = queue->sub_count;
    queue->head = (queue->head + 1) % queue->capacity;
    queue->current_count++;
    pthread_cond_broadcast(&queue->cond_msg);
    pthread_mutex_unlock(&queue->mutex);
}

char* getMsg(TQueue *queue, pthread_t thread) {
    pthread_mutex_lock(&queue->mutex);
    int thread_idx = -1;
    for (int i = 0; i < queue->sub_count; i++){
        if (pthread_equal(queue->sub_threads[i], thread)) {
            thread_idx = i;
            break;
        }
    }
    if (thread_idx == -1) {
        pthread_mutex_unlock(&queue->mutex);
        perror("Błąd getMsg: brak wątku");
        return NULL;
    }
    while(queue->sub_read_index[thread_idx] == queue->head) {
        if (queue->current_count == queue->capacity) {
            break;
        }
        pthread_cond_wait(&queue->cond_msg, &queue->mutex);
    }
    int msg_idx = queue->sub_read_index[thread_idx];
    char *orig_msg = queue->messages[msg_idx];
    char *msg_copy = NULL; 
    if (orig_msg) msg_copy = strdup(orig_msg);

    if (queue->ref_counts[msg_idx] > 0) {
        queue->ref_counts[msg_idx]--;
    }

    queue->sub_read_index[thread_idx] = (msg_idx + 1) % queue->capacity;

    while (queue->current_count > 0 && queue->ref_counts[queue->tail] == 0) {
        if (queue->messages[queue->tail]) {
            free(queue->messages[queue->tail]);
            queue->messages[queue->tail] = NULL;
        }
        queue->tail = (queue->tail + 1) % queue->capacity;
        queue->current_count--;
        pthread_cond_signal(&queue->cond_space);
    }
    
    pthread_mutex_unlock(&queue->mutex);
    return msg_copy;
}

int getAvailable(TQueue *queue, pthread_t thread) {
    pthread_mutex_lock(&queue->mutex);
    int thread_idx = -1;
    for (int i = 0; i < queue->sub_count; i++){
        if (pthread_equal(queue->sub_threads[i], thread)) {
            thread_idx = i;
            break;
        }
    }
    if (thread_idx == -1) {
        pthread_mutex_unlock(&queue->mutex);
        perror("Błąd getAvailable: brak wątku");
        return 1; 
    }
    int read_idx = queue->sub_read_index[thread_idx];
    int available = (queue->head - read_idx + queue->capacity) % queue->capacity;
    if (available == 0 && queue->current_count == queue->capacity) {
         available = queue->capacity;
    }
    pthread_mutex_unlock(&queue->mutex);
    return available;
}

int removeMsg(TQueue *queue, char *msg) {
    pthread_mutex_lock(&queue->mutex);
    int curr = queue->tail;
    int found = 0;
    for (int i = 0; i < queue->current_count; i++) {
        if (queue->messages[curr] == msg) {
            free(queue->messages[curr]);
            queue->messages[curr] = NULL;
            
            // aktualizacja subskrybentów
            for(int k=0; k < queue->sub_count; k++) {
                if (queue->sub_read_index[k] == curr) {
                    queue->sub_read_index[k] = (curr + 1) % queue->capacity;
                    if (queue->ref_counts[curr] > 0) {
                        queue->ref_counts[curr]--;
                    }
                }
            }
            
            found = 1;
            break;
        }
        curr = (curr + 1) % queue->capacity;
    }
    if (found) {
        while (queue->current_count > 0 && queue->ref_counts[queue->tail] == 0) {
            if (queue->messages[queue->tail] != NULL) {
                free(queue->messages[queue->tail]);
                queue->messages[queue->tail] = NULL;
            }
            queue->tail = (queue->tail + 1) % queue->capacity;
            queue->current_count--;
            pthread_cond_signal(&queue->cond_space);
        }
    }
    pthread_mutex_unlock(&queue->mutex);
    return found;
}

void setSize(TQueue *queue, int size) {
    if (size <= 0) return;
    pthread_mutex_lock(&queue->mutex);
    // usuniecie nadmiarowych wiadomosci
    while (queue->current_count > size) {
        if (queue->messages[queue->tail] != NULL) {
            free(queue->messages[queue->tail]);
            queue->messages[queue->tail] = NULL;
        }
        
        // aktualizacja subskrybentow zeby czytali aktualna wiadomosc
        for(int i=0; i < queue->sub_count; i++) {
             if (queue->sub_read_index[i] == queue->tail) {
                 queue->sub_read_index[i] = (queue->sub_read_index[i] + 1) % queue->capacity;
             }
        }
        queue->tail = (queue->tail + 1) % queue->capacity;
        queue->current_count--;
    }

    // alokacja nowych tablic
    char **new_messages = (char**)calloc(size, sizeof(char*));
    int *new_ref_counts = (int*)calloc(size, sizeof(int));
    // sprawdzenie poprawnosci alokacji
    if (!new_messages || !new_ref_counts) {
        if(new_messages) free(new_messages);
        if(new_ref_counts) free(new_ref_counts);
        pthread_mutex_unlock(&queue->mutex);
        return; 
    }

    // przepisanie danych do nowego bufora
    int old_idx = queue->tail;
    
    // tablica pomocnicza do mapowania nowej tablicy
    int *index_map = (int*)malloc(sizeof(int) * queue->capacity);
    for (int k=0; k < queue->capacity; k++) index_map[k] = -1; // -1 oznacza brak/usunięte

    for (int i = 0; i < queue->current_count; i++) {
        new_messages[i] = queue->messages[old_idx];
        new_ref_counts[i] = queue->ref_counts[old_idx];
        
        index_map[old_idx] = i;
        old_idx = (old_idx + 1) % queue->capacity;
    }

    // aktualizacja indeksów subskrybentów
    for (int i = 0; i < queue->sub_count; i++) {
        int old_read = queue->sub_read_index[i];
        // obsluga przypadku w ktorym subskrybent czekal na nowa wiadomosc
        if (old_read == queue->head && queue->current_count < queue->capacity) {
            queue->sub_read_index[i] = queue->current_count;
        } 
        else if (index_map[old_read] != -1) {
            // tych ktorzy wskazywali na istniejaca wiadomosc mapujemy na nowy indeks
            queue->sub_read_index[i] = index_map[old_read];
        } 
        else {
             // subskrybent wskazywał na coś co usunęliśmy lub na stary head wskazujemy na poczatek kolejki
             queue->sub_read_index[i] = queue->current_count;
        }
    }
    
    free(index_map);

    // podmiana tablic w strukturze
    free(queue->messages);
    free(queue->ref_counts);
    
    queue->messages = new_messages;
    queue->ref_counts = new_ref_counts;
    
    // aktualizacja danych
    queue->capacity = size;
    queue->tail = 0;
    queue->head = queue->current_count;
    // obsluga sytuacji w ktorej nowa kolejka jest pelna jesli nie jest to po prostu zwroci ta sama liczbe a nie poczatek czyli 0
    queue->head = queue->head % size;
    // moglo zwolnic sie miejsce wiec powiadamiamy producentow
    pthread_cond_broadcast(&queue->cond_space);
    pthread_mutex_unlock(&queue->mutex);
}

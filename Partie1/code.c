#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// Déclaration des mutex pour les segments
pthread_mutex_t AB = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t BC = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CD = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t BF = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t EC = PTHREAD_MUTEX_INITIALIZER;

// Fonction utilitaire : traverser un tronçon
void traverse(const char* metro, const char* segment, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    printf("%s entre dans le segment %s\n", metro, segment);

    sleep(2); // temps d'occupation du tronçon

    printf("%s quitte le segment %s\n", metro, segment);
    pthread_mutex_unlock(mutex);
}

// MR1 : A → B → C → D
void* metro_MR1(void* arg) {
    char* name = (char*) arg;

    traverse(name, "AB", &AB);
    traverse(name, "BC", &BC);
    traverse(name, "CD", &CD);

    printf("%s est arrivé à D\n", name);
    return NULL;
}

// MR2 : F → B → A
void* metro_MR2(void* arg) {
    char* name = (char*) arg;

    traverse(name, "BF", &BF);
    traverse(name, "AB", &AB);

    printf("%s est arrivé à A\n", name);
    return NULL;
}

// MR3 : E → C → B
void* metro_MR3(void* arg) {
    char* name = (char*) arg;

    traverse(name, "EC", &EC);
    traverse(name, "BC", &BC);

    printf("%s est arrivé à B\n", name);
    return NULL;
}

int main() {
    pthread_t t1, t2, t3, t4, t5, t6;

    // Création de plusieurs métros sur chaque ligne
    pthread_create(&t1, NULL, metro_MR1, "MR1-1");
    pthread_create(&t2, NULL, metro_MR1, "MR1-2");

    pthread_create(&t3, NULL, metro_MR2, "MR2-1");
    pthread_create(&t4, NULL, metro_MR2, "MR2-2");

    pthread_create(&t5, NULL, metro_MR3, "MR3-1");
    pthread_create(&t6, NULL, metro_MR3, "MR3-2");

    // Attente
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    pthread_join(t5, NULL);
    pthread_join(t6, NULL);

    return 0;
}


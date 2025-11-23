#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

FILE *log_file = NULL;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

// Mutex pour chaque station
typedef struct {
    char nom[10];
    pthread_mutex_t mutex;
} Station;

Station stations[] = {
    {"A", PTHREAD_MUTEX_INITIALIZER}, {"B", PTHREAD_MUTEX_INITIALIZER},
    {"C", PTHREAD_MUTEX_INITIALIZER}, {"D", PTHREAD_MUTEX_INITIALIZER},
    {"E", PTHREAD_MUTEX_INITIALIZER}, {"F", PTHREAD_MUTEX_INITIALIZER},
    {"G", PTHREAD_MUTEX_INITIALIZER}, {"H", PTHREAD_MUTEX_INITIALIZER},
    {"I", PTHREAD_MUTEX_INITIALIZER}, {"J", PTHREAD_MUTEX_INITIALIZER},
    {"K", PTHREAD_MUTEX_INITIALIZER}, {"L", PTHREAD_MUTEX_INITIALIZER},
    {"M", PTHREAD_MUTEX_INITIALIZER}, {"N", PTHREAD_MUTEX_INITIALIZER},
    {"O", PTHREAD_MUTEX_INITIALIZER}, {"P", PTHREAD_MUTEX_INITIALIZER},
    {"V", PTHREAD_MUTEX_INITIALIZER}
};

int nb_stations = sizeof(stations) / sizeof(Station);

typedef struct {
    char couleur[10];
    char *trajet[20];
    int nb_stations_trajet;
    int id_metro;
} Metro;

void log_event(const char* couleur, int id, const char* action, const char* station) {
    pthread_mutex_lock(&file_mutex);
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    fprintf(log_file, "{\"time\":\"%02d:%02d:%02d\",\"ligne\":\"%s\",\"id\":%d,\"action\":\"%s\",\"station\":\"%s\"},\n",
            t->tm_hour, t->tm_min, t->tm_sec, couleur, id, action, station);
    fflush(log_file);
    
    pthread_mutex_unlock(&file_mutex);
}

int trouver_station(char *nom) {
    for (int i = 0; i < nb_stations; i++) {
        if (strcmp(stations[i].nom, nom) == 0) return i;
    }
    return -1;
}

void *deplacement_metro(void *arg) {
    Metro *metro = (Metro *)arg;
    
    // Départ : occuper la première station
    int idx = trouver_station(metro->trajet[0]);
    if (idx != -1) {
        pthread_mutex_lock(&stations[idx].mutex);
        log_event(metro->couleur, metro->id_metro, "DEPART", metro->trajet[0]);
        sleep(1); // Rester 1 seconde à la station de départ
        pthread_mutex_unlock(&stations[idx].mutex);
    }
    
    // Parcourir toutes les stations
    for (int i = 1; i < metro->nb_stations_trajet; i++) {
        char segment[10];
        sprintf(segment, "%s%s", metro->trajet[i-1], metro->trajet[i]);
        
        log_event(metro->couleur, metro->id_metro, "TRAJET", segment);
        sleep(2); // Temps de trajet entre deux stations
        
        // Arriver à la station suivante
        int idx_dest = trouver_station(metro->trajet[i]);
        if (idx_dest != -1) {
            log_event(metro->couleur, metro->id_metro, "ATTEND", metro->trajet[i]);
            
            // Attendre que la station soit libre
            pthread_mutex_lock(&stations[idx_dest].mutex);
            log_event(metro->couleur, metro->id_metro, "ARRIVE", metro->trajet[i]);
            
            // Rester 1 seconde à la station
            sleep(1);
            
            // Libérer la station
            log_event(metro->couleur, metro->id_metro, "QUITTE", metro->trajet[i]);
            pthread_mutex_unlock(&stations[idx_dest].mutex);
        }
    }
    
    log_event(metro->couleur, metro->id_metro, "TERMINE", metro->trajet[metro->nb_stations_trajet-1]);
    return NULL;
}

int main() {
    log_file = fopen("events.json", "w");
    if (!log_file) {
        perror("Erreur ouverture fichier");
        return 1;
    }
    
    fprintf(log_file, "[\n");
    
    // UN SEUL TRAIN PAR LIGNE
    Metro ligne_bleue = {"Bleue", {"B", "C", "I"}, 3, 1};
    Metro ligne_rouge = {"Rouge", {"E", "F", "G", "H", "I", "J"}, 6, 1};
    Metro ligne_marron = {"Marron", {"D", "K"}, 2, 1};
    Metro ligne_verte = {"Verte", {"A", "P", "L", "G", "M", "N", "O"}, 7, 1};
    Metro ligne_noire = {"Noire", {"M", "V"}, 2, 1};
    
    pthread_t threads[5];
    
    printf("╔════════════════════════════════════════╗\n");
    printf("║  Simulation Métro - Exclusion Mutuelle ║\n");
    printf("║  1 train par ligne                    ║\n");
    printf("║  Max 1 train par station              ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    
    // Lancer les 5 trains simultanément
    pthread_create(&threads[0], NULL, deplacement_metro, &ligne_bleue);
    pthread_create(&threads[1], NULL, deplacement_metro, &ligne_rouge);
    pthread_create(&threads[2], NULL, deplacement_metro, &ligne_marron);
    pthread_create(&threads[3], NULL, deplacement_metro, &ligne_verte);
    pthread_create(&threads[4], NULL, deplacement_metro, &ligne_noire);
    
    // Attendre la fin de tous les threads
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
    
    fprintf(log_file, "{}\n]\n");
    fclose(log_file);
    
    printf("\n✓ Simulation terminée!\n");
    printf("✓ Ouvrez simulation.html dans votre navigateur.\n");
    
    // Détruire les mutex
    for (int i = 0; i < nb_stations; i++) {
        pthread_mutex_destroy(&stations[i].mutex);
    }
    
    return 0;
}

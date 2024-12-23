/*
 * ULB-ID:
 * paug0002
 * jche0027
 * rrab0007
*/


// -------------------------  Implementation -------------------------

#include "imgdist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdatomic.h>
#include <dirent.h>
#include "../commun/commun.h"

// -------------------------  Global variables  -------------------------

#define PORT 5555
#define BUFFER_SIZE 2048
#define NUM_THREADS 3
#define MAX_CONNECTION 1000
#define CONNECTION_WAITING 10

int server_fd;
pthread_t threads[NUM_THREADS]; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile sig_atomic_t stop_threads = 0;

// ----------------------------  Structure  ----------------------------

typedef struct {
    int index;
    int index_end;
    int number_of_files;
    char* fileName;
    char* buffer[BUFFER_SIZE];
    char imgData[BUFFER_SIZE];
    unsigned long long int distancePHash;
} SharedMemory;

struct sockaddr_in address;
SharedMemory data;

// ----------------------------  Fonctions  ----------------------------


/**
 * @brief Configurer le socket du serveur et le lier à un port spécifique.
 * 
 * Cette fonction crée un socket, définit les options du socket, lie le socket au port spécifié,
 * et commence à écouter les connexions entrantes.
 * 
 * @return void
 */

void setupServer(){
    server_fd = checked(socket(AF_INET, SOCK_STREAM, 0));
    int opt = 1;
    checked(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    checked(bind(server_fd, (struct sockaddr*)&address, sizeof(address)));
    checked(listen(server_fd, CONNECTION_WAITING));
    printf("[+]Setup Server\n");
}


/**
 * @brief Fonction de traitement d'une image.
 * 
 * Cette fonction calcule le code de hachage perceptif de deux images et met à jour les données partagées 
 * si la distance entre les images est inférieure à la distance actuelle.
 * 
 * @param img1 Le chemin de la première image.
 * @return 1 si une erreur s'est produite lors du calcul du code de hachage, sinon 0.
 */

int process(){
    while (data.index < data.index_end){
        // Déclaration
        uint64_t hash1, hash2;
        char* img = data.buffer[data.index];

        // Calcule du code de hachage perceptif :

        // image de la banque
        if (!PHash(img, &hash2)) {
            perror("[-]PHash Banq");
            return 1;
        }
        // image recus depuis le client
        if (!PHash(data.imgData, &hash1)) {
            perror("[-]PHash Client");
            return 1;
        }
        // resultat
        unsigned int distance = DistancePHash(hash1, hash2);

        // Bloc critique pour éviter les accès concurrents aux données partagées
        pthread_mutex_lock(&mutex);
        if (distance < data.distancePHash){  // Mise à jour des données partagées
            data.distancePHash = distance;
            data.fileName = img;
        }
        data.index++;
        pthread_mutex_unlock(&mutex);
        // Fin du bloc critique

        printf("Distance entre les images (%s) et (%s) : %d\n", data.imgData, img, distance);
    }
   return 0;
}


/**
 * @brief Parcours le répertoire spécifié et stocke les noms des fichiers dans un tableau.
 * 
 * @param directory_path Le chemin du répertoire à parcourir.
 * @return 1 si une erreur (repertoire vide), sinon 0.
 */

int browse_directory(const char *directory_path) {
    // Ouvrir le répertoire
    DIR *directory = opendir(directory_path);
    // Vérifier si l'ouverture a réussi
    if (directory == NULL) {
        printf("[-]Open Banq Image\n");
        return -1;
    }
    printf("[+]Open Banq Image\n");

    // Lire les fichiers du répertoire :
    // Bloc critique 
    struct dirent *file;
    pthread_mutex_lock(&mutex);
    data.number_of_files = 0;

    while ((file = readdir(directory)) != NULL) {
        // Ignorer les entrées spéciales "." et ".."
        if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0) {
            char filePath[BUFFER_SIZE];
            snprintf(filePath, sizeof(filePath), "%s%s", directory_path, file->d_name);
            // Allouer de la mémoire pour le chemin du fichier et le copier
            data.buffer[data.number_of_files] = strdup(filePath);
            data.number_of_files = data.number_of_files + 1;
        }
    }
    // Fermer le répertoire
    pthread_mutex_unlock(&mutex);
    closedir(directory);
    return 0;
}


/**
 * @brief Fonction de traitement des signaux.
 * 
 * Cette fonction est appelée lorsqu'un signal est reçu. Elle gère les signaux SIGPIPE et SIGINT.
 * Pour SIGPIPE, elle imprime un message indiquant le signal reçu et met fin au programme.
 * Pour SIGINT, elle imprime un message indiquant le signal reçu et met fin au programme.
 * 
 * La fonction signale également aux threads de s'arrêter en plaçant le drapeau stop_threads à 1.
 * Elle attend ensuite que les threads terminent leur exécution.
 * Enfin, elle annule toute opération d'écriture en cours sur le socket client, ferme le socket serveur et quitte le programme.
 * 
 * @param signum Le signal.
 */

void handler(int signum) {
    if (signum == SIGPIPE){
        printf("\nSignal SIGPIPE reçu. Arrêt du programme en cours...\n");
    }
    else if (signum == SIGINT){
        printf("\nSignal SIGINT reçu. Arrêt du programme en cours...\n");
    }
    // Signal les threads de s'arreter
    pthread_mutex_lock(&mutex);
    stop_threads = 1;
    pthread_mutex_unlock(&mutex);
    // Fermeture propre
    // Libérer la allouée pour les noms de fichiers
    for (int i = 0; i < data.number_of_files; i++) {
        free(data.buffer[i]);
    }
    // free(data.imgData);
    close(server_fd);
    exit(EXIT_SUCCESS);
}

// -----------------------------  Corps du code  -----------------------------


int main(int argc, char* argv[]) {

    // --- CREATION DU SERVEUR ---

    setupServer();

    // --- INITIALISATION ---

    char message[128];
    int current_connections = 0;
    data.distancePHash = UINT64_MAX;
    checked(browse_directory("img/"));
    // data.imgData = (char*)malloc(BUFFER_SIZE);
    
    // partage du travail 
    int reste;
    if (data.number_of_files%3 == 0){
        reste = 0;
    }
    else{
        reste = data.number_of_files%3;
    }
    int index_end_1 = (data.number_of_files/3);
    int index_end_2 = (2*index_end_1) + reste;

    // handler
    signal(SIGPIPE, handler);
    signal(SIGINT, handler);


    // --- CONNECTION CLIENT ---

    while (1){
        // accepter une nouvelle connexion
        socklen_t addrlen = sizeof(address);
        int client_fd = checked(accept(server_fd, (struct sockaddr *)&address, &addrlen));
        if (current_connections >= MAX_CONNECTION){
            printf("Maximum number of simultaneous connections reached ! (%d)\n", MAX_CONNECTION);
            close(server_fd);
            continue;
        }
        // --- TRAITEMENT ---
        // lecture du file descriptor
        checked(recv(client_fd, &data.imgData, sizeof(data.imgData)));

        // création des threads
        data.index = 0;
        data.index_end = index_end_1;
        checked(pthread_create(&threads[0], NULL,(void *(*)(void*)) process, NULL));
        checked(pthread_join(threads[0], NULL));
        data.index = index_end_1;
        data.index_end = index_end_2;
        checked(pthread_create(&threads[1], NULL, (void *(*)(void*)) process, NULL));
        checked(pthread_join(threads[1], NULL));
        data.index = index_end_2;
        data.index_end = data.number_of_files;
        checked(pthread_create(&threads[2], NULL, (void *(*)(void*)) process, NULL));
        checked(pthread_join(threads[2], NULL));
        
        // Ecriture sur le socket du client
        if (data.distancePHash < UINT64_MAX){
            sprintf(message, "Most similar image found: '%s' with a distance of %llu.\n", data.fileName, data.distancePHash);
        }
        else{
            sprintf(message, "No similar image found (no comparison could be performed successfully).\n");
        }
        checked_wr(write(client_fd, message, sizeof(message)));

        // compteur + remise à defaut
        data.distancePHash = UINT64_MAX;
        current_connections++;
    }
    // free(data.imgData);
    for (int i = 0; i < data.number_of_files; i++) {
        free(data.buffer[i]);
    }
    close(server_fd);
    return 0;
}

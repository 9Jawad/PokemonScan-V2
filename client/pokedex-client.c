/*
 * ULB-ID:
 * paug0002
 * jche0027
 * rrab0007
*/


// --------------------- Implementation --------------------------------

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "../commun/commun.h"

// --------------------- Global variables --------------------------------

#define PORT 5555
#define BUFFER_SIZE 2048

int client_socket;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t sender_thread, receiver_thread;
static volatile sig_atomic_t stop_threads = 0;

// -------------------------------- Fonctions --------------------------------


// Fonction pour configurer l'adresse du serveur
int SetupServerAddress(const char *server_ip, struct sockaddr_in *server_addr) {
    // Initialise la structure de l'adresse serveur
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);
    // Convertit l'adresse IP fournie en format binaire et verifit si elle est valide
    if (inet_pton(AF_INET, server_ip, &server_addr->sin_addr) < 0) {
        perror("IP error");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


// Fonction pour envoyer une image au serveur
void* SendImages() {
    // Tant que les threads ne doivent pas s'arreter
    while (!stop_threads) {
        // init
        char input_buffer[BUFFER_SIZE];
        memset(&input_buffer, '\0', sizeof(input_buffer));
        printf("\nVeuillez entrer le chemin relatif vers l'image.bmp correspondante\n");
        printf("('CTRL + C' pour arrêter la lecture) : ");

        // Lit la ligne depuis stdin
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            break;
        }
        if (strcmp(input_buffer, "exit") == 0){
            printf("[+]Stop Input");
            stop_threads = 1;
            break;
        }

        pthread_mutex_lock(&mutex);
        size_t len = strlen(input_buffer);
        // Supprime le saut a la ligne si besoin
        if (len > 0 && input_buffer[len - 1] == '\n') {
            input_buffer[len - 1] = '\0';
        }
        // Ouvre le fichier image.bmp
        //FILE *image_file = fopen(input_buffer, "rb");
        // if (image_file == NULL) {
        //    printf("[-]Open File\n");
        //    pthread_mutex_unlock(&mutex);
        //    continue;
        //}
        //printf("[+]Open file\n");

        // Déterminer la taille du fichier
        //fseek(image_file, 0, SEEK_END);
        //long int filesize = ftell(image_file);
        //rewind(image_file);

        // Allouer un buffer pour stocker le contenu binaire du fichier
        //char* image_data = (char*)malloc(sizeof(char)*filesize);
        //if (image_data == NULL) {
        //    printf("[-]Allocation Image Data\n");
        //    fclose(image_file);
        //    exit(EXIT_FAILURE);
        //}
        //printf("[+]Allocation Image Data\n");

        // Stocke le contenu binaire de l'image.bmp
        //size_t bytes_read = fread(image_data, sizeof(char), filesize, image_file);
        //if (!(bytes_read == (size_t)filesize)){
        //    printf("[-]Image Data\n");
        //    free(image_data);
        //    fclose(image_file);
        //    exit(EXIT_FAILURE);
        //}
        //printf("[+]Image Data\n");

        // Envoie le contenu au serveur depuis le socket
        checked_wr(write(client_socket, input_buffer, len));
        printf("Envoi de l'image (%s) au serveur...\n", input_buffer);
        //free(image_data);
        //fclose(image_file);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}


void* ReceiveResponses() {
    // Tant que les threads ne doivent pas s'arreter
    while (!stop_threads) {
        char server_response[BUFFER_SIZE];
        // Lit les reponses du serveur et les affiche
        ssize_t bytes_received = read(client_socket, server_response, sizeof(server_response));
        if (bytes_received > 0){
            pthread_mutex_lock(&mutex);
            printf("\n[+]Reception Response\n");
            printf("%s\n", server_response);
            pthread_mutex_unlock(&mutex);
        }
        else if (bytes_received < 0){
            printf("\n[-]Reception Response\n");
        }
    }
    return NULL;
}


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

    // Ferme le socket client
    close(client_socket);
    exit(EXIT_SUCCESS);

}


// -------------------------------- Corps du code --------------------------------

int main(int argc, char *argv[]) {

    signal(SIGINT, handler);
    signal(SIGPIPE, handler);

    // Verifie le nombre d'arguments pour recuperer l'adresse IP du serveur
    const char *server_ip = "127.0.0.1"; // Adresse IP par defaut
    if (argc > 1) {
        server_ip = argv[1]; // Utilise l'adresse IP fournie en argument si elle est donnee
    }
    // Cree le socket client
    client_socket = checked(socket(AF_INET, SOCK_STREAM, 0));

    // Configure l'adresse du serveur
    struct sockaddr_in server_addr;
    if (SetupServerAddress(server_ip, &server_addr) != EXIT_SUCCESS){
        printf("[-]Setup Server\n");
        return EXIT_FAILURE;
    }
    printf("[+]Setup Server\n");

    // Demande de connexion au serveur
    checked(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)));

    // Cree les threads pour lire le stdin + envoyer l'image au serveur et la reception du stdout du serveur
    pthread_create(&sender_thread, NULL, SendImages, NULL);
    pthread_create(&receiver_thread, NULL, ReceiveResponses, NULL);

    // Attend la fin des threads
    pthread_join(sender_thread, NULL);
    pthread_join(receiver_thread, NULL);

    // Ferme le socket client
    close(client_socket);
    return 0;
}

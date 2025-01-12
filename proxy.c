#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include "./simpleSocketAPI.h"
#include "proxyapi.h"
#include <errno.h>


// Fonction pour gérer une session FTP


volatile sig_atomic_t stop = 0;

int main() {
    struct sockaddr_storage clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    // Création de la socket de rendez-vous
    int serverSock = createServerSocket();
    if (serverSock == -1) {
        printf("Server socket creation failed, exiting...\n");
        return -1;
    }

    // Gérer les processus zombies (signal SIGCHLD)
    signal(SIGCHLD, SIG_IGN);
    printf("Prêt à démarrer\n");
    // Boucle principale pour accepter les connexions
    while (!stop) {
        // exit(0);
        const int clientSock = accept(serverSock, (struct sockaddr *) &clientAddr, &addrLen);
        if (clientSock < 0) {
            printf("Erreur accept : %s erro\n", strerror(errno));

            continue;
        }
        printf("Nouvelle connexion acceptée\n");

        // Créez un processus enfant pour gérer la connexion
        pid_t pid = fork();
        if (pid == -1) {
            perror("Erreur fork");
            close(clientSock);
        } else if (pid == 0) {
            // Processus enfant
            close(serverSock); // Le processus enfant n'a pas besoin de la socket d'écoute
            handleFTP(clientSock);
        } else {
            // bind signal
            // Processus parent
            close(clientSock); // Le processus parent n'a pas besoin de cette connexion
        }
    }

    close(serverSock);
    return 0;
}
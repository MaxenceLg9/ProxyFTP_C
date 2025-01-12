//
// Created by maxence on 04/01/25.
//

#include "proxyapi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "simpleSocketAPI.h"

#define BUFFER_SIZE 1024

void handleFTP(int clientSock) {
    char buffer[BUFFER_SIZE];
    int serverSock;
    ssize_t bytes;

    char *response = "220 Proxy FTP ready\r\n";
    send(clientSock, response, strlen(response), 0);

    // Recevoir la commande USER du client
    bytes = read(clientSock, buffer, BUFFER_SIZE - 1);
    if (bytes <= 0) {
        perror("Erreur lecture client");
        close(clientSock);
        exit(EXIT_FAILURE);
    }
    buffer[bytes] = '\0';
    printf("Commande reçue : %s", buffer);

    // Vérifiez si c'est une commande USER
    if (strncmp(buffer, "USER ", 5) == 0) {
        char *userPart = buffer + 5;
        char *sep = strchr(userPart, '@');
        if (sep) {
            *sep = '\0';
            char *username = userPart;
            char *servername = sep + 1;
            printf("%s:21\n",servername);
            //end serv name with \0
            sep = strchr(servername, '\r');
            if (sep) {
                *sep = '\0';
            }

            // Connectez-vous au serveur FTP
            if (connect2Server(servername, "21", &serverSock) == 0) {
                printf("Connecté au serveur FTP : %s\n", servername);

                // Relayer la commande USER vers le serveur
                snprintf(buffer, BUFFER_SIZE, "USER %s\r\n", username);
                write(serverSock, buffer, strlen(buffer));
                printf("Envoyé : %s", buffer);

                while ((bytes = read(serverSock, buffer, BUFFER_SIZE - 1)) > 0) {
                    printf("Réponse serveur : %s\n", buffer);
                }
                sprintf(buffer, "USER %s", username);
                send(serverSock, buffer, strlen(buffer), 0);

                while ((bytes = read(serverSock, buffer, BUFFER_SIZE - 1)) > 0) {
                    printf("Réponse serveur : %s\n", buffer);
                }
                // Relayez toutes les commandes/réponses
                while ((bytes = read(clientSock, buffer, BUFFER_SIZE - 1)) > 0) {
                    buffer[bytes] = '\0';
                    printf("Commande client : %s\n", buffer);
                    write(serverSock, buffer, bytes);

                    // Réponse du serveur
                    bytes = read(serverSock, buffer, BUFFER_SIZE - 1);
                    if (bytes <= 0) break;
                    buffer[bytes] = '\0';
                    printf("Réponse serveur : %s", buffer);
                    write(clientSock, buffer, bytes);
                }
            }
            close(serverSock);
        }
    }

    close(clientSock);
    exit(EXIT_SUCCESS); // Le processus enfant se termine après la session
}

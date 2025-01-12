#include  <stdio.h>
#include  <sys/socket.h>
#include  <netdb.h>
#include  <string.h>
#include  <unistd.h>
#include  <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ifaddrs.h> // For listing network interfaces

int connect2Server(const char *serverName, const char *port, int *descSock) {
	int ecode;                     // Retour des fonctions
	struct addrinfo *res,*resPtr;  // Résultat de la fonction getaddrinfo
	struct addrinfo hints;		   // Structure pour contrôler getaddrinfo
	bool isConnected;      // booléen indiquant que l'on est bien connecté

	// Initailisation de hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;  // TCP
	hints.ai_family = AF_UNSPEC;      // les adresses IPv4 et IPv6 seront présentées par
	// la fonction getaddrinfo

	//Récupération des informations sur le serveu
	ecode = getaddrinfo(serverName,port,&hints,&res);
	if (ecode){
		fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
		return -1;
	}

	resPtr = res;

	isConnected = false;

	while(!isConnected && resPtr!=NULL){
		//Création de la socket IPv4/TCP
		*descSock = socket(resPtr->ai_family, resPtr->ai_socktype, resPtr->ai_protocol);
		if (*descSock == -1) {
			perror("Erreur creation socket");
			return -1;
		}

		//Connexion au serveur
		ecode = connect(*descSock, resPtr->ai_addr, resPtr->ai_addrlen);
		if (ecode == -1) {
			resPtr = resPtr->ai_next;
			close(*descSock);
		}
		// On a pu se connecter
		else isConnected = true;
	}
	freeaddrinfo(res);

	// On retourne -1 si pas possible de se connecter
	if (!isConnected){
		perror("Connexion impossible");
		return -1;
	}

	//On retourne 0 si on a pu établir la connexion TCP
	return 0;
}

int createServerSocket() {
     struct addrinfo hints, *res;


     // Initialize the hints structure
     memset(&hints, 0, sizeof(hints));
     hints.ai_family = AF_INET;          // Use IPv4
     hints.ai_socktype = SOCK_STREAM;    // Use TCP
     hints.ai_flags = AI_PASSIVE;        // Use wildcard IP address (0.0.0.0)

     // Use default FTP port "21"
     const char *defaultPort = "2121";

     // Retrieve socket address information
     int ecode = getaddrinfo(NULL, defaultPort, &hints, &res);
     if (ecode != 0) {
         fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ecode));
         return -1;
     }

     // Create the socket
     int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
     if (sockfd == -1) {
         perror("Socket creation failed");
         freeaddrinfo(res);
         return -1;
     }

     // Bind the socket to the default IP and port
     ecode = bind(sockfd, res->ai_addr, res->ai_addrlen);
     if (ecode == -1) {
         perror("Socket bind failed");
         close(sockfd);
         freeaddrinfo(res);
         return -1;
     }

     // Free the memory allocated by getaddrinfo
     freeaddrinfo(res);

	listen(sockfd, 5); // Listen for incoming connections
    // Print all network interfaces and their IPs
    struct ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        close(sockfd);
        return -1;
    }

	printf("Proxy FTP ready, listening on:\n");
	for (const struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL) continue;

		if (ifa->ifa_addr->sa_family == AF_INET) {
			char ip[INET_ADDRSTRLEN];
			struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
			inet_ntop(AF_INET, &sa->sin_addr, ip, sizeof(ip));

			printf("  - %s:%s (%s)\n", ip, "2121", ifa->ifa_name);
		}
	}

    freeifaddrs(ifaddr);
    return sockfd; // Return the server socket descriptor
}
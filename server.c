#include "helpers.h"
#include "structs.h"


int main (int argc, char *argv[]) {
    
    if (argc != 2) {
      printf("\nWrong argument count: %s", argv[0]);
      return 1;
    } 

    /* database for clients information */
    clDataP* clientsTable;
    initTable(&clientsTable);
    
    /* files descriptors of inputs to server */
    struct pollfd pfds[INITIAL_CONECTIONS];
    int nfds = 0;
    
    /* to read commands to server (stdin input) */
    char* command = malloc(MAX_STDIN);

    /* subscriber file descriptor, innitialy set to -1 */
    int subsfd = -1;

    /* udp input */
    udp_message* message_udp = malloc(sizeof(udp_message));
    subscriberRequest request;

    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    /* get port number as short unsigned */
    uint16_t port;
    int vef = sscanf(argv[1], "%hu", &port) ;
    DIE(vef != 1, "Given port is invalid"); 

    /* TCP socket */
    int shoutfd = socket(AF_INET    , SOCK_STREAM, 0);
    DIE(shoutfd < 0, "socket");

    /* UDP socket */
    int listenfd = socket(AF_INET   , SOCK_DGRAM, 0);
    DIE(listenfd < 0, "socket");   

    /* make sockets reusable */
    int enable = 1;
    if (setsockopt(shoutfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    perror("setsockopt(SO_REUSEADDR) failed");
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
      perror("setsockopt(SO_REUSEADDR) failed"); 
    
    /* disable Nagle's algorithm */ 
    setsockopt(shoutfd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));

    /* set server parameters */
    struct sockaddr_in serv_addr;
    
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);   
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    /* bind server address to the socket */
    vef = bind(shoutfd, (   const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(vef < 0, "bind");

    vef = bind(listenfd,    (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(vef < 0, "bind");

    /* wait for connections */
    vef = listen(shoutfd, INITIAL_CONECTIONS);  
    DIE(vef < 0, "listen");

    /* read user data from standard input */
    pfds[nfds].fd = STDIN_FILENO;
    pfds[nfds].events = POLLIN;
    nfds++;

    /* get udp messages using the channel */
    pfds[nfds].fd = listenfd;
    pfds[nfds].events = POLLIN;
    nfds++;

    /* get/send tcp messages using the channel */
    pfds[nfds].fd = shoutfd;
    pfds[nfds].events = POLLIN;
    nfds++;
    
    /* server loop */
    while (1) {
      /* wait for readiness notification */
      poll(pfds, nfds, -1);

      /* handle new connections */
      if ((pfds[2].revents & POLLIN) != 0) {
          
          /* get subscriber data */
          struct sockaddr_in client_addr;
          socklen_t clen = sizeof(client_addr);
          char *id_client = malloc(MAX_STDIN);
          
          /* accept conection from subscriber */
          subsfd = accept(shoutfd, (struct sockaddr *)&client_addr, &clen);
          DIE(subsfd < 0, "accept");

          /* disable Nagle's algorithm */
          setsockopt(subsfd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));

          /* receives subscriber id */
          vef = recv(subsfd, (void*)id_client, sizeof(id_client), 0);
          DIE(vef <= 0, "receive");

          /* checks if client already connected */
          clDataP client = searchTable(clientsTable, id_client);
          if(client != NULL && client->fd != -1){
              printf("Client %s already connected.\n", id_client);
              close(subsfd);
              continue;
          }
          
          printf("New client %s connected from %s:%hu.\n", id_client,
          inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

          /* adds client to databse */
          clientsTable[subsfd]->fd = subsfd;
          clientsTable[subsfd]->id = id_client;

          /* check client SF */
          if(client != NULL && client->id != NULL && clientsTable[subsfd]->topicList != NULL) {
          sendMessages(client->topicList, subsfd);
          clientsTable[subsfd]->topicList = client->topicList;
          }
          else clientsTable[subsfd]->topicList = NULL;

          /* add new client as input */
          pfds[nfds].fd = subsfd;
          pfds[nfds].events = POLLIN;
          nfds++;
      }
      /* handle stdin commands */
      else if ((pfds[0].revents & POLLIN)) {
          fgets(command, MAX_STDIN, stdin);
          if (strcmp(command, "exit\n") == 0){
            for (int i = 0; i < nfds; i++)
            {
              close(pfds[i].fd);
            }
            break;
          }
      /* handle udp messages */
      } else if ((pfds[1].revents & POLLIN)) {

        /* receive udp message */
        memset((char*)message_udp, 0, sizeof(udp_message));
        vef = recv(listenfd, (void*)message_udp, sizeof(udp_message), 0);
        DIE(vef <= 0, "receive");

        /* add messages to database (if SF - enabled) */
        addSF(clientsTable, message_udp);

        /* send message to subscribed clients */
        for (int pollcounter = 3; pollcounter < nfds; pollcounter++)
        {
          if (searchTopic(clientsTable[pfds[pollcounter].fd], message_udp->topic) != NULL){
            vef = send(pfds[pollcounter].fd, (void*)message_udp, sizeof(udp_message), 0);
            DIE(vef <= 0, "send");
          }
        } 
      } 
      /* handle tcp messages */
      else {
        for (int pollcounter = 3; pollcounter < nfds; pollcounter++)
        {
          /* receive tcp message */
          if (pfds[pollcounter].revents & POLLIN) {
          vef = recv(pfds[pollcounter].fd, (void*)&request, sizeof(subscriberRequest), 0);
          DIE(vef < 0, "receive");

          /* client disconected */
          if (vef == 0) {
            printf("Client %s disconnected.\n", clientsTable[pfds[pollcounter].fd]->id);

            /* set client as disconected in database */
            clientsTable[pfds[pollcounter].fd]->fd = -1;
            
            /* delete client from input stream */
            close(pfds[pollcounter].fd);
            for (int pollmove = pollcounter; pollmove < nfds - 1; pollmove++) {
              pfds[pollmove] = pfds[pollmove + 1];
            }
            nfds--;
            continue;;
          }

          /* subscribe */
          if (strcmp(request.SF, "-1") != 0){
              if (searchTopic(clientsTable[pfds[pollcounter].fd], request.topic_n) == NULL)
                addTopic(clientsTable[pfds[pollcounter].fd], request);
          } 
          /* unsubscribe */
          else{
            if (searchTopic(clientsTable[pfds[pollcounter].fd], request.topic_n) != NULL)
                removeTopic(clientsTable[pfds[pollcounter].fd], request.topic_n);
          }
          }
        }
      }
    }
    free(message_udp);
    return 0;
}

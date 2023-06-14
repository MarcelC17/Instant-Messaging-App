#include "structs.h"
#include "helpers.h"


int main (int argc, char *argv[]) {

    if (argc != 4) {
      printf("\nWrong argument count: %s", argv[0]);
      return 1;
    }   

    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    /* mesage to server */
    subscriberRequest send_message;
    /* default SF value */
    char* missing_SF = "-1";
    /* "not specified" SF value */
    char* notgiven_SF = "2";
    /* stdin command name */
    char* commandName;
    
    /* macro error value*/
    int vef;

    /* files descriptors of inputs to subscriber */
    struct pollfd pfds[INITIAL_CONECTIONS];
    int nfds = 0;

    /* command buffer */
    char commands[MAX_STDIN] = "start";

    /* TCP message from server */
    udp_message *recv_message = malloc(sizeof(udp_message));

    /* default socket file descriptor value */
    int sockfd = -1;   

    /* read user data from standard input */
    pfds[nfds].fd = STDIN_FILENO;
    pfds[nfds].events = POLLIN;
    nfds++;

    
    /* get port number as short unsigned */
    uint16_t port;
    sscanf(argv[3], "%hu", &port);

    /* TCP socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    /* disable Nagle's algorithm */ 
    int enable = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
    
    /* set subscriber conection parameters */
    struct sockaddr_in serv_addr;

    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    vef = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
    DIE(vef <= 0, "inet_pton");
    
    /* conect to server */
    vef = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(vef < 0, "connect");

    /* add TCP input*/
    pfds[nfds].fd = sockfd;
    pfds[nfds].events = POLLIN;
    nfds++;

    /* send id to server */
    send(sockfd, (void*) argv[1], sizeof(argv[1]), 0);

    /* client loop */
    while (1) {        
      /* wait for readiness notification */
      poll(pfds, nfds, -1);

      /* handle message from server */
      if ((pfds[1].revents & POLLIN)) {
          /*receive message */
          memset((char*)recv_message, 0, sizeof(udp_message));
          vef = recv(pfds[1].fd, (void*)recv_message, sizeof(udp_message), 0);
          DIE(vef < 0, "receive");
          /* server disconected */
          if (vef == 0) {
            close(pfds[1].fd);
            break;
          }

          /* print message */
          printf("%s - ", recv_message->topic);
          /* TYPE 0 */
          if (recv_message->type == (uint8_t)0){
            if ((uint8_t)*(recv_message->content) == (uint8_t)0)
              printf("INT - %d\n", ntohl(*(uint32_t*) (recv_message->content + 1)));
            else
              printf("INT - -%d\n", ntohl(*(uint32_t*) (recv_message->content + 1)));
          }
          /* TYPE 1 */
          if(recv_message->type == (uint8_t)1) printf("SHORT_REAL - %f\n", (ntohs(*((unsigned short int*)recv_message->content))) / 100.00);
          /* TYPE 2 */
          if(recv_message->type == 2) {
            if ((uint8_t)*(recv_message->content) == (uint8_t)0)
              printf("FLOAT - %f\n",  ntohl(*(uint32_t*) (recv_message->content + 1)) / pow(10, *(uint8_t*)(recv_message->content + sizeof(uint32_t)+1)));
            else
              printf("FLOAT - -%f\n",  ntohl(*(uint32_t*)(recv_message->content + 1)) / pow(10, *(uint8_t*)(recv_message->content + sizeof(uint32_t)+1)));
          }
          /* TYPE 3 */
          if(recv_message->type == (uint8_t)3) printf("STRING - %s\n", recv_message->content);
      }
       /* handle sdtin*/
      else if ((pfds[0].revents & POLLIN)) {
        /* get command */
        fgets(commands, INITIAL_CONECTIONS, stdin);

        /* shut down subscriber */
        commandName = strtok(commands," ");
        if (strcmp(commandName, "exit\n") == 0){
            close(sockfd);
            break;
        }

        /* suncribe to topic*/
        if (strcmp(commandName, "subscribe") == 0){
          strcpy(send_message.topic_n, strtok(NULL," "));
          char* SF = strtok(NULL," ");
          /* check for SF*/
          if (SF) strcpy(send_message.SF, SF);
          else {
            strcpy(send_message.SF, notgiven_SF);
            send_message.topic_n[strlen(send_message.topic_n) - 1] = 0;
          }  
          send(sockfd, (void*)&send_message, sizeof(subscriberRequest), 0);
          printf("Subscribed to topic.\n");
          
        }
        /* unsuncribe from topic*/
        if (strcmp(commandName, "unsubscribe") == 0){
          strcpy(send_message.topic_n, strtok(NULL," "));
          send_message.topic_n[strlen(send_message.topic_n) - 1] = 0;
          strcpy(send_message.SF, missing_SF);
          send(sockfd, (void*)&send_message, sizeof(subscriberRequest), 0);
          printf("Unsubscribed from topic.\n");
        }
      
    }
  }
  free(recv_message);
  return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define INITIAL_CONECTIONS 100
#define TOPIC_SIZE 50
#define CONTENT_SIZE 1500
#define MAX_STDIN 100
/* UDP message */
typedef struct{
  char topic[TOPIC_SIZE];
  uint8_t type;
  char content[CONTENT_SIZE];
}udp_message;
/* list of messages */
typedef struct node_m{
  udp_message* message;
  struct node_m* next;
}message_n, *message_nP;

/* topic data */
typedef struct{
  char topic_n[MAX_STDIN];
  char SF[MAX_STDIN];
  message_nP messageList;
  message_nP last;
}topic_info, subscriberRequest;

/* list of topics */
typedef struct node{
  topic_info topic;
  struct node* next;
}*topic_np,topic_n;

/* client */
typedef struct{
  int fd;
  char* id;
  topic_np topicList;
}clData,*clDataP;

/* create client database */
void initTable(clDataP** clientsTable);

/* search client database for id */
clDataP searchTable(clDataP* clientsTable, char* id);

/* search topic list for topic */
topic_np searchTopic(clDataP client, char* topic_n);

/* add topic to topic list */
void addTopic(clDataP client, topic_info topic);

/* remove topic with "name" from client's topic list */
void removeTopic(clDataP client, char* name);

/* free topic list */
void freeTopics(clDataP client);

/* add messages to clients with enabled SF */
void addSF(clDataP* clientsTable, udp_message* message);

/* send messages of the client */
void sendMessages(topic_np topicList, int fd);

/* print message list */
void printMessages(topic_np topicList);

/* print topics */
void printTopics(clDataP client);

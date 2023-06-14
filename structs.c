#include "structs.h" 

void initTable(clDataP** clientsTable){
        *clientsTable = malloc(INITIAL_CONECTIONS* sizeof(clDataP*));
        for(int clientCounter = 0; clientCounter < INITIAL_CONECTIONS; clientCounter++){
        (*clientsTable)[clientCounter] = malloc(sizeof(clData));
        (*clientsTable)[clientCounter]->fd = -1;
        (*clientsTable)[clientCounter]->id = NULL;
        (*clientsTable)[clientCounter]->topicList = NULL;
    }
}

clDataP searchTable(clDataP* clientsTable, char* id){
    for(int clientCounter = 0; clientCounter < INITIAL_CONECTIONS; clientCounter++){

        if (clientsTable[clientCounter]->id != NULL)
        {        
            if (strcmp(clientsTable[clientCounter]->id, id) == 0)
            {
               return clientsTable[clientCounter];
            }
        }    
    }
    return NULL;
}

topic_np searchTopic(clDataP client, char* topic){
    topic_np head = client->topicList;
    while (head != NULL)
    {
        if (strcmp(head->topic.topic_n, topic) == 0)
        return head;
        head = head->next;
    }
    return NULL;
}

void addTopic(clDataP client, topic_info topic){
    topic_np newTopic = malloc(sizeof(topic_n));
    strcpy(newTopic->topic.topic_n, topic.topic_n);
    strcpy(newTopic->topic.SF, topic.SF);
    if (client->topicList != NULL){
        newTopic->next = client->topicList;
        client->topicList = newTopic;
    } else {
        client->topicList = newTopic;
        newTopic->next = NULL;
    }
}

void removeTopic(clDataP client, char* name){
    
    topic_np head = client->topicList;
    topic_np temp;
    if (head == NULL) return;
    //delete head
    if(strcmp(head->topic.topic_n, name) == 0){
        client->topicList = head->next;
        free(head);
        return;
    }
    //delete center
    while (head->next != NULL)
    {
        if (strcmp(head->next->topic.topic_n, name) == 0){
            temp = head->next;
            head->next= head->next->next;
            free(temp);
            return;
        }
        temp = head;
        head = head->next;
    }
    //delete last
    if (strcmp(head->topic.topic_n, name) == 0){
        head->next = NULL;
        free(temp);
    }   
}

void freeTopics(clDataP client){
    topic_np head;
    while (client->topicList != NULL)
    {
        head = client->topicList;
        client->topicList = client->topicList->next;
        free(head);
    }
    client->topicList = NULL;
}

void addMessage(topic_np topic_n, udp_message* message){
    message_nP newMessage = malloc(sizeof(message_n));
    newMessage->next = NULL;
    newMessage->message = malloc(sizeof(udp_message));
    memcpy(newMessage->message, message, sizeof(udp_message));

    if (topic_n->topic.messageList != NULL){
        // printf("2");
        topic_n->topic.last->next = newMessage;
        topic_n->topic.last = newMessage;
    } else {
        // printf("3");
        topic_n->topic.last = newMessage;
        topic_n->topic.messageList = newMessage;
    }
}

void addSF(clDataP* clientsTable, udp_message* message){
    topic_np head;
    for(int clientCounter = 0; clientCounter < INITIAL_CONECTIONS; clientCounter++){
        if (clientsTable[clientCounter]->id != NULL && clientsTable[clientCounter]->topicList != NULL 
        && (clientsTable[clientCounter]->fd == -1))
        {        
            head = clientsTable[clientCounter]->topicList;
            while (head != NULL)
            {
                if ((strcmp(head->topic.SF,"1\n") == 0) && (strcmp(head->topic.topic_n, message->topic) == 0)){
                    // printf("1 ");
                    addMessage(head, message);
                }
                head = head->next;
            }
        }    
    }
}

void sendMessages(topic_np topicList, int fd){
    topic_np head = topicList;
    message_nP message;
    while (head != NULL)
    {
        if (strcmp(head->topic.SF,"1\n") == 0){
            while (head->topic.messageList != NULL)
            {
                message = head->topic.messageList;
                head->topic.messageList = head->topic.messageList->next;
                send(fd, (void*)message->message, sizeof(udp_message), 0);
                free(message);
                
            }
        }
        head = head->next;
    }
}



void printTopics(clDataP client){
    topic_np head = client->topicList;
    while (head != NULL)
    {
        printf("%s\n",head->topic.topic_n);
        head = head->next;
    }
}

void printMessages(topic_np topicList){
    topic_np head = topicList;
    int counter;
    while (head != NULL)
    {
        printf("%s", head->topic.topic_n);
        if (strcmp(head->topic.SF,"1\n") == 0){
            counter = 0;
            while (head->topic.messageList != NULL)
            {
                printf("\n1\n");
                counter++;
                head->topic.messageList = head->topic.messageList->next;
            }
            printf("%d\n", counter);
        }
        head = head->next;
    }
}



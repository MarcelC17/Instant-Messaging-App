# Client - server app 

`Author : Codreanu Marcel` `Group : 325CD`

---

- [Description](#descritpion)
- [ARP](#arp)
- [IP](#ip)

---

## Description

Streaming application which receives messages from UDP clients, filters and sends them to TCP subscribers based on their subscriptions. 

---

## Server

Forwarding interface, connected to UDP(clients) and TCP(subscribers). Stores subscriber information in a hashmap with TCP file descriptor as a key and client data as value (**server database**).  Implemented using I/O Multiplexing for the following blocking calls:

1. **accept() - handles new connections to the server**

* information of new subscribers is added to database  
* subscribes reconection to the server is handled according to store and forward policy

*clDataP searchTable(clDataP* clientsTable, char* id)
Finds client with given *id in the table.
__ clientsTable - server database (subscriber hashmap)
__ id - client id 

*void sendMessages(topic_np topicList, int fd)
Sends messages received by the client while disconnected.
__ *topicList - clients list of topics
__ fd - socket file descriptor (subscriber to receive messages)


2. **stdin commands** - user commands to the server
Valid commands:
exit - close server

3. **recv()**

UDP Clients
Server receive messages from clients and forwards them via TCP

*addSF(clientsTable, message_udp)
Messages of disconected clients are added to the database. Filters by sf and fd flags of a client. fd is -1 if client disconected.
__ clientsTable - subscriber database
__ message_udp - message received from clients

TCP Clients
Send subscribe and unsubscribe messages to the server. Checks client connection.

*topic_np searchTopic(clDataP client, char* topic_n)
Searches subscriber's list of topics for topic_n
__ client - subscriber to the server
__ topic_n - topic name

void addTopic(clDataP client, topic_info topic);
Adds topic to client's topic list

void removeTopic(clDataP client, char* name);
Removes topic from client's topic list
__ name - topic name

---

Subscriber

Receives messages from the server via tcp and prints them. The following commands are valid:

- subscribe <topic_name>  <SF_flag>
* unsubscribe <topic_name>
* exit

If no SF flag is given by subscribe command value is set to  2

---

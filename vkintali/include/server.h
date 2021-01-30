#ifndef SERVER_H_
#define SERVER_H_

char * getipaddress(char * str);
int accept_connection(int server_socket, std::vector<struct ClientList> * clientlist);
void create_connection(int * sockfd,char* port);
//void refresh_list_server(int *fdaccept, std::vector<struct ClientList> * clientlist);
void refresh_list_server(int fdaccept, std::vector<struct ClientList> * clientlist, char * list);
void server_send(int sock_index,char * cmd,std::vector<struct ClientList> * clientlist);
void server_broadcast(int sock_index,char * cmd, std::vector<struct ClientList> * clientlist);
void delete_fd(int sock_index, std::vector<struct ClientList>*clientlist);
int validatenumber(char * portno);
void validateport(char * str, char * port);
int unblockip(int sock_index, char * buffer, std::vector<struct ClientList>*clientlist);
void displayblocked(std::vector<struct ClientList>*clientlist,char * buffer);
bool validateip(char * ipaddress);
void blockip(int sock_index, char * buffer, std::vector<struct ClientList> *clientlist);
int sendall(int s, char *buf);

#endif

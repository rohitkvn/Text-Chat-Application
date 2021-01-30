#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <vector>
#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 256
#define STDIN 0
#define CMD_SIZE 100

#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"
#include "../include/client.h"

using namespace std;

struct block_list{
        char IPA[INET_ADDRSTRLEN];
        struct block_list *next;
        };

struct buffer_msg{
        char message[BUFFER_SIZE];
        struct buffer_msg *msglist;
        };

struct ClientList{
        int portno;
        int fd;
        int fd_client;
        int msgs_sent;
        int msgs_recv;
        char IP[INET_ADDRSTRLEN];
        char FQDN[128];
        vector <string> blocked;
        /*char vblockedips[4][INET_ADDRSTRLEN];*/
        struct block_list blockedips;
        int login;
        };



int connect_to_host( char *server_ip, char* server_port) {
	int sockfd;
	struct addrinfo hints, *res;

	/* Set up hints structure */	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	/* Fill up address structures */	
	if (getaddrinfo(server_ip, server_port, &hints, &res) != 0)
		perror("getaddrinfo failed");

	/* Socket */
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd < 0)
		perror("Failed to create socket");
	/* Connect */
	if(connect(sockfd, res->ai_addr, res->ai_addrlen) < 0){
		perror("Connect failed");
		return 0;
	}
	freeaddrinfo(res);

	return sockfd;
}

void split_string(std::string const &str, const char delim,
			std::vector<std::string> &out)
{
	size_t start;
	size_t end = 0;

	while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
	{
		end = str.find(delim, start);
		out.push_back(str.substr(start, end - start));
	}
}

//https://www.techiedelight.com/split-string-cpp-using-delimiter/


void refresh_list_client(std::vector<struct ClientList> *clientlist, char * list){
	cout<<"entered refresh_list_client "<<list<<endl;
	vector<string> listarray;
	clientlist->clear();
	const char delim = ',';
	split_string(list,delim,listarray);
	//struct ClientList * new_client = new ClientList;
	for(int i=0; i<listarray.size();i++){
	struct ClientList * new_client = new ClientList;
	memset(&new_client->IP,'\0',INET_ADDRSTRLEN);
	memset(&new_client->FQDN,'\0',128);
	new_client->portno = stoi(listarray[i]);
	i++;
	int j =0;
	while(listarray[i][j]!='\0'){
	new_client->IP[j] = listarray[i][j];
	j++;}
	i++;
	int k =0;
	while(listarray[i][k]!='\0'){
	new_client->FQDN[k] = listarray[i][k];
	k++;}
	new_client->login = 1;
	clientlist->push_back(*new_client);
	}
	vector<struct ClientList>:: iterator it;
	

}

bool verifyIP(char * IPAddress, std::vector<struct ClientList> *clientlist){
	bool result = false;
	vector<struct ClientList>::iterator it;
	for(it = clientlist->begin();it!=clientlist->end();it++){
		if(strcmp(it->IP,IPAddress)==0)
			result = true;
	}
	return result;

}

int login_client(char * cmd,char * myportno, std::vector<struct ClientList> *clientlist,fd_set *master_list){
	cout<<"entered login_client function"<<endl;
	char server_IP[INET_ADDRSTRLEN];
       	char server_Port[6];
	char *command = (char*) malloc(sizeof(char)*CMD_SIZE);
        char *command_new = (char*) malloc(sizeof(char)*CMD_SIZE);
	memset(&server_IP,'\0',INET_ADDRSTRLEN);
	memset(&server_Port,'\0',6);
	memset(command,'\0',CMD_SIZE);
	memset(command_new,'\0',CMD_SIZE);
	int cmd_len, sockfd;
       	cmd_len = strlen(cmd);
       	
	for (int j = 0; j< cmd_len; j++){
               if(cmd[j]==' '){
                       command_new = cmd+j+1;
                       cmd= command_new;
			server_IP[j]='\0';
                       break;
               }
               else{
                       server_IP[j] = cmd[j];
               }
       }

       cmd_len = strlen(cmd);
       for (int j = 0; j< cmd_len; j++){
               if(cmd[j]==' '){
                       command_new = cmd+j+1;
                       cmd= command_new;
                       break;
               }
               else{
                       server_Port[j] = cmd[j];
               }
       }
	bool ipvalid = validateip(server_IP);
	int validport = validatenumber(server_Port);
	if((ipvalid==false)||validport==-1)
		return -1;
       sockfd =  connect_to_host(server_IP, server_Port);
	if(sockfd == 0)
		return 0;
	else{
		send(sockfd,myportno,6,0);
       		char * list = (char*)malloc(sizeof(char)*BUFFER_SIZE);
       		memset(list, '\0', BUFFER_SIZE);
       		int recv_bytes;
       		if(recv_bytes = recv(sockfd, list, BUFFER_SIZE-1, 0)!=0){
       			refresh_list_client(clientlist, list);
		}
		
		vector<struct ClientList>:: iterator it;
		return sockfd;
	}
}


void receiveall(int sockfd){
	char * buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
	char * list = (char*)malloc(sizeof(char)*BUFFER_SIZE);
	memset(list, '\0', BUFFER_SIZE);
	memset(buffer, '\0', BUFFER_SIZE);
	int bytes_to_recv=0,recvd_bytes=0, recvd_total=0;
	bytes_to_recv = recv(sockfd,list,4,0);
	while(recvd_total < bytes_to_recv){
		recvd_bytes = recv(sockfd,list,BUFFER_SIZE,0);
		recvd_total = recvd_total+recvd_bytes;
		strcat(buffer,list);
		memset(list, '\0', BUFFER_SIZE);
		
	}
}	
		
	
int client_accept(int server_socket){
        struct sockaddr_in client_addr;
        char * list= (char*) malloc(BUFFER_SIZE);
        memset(list,'\0', BUFFER_SIZE);
        int caddr_len = sizeof(client_addr);
        int fdaccept = accept(server_socket, (struct sockaddr *)&client_addr,(socklen_t *) &caddr_len);
        if(fdaccept < 0)
                perror("Accept failed.");
        printf("\nserver connected!\n");
        return fdaccept;
}





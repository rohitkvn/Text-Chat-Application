#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <typeinfo>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>
#include <set>
#include <bits/stdc++.h>

#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"
#include "../include/client.h"


using namespace std;
/**
* @charanre_assignment1
* @author  Charan Reddy Bodennagari <charanre@buffalo.edu>
* @version 1.0
*
* @section LICENSE
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details at
* http://www.gnu.org/copyleft/gpl.html
*
* @section DESCRIPTION
*
* This contains the main function. Add further description here....
*/


unordered_map <string,vector<string>> bufferedmsgs;
unordered_map <string,vector<string>> :: iterator itr;

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
        //char vblockedips[4][INET_ADDRSTRLEN];
        struct block_list blockedips;
        int login;
        };


void delete_fd(int sock_index, std::vector<struct ClientList>*clientlist){
	vector<struct ClientList>::iterator i;
        for( i = clientlist->begin();i!=clientlist->end();i++){
		if(i->fd == sock_index){
			clientlist->erase(i);
			break;
		}	
        }
}

int validatenumber(char * portno){
	for (int i = 0; portno[i] != '\0'; i++) {
        	if (portno[i] < '0' || portno[i] > '9')
            	return -1;
	}	
	return stoi(portno);
}

/*void sendall(int sockfd, char * buffer){
	int buffer_len=0, bytes_sent =-1,bytes_total;
	buffer_len = strlen(buffer);
	cout<<"send all" << sockfd << " : "<<buffer<<endl;
	while(bytes_total < buffer_len){
		cout<<*(&buffer+bytes_total)<<endl;
		bytes_sent = send(sockfd, *(&buffer+bytes_total),buffer_len-bytes_total,0);
		bytes_total = bytes_total+bytes_sent;
		cout<< "send all in while"<< bytes_sent<<endl;
		if(bytes_sent = -1)
			break;
	} 

}*/

int sendall(int s, char *buf)
{
 	int total = 0; 
 	int bytesleft = strlen(buf);
 	int n;
	cout<< "inside send all function "<<endl;
 	while(total < bytesleft) {
		cout<< "sending data in send all "<<buf+total<<endl;
 		n = send(s, buf+total, bytesleft, 0);
 			if (n == -1) { break; }
 			total += n;
 		bytesleft -= n;
		cout<< "tota; "<<total<<" bytes left "<<bytesleft<<endl;
 	}

 	return n==-1?-1:0; 
}

bool validateip(char * ipaddress){
	cout<<"inside ip validate function:"<<ipaddress<<":"<<endl;
	
	char  *octet = (char*)malloc(INET_ADDRSTRLEN);
	return inet_pton(AF_INET,ipaddress,octet);
}	


void validateport(char * str, char * port){
    int portno = validatenumber(port);
    if(portno == -1){
            cse4589_print_and_log("[%s:ERROR]\n", str);
            cse4589_print_and_log("[%s:END]\n", str);
    }
    else{
            cse4589_print_and_log("[%s:SUCCESS]\n", str);
            cse4589_print_and_log("PORT:%d\n", portno);
            cse4589_print_and_log("[%s:END]\n", str);
    }
}

bool compareport(struct ClientList list1, struct ClientList list2) 
{ 
    return (list1.portno < list2.portno); 
} 

void refresh_list_server(int fdaccept, vector<struct ClientList> * clientlist, char * list){
	//char * list = ((char *) malloc (BUFFER_SIZE));
        char *delim = ",";
	char portno[5];
	int listlen =0,bytes_sent=0;
	
	vector<struct ClientList>::iterator i;
        for( i = clientlist->begin();i!=clientlist->end();i++){
        sprintf(portno,"%d",i->portno);
	strcat(list,portno);
        strcat(list,delim);
        strcat(list,i->IP);
        strcat(list,delim);
        strcat(list,i->FQDN);
        strcat(list,delim);
        }
       /* bytes_sent = send(*fdaccept, list, strlen(list), 0);
	if(bytes_sent == -1){
		cout<< "refresh bits not sent"<<endl;
	}*/
	sendall(fdaccept, list);
}
char * getipaddress(char * str){

        int udppingsock,x;
        struct addrinfo hints, *server;
        struct sockaddr_in my_ip;
        char s[INET_ADDRSTRLEN];
        memset (&hints, 0 , sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;


        if((x = getaddrinfo("8.8.8.8", "53", &hints, &server))!=0){
		cse4589_print_and_log("[%s:ERROR]\n", str);
		cse4589_print_and_log("[%s:END]\n", str);
                exit(1);
        }
        for ( server; server!=NULL; server = server->ai_next){
        udppingsock = socket (AF_INET, SOCK_DGRAM,0);
        connect (udppingsock, server->ai_addr, server ->ai_addrlen);
	
        }

        bzero(&my_ip, sizeof(my_ip));
        int len = sizeof(my_ip);
        if((getsockname(udppingsock, (struct sockaddr *) &my_ip,(socklen_t *) &len))==-1){
		cse4589_print_and_log("[%s:ERROR]\n", str);
		cse4589_print_and_log("[%s:END]\n", str);
	}
	else{
        inet_ntop(AF_INET, &my_ip.sin_addr, s, sizeof(s));

	cse4589_print_and_log("[%s:SUCCESS]\n", str);

	cse4589_print_and_log("IP:%s\n", s);
	
	cse4589_print_and_log("[%s:END]\n", str);
	}
	return s;
}


int accept_connection(int server_socket, vector<struct ClientList> * clientlist){

        struct sockaddr_in client_addr;
	char * list= (char*) malloc(BUFFER_SIZE);
	char portchar [6]={'\0'};
	memset(list,'\0', BUFFER_SIZE);
	int caddr_len = sizeof(client_addr);
	int portint;

        int fdaccept = accept(server_socket, (struct sockaddr *)&client_addr,(socklen_t *) &caddr_len);
        if(fdaccept < 0)
        	perror("Accept failed.");
        printf("\nRemote Host connected!\n");
        
	recv(fdaccept, &portchar, 6,0);
	portint = stoi(portchar);	
	struct sockaddr_in  * ipv4 = (struct sockaddr_in *)&client_addr;
        struct in_addr ipv4address= ipv4->sin_addr;
        char s[INET_ADDRSTRLEN];
        char hostname[NI_MAXHOST];
        char *ipv4addr;
        socklen_t len_sockaddr_in = sizeof(&ipv4);
        inet_ntop(AF_INET, &ipv4addr,s,sizeof(s));
        ipv4addr=inet_ntoa(client_addr.sin_addr);
        
	/*inet_pton(AF_INET,(ipv4->sin_addr));*/
        getnameinfo((struct sockaddr *)&client_addr,caddr_len,hostname,(socklen_t) sizeof(hostname),NULL,0,NI_NAMEREQD);
        struct ClientList * new_client = new ClientList;
        strcpy(new_client->IP,ipv4addr);
 	strcpy(new_client->FQDN,hostname);
        new_client->portno = portint;
        new_client->fd = fdaccept;
	new_client->login=1;
        new_client->fd_client = -1;
	new_client->msgs_sent = 0;
	new_client->msgs_recv=0;
	new_client->blockedips.next= NULL;
	clientlist->push_back(* new_client);
	bufferedmsgs.insert(pair<string,vector<string>>(new_client->IP,{}));
	sort(clientlist->begin(),clientlist->end(),compareport);
	refresh_list_server(fdaccept,clientlist,list);

	return fdaccept;
}



void server_send(int sock_index,char * cmd, std::vector<struct ClientList> * clientlist){
	cout<< "inside server send function" <<cmd<<endl;
	vector<struct ClientList> ::iterator it;
	char * command = (char*) malloc(sizeof(char)*BUFFER_SIZE);
	char * command_new = (char*) malloc(sizeof(char)*BUFFER_SIZE*2);
	char * buffertosend = (char*)malloc(sizeof(char)*BUFFER_SIZE*2);
	memset(command_new,'\0',BUFFER_SIZE*2);
	memset(buffertosend,'\0',BUFFER_SIZE*2);
	char src_ip[INET_ADDRSTRLEN];
	int cmd_len = strlen(cmd);	
	//char * delim =' ';	
	for(it=clientlist->begin();it!=clientlist->end();it++){
		if(it->fd == sock_index)
			strcpy(src_ip,it->IP);
	}		
	for (int j = 0; j< cmd_len; j++){
        if(cmd[j]==' '){
        	command_new = cmd+j;
               	cmd= command_new;
		command[j]='\0';
                break;
                }
        else{
                command[j] = cmd[j];
                }
        }
	strcat(buffertosend,src_ip);
	strcat(buffertosend,command_new);
		

	char IPAddress[INET_ADDRSTRLEN];
	memset(&IPAddress,'\0',INET_ADDRSTRLEN);
	bool IP_in_list= false;
	for (it = clientlist->begin();it!=clientlist->end();it++){
		if((strcmp(((const char*)(it->IP)),((const char*)(command))))==0){
			for(auto v : it->blocked){
				if(strcmp(v.c_str(),src_ip)==0){
					return;
				}
			}
			IP_in_list = true;
			if(it->fd_client == -1){
				char  server_port[6];
				sprintf(server_port,"%d",it->portno);	
				it->fd_client = connect_to_host( it->IP, server_port);	
			}
			if(it->login==1){
				
				sendall(it->fd_client, buffertosend);
				cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
				cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", src_ip,command, command_new+1);
				cse4589_print_and_log("[%s:END]\n", "RELAYED");
				it->msgs_recv++;
				break;
			}
			if(it->login == 0){

			}
		/* Implement buffer messaging*/
		}
	}
	if(!IP_in_list){
		perror("IP not in list of clients connected");
	}
}

void server_broadcast(int sock_index,char * cmd, std::vector<struct ClientList> * clientlist){
	cout<< "inside server send function" <<endl<<endl;
        vector<struct ClientList> ::iterator it;
        char * command = (char*) malloc(sizeof(char)*BUFFER_SIZE);
        char * command_new = (char*) malloc(sizeof(char)*BUFFER_SIZE);
	char * buffertosend = (char*)malloc(sizeof(char)*BUFFER_SIZE);
	memset(command_new,'\0',BUFFER_SIZE*2);
	memset(buffertosend,'\0',BUFFER_SIZE*2);
        char src_ip[INET_ADDRSTRLEN];
        int cmd_len = strlen(cmd);

        for(it=clientlist->begin();it!=clientlist->end();it++){
                if(it->fd == sock_index)
                        strcpy(src_ip,it->IP);
        }
	strcpy(buffertosend,src_ip);
	strcat(buffertosend," ");
	strcpy(command,"255.255.255.255");
	strcat(buffertosend,cmd);
        char IPAddress[INET_ADDRSTRLEN];
        bool IP_in_list= false;
        for (it = clientlist->begin();it!=clientlist->end();it++){
                if((strcmp(((const char*)(it->IP)),((const char*)(src_ip))))==0){
			continue;
		}
		else{
                        IP_in_list = true;
                        if(it->fd_client == -1){
                                char  server_port[6];
                                sprintf(server_port,"%d",it->portno);
                                it->fd_client = connect_to_host( it->IP, server_port);
                        }
                        if(it->login==1){
				sendall(it->fd_client, buffertosend);
				/*cout<< command<< " sending to "<<it->IP<< " on "<<it->portno<< " sockfd"<< it->fd<< " " <<it->fd_client<<endl;*/
				cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
				cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", src_ip, command, cmd);
				cse4589_print_and_log("[%s:END]\n", "RELAYED");
				it->msgs_recv++;
				//break;
                        }
			else if (it->login ==2){
				for(itr = bufferedmsgs.begin();itr!=bufferedmsgs.end();itr++){
					if(strcmp(itr->first.c_str(),it->IP)==0){
						itr->second.push_back(cmd);
					}
				}
			}
                /* Implement buffer messaging*/
                }
        }
        if(!IP_in_list){
                perror("IP not in list of clients connected");
        }

}

/* This function removes the blocked IP given in the input from the blocked list of structure
 * it returns 1 on successful removal
 * returns -1 if the IP is invalid/ not present in th blocked list */

int unblockip(int sock_index, char * buffer, std::vector<struct ClientList>*clientlist){
	cout << "unblock ip buffer is "<<buffer<<endl;
	vector<struct ClientList> :: iterator myit;
        
	for(myit= clientlist->begin();myit!=clientlist->end();myit++){
		if(myit->fd==sock_index){
			int pos =0;
			for(auto i:myit->blocked){
				if(strcmp(i.c_str(), buffer)==0){
					myit->blocked.erase(myit->blocked.begin()+pos);
					return 1;
				}
				pos++;
			}
		}  

	}	
	return -1;
}

void displayblocked(std::vector<struct ClientList>*clientlist, char * buffer){
	cout<<"inside displayblockedi"<<buffer<<endl;
	vector<struct ClientList> :: iterator myit;
	set <string> blocklist;
	vector<struct ClientList> :: iterator newit;
	int num =1;
	cse4589_print_and_log("[%s:SUCCESS]\n", "BLOCKED");
	for(myit = clientlist->begin(); myit!=clientlist->end();myit++){
		if(strcmp(myit->IP,buffer)==0){
                for(auto i:myit->blocked){
			for(newit = clientlist->begin(); newit!=clientlist->end();newit++){
				if (strcmp(newit->IP,i.c_str())==0){
					cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",num , newit->FQDN, newit->IP, newit->portno);
					num++;
				}
			}
                }
        }
	}
	cse4589_print_and_log("[%s:END]\n", "BLOCKED");

}	

void blockip(int sock_index, char * buffer, std::vector<struct ClientList> *clientlist){
	cout << "inside server block"<<"buffer "<<buffer<<endl;
        int cmd_len = strlen(buffer);
        char * newip=(char*) malloc (BUFFER_SIZE);
        memset(newip,'\0',BUFFER_SIZE);
	vector<struct ClientList> :: iterator myit;
       
	for (int j = 0; j<=cmd_len; j++){
       		if(buffer[j]==' '||(buffer[j]=='\0')){
       		newip[j]='\0';
       		break;
       	}
       	else{
       		newip[j] = buffer[j];
       		cout<< buffer[j]<<endl;
       	}
       	}
       
	for(myit = clientlist->begin(); myit!=clientlist->end();myit++){
       		if(myit->fd==sock_index){
       			if(myit->blocked.size()==0){
				myit->blocked.push_back(newip);
				break;
			}
       			for(auto i: myit->blocked){
				if(strcmp(i.c_str(),newip)==0){
				}
       				else{
					myit->blocked.push_back(newip);
				}
			}

       		}
       	}
}

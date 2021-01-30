
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <typeinfo>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <set>
#include <bits/stdc++.h>
#include <unordered_set>
#define STDIN 0
#define TRUE 1
#define SIZE_CMD 100
#define SIZE_BUFFER 256

#include "../include/global.h"
#include "../include/logger.h"

int host_connection(char *server_ip_address, char *server_port_no);
void print_author();
void command_parser(char *cmd, char *new_cmnd);
void list_client_refresh(std::vector<struct ListClient> *list_clients, char *list);
bool checkIP(char *ip_address, std::vector<struct ListClient> *list_clients);
int client_login(char *cmd, char *port_no, std::vector<struct ListClient> *list_clients, fd_set *master_list);
int accepting_client(int server_socket);
void process_newline(std::string const &str, const char delim, std::vector<std::string> &out);
char *find_ip(char *str);
int connection_accept(int server_socket, std::vector<struct ListClient> *list_clients);
void createing_connection(int *sock_fd, char *port);
void list_server_refresh(int accept_fd, std::vector<struct ListClient> *list_clients, char *list);
void sending_server(int sock_index, char *cmd, std::vector<struct ListClient> *list_clients);
void broadcasting_server(int sock_index, char *cmd, std::vector<struct ListClient> *list_clients);
void remove_fd(int sock_index, std::vector<struct ListClient> *list_clients);
int number_validation(char *portno);
void port_validation(char *str, char *port);
int ip_unblocking(int sock_index, char *buffer, std::vector<struct ListClient> *list_clients);
void show_blocked(std::vector<struct ListClient> *list_clients, char *buffer);
bool ip_validation(char *ipaddress);
void ip_blocking(int sock_index, char *buffer, std::vector<struct ListClient> *list_clients);
int sending_to_all(int s, char *buf);

using namespace std;
unordered_map<string, vector<string>> msg_buffer;
unordered_map<string, vector<string>>::iterator itr;


struct ListClient
{	
	int login;
	int portno;
	int msgs_sent;
	int msgs_recv;
	int fd;
	int fd_client;
	char IP[INET_ADDRSTRLEN];
	char FQDN[128];
	vector<string> blocked;
	
};

void print_author()
{
	cse4589_print_and_log("[%s:SUCCESS]\n", "AUTHOR");
	cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "vkintali");
	// cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "pgaddam");
	cse4589_print_and_log("[%s:END]\n", "AUTHOR");
}

void printing_client_list(char *new_cmnd, std::vector<struct ListClient> *list_clients)
{
	int k = 1;
	vector<struct ListClient>::iterator temp_itr;
	cse4589_print_and_log("[%s:SUCCESS]\n", new_cmnd);
	for (temp_itr = list_clients->begin(); temp_itr != list_clients->end(); temp_itr++)
	{
		if (temp_itr->login == 1)
		{
			cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", k, temp_itr->FQDN, temp_itr->IP, temp_itr->portno);
			k++;
		}
	}
	cse4589_print_and_log("[%s:END]\n", new_cmnd);
}

int host_connection(char *server_ip_address, char *server_port_no)
{
	struct addrinfo hints, *result;
	int sock_fd;
	/* Set up hints structure */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	/* Fill up address structures */
	if (getaddrinfo(server_ip_address, server_port_no, &hints, &result) != 0)
		perror("failed getting addrinfo");

	/* Socket */
	sock_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sock_fd < 0)
		perror("Socket Creation Failed");
	/* Connect */
	if (connect(sock_fd, result->ai_addr, result->ai_addrlen) < 0)
	{
		perror("Connect failure");
		return 0;
	}
	freeaddrinfo(result);
	return sock_fd;
}

void process_newline(std::string const &str, const char separator,
					 std::vector<std::string> &out)
{
	size_t right = 0;
	size_t left;
	while ((left = str.find_first_not_of(separator, right)) != std::string::npos)
	{
		right = str.find(separator, left);
		out.push_back(str.substr(left, right - left));
	}
}

void list_client_refresh(std::vector<struct ListClient> *list_clients, char *list)
{
	list_clients->clear();
	vector<string> array_list;
	const char separator = ',';
	process_newline(list, separator, array_list);
	for (int i = 0; i < array_list.size(); i++)
	{
		struct ListClient *new_client = new ListClient;
		memset(&new_client->IP, '\0', INET_ADDRSTRLEN);
		memset(&new_client->FQDN, '\0', 128);
		new_client->portno = stoi(array_list[i]);
		int j = 0;
		i++;
		while (array_list[i][j] != '\0')
		{
			new_client->IP[j] = array_list[i][j];
			j++;
		}
		i++;
		int k = 0;
		while (array_list[i][k] != '\0')
		{
			new_client->FQDN[k] = array_list[i][k];
			k++;
		}
		new_client->login = 1;
		list_clients->push_back(*new_client);
	}
}

bool checkIP(char *ip_address, std::vector<struct ListClient> *list_clients)
{
	vector<struct ListClient>::iterator temp_itr;
	bool flag = false;
	for (temp_itr = list_clients->begin(); temp_itr != list_clients->end(); temp_itr++)
	{
		if (strcmp(temp_itr->IP, ip_address) == 0)
			flag = true;
	}
	return flag;
}

int client_login(char *cmd, char *port_no, std::vector<struct ListClient> *list_clients, fd_set *master_list)
{
	char server_IP[INET_ADDRSTRLEN];
	memset(&server_IP, '\0', INET_ADDRSTRLEN);
	char server_Port[6];
	memset(&server_Port, '\0', 6);
	char *new_cmnd = (char *)malloc(sizeof(char) * SIZE_CMD);
	memset(new_cmnd, '\0', SIZE_CMD);
	char *command_new = (char *)malloc(sizeof(char) * SIZE_CMD);
	memset(command_new, '\0', SIZE_CMD);
	int len_cmd, sock_fd;
	len_cmd = strlen(cmd);

	for (int i = 0; i < len_cmd; i++)
	{
		if (cmd[i] == ' ')
		{
			command_new = cmd + i + 1;
			cmd = command_new;
			server_IP[i] = '\0';
			break;
		}
		else
		{
			server_IP[i] = cmd[i];
		}
	}
	len_cmd = strlen(cmd);
	for (int i = 0; i < len_cmd; i++)
	{
		if (cmd[i] == ' ')
		{
			command_new = cmd + i + 1;
			cmd = command_new;
			break;
		}
		else
		{
			server_Port[i] = cmd[i];
		}
	}
	int valid_port = number_validation(server_Port);
	bool valid_ip = ip_validation(server_IP);
	if ((valid_ip == false) || valid_port == -1)
		return -1;
	sock_fd = host_connection(server_IP, server_Port);
	if (sock_fd == 0)
		return 0;
	else
	{
		send(sock_fd, port_no, 6, 0);
		int bytes_recvd;
		char *lst = (char *)malloc(sizeof(char) * SIZE_BUFFER);
		memset(lst, '\0', SIZE_BUFFER);
		if (bytes_recvd = recv(sock_fd, lst, SIZE_BUFFER - 1, 0) != 0)
		{
			list_client_refresh(list_clients, lst);
		}
		vector<struct ListClient>::iterator it;
		return sock_fd;
	}
}

void receiveall(int sock_fd)
{
	char *buf = (char *)malloc(sizeof(char) * SIZE_BUFFER);
	memset(buf, '\0', SIZE_BUFFER);
	char *lst = (char *)malloc(sizeof(char) * SIZE_BUFFER);
	memset(lst, '\0', SIZE_BUFFER);
	int receving_bytes = 0, bytes_recvd = 0, total_recvd = 0;
	receving_bytes = recv(sock_fd, lst, 4, 0);
	while (total_recvd < receving_bytes)
	{
		bytes_recvd = recv(sock_fd, lst, SIZE_BUFFER, 0);
		total_recvd = total_recvd + bytes_recvd;
		strcat(buf, lst);
		memset(lst, '\0', SIZE_BUFFER);
	}
}

int accepting_client(int server_socket)
{
	struct sockaddr_in client_addr;
	int len_client_addr = sizeof(client_addr);
	int accept_fd = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&len_client_addr);
	if (accept_fd < 0)
		perror("Accepting failure.");
	return accept_fd;
}

void remove_fd(int sock_index, std::vector<struct ListClient> *list_clients)
{
	vector<struct ListClient>::iterator temp_itr;
	for (temp_itr = list_clients->begin(); temp_itr != list_clients->end(); temp_itr++)
	{
		if (sock_index == temp_itr->fd)
		{
			list_clients->erase(temp_itr);
			break;
		}
	}
}

int number_validation(char *portno)
{
	for (int i = 0; portno[i] != '\0'; i++)
	{
		if (portno[i] > '9' || portno[i] < '0')
			return -1;
	}
	return stoi(portno);
}

int sending_to_all(int s, char *buf)
{
	int sent_total = 0;
	int n;
	int remaining_bytes = strlen(buf);
	while (sent_total < remaining_bytes)
	{
		n = send(s, buf + sent_total, remaining_bytes, 0);
		if (n == -1)
		{
			break;
		}
		sent_total += n;
		remaining_bytes -= n;
	}
	if (n == -1)
	{
		return -1;
	}
	else
	{
		return 0;
	}
	// return n == -1 ? -1 : 0;
}

bool ip_validation(char *ipaddress)
{
	char *temp = (char *)malloc(INET_ADDRSTRLEN);
	return inet_pton(AF_INET, ipaddress, temp);
}

void port_validation(char *str, char *port)
{
	int port_no = number_validation(port);
	if (port_no == -1)
	{
		cse4589_print_and_log("[%s:ERROR]\n", str);
		cse4589_print_and_log("[%s:END]\n", str);
	}
	else
	{
		cse4589_print_and_log("[%s:SUCCESS]\n", str);
		cse4589_print_and_log("PORT:%d\n", port_no);
		cse4589_print_and_log("[%s:END]\n", str);
	}
}

bool port_comparison(struct ListClient list1, struct ListClient list2)
{
	return (list1.portno < list2.portno);
}

void list_server_refresh(int accept_fd, vector<struct ListClient> *list_clients, char *list)
{
	char *separator = ",";
	char portno[5];
	vector<struct ListClient>::iterator temp_itr;
	for (temp_itr = list_clients->begin(); temp_itr != list_clients->end(); temp_itr++)
	{
		sprintf(portno, "%d", temp_itr->portno);
		strcat(list, portno);
		strcat(list, separator);
		strcat(list, temp_itr->IP);
		strcat(list, separator);
		strcat(list, temp_itr->FQDN);
		strcat(list, separator);
	}
	sending_to_all(accept_fd, list);
}

char *find_ip(char *str)
{
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	int udp_sock, temp;
	struct sockaddr_in temp_ip;
	char serv[INET_ADDRSTRLEN];
	temp = getaddrinfo("8.8.8.8", "53", &hints, &res);
	
	if ((temp) != 0)
	{
		cse4589_print_and_log("[%s:ERROR]\n", str);
		cse4589_print_and_log("[%s:END]\n", str);
		exit(1);
	}
	for (res; res != NULL; res = res->ai_next)
	{
		udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
		connect(udp_sock, res->ai_addr, res->ai_addrlen);
	}
	bzero(&temp_ip, sizeof(temp_ip));
	int len = sizeof(temp_ip);
	if ((getsockname(udp_sock, (struct sockaddr *)&temp_ip, (socklen_t *)&len)) == -1)
	{
		cse4589_print_and_log("[%s:ERROR]\n", str);
		cse4589_print_and_log("[%s:END]\n", str);
	}
	else
	{
		inet_ntop(AF_INET, &temp_ip.sin_addr, serv, sizeof(serv));
		cse4589_print_and_log("[%s:SUCCESS]\n", str);
		cse4589_print_and_log("IP:%s\n", serv);
		cse4589_print_and_log("[%s:END]\n", str);
	}
	return serv;
}

int connection_accept(int server_socket, vector<struct ListClient> *list_clients)
{
	char char_port_no[6] = {'\0'};
	int int_port_no;
	struct sockaddr_in client_addr;
	int len_client_addr = sizeof(client_addr);
	char *lst = (char *)malloc(SIZE_BUFFER);
	memset(lst, '\0', SIZE_BUFFER);
	int accept_fd = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&len_client_addr);
	if (accept_fd < 0)
		perror("Accept failed.");
	printf("\nRemote Host connected!\n");
	recv(accept_fd, &char_port_no, 6, 0);
	int_port_no = stoi(char_port_no);
	struct sockaddr_in *ipv4 = (struct sockaddr_in *)&client_addr;
	char serv[INET_ADDRSTRLEN];
	char host[NI_MAXHOST];
	char *ipv4addr;
	socklen_t len_sockaddr_in = sizeof(&ipv4);
	inet_ntop(AF_INET, &ipv4addr, serv, sizeof(serv));
	ipv4addr = inet_ntoa(client_addr.sin_addr);

	getnameinfo((struct sockaddr *)&client_addr, len_client_addr, host, (socklen_t)sizeof(host), NULL, 0, NI_NAMEREQD);
	struct ListClient *new_client = new ListClient;
	strcpy(new_client->IP, ipv4addr);
	strcpy(new_client->FQDN, host);
	new_client->portno = int_port_no;
	new_client->fd = accept_fd;
	new_client->login = 1;
	new_client->fd_client = -1;
	new_client->msgs_sent = 0;
	new_client->msgs_recv = 0;
	list_clients->push_back(*new_client);
	msg_buffer.insert(pair<string, vector<string>>(new_client->IP, {}));
	sort(list_clients->begin(), list_clients->end(), port_comparison);
	list_server_refresh(accept_fd, list_clients, lst);
	return accept_fd;
}

void sending_server(int sock_index, char *cmd, std::vector<struct ListClient> *list_clients)
{
	vector<struct ListClient>::iterator temp_itr;
	char src_ip[INET_ADDRSTRLEN];
	int len_cmd = strlen(cmd);
	char *new_cmnd = (char *)malloc(sizeof(char) * SIZE_BUFFER);
	char *command_new = (char *)malloc(sizeof(char) * SIZE_BUFFER * 2);
	memset(command_new, '\0', SIZE_BUFFER * 2);
	char *buffer_send = (char *)malloc(sizeof(char) * SIZE_BUFFER * 2);
	memset(buffer_send, '\0', SIZE_BUFFER * 2);

	for (temp_itr = list_clients->begin(); temp_itr != list_clients->end(); temp_itr++)
	{
		if (sock_index==temp_itr->fd)
			strcpy(src_ip, temp_itr->IP);
	}
	for (int j = 0; j < len_cmd; j++)
	{
		if (cmd[j] == ' ')
		{
			command_new = cmd + j;
			cmd = command_new;
			new_cmnd[j] = '\0';
			break;
		}
		else
		{
			new_cmnd[j] = cmd[j];
		}
	}
	strcat(buffer_send, src_ip);
	strcat(buffer_send, command_new);
	char ip_address[INET_ADDRSTRLEN];
	memset(&ip_address, '\0', INET_ADDRSTRLEN);
	bool flag_ip = false;
	for (temp_itr = list_clients->begin(); temp_itr != list_clients->end(); temp_itr++)
	{
		if ((strcmp(((const char *)(temp_itr->IP)), ((const char *)(new_cmnd)))) == 0)
		{
			for (auto v : temp_itr->blocked)
			{
				if (strcmp(v.c_str(), src_ip) == 0)
				{
					return;
				}
			}
			flag_ip = true;
			if (temp_itr->fd_client == -1)
			{
				char server_port_no[6];
				sprintf(server_port_no, "%d", temp_itr->portno);
				temp_itr->fd_client = host_connection(temp_itr->IP, server_port_no);
			}
			if (temp_itr->login == 1)
			{
				sending_to_all(temp_itr->fd_client, buffer_send);
				cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
				cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", src_ip, new_cmnd, command_new + 1);
				cse4589_print_and_log("[%s:END]\n", "RELAYED");
				temp_itr->msgs_recv++;
				break;
			}
		}
	}
	if (!flag_ip)
	{
		perror("IP is not valid for the connected clients");
	}
}

void broadcasting_server(int sock_index, char *cmd, std::vector<struct ListClient> *list_clients)
{
	vector<struct ListClient>::iterator temp_itr;
	char src_ip[INET_ADDRSTRLEN];
	char *new_cmnd = (char *)malloc(sizeof(char) * SIZE_BUFFER);
	char *command_new = (char *)malloc(sizeof(char) * SIZE_BUFFER);
	memset(command_new, '\0', SIZE_BUFFER * 2);
	char *buffer_send = (char *)malloc(sizeof(char) * SIZE_BUFFER);
	memset(buffer_send, '\0', SIZE_BUFFER * 2);
	for (temp_itr = list_clients->begin(); temp_itr != list_clients->end(); temp_itr++)
	{
		if (temp_itr->fd == sock_index)
			strcpy(src_ip, temp_itr->IP);
	}
	strcpy(buffer_send, src_ip);
	strcat(buffer_send, " ");
	strcpy(new_cmnd, "255.255.255.255");
	strcat(buffer_send, cmd);
	bool flag_ip = false;
	char ip_address[INET_ADDRSTRLEN];
	for (temp_itr = list_clients->begin(); temp_itr != list_clients->end(); temp_itr++)
	{
		if ((strcmp(((const char *)(temp_itr->IP)), ((const char *)(src_ip)))) == 0)
		{
			continue;
		}
		else
		{
			flag_ip = true;
			if (temp_itr->fd_client == -1)
			{
				char server_port_no[6];
				sprintf(server_port_no, "%d", temp_itr->portno);
				temp_itr->fd_client = host_connection(temp_itr->IP, server_port_no);
			}
			if (temp_itr->login == 1)
			{
				sending_to_all(temp_itr->fd_client, buffer_send);
				cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
				cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", src_ip, new_cmnd, cmd);
				cse4589_print_and_log("[%s:END]\n", "RELAYED");
				temp_itr->msgs_recv++;
			}
			else if (temp_itr->login == 2)
			{
				for (itr = msg_buffer.begin(); itr != msg_buffer.end(); itr++)
				{
					if (strcmp(itr->first.c_str(), temp_itr->IP) == 0)
					{
						itr->second.push_back(cmd);
					}
				}
			}
		}
	}
	if (!flag_ip)
	{
		perror("IP is not valid for the connected clients");
	}
}

int ip_unblocking(int sock_index, char *buffer, std::vector<struct ListClient> *list_clients)
{
	vector<struct ListClient>::iterator temp_itr;
	for (temp_itr = list_clients->begin(); temp_itr != list_clients->end(); temp_itr++)
	{
		if (temp_itr->fd == sock_index)
		{
			int count = 0;
			for (auto i : temp_itr->blocked)
			{
				if (strcmp(i.c_str(), buffer) == 0)
				{
					temp_itr->blocked.erase(temp_itr->blocked.begin() + count);
					return 1;
				}
				count++;
			}
		}
	}
	return -1;
}

void show_blocked(std::vector<struct ListClient> *list_clients, char *buffer)
{
	int count = 1;
	vector<struct ListClient>::iterator temp_itr;
	vector<struct ListClient>::iterator temp_i;
	set<string> blocklist;
	cse4589_print_and_log("[%s:SUCCESS]\n", "BLOCKED");
	for (temp_itr = list_clients->begin(); temp_itr != list_clients->end(); temp_itr++)
	{
		if (strcmp(temp_itr->IP, buffer) == 0)
		{
			unordered_set<string> blockediplist;
			unordered_set<string>::iterator listitr;
			for(auto i : temp_itr->blocked){
				if(blockediplist.find(i)==blockediplist.end())
					
				blockediplist.insert(i);
			}
			for (temp_i = list_clients->begin(); temp_i != list_clients->end(); temp_i++)
			{
				for (auto i : blockediplist)
				{
					if ((strcmp(temp_i->IP, i.c_str()) == 0))
					{
						cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", count, temp_i->FQDN, temp_i->IP, temp_i->portno);
						count++;
					}
				}
			}
		}
	}
	cse4589_print_and_log("[%s:END]\n", "BLOCKED");
}

void ip_blocking(int sock_index, char *buffer, std::vector<struct ListClient> *list_clients)
{
	char *newip = (char *)malloc(SIZE_BUFFER);
	memset(newip, '\0', SIZE_BUFFER);
	vector<struct ListClient>::iterator temp_itr;
	int len_cmd = strlen(buffer);
	for (int i = 0; i <= len_cmd; i++)
	{
		if (buffer[i] == ' ' || (buffer[i] == '\0'))
		{
			newip[i] = '\0';
			break;
		}
		else
		{
			newip[i] = buffer[i];
			cout << buffer[i] << endl;
		}
	}
	for (temp_itr = list_clients->begin(); temp_itr != list_clients->end(); temp_itr++)
	{
		if (temp_itr->fd == sock_index)
		{
				temp_itr->blocked.push_back(newip);
				return;
		}
	}
}

void getip(char *new_cmnd, char *ip)
{
	int len_cmnd = strlen(new_cmnd);
	char *server_IP = (char *)malloc(INET_ADDRSTRLEN);
	for (int i = 0; i < len_cmnd; i++)
	{
		if (new_cmnd[i] == ' ')
		{
			ip[i] = '\0';
			break;
		}
		else
		{
			ip[i] = new_cmnd[i];
		}
	}
}

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char *argv[])
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/* Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
	if (argc != 3)
	{
		printf("Usage:%s s/c [port]\n", argv[0]);
		exit(-1);
	}

	int head_socket,server_socket, selret, sock_index, accept_fd = 0, caddr_len;
	char* port = argv[2];
	struct addrinfo hints, *res;
	fd_set master_list, watch_list;
	vector<struct ListClient> list_clients;
	vector<struct ListClient>::iterator temp_itr;
	vector<string> myblocklist;
	bool login = false;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	int yes = 1;

	if (number_validation(argv[2]) != -1)
	{
		if (getaddrinfo(NULL, argv[2], &hints, &res) != 0)
			perror("failed getting addrinfo");

		/* Socket */
		server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (server_socket < 0)
			perror("Socket Creation Failed");
		setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		/* Bind */
		if (bind(server_socket, res->ai_addr, res->ai_addrlen) < 0)
			perror("Bind failure");

		freeaddrinfo(res);

		/* Listen */
		if (listen(server_socket, MAX_CONNECTIONS) < 0)
			perror("Unable to listen on port");
	}
	/* ---------------------------------------------------------------------------- */

	/* Zero select FD sets */
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);

	/* Register the listening socket */
	FD_SET(server_socket, &master_list);
	/* Register STDIN */
	FD_SET(STDIN, &master_list);

	head_socket = server_socket;

	if (strcmp(argv[1], "s") == 0)
	{
		while (TRUE)
		{
			memcpy(&watch_list, &master_list, sizeof(master_list));

			printf("\n[PA1-Server@CSE489/589]$ ");
			fflush(stdout);

			/* select() system call. This will BLOCK */
			selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
			if (selret < 0)
				perror("select failure.");
			/* Check if we have sockets/STDIN to process */
			if (selret > 0)
			{
				/* Loop through socket descriptors to check which ones are ready */
				for (sock_index = 0; sock_index <= head_socket; sock_index += 1)
				{
					// char sock_index_char[2];
					// sprintf(sock_index_char, "%d", sock_index);
					if (FD_ISSET(sock_index, &watch_list))
					{
						/* Check if new new_cmnd on STDIN */
						if (sock_index == STDIN)
						{
							char *cmd = (char *)malloc(sizeof(char) * SIZE_CMD);
							memset(cmd, '\0', SIZE_CMD);
							if (fgets(cmd, SIZE_CMD - 1, stdin) == NULL) /*Mind the newline character that will be written to cmd*/
								exit(-1);
							//Remove new line 
							int cmd_size = sizeof(cmd);
							int cmd_len = strlen(cmd);
							if (cmd[cmd_len - 1] == '\n')
							{
								cmd[cmd_len - 1] = '\0';
								cmd_len = strlen(cmd);
							}
							/*Process PA1 commands here ...*/
							char *new_cmnd = (char *)malloc(sizeof(char) * SIZE_CMD);
							memset(new_cmnd, '\0', SIZE_CMD);
							char *command_new = (char *)malloc(sizeof(char) * SIZE_CMD);
							for (int i = 0; i < cmd_len; i++)
							{
								if (cmd[i] == ' ')
								{
									command_new = cmd + i + 1;
									cmd = command_new;
									break;
								}
								else
								{
									new_cmnd[i] = cmd[i];
								}
							}
							/*Process PA1 server commands here ...*/
							if (strcmp(new_cmnd, "AUTHOR") == 0)
							{
								print_author();
							}
							if (strcmp(new_cmnd, "IP") == 0)
							{
								find_ip(new_cmnd);
							}
							else if (strcmp(new_cmnd, "PORT") == 0)
							{
								port_validation(new_cmnd, port);
							}
							else if (strcmp(new_cmnd, "LIST") == 0)
							{
								printing_client_list(new_cmnd, &list_clients);
							}
							else if (strcmp(new_cmnd, "BLOCKED") == 0)
							{
								if (ip_validation(command_new) == false || checkIP(command_new, &list_clients) == false)
								{
									cse4589_print_and_log("[%s:ERROR]\n", new_cmnd);
									cse4589_print_and_log("[%s:END]\n", new_cmnd);
									break;
								}
								show_blocked(&list_clients, command_new);
							}
							else if (strcmp(new_cmnd, "STATISTICS") == 0)
							{
								vector<struct ListClient>::iterator temp_itr;
								int j = 1;
								cse4589_print_and_log("[%s:SUCCESS]\n", new_cmnd);
								for (temp_itr = list_clients.begin(); temp_itr != list_clients.end(); temp_itr++)
								{
									const char *loginstatus;
									if (temp_itr->login)
									{
										loginstatus ="logged-in";
									}
									else
									{
										loginstatus = "logged-out";
									}
									
									cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", j, temp_itr->FQDN, temp_itr->msgs_sent, temp_itr->msgs_recv, loginstatus);
									j++;
								}
								cse4589_print_and_log("[%s:END]\n", new_cmnd);
							}
						}

						/* Check if new client is requesting connection */
						else if (sock_index == server_socket)
						{
							//connection_accept
							int accept_fd = connection_accept(server_socket, &list_clients);
							if (accept_fd < 0)
								perror("Accept failed.");
							printf("\nRemote Host connected!\n");
							FD_SET(accept_fd, &master_list);
							if (accept_fd > head_socket)
								head_socket = accept_fd;
						}
						/* Read from existing clients */
						else
						{
							/* Initialize buffer to receieve response */
							char *buffer = (char *)malloc(sizeof(char) * SIZE_BUFFER * 2);
							memset(buffer, '\0', SIZE_BUFFER * 2);
							char *new_cmnd = (char *)malloc(sizeof(char) * SIZE_BUFFER * 2);
							memset(new_cmnd, '\0', SIZE_BUFFER * 2);
							char *command_new = (char *)malloc(sizeof(char) * SIZE_BUFFER * 2);
							memset(command_new, '\0', SIZE_BUFFER * 2);
							if (recv(sock_index, buffer, SIZE_BUFFER * 2, 0) <= 0)
							{
								close(sock_index);
								printf("Remote Host terminated connection!\n");
								remove_fd(sock_index, &list_clients);
								/* Remove from watched list */
								FD_CLR(sock_index, &master_list);
							}

							else
							{
								/*Process incoming data from existing clients here ...*/
								for (temp_itr = list_clients.begin(); temp_itr != list_clients.end(); temp_itr++)
								{
									if (temp_itr->fd == sock_index)
									{
										temp_itr->msgs_sent++;
										break;
									}
								}
								//printf("ECHOing it back to the remote host ... ");
								int cmd_len = strlen(buffer);
								for (int i = 0; i < cmd_len; i++)
								{
									if ((buffer[i] == ' ') || (buffer[i] == '\0'))
									{
										command_new = buffer + i + 1;
										buffer = command_new;
										new_cmnd[i] = '\0';
										break;
									}
									else
									{
										new_cmnd[i] = buffer[i];
									}
								}
								if (strcmp(new_cmnd, "REFRESH") == 0)
								{
									char *lst = (char *)malloc(SIZE_BUFFER);
									memset(lst, '\0', SIZE_BUFFER);
									list_server_refresh(sock_index, &list_clients, lst);
								}
								else if (strcmp(new_cmnd, "SEND") == 0)
								{
									sending_server(sock_index, buffer, &list_clients);
								}
								else if (strcmp(new_cmnd, "LOGIN") == 0)
								{
									for (temp_itr = list_clients.begin(); temp_itr != list_clients.end(); temp_itr++)
									{
										if (temp_itr->fd == sock_index)
										{
											temp_itr->login = 1;
											temp_itr->msgs_sent--;
											break;
										}
									}
								}
								else if (strcmp(new_cmnd, "LOGOUT") == 0)
								{
									for (temp_itr = list_clients.begin(); temp_itr != list_clients.end(); temp_itr++)
									{
										if (temp_itr->fd == sock_index)
										{
											temp_itr->login = 0;
											temp_itr->msgs_sent--;
											break;
										}
									}
								}
								else if (strcmp(new_cmnd, "BLOCK") == 0)
								{
									ip_blocking(sock_index, buffer, &list_clients);
								}
								else if (strcmp(new_cmnd, "UNBLOCK") == 0)
								{
									ip_unblocking(sock_index, buffer, &list_clients);
								}
								else if (strcmp(new_cmnd, "BLOCKED") == 0)
								{
									show_blocked(&list_clients, buffer);
								}
								else if (strcmp(new_cmnd, "BROADCAST") == 0)
								{
									broadcasting_server(sock_index, buffer, &list_clients);
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		int login = 0;
		while (TRUE)
		{
			memcpy(&watch_list, &master_list, sizeof(master_list));
			selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
			int sock_fd;
			if (selret < 0)
				perror("select failure.");
			if (selret > 0)
			{
				/* Loop through socket descriptors to check which ones are ready */
				for (int sock_index = 0; sock_index <= head_socket; sock_index += 1)
				{
					if (FD_ISSET(sock_index, &watch_list))
					{
						/* Check if new new_cmnd on STDIN */
						if (sock_index == STDIN)
						{
							vector<string> recvd_cmnd;
							char *cmd = (char *)malloc(sizeof(char) * SIZE_BUFFER * 2);
							process_newline(cmd, ' ', recvd_cmnd);
							for (auto i : recvd_cmnd)
								memset(cmd, '\0', SIZE_BUFFER * 2);
							char *cmd_orig = (char *)malloc(sizeof(char) * SIZE_BUFFER * 2);						
							memset(cmd_orig, '\0', SIZE_BUFFER * 2);
							if (fgets(cmd, SIZE_BUFFER * 2 - 1, stdin) == NULL)
								exit(-1);
							int cmd_len = strlen(cmd);
							if (cmd[cmd_len - 1] == '\n')
							{
								cmd[cmd_len - 1] = '\0';
								cmd_len = strlen(cmd);
							}
							strcpy(cmd_orig, cmd);
							char *new_cmnd = (char *)malloc(sizeof(char) * SIZE_CMD);
							memset(new_cmnd, '\0', SIZE_CMD);
							char *command_new = (char *)malloc(sizeof(char) * SIZE_CMD);
							for (int i = 0; i < cmd_len; i++)
							{
								if (cmd[i] == ' ')
								{
									command_new = cmd + i + 1;
									cmd = command_new;
									new_cmnd[i] = '\0';
									break;
								}
								else
								{
									new_cmnd[i] = cmd[i];
								}
							}
							char *iparg = (char *)malloc(INET_ADDRSTRLEN);
							getip(cmd, iparg);
							/*Process PA1 client commands here ...*/
							if (strcmp(new_cmnd, "IP") == 0)
							{
								find_ip(new_cmnd);
							}
							else if (strcmp(new_cmnd, "AUTHOR") == 0)
							{
								print_author();
							}
							else if (strcmp(new_cmnd, "PORT") == 0)
							{
								port_validation(new_cmnd, port);
							}
							else if (strcmp(new_cmnd, "EXIT") == 0)
							{
								return 0;
							}
							else if (strcmp(new_cmnd, "LIST") == 0)
							{
								if (login)
								{
									printing_client_list(new_cmnd, &list_clients);
								}
								else
								{
									cse4589_print_and_log("[%s:ERROR]\n", new_cmnd);
									cse4589_print_and_log("[%s:END]\n", new_cmnd);
								}
							}
							else if (strcmp(new_cmnd, "REFRESH") == 0)
							{
								char *list = (char *)malloc(sizeof(char) * SIZE_BUFFER);
								memset(list, '\0', SIZE_BUFFER);
								int refresh_sent = send(sock_fd, new_cmnd, strlen(new_cmnd), 0);
								if (refresh_sent == -1)
									perror("failed to send refresh");
								if (int recv_bytes = recv(sock_fd, list, SIZE_BUFFER - 1, 0) != 0)
									list_client_refresh(&list_clients, list);
							}
							else if (strcmp(new_cmnd, "LOGIN") == 0)
							{
								if (login == 0)
								{
									sock_fd = client_login(cmd, argv[2], &list_clients, &master_list);
								}
								if (login == 2)
								{
									send(sock_fd, new_cmnd, strlen(new_cmnd), 0);
									login = 1;
								}
								if (sock_fd > head_socket)
								{
									head_socket = sock_fd;
								}
								if (sock_fd <= 0)
								{
									cse4589_print_and_log("[%s:ERROR]\n", new_cmnd);
									cse4589_print_and_log("[%s:END]\n", new_cmnd);
									break;
								}
								login = 1;
								cse4589_print_and_log("[%s:SUCCESS]\n", new_cmnd);
								cse4589_print_and_log("[%s:END]\n", new_cmnd);
							}
							else if (strcmp(new_cmnd, "LOGOUT") == 0)
							{
								send(sock_fd, cmd_orig, cmd_len, 0);
								login = 2;
								cse4589_print_and_log("[%s:SUCCESS]\n", new_cmnd);
								cse4589_print_and_log("[%s:END]\n", new_cmnd);
							}
							else if (strcmp(new_cmnd, "SEND") == 0)
							{
								int bytes_sent = 0;
								cmd_len = strlen(cmd_orig);
								if (ip_validation(iparg) == false || checkIP(iparg, &list_clients) == false)
								{
									cse4589_print_and_log("[%s:ERROR]\n", new_cmnd);
									cse4589_print_and_log("[%s:END]\n", new_cmnd);
									break;
								}
								bytes_sent = sending_to_all(sock_fd, cmd_orig);
								cse4589_print_and_log("[%s:SUCCESS]\n", new_cmnd);
								cse4589_print_and_log("[%s:END]\n", new_cmnd);
							}
							else if (strcmp(new_cmnd, "BROADCAST") == 0)
							{
								int bytes_sent = 0;
								cmd_len = strlen(cmd_orig);
								bytes_sent = send(sock_fd, cmd_orig, cmd_len, 0);
								if (bytes_sent < cmd_len)
								{
									bytes_sent = bytes_sent + send(sock_fd, cmd_orig, cmd_len, 0);
									if (bytes_sent == -1)
									{
										perror("unable to send message");
									}
								}
							}
							else if (strcmp(new_cmnd, "BLOCK") == 0)
							{
								int flag_blocked = 0;
								cmd_len = strlen(cmd_orig);
								for (auto i : myblocklist)
								{
									cout << i << cmd << strcmp(i.c_str(), cmd) << endl;
									if (strcmp(i.c_str(), cmd) == 0)
									{
										cse4589_print_and_log("[%s:ERROR]\n", new_cmnd);
										cse4589_print_and_log("[%s:END]\n", new_cmnd);
										flag_blocked = 1;
										break;
									}
								}
								if (ip_validation(cmd) == 0 || checkIP(cmd, &list_clients) == false)
								{
									cse4589_print_and_log("[%s:ERROR]\n", new_cmnd);
									cse4589_print_and_log("[%s:END]\n", new_cmnd);
									break;
								}
								if (flag_blocked == 0)
								{
									int bytes_sent = 0;
									if (bytes_sent < cmd_len)
									{
										bytes_sent += send(sock_fd, cmd_orig, cmd_len, 0);
										if (bytes_sent == -1)
										{
											perror("unable to send message");
										}
									}
									cse4589_print_and_log("[%s:SUCCESS]\n", new_cmnd);
									cse4589_print_and_log("[%s:END]\n", new_cmnd);
								}
								myblocklist.push_back(cmd);
							}
							else if (strcmp(new_cmnd, "UNBLOCK") == 0)
							{
								int bytes_sent = 0;
								cmd_len = strlen(cmd_orig);
								if (ip_validation(cmd) == false || checkIP(cmd, &list_clients) == false)
								{
									cse4589_print_and_log("[%s:ERROR]\n", new_cmnd);
									cse4589_print_and_log("[%s:END]\n", new_cmnd);
									break;
								}
								if (bytes_sent < cmd_len)
								{
									bytes_sent += send(sock_fd, cmd_orig, cmd_len, 0);
									if (bytes_sent == -1)
									{
										perror("unable to send message");
									}
								}
								cse4589_print_and_log("[%s:SUCCESS]\n", new_cmnd);
								cse4589_print_and_log("[%s:END]\n", new_cmnd);
							}
							else if (strcmp(new_cmnd, "BLOCKED") == 0)
							{
								int bytes_sent = 0;
								cmd_len = strlen(cmd_orig);
								if (bytes_sent < cmd_len)
								{
									bytes_sent += send(sock_fd, cmd_orig, cmd_len, 0);
									if (bytes_sent == -1)
									{
										perror("unable to send message");
									}
								}
							}
						}
						else if (sock_index == server_socket)
						{
							int accept_fd = accepting_client(server_socket);
							if (accept_fd < 0)
								perror("Accept failure.");
							printf("\nRemote Host connected!\n");
							FD_SET(accept_fd, &master_list);
							if (accept_fd > head_socket)
								head_socket = accept_fd;
							char *buffer = (char *)malloc(SIZE_BUFFER);
						}
						else
						{
							char *buffer = (char *)malloc(sizeof(char) * SIZE_BUFFER * 2);
							memset(buffer, '\0', SIZE_BUFFER * 2);
							char *temp_buf = (char *)malloc(sizeof(char) * SIZE_BUFFER * 2);
							char *message = (char *)malloc(sizeof(char) * SIZE_BUFFER * 2);
							memset(message, '\0', SIZE_BUFFER * 2);
							int buffer_len = strlen(buffer);
							int bytes_recvd = recv(sock_index, buffer, SIZE_BUFFER * 2, 0);
							char srcip[INET_ADDRSTRLEN];
							strcpy(temp_buf, buffer);
							for (int i = 0; i < strlen(buffer); i++)
							{
								if (buffer[i] == ' ')
								{
									message = buffer + i + 1;
									break;
								}
								else
									srcip[i] = buffer[i];
							}
							cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
							cse4589_print_and_log("msg from:%s\n[msg]:%s\n", srcip, message);
							cse4589_print_and_log("[%s:END]\n", "RECEIVED");
						}
					}
				}
			}
		}
	}
	return 0;
}

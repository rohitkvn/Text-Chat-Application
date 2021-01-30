#ifndef CLIENT_H_
#define CLIENT_H_

int connect_to_host(char *server_ip, char* server_port);
void Author();
void parsecommand(char * cmd, char * command);
void refresh_list_client(std::vector<struct ClientList> *clientlist, char * list);
bool verifyIP(char * IPAddress, std::vector<struct ClientList> *clientlist);
int login_client(char * cmd,char * myportno, std::vector<struct ClientList> *clientlist,fd_set *master_list);
int client_accept(int server_socket);
void split_string(std::string const &str, const char delim,std::vector<std::string> &out);
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <fstream>
using namespace std;
#include <fcntl.h>
#include <sys/wait.h>
#define PORT 55666


void download_file(int sockfd, const char *path){

	cout<<"Path is "<<path<<endl;
	FILE *to_save = fopen(path,"a"); char buffer[512];
	if(to_save==NULL) perror("Cannot create file");

	else{
		bzero(buffer,sizeof(buffer));
		int block_size=0;
		while((block_size = recv(sockfd, buffer, sizeof(buffer), 0)) > 0){
			int bytes_written = fwrite(buffer,sizeof(char),block_size,to_save);
			if(bytes_written<block_size) cout<<"Error in write";
			bzero(buffer,sizeof(buffer));
			if(block_size==0||block_size!=512) break;
			if(block_size<0){cout<<"Error";}
		}
		cout<<"File recieved\n";
		fclose(to_save);
	}

}

void upload_file(int sockfd, const char *path){

	cout<<"Path is "<<path<<endl;
	FILE *to_send= fopen(path,"r"); char buffer[512];

	if(to_send==NULL) perror("Cannot open file");

	else{
		bzero(buffer,sizeof(buffer));
		int block_size=0;

		while((block_size = fread(buffer, sizeof(char), sizeof(buffer),to_send)) > 0){
			
			if(send(sockfd,buffer,block_size,0) < 0) cout<<"Failed to send file";
			bzero(buffer,sizeof(buffer));
		}
		cout<<"File sent\n";
		fclose(to_send);
		close(sockfd);
	}
}

int main(int argc, char *argv[]){
	char basepath[100]="p2pfiles1/";
	int option; char msg[100]; char buffer[256];
	struct sockaddr_in server_addr, client_addr;
    struct hostent *server;
	socklen_t client_length, pid,i;
	
	//publish 1 fetch 2 listening 3

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) cout<<"Error Opening Socket"<<endl;
	else cout<<"socket for connecting to server created"<<endl;

	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable)) < 0)
    	cout<<"setsockopt reuse addr failed"<<endl;

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&enable, sizeof(enable)) < 0) 
        cout<<"setsockopt reuse port failed"<<endl;

	server = gethostbyname(argv[1]);
    if (server == NULL) cout<<"Error no such host"<<endl;

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&server_addr.sin_addr.s_addr,server->h_length);
    server_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sockfd,(const sockaddr*)&server_addr,sizeof(server_addr)) < 0) perror("Error Connecting");
	else cout<<"Connected to server"<<endl;
	
	int process_id = fork();
	while(1){
		if(process_id){
		cout<<"Enter option 1 for publish and 2 for fetch: ";
		cin>>option;
		
		if(option==1){
			cout<<"Enter filename for publish: ";
			cin>>msg; 

			char new_msg[100] = "publish$"; strcat(new_msg,msg);
			strcat(new_msg,"$"); strcat(new_msg,argv[3]);
			strcat(new_msg,"$");
			cout<<"final msg "<<new_msg<<"\n";
			int n = send(sockfd,new_msg,sizeof(new_msg),0);
    		if (n < 0) cout<<"Error in write"<<endl;
			
			n = recv(sockfd,buffer,255,0);
    		if (n < 0) cout<<"Error in read"<<endl;
			cout<<buffer<<endl;
		}

		else if(option==2){
			cout<<"Enter filename for searching: ";
			cin>>msg; 
			char fmsg[100]="";
			strcat(fmsg,"fetch$"); strcat(fmsg,msg);
			strcat(fmsg,"$"); strcat(fmsg,argv[3]);
			strcat(fmsg,"$");
			cout<<"final msg "<<fmsg<<"\n";
			int n = send(sockfd,fmsg,sizeof(fmsg),0);
    		if (n < 0) cout<<"Error in write"<<endl;
			
    		char port[8],ip[40];
    		int val=0,k=0;
    		char buffer[256];
			n = recv(sockfd,buffer,255,0);
    		
			if (n < 0) cout<<"Error in recv"<<endl;
    		cout<<"The message recvd is:"<<buffer<<"\n";
    		while(buffer[k]!='$')
				ip[val++]=buffer[k++];
			ip[val]='\0';
			val=0; 
			k++;
			
			while(buffer[k]!='$')
				port[val++]=buffer[k++];
			port[val]='\0';

			
			cout<<ip<<"\n"<<port<<"\n";
			if(strcmp(ip,"no_such_file")!=0)
				cout<<"No provision for file exists\n";
			int client_socket = socket(AF_INET, SOCK_STREAM, 0); 
    		if (client_socket < 0)	cout<<"Could not create client socket"<<endl;

			struct hostent *server;			
			server = gethostbyname(ip);
			if(server==NULL)
				break;

			struct sockaddr_in server_socket_addr;
			bzero((char *) &server_socket_addr, sizeof(server_socket_addr));
    		server_socket_addr.sin_family = AF_INET;
    		bcopy((char *)server->h_addr,(char *)&server_socket_addr.sin_addr.s_addr,server->h_length);
    		server_socket_addr.sin_port = htons(atoi(port)); //set port
			
			cout<<"Trying to connect to host at ip,port"<<ip<<" "<<port<<"\n";
			if (connect(client_socket,(const sockaddr*)&server_socket_addr,sizeof(server_socket_addr)) < 0) perror("Error connecting");
			
			cout<<"Msg:"<<msg<<"";
			n = send(client_socket,msg,sizeof(msg),0);
			
			download_file(client_socket,msg);
    		if (n < 0) cout<<"Error Writing To Socket"<<endl;
			}
		}


		else {
            close(sockfd); 
			
			struct sockaddr_in client_as_server, client_addr;
			int server_socket = socket(AF_INET, SOCK_STREAM, 0); 
    		if (server_socket < 0)	cout<<"Could not create server socket"<<endl;

			int enable = 1;
			if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable)) < 0)
    			cout<<"setsockopt reuse addr failed"<<endl;

			if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&enable, sizeof(enable)) < 0) 
        		cout<<"setsockopt reuse port failed"<<endl;
	
			bzero((char *) &client_as_server, sizeof(client_as_server));
			int portno = atoi(argv[3]);
			
			client_as_server.sin_family = AF_INET;
    		client_as_server.sin_port = htons(portno);
			client_as_server.sin_addr.s_addr = INADDR_ANY; //do not give ANY

			if (bind(server_socket, (struct sockaddr *) &client_as_server, sizeof(client_as_server)) < 0) 
				perror("Bind as server failed");
    		
			listen(server_socket, 5);
		
			socklen_t length = sizeof(&client_addr);
			int newsockfd = accept(server_socket,(struct sockaddr *) &client_addr, &length);
			if (newsockfd < 0) perror("ERROR on accept");
			int pid = fork();
			if (pid < 0) perror("ERROR on fork");

			if (pid == 0) {
				cout<<"Read event occured\n";
				char msg_buffer[256];
				int n = recv(newsockfd,msg_buffer,sizeof(msg_buffer),0);
				if (n < 0) perror("I am here: Error in read");
				if(n==0) close(newsockfd);
				cout<<"came from peer: "<<msg_buffer<<"\n";
				
				strcat(basepath,msg_buffer);
				upload_file(newsockfd,msg_buffer);
				exit(0);
			}

			else close(newsockfd);
			
			close(server_socket); 
			exit(0);
		}

	}
}

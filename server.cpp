#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include<iostream>
using namespace std;
#define MAXMSG  512
#define PORT 55667

int main(int argc, char *argv[])
{
    fd_set read_fds,active_fds; 
    int sockfd, newsockfd, portno; 
    socklen_t client_length, num_of_chars_rw, pid,i;
	struct sockaddr_in client_addr, server_addr;

	char buffer[256];
	if (argc < 2)	cout<<"No port provided"<<endl;
    
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) cout<<"Error Opening Socket";
	
	bzero((char *) &server_addr, sizeof(server_addr));
	portno = atoi(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
        cout<<"Error binding";
	listen(sockfd, 5);
	
	FD_ZERO (&active_fds); FD_SET(sockfd, &active_fds);

	while(1) 
	{

		read_fds = active_fds;
      	if (select (FD_SETSIZE, &read_fds, NULL, NULL, NULL) < 0) cout<<"Error in select function";

        for (i = 0; i <=FD_SETSIZE; ++i)
			{
			if (FD_ISSET (i, &read_fds))
				{
                    if(i==sockfd)
                    {	
                        //new connection
                        client_length=sizeof(client_addr);
                        int newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_length);
                        if (newsockfd < 0) cout<<"Error on accept";
                        cout<<"Connection accepted\n";

                        char str[1024];
                        inet_ntop(AF_INET, &(client_addr.sin_addr), str, INET_ADDRSTRLEN);
                        cout<<str;
                        FD_SET (newsockfd, &active_fds);
                    }

                    else 
                    {
                        char str[1024]; char file[1024]; int port_num;
                        inet_ntop(AF_INET, &(client_addr.sin_addr), str, INET_ADDRSTRLEN);
                        
                        int n = recv(i,buffer,sizeof(buffer),0); 
                        if(n<0) cout<<"Error reading\n";
                        if(n==0) { close(i); FD_CLR(i, &active_fds); }
                        cout<<"msg is "<<buffer<<"\n";
                        
                        char type[8], filename[100], port[10]; 
                        bzero(&type,sizeof(type)); bzero(&filename,sizeof(filename)); bzero(&port,sizeof(port));
                        int val=0,k=0;

                        {
                            while(buffer[k]!='$')
                                type[val++]=buffer[k++];
                            type[val]='\0';
                            val=0; 
                            k++;
                            while(buffer[k]!='$')
                                filename[val++]=buffer[k++];
                            filename[val]='\0';
                            val=0;
                            k++;
                            while(buffer[k]!='$')
                                port[val++]=buffer[k++];
                            port[val]='\0';

                        }
                    
                        
                        cout<<"string to search "<<filename<<"\n";
                        if(strcmp(type,"publish")==0){
                        
                            ofstream myfile;
                            cout<<"file opened";
                            myfile.open(filename,ios::app);
                            myfile <<filename<<" "<<str<<" "<<port<<"\n";
                            myfile.close();
                            n = send(i,"saved",5,0);
                            if (n < 0) cout<<"Error writing to socket";
                        } 
                        else if(strcmp(type,"fetch")==0){
                            ifstream in(filename);
                            cout<<"file opened\n";
                            char f[50],ip[50],port[10]; char smsg[256]; smsg[0]='\0';
                            
                            int flag=0;
                            while (in >> f >> ip >> port)
                            {
                                if(strcmp(filename,f)==0){
                                    cout<<ip<<"\n"<<port<<"\n";
                                    flag=1;
                                    strcat(smsg,ip);strcat(smsg,"$");strcat(smsg,port);strcat(smsg,"$");	
                                }
                            }
                            
                            in.close();
                            if(flag==0) strcat(smsg,"no_such_file$sorry$");
                            cout<<"final output "<<smsg<<"\n";

                            int n = send(i,smsg,sizeof(smsg),0);
                            if (n < 0) cout<<"Error writing to socket\n"; 
                            
                            if(n==0){cout<<"closing connection\n"; close(i); FD_CLR(i,&active_fds);}
                        }

                    }
				}
			}				 

	}

	close(sockfd);


}

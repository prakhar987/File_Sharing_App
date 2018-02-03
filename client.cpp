//Akshat Tandon
//Prakhar Pandey
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>

#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>

#define MAXSTRINGLENGTH 100
#define PORT "3496" 
#define MAXDATASIZE 10000

using namespace std;

/* Buffer to store the parsed command */
struct CommandBuffer{
	char name[MAXSTRINGLENGTH];
	char flag[MAXSTRINGLENGTH];
	char arg1[MAXSTRINGLENGTH];
	char arg2[MAXSTRINGLENGTH];	
	int cmdnum;
	int flagnum;
};
typedef struct CommandBuffer CommandBuffer;

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void print_command_buffer(CommandBuffer *buf){
	printf("Command: %s\n",buf->name);
	printf("Flag: %s\n",buf->flag);
	printf("ARG1: %s\n",buf->arg1);
	printf("ARG2: %s\n",buf->arg2);
}

int prepare_message(char message[], CommandBuffer *buf){
	strcpy(message,"");
	sprintf(message,"X%d%dX%sX%s",buf->cmdnum, buf->flagnum, buf->arg1, buf->arg2);

}

int parser(char cmdline[], CommandBuffer *buf){
	char *p;
	p = strtok(cmdline," \t\r\f\n\v\b");
	//printf("** %s\n",p);
	if(p == NULL){
		printf("Invalid Entry\n");
		return -1;
	}
	strcpy(buf->name, p);
	if(strcmp(buf->name, "IndexGet") == 0){
		buf->cmdnum = 0;
		p = strtok(NULL," \t\r\f\n\v\b");
		if(p == NULL){
			printf("Enter flag\n");
			return -1;
		}
		strcpy(buf->flag, p);
		if(strcmp(buf->flag, "shortlist") == 0){
			buf->flagnum = 0;
			p = strtok(NULL," \t\r\f\n\v\b");
			if(p == NULL){
				printf("Enter arguments\n");
				return -1;
			}
			strcpy(buf->arg1, p);
			p = strtok(NULL," \t\r\f\n\v\b");
			if(p==NULL){
				printf("Enter second argument\n");
				return -1;
			}
			strcpy(buf->arg2, p);
		}
		else if(strcmp(buf->flag, "longlist") == 0){
			buf->flagnum = 1;
			p = strtok(NULL," \t\r\f\n\v\b");
			if(p != NULL){
				printf("Requires no arguments\n");
				return -1;
			}
		}
		else if(strcmp(buf->flag, "regex") == 0){
			buf->flagnum = 2;
			p = strtok(NULL," \t\r\f\n\v\b");
			if(p == NULL){
				printf("Enter arguments\n");
				return -1;
			}
			strcpy(buf->arg1, p);
		}

		else{
			printf("No such flag\n");
			return -1;
		}
		p = strtok(NULL," \t\r\f\n\v\b");
		if(p != NULL){
			printf("Extra arguments\n");
			return -1;
		}
	}
	else if(strcmp(buf->name, "FileHash") == 0){
		buf->cmdnum = 1;
		p = strtok(NULL," \t\r\f\n\v\b");
		if(p == NULL){
			printf("Flag missing\n");
			return -1;
		}
		strcpy(buf->flag, p);
		printf("flag == %s\n",p);
		if(strcmp(p,"verify") == 0){
			buf->flagnum = 0;
			p = strtok(NULL," \t\r\f\n\v\b");
			if(p == NULL){
				printf("Enter file name\n");
				return -1;
			}
			strcpy(buf->arg1, p);
		}
		else if(strcmp(p,"checkall") == 0){
			buf->flagnum = 1;
			p = strtok(NULL," \t\r\f\n\v\b");
			if( p != NULL){
				printf("No arguments required\n");
				return -1;
			}
		}
		else{
			printf("No such flag\n");
			return -1;
		}
		p = strtok(NULL," \t\r\f\n\v\b");
		if(p != NULL){
			printf("Extra arguments\n");
			return -1;
		}

	}
	else if(strcmp(buf->name, "FileDownload") == 0){
		buf->cmdnum = 2;
		p = strtok(NULL," \t\r\f\n\v\b");
		if(p == NULL){
			printf("Enter flag\n");
			return -1;
		}
		if(!(strcmp(p,"TCP") == 0 || strcmp(p,"UDP") == 0)){
			printf("Enter correct flag");
			return -1;
		}
		strcpy(buf->flag, p);
		if(strcmp(buf->flag,"TCP")){
			buf->flagnum = 0;
		}
		else{
			buf->flagnum = 1;
		}
		p = strtok(NULL," \t\r\f\n\v\b");
		if(p == NULL){
			printf("Enter file name\n");
			return -1;
		}
		strcpy(buf->arg1, p);
		p = strtok(NULL," \t\r\f\n\v\b");
		if(p != NULL){
			printf("Extra arguments\n");
			return -1;
		}

	}
	else{
		printf("No such command\n");
		return -1;
	}
	return 0;

}

void initiaize_command_buffer(CommandBuffer *buf){
	strcpy(buf->name,"");
	strcpy(buf->flag,"");
	strcpy(buf->arg1,"");
	strcpy(buf->arg2,"");
	buf->cmdnum = -1;
	buf->flagnum = -1;
}

int main(int argc, char *argv[]){
	int sockfd, numbytes, counter = 0;  
	char response_buf[MAXDATASIZE], message[MAXSTRINGLENGTH];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	FILE *fd ;
	char s[INET6_ADDRSTRLEN];
	printf("\n");
	CommandBuffer buf;
	char str[MAXSTRINGLENGTH];
	bool init_fmap = false;
	string a,b;

	std::map <string, string> filemap;
	std::map <string,bool> filecount;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	initiaize_command_buffer(&buf);
	buf.cmdnum = 1;
	buf.flagnum = 1;
	prepare_message(message, &buf);
	if(send(sockfd, message, strlen(message) + 10, 0) == -1){
		perror("send");
	}
	if ((numbytes = recv(sockfd, response_buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	}
	response_buf[numbytes] = '\0';
	FILE *fpq = fopen(".hashfile.txt", "w");

	int results = fputs(response_buf, fpq);
	if (results == EOF) {
	    // Failed to write do error code here.
	    printf("Failed to save the file.\n");
	}
	else{
		//printf("File downloaded successfully.\n");
	}
	fclose(fpq);
	std::ifstream infile(".hashfile.txt");
	std::string line;
	while (std::getline(infile, line))
	{
	    std::istringstream iss(line);
	   
	    iss >> a >> b;
	    filemap[b] = a;
	    filecount[b] = false;
	    // process pair (a,b)
	}
	std::map<string, string>::iterator it;
	std::map<string, bool>::iterator bit;
	//for (it=filemap.begin(); it!=filemap.end(); ++it)
    //	std::cout << it->first << " => " << it->second << '\n';


	while(1){
		strcpy(str,"");
		initiaize_command_buffer(&buf);
		//printf("D:> ");
		if(init_fmap == false){

		}
		if(fgets(str, MAXSTRINGLENGTH, stdin) == NULL){
			//printf("\n");
			printf("Error in accepting line: fgets\n");			
		}
		//printf("sending command: #%s#\n",str);
		if(parser(str, &buf) == 0){
			//printf("Correct command\n");
			//print_command_buffer(&buf);
		//-----------------
			// loop through all the results and connect to the first we can
			for(p = servinfo; p != NULL; p = p->ai_next) {
				if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
					perror("client: socket");
					continue;
				}

				if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
					perror("client: connect");
					close(sockfd);
					continue;
				}

				break;
			}

			if (p == NULL) {
				fprintf(stderr, "client: failed to connect\n");
				return 2;
			}

			inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
					s, sizeof s);
			printf("client: connecting to %s\n", s);

			//freeaddrinfo(servinfo); // all done with this structure

			//-----------
			printf("Goign to prepare message\n");
			prepare_message(message, &buf);
			printf("Message is: %s\n",message);
			if(send(sockfd, message, strlen(message) + 10, 0) == -1){
				perror("send");
			}
			printf("Message sent\n");
			int idx;
			for(idx = 0; idx < MAXDATASIZE-1; idx++){
				response_buf[idx] = ' ';
			}
			switch(buf.cmdnum){
				
				case 0:	//IndexGet flag
				if(buf.flagnum == 1 || buf.flagnum == 2 || buf.flagnum == 0){
					printf("Waiting to receive\n");
					if ((numbytes = recv(sockfd, response_buf, MAXDATASIZE-1, 0)) == -1) {
					    perror("recv");
					    break;
					    //exit(1);
					}
					response_buf[numbytes] = '\0';
					printf("**List of files**\n");
					printf("%s",response_buf);
				}
				else if(buf.flagnum == 1){
					// to be implemented
				}
				
				break;

				case 1:
				if(buf.flagnum == 0 || buf.flagnum == 1){
					printf("Waiting to receive\n");
					if ((numbytes = recv(sockfd, response_buf, MAXDATASIZE-1, 0)) == -1) {
					    perror("recv");
					    break;
					    //exit(1);
					}
					response_buf[numbytes] = '\0';
					printf("%s",response_buf);

				}
				break;

				case 2:
				if(buf.flagnum == 0 || buf.flagnum == 1){
					printf("Waiting to receive\n");
					if ((numbytes = recv(sockfd, response_buf, MAXDATASIZE-1, 0)) == -1) {
					    perror("recv");
					    break;
					    //exit(1);  
					}
					response_buf[numbytes] = '\0';
					if(strcmp(response_buf,"No such file.") != 0){
						FILE *fp = fopen(buf.arg1, "w");

						int results = fputs(response_buf, fp);
						if (results == EOF) {
						    // Failed to write do error code here.
						    printf("Failed to save the file.\n");
						}
						else{
							printf("File downloaded successfully.\n");
							//printf("Name: %s\n",buf.arg1);
							printf("Size: %d bytes\n",numbytes);
							//printf("Date: ");
							char temp_str[100];
							sprintf(temp_str,"date -r %s",buf.arg1);
							system(temp_str);
							//printf("MD5hash: ");
							sprintf(temp_str,"md5sum %s",buf.arg1);
							system(temp_str);
						}
						fclose(fp);


					}
					else{
						printf("%s\n",response_buf);
					}					
				}
				break;

				default:
				printf("switch not possible\n");
				break;

			}

		}
		else {
			//printf("Incorrect command\n");
		}
		counter++;
		if(counter == 2){
			counter = 0;
			for(p = servinfo; p != NULL; p = p->ai_next) {
				if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
					perror("client: socket");
					continue;
				}

				if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
					perror("client: connect");
					close(sockfd);
					continue;
				}

				break;
			}

			if (p == NULL) {
				fprintf(stderr, "client: failed to connect\n");
				return 2;
			}

			inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
					s, sizeof s);
			printf("client: connecting to %s\n", s);

			initiaize_command_buffer(&buf);
			buf.cmdnum = 1;
			buf.flagnum = 1;
			prepare_message(message, &buf);
			if(send(sockfd, message, strlen(message) + 10, 0) == -1){
				perror("send");
			}
			if ((numbytes = recv(sockfd, response_buf, MAXDATASIZE-1, 0)) == -1) {
			    perror("recv");
			}
			response_buf[numbytes] = '\0';
			fpq = fopen(".hashfile.txt", "w");

			int result = fputs(response_buf, fpq);
			if (result == EOF) {
			    // Failed to write do error code here.
			    printf("Failed to save the file.\n");
			}
			else{
				//printf("File downloaded successfully.\n");
			}
			fclose(fpq);
			std::ifstream inf(".hashfile.txt");
			std::string ln;
			for (bit=filecount.begin(); bit!=filecount.end(); ++bit)
    			bit->second = false;
			while (std::getline(inf, ln))
			{
			    std::istringstream iss(ln);
			   
			    iss >> a >> b;
			    //cout<<"#a "<<a<<"#b "<<b<<endl;
			    if(filemap.count(b) <= 0){
			    	cout<<"New file added: "<<b<<endl;
			    	filemap[b] = a;
			    	filecount[b] = true;
			    	
			    }
			    else if(filemap.count(b) >0){
			    	filecount[b] = true;
			    	it = filemap.find(b);
			    	if( (it->second) != a){
			    		cout<<"File modified: "<<b<<endl;
			    		filemap[b] = a;
			    		
			    	}
			    	else{
			    		//cout<<"Not modified: "<<b<<endl;
			    	}

			    }
			    
			}
			for (bit=filecount.begin(); bit!=filecount.end(); ++bit)
    		{
    			if(bit->second == false){
    				cout<<"File deleted: "<<bit->first<<endl;
    				filecount.erase(bit);
    			} 

    		}

			

		}
		

	}
}
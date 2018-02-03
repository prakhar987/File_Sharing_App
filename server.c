#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>


#define _XOPEN_SOURCE
#define PORT "3495"  // the port users will be connecting to
#define MAXSTRINGLENGTH 100
#define BACKLOG 10	 // how many pending connections queue will hold

struct CommandBuffer{
	char arg1[MAXSTRINGLENGTH];
	char arg2[MAXSTRINGLENGTH];	
	int cmdnum;
	int flagnum;
};
typedef struct CommandBuffer CommandBuffer;

void initiaize_command_buffer(CommandBuffer *buf){
	strcpy(buf->arg1,"");
	strcpy(buf->arg2,"");
	buf->cmdnum = -1;
	buf->flagnum = -1;
}

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int parse_message(char message[], CommandBuffer *buf){
	printf("Entered parse message: #%s#\n",message);
	char *p;
	//buf->cmdnum = (message[1] - '0');
	printf("came 1");
	buf->flagnum = (message[2] - '0');
	printf("%d--%d",buf->cmdnum,buf->flagnum);
	p = strtok(message+4,"X");
	strcpy(buf->arg1, p);
	p = strtok(NULL,"X");
	strcpy(buf->arg2, p);
}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN], message[MAXSTRINGLENGTH];
	char temp_str[MAXSTRINGLENGTH];
	int rv,rc;
    
 	char filename[50];   /* filename to send */
	int fd;
	off_t offset = 0;  
	struct stat stat_buf;

	time_t start_time , end_time;
	int enter = 1;
	struct tm tm; 

	FILE *fp;
	long lSize;
	char *fbuf;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		printf("**");
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
		get_in_addr((struct sockaddr *)&their_addr),
		s, sizeof s);
		printf("server: got connection from %s\n", s);
        

		if (!fork()) { // this is the child proces
			printf("forked\n");
    		/* get the file name from the client */
    		//CommandBuffer cbuf;
    		rc = recv(new_fd, message, sizeof(message) + 10, 0);
    		printf("Message received: #%s#\n",message);
    		if (rc == -1) {
      		fprintf(stderr, "recv failed: %s\n", strerror(errno));
      		exit(1);
    		}
    		//initiaize_command_buffer(&cbuf);
    		//parse_message(message, &cbuf);
    		int cmdnum = message[1] - '0';
    		int flagnum = message[2] - '0';
    		char arg1[MAXSTRINGLENGTH], arg2[MAXSTRINGLENGTH];
    		strcpy(arg1, "");
    		strcpy(arg2, "");
    		char *pch;
    		pch = strtok(message + 4, "X");
    		if(pch != NULL){
    			strcpy(arg1, pch);
    			pch = strtok(NULL, "X");
    			if(pch != NULL){
    				strcpy(arg2, pch);
    			}
    		}
    		printf("cmnum-%d flagnum-%d\n",cmdnum,flagnum);
    		printf("arg1 = %s\n", arg1);
    		printf("arg2 = %s\n",arg2);
    		printf("message parsed\n");
    		switch(cmdnum){
    			case 0:
    			if(flagnum == 0){
    				printf("Entered shortlist\n");
    				//pass THIS IS WHERE WE PUT shortlist
    				// !@#!$##@!$#$@%#@%#@%#@%#@@#%#$%$#%$%$%$@#$%$#%$#%#%$%@#
	    			char *p,*q;
		            //p = strtok(message,"X");
		            //p = strtok(NULL,"X"); // p is pointer to first time
		            //q =  strtok(NULL,"X"); // q to second
	             
					sprintf(temp_str,"find -newerct %s ! -newerct %s -ls  > .ak.txt",arg1,arg2 );
					system(temp_str);
					strcpy(filename,".ak.txt");

    			}
    			else if(flagnum == 1){
    				printf("Entered\n");
    				strcpy(temp_str, "ls -l > .ak.txt" );
    				system(temp_str);
    				strcpy(filename,".ak.txt");				    
    			}
    			
    			

    			else if(flagnum == 2){
    				char *p;
                  //p = strtok(message,"X");
                  //p = strtok(NULL,"X");
                  sprintf(filename," stat %s > .ak.txt ",arg1);
    			  system(filename);
 
    			  strcpy(filename,".ak.txt");
					
    			}
    			break;

    			case 1:
    			printf("Entered FileHash\n");
    			if(flagnum == 0){
    				printf("Entered verify\n");
    				if( access( arg1, F_OK ) != -1 ) {
    					printf("File exists\n");
    					sprintf(temp_str,"md5sum %s > .ak.txt",arg1);
    					system(temp_str);
    					sprintf(temp_str,"date -r %s >> .ak.txt",arg1);
    					system(temp_str);
    					printf("system done\n");

						fp = fopen ( ".ak.txt" , "rb" );
						if( !fp ) perror(".ak.txt not opened"),exit(1);

						fseek( fp , 0L , SEEK_END);
						lSize = ftell( fp );
						rewind( fp );

						/* allocate memory for entire content */
						fbuf = calloc( 1, lSize+1 );
						if( !fbuf ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

						/* copy the file into the fbuf */
						if( 1!=fread( fbuf , lSize, 1 , fp) )
						  fclose(fp),free(fbuf),fputs("entire read fails",stderr),exit(1);

						printf("Output-> %s\n",fbuf);
						char *pq = NULL;
						pq = strchr(fbuf,'\n');
						if(pq != NULL){
							printf("Entered\n");
							*pq = ' ';
							printf("Output-> %s\n",fbuf);
						}
						else{
							printf("end line character not found\n");
						}
						fclose(fp);
						fp = fopen(".ak.txt", "w");

						int results = fputs(fbuf, fp);
						if (results == EOF) {
						    // Failed to write do error code here.
						    printf("Failed to write to .ak.txt\n");
						}
						else{
							printf("Write to .ak.txt successful\n");
						}
						fclose(fp);
											
						free(fbuf);
					    // file exists
					} else {
					    // file doesn't exist
					    sprintf(temp_str,"File does not exist %s\n",arg1);
					    FILE *filep = fopen(".ak.txt", "w");

						int results = fputs(temp_str, filep);
						if (results == EOF) {
						    // Failed to write do error code here.
						    printf("Failed to write to .ak.txt\n");
						}
						else{
							printf("Write to .ak.txt successful\n");
						}
						fclose(filep);
					}
					strcpy(filename,".ak.txt");
    			}
    			else if(flagnum == 1){
    				printf("Entered checkall\n");
    				//sprintf(temp_str,"md5sum %s > .ak.txt",arg1);
    				system("ls > .ak.txt");
    				//*******
    				fp = fopen ( ".ak.txt" , "rb" );
					if( !fp ) perror(".ak.txt not opened"),exit(1);

					fseek( fp , 0L , SEEK_END);
					lSize = ftell( fp );
					rewind( fp );

					/* allocate memory for entire content */
					fbuf = calloc( 1, lSize+1 );
					if( !fbuf ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

					/* copy the file into the fbuf */
					if( 1!=fread( fbuf , lSize, 1 , fp) )
					  fclose(fp),free(fbuf),fputs("entire read fails",stderr),exit(1);

					fclose(fp);
					//********
					printf("----** String is #%s#\n",fbuf);
					pch = strtok (fbuf, " \t\r\f\n\v\b");
					FILE *fpa = fopen(".ak.txt", "w");
					fclose(fpa);
					while (pch != NULL)
					{
						printf("Filename #%s#\n",pch);
   						sprintf(temp_str,"md5sum %s > .bk.txt",pch);
   						system(temp_str);
   						sprintf(temp_str,"date -r %s >> .bk.txt",pch);
   						system(temp_str);
						//*******
	    				fp = fopen ( ".bk.txt" , "rb" );
						if( !fp ) perror(".bk.txt not opened"),exit(1);

						fseek( fp , 0L , SEEK_END);
						lSize = ftell( fp );
						rewind( fp );

						/* allocate memory for entire content */
						char *cbuf = calloc( 1, lSize+1 );
						if( !cbuf ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

						/* copy the file into the cbuf */
						if( 1!=fread( cbuf , lSize, 1 , fp) )
						  fclose(fp),free(cbuf),fputs("entire read fails",stderr),exit(1);

						char *pq = NULL;
						pq = strchr(cbuf,'\n');
						if(pq != NULL){
							printf("Entered\n");
							*pq = ' ';
							printf("Output-> %s\n",cbuf);
						}
						else{
							printf("end line character not found(inside)\n");
						}
						fclose(fp);

						//******
						fpa = fopen(".ak.txt", "a");

						int results = fputs(cbuf, fpa);
						if (results == EOF) {
						    // Failed to write do error code here.
						    printf("Failed to write to .ak.txt\n");
						}
						else{
							printf("Write to .ak.txt successful\n");
						}
						fclose(fpa);
						//********
						free(cbuf);
					  	pch = strtok (NULL, " \t\r\f\n\v\b");
					}
					free(fbuf);
					strcpy(filename,".ak.txt");
    			}
    			break;

    			case 2:
    			printf("Entered FileDownload\n");
    			if( access( arg1, F_OK ) != -1 ){
    				printf("File exists\n");
    				strcpy(filename,arg1);
    			}
    			else{
    				strcpy(temp_str,"No such file.");
				    FILE *filep = fopen(".ak.txt", "w");

					int results = fputs(temp_str, filep);
					if (results == EOF) {
					    // Failed to write do error code here.
					    printf("Failed to write to .ak.txt\n");
					}
					else{
						printf("Write to .ak.txt successful\n");
					}
					fclose(filep);
					strcpy(filename,".ak.txt");

    			}
    			break;

    			case 3:
    			break;

    			default:
    			printf("switch error\n");
    			break;
    		}
    		fprintf(stderr, "received request to send file %s\n", filename);
		    /* open the file to be sent */
		    fd = open(filename, O_RDONLY);
		    if (fd == -1) {
		      fprintf(stderr, "unable to open '%s': %s\n", filename, strerror(errno));
		      exit(1);
		    }

		    /* get the size of the file to be sent */
		    fstat(fd, &stat_buf);

		    /* copy file using sendfile */
		    offset = 0;
		    rc = sendfile (new_fd, fd, &offset, stat_buf.st_size);
		    if (rc == -1) {
		      fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
		      exit(1);
		    }
		    if (rc != stat_buf.st_size) {
		      fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n",
		              rc,
		              (int)stat_buf.st_size);
		      exit(1);
		    }
		    
			//exit(0);
			printf("Exited\n");
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}

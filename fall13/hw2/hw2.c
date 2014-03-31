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
#include <pthread.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

#define BACKLOG 10     // how many pending connections queue will hold

void sigchld_handler(int s)
{
  while(waitpid(-1, NULL, WNOHANG) > 0);
}
void* handle_client_thread(void *s); 
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char* dir_path;

int main(int argc, char** argv)
{
  int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  char* port = argv[1];
  dir_path = argv[2];

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP
  if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
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
  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    return 2;
  }
  freeaddrinfo(servinfo); // all done with this structure
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
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("Error Accepting Connection");
      exit(1);
    }
    //CREATE A NEW THREAD HERE
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    printf("server: got connection from %s\n", s);

    pthread_t client_thread;
    pthread_create(&client_thread,0,handle_client_thread,(void*)new_fd);    
    //close(new_fd);  // parent doesn't need this
  }
  shutdown(new_fd, SHUT_RDWR);
  return 0;
}

void *handle_client_thread(void *arg) {
  int a = (int)arg;
  char get_req[1024*1024];
  char resp_buf[1024*1024];
  char header[1024*1024];
  char req_dir[100];
  char req_protocol[20];
  char fullpath[100];
  char fullpath1[100];
  char* content_type;
  struct stat buf; 
  FILE *file;
  memset(get_req,0,sizeof(get_req));
  while(!strstr(get_req,"\r\n\r\n")) {
    int recv_count = recv(a, get_req, sizeof(get_req), 0);
  if(recv_count<0) { perror("Receive failed");exit(1); }
  }
  sscanf(get_req, "GET %s %s\n", req_dir, req_protocol);
  sprintf(fullpath, "%s%s", dir_path, req_dir);
  int temp;
  strcpy(fullpath1, fullpath);

  if(stat(fullpath, &buf) == -1){

    //not a dir not a file
    sprintf(header,"HTTP/1.0 404 File Not Found.\r\nContent-Type: text/html\r\n\r\n<html><body><h2>Error 404: The requested file does not exist.</h2></body></html>");
  }
  else{
    //Check if it's a dir
    if(buf.st_mode & S_IFDIR){
      //Then it is a dir
      
      //Add index.html to the end
      if(strlen(strrchr(fullpath, '/')) <=  strlen("/"))
        strcat(fullpath, "index.html");
      else
        strcat(fullpath, "/index.html");
    }

    if(stat(fullpath, &buf) == -1){
      //display HTML for dict

      DIR *d;
      struct dirent *dir;
      d = opendir(fullpath1);
      if(d){
        char listitem[1000];
        strcat(header, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h2>Directory Listing: </h2><br><br><ul>");
        while ((dir = readdir(d)) != NULL){
          sprintf(listitem,"<li><a href=%s>%s</a></li> ", dir->d_name, dir->d_name);
          strcat(header, listitem);
        }
        strcat(header, "</ul></body></html>");
        closedir(d);
        }
    }
    else{
      if(strcasestr(fullpath,".html")) 
	content_type = "text/html";
      else if(strcasestr(fullpath,".txt")) 
	content_type =  "text/plain";
      else if(strcasestr(fullpath,".jpg"))
        content_type =  "image/jpeg";
      else if(strcasestr(fullpath,".gif")) 
	content_type =  "image/gif";
      else if(strcasestr(fullpath,".png")) 
	content_type =  "image/png";
      else if(strcasestr(fullpath,".pdf"))
        content_type =  "application/pdf";
      else if(strcasestr(fullpath,".ico"))
	content_type =  "image/x-icon";
      else
	content_type = "application/octet-stream";
  
      sprintf(header,"HTTP/1.0 200 OK.\r\nContent-Type: %s\r\n\r\n", content_type);
    }   
  }   
  // Send header from above
  send(a,header,strlen(header),0);
  
  if(stat(fullpath, &buf) != -1){
    file = fopen(fullpath,"r");
    while((temp=fread(resp_buf,1,sizeof(resp_buf),file))) {
      send(a,resp_buf,temp,0);
    }
  } 
  //shutdown(a, SHUT_RDWR);
  //fclose(file);
  close(a);
  
}


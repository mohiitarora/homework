#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

int main(int argc, char** argv) 
{	
  struct addrinfo hints;
  struct addrinfo * result, * rp;
  int sock_fd, s;
  char* filename;

  hints.ai_socktype = SOCK_STREAM;
  memset(&hints,0,sizeof(struct addrinfo));
  //Clean up: separate out domain and path and find filename
  char* input_domain = argv[1];
  if(strlen(input_domain)>7){
    input_domain += 7; 
  }
  char* path = strchr(input_domain,'/');
  if(path == NULL){
    path = "/\0";
    filename = "index.html";
  }
  else{
    if(strlen(strrchr(path, '/')) <=  strlen("/")){
      filename = "index.html";
    }
    else{
      char* a  = strrchr(path, '/');
      char b[50];
      sscanf(a, "/%s", b);
      filename = &b;
    }
  }
  
  char domain[100];
  char* ptr = &domain;
  strcpy(ptr, input_domain);
  char* tptr = strchr(ptr,'/');
  if(tptr != NULL){
      *tptr = '\0';
    }
  //<--clean up complete- all data extracted!!! 

  //resolve ip
  s = getaddrinfo(domain,"80",&hints,&result);
  if (0 != s){
    perror("error populating address structure");
    exit(1);
  }
 
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sock_fd == -1)
      continue;

    if (connect(sock_fd, rp->ai_addr, rp->ai_addrlen) != -1)
      break; /* Success */

    close(sock_fd);
  }
  if (rp == NULL) {
    fprintf(stderr, "could not connect\n");
    exit(1);
  }
  freeaddrinfo(result);

  //send and receive -->
  //create format for request-->
  char get_req[500];
  sprintf(get_req, "GET %s HTTP/1.0 \r\n\r\n", path);

  //send-->
  send(sock_fd, get_req, strlen(get_req), 0);
  char buf[1024*1024];
  memset(&buf,0,sizeof(buf));


  //recv-->
  char code1[20];
  char code2[5];
  char code3[5];
  int recv_count = recv(sock_fd, buf, 1024*1024, 0);
  if(recv_count<0) { perror("Receive failed");	exit(1); }
  sscanf(buf, "%s %s %s\n", code1, code2, code3);
  char* t_content_type = strstr(buf, "Content-Type:");
  char content_type[20];
  sscanf(t_content_type, "Content-Type: %s;", content_type);
  //printf(buf);
  if(strcmp(code2,"200") != 0){
    perror("Invalid code other than 200!");
    exit(1);
  }

  FILE *file;
  file = fopen(filename,"wb");

  char* body = strstr(buf, "\r\n\r\n");
  body += 4;
  
  fwrite(body,1,recv_count - (body - buf),file);

  while(recv_count != 0){
    recv_count = recv(sock_fd, buf, 1024*1024, 0);
    if( buf != NULL)
      fwrite(buf, 1, recv_count,file);
  }
  fclose(file);
  shutdown(sock_fd,SHUT_RDWR);
}

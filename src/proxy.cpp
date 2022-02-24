#include "proxy.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

#include "Cache.h"
#include "Service.h"

struct client_info {
  int socket_fd;
  std::string IP;
  int ID;
  Cache * cache_p;
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t c_mutex = PTHREAD_MUTEX_INITIALIZER;
std::ofstream logFile("/var/log/erss/proxy.log");
//std::ofstream logFile("proxy.log");
bool Proxy::try_build_server(const char * PORT) {
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo * host_info_list;
  const char * hostname = NULL;
  const char * port = PORT;

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    pthread_mutex_lock(&mutex);
    logFile << "Error: cannot get address info for host" << std::endl;
    logFile << "  (" << hostname << "," << port << ")" << std::endl;
    pthread_mutex_unlock(&mutex);
    return false;
  }  //if

  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    pthread_mutex_lock(&mutex);
    logFile << "Error: cannot create socket" << std::endl;
    logFile << "  (" << hostname << "," << port << ")" << std::endl;
    pthread_mutex_unlock(&mutex);
    return false;
  }  //if

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    pthread_mutex_lock(&mutex);
    logFile << "Error: cannot bind socket" << std::endl;
    logFile << "  (" << hostname << "," << port << ")" << std::endl;
    pthread_mutex_unlock(&mutex);
    return false;
  }  //if

  status = listen(socket_fd, 100);
  if (status == -1) {
    pthread_mutex_lock(&mutex);
    logFile << "Error: cannot listen on socket" << std::endl;
    logFile << "  (" << hostname << "," << port << ")" << std::endl;
    pthread_mutex_unlock(&mutex);
    return false;
  }  //if

  this->my_sockfd = socket_fd;
  freeaddrinfo(host_info_list);
  return true;
}

int Proxy::try_accept(int sockfd, std::string & ip) {
  int new_fd;
  socklen_t sin_size;
  struct sockaddr_storage their_addr;
  sin_size = sizeof(their_addr);
  new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
  if (new_fd == -1) {
    pthread_mutex_lock(&mutex);
    logFile << "Error: cannot accept the connection" << std::endl;
    pthread_mutex_unlock(&mutex);
    return -1;
  }
  struct sockaddr_in * addr = (struct sockaddr_in *)&their_addr;
  ip = std::string(inet_ntoa(addr->sin_addr));
  return new_fd;
}

void * process(void * clientInfo) {
  int fd = ((struct client_info *)(clientInfo))->socket_fd;
  std::string user_ip = ((struct client_info *)(clientInfo))->IP;
  int user_id = ((struct client_info *)(clientInfo))->ID;
  Cache * cache_p = ((struct client_info *)(clientInfo))->cache_p;
  Service service(fd, &mutex, &logFile, user_ip, user_id, cache_p, &c_mutex);
  try {
    service.process();
  }
  catch (std::exception & e) {
    pthread_mutex_lock(&mutex);
    logFile << user_id << ": " << e.what() << std::endl;
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

int Proxy::run() {
  if (try_build_server("12345") == false) {
    return -1;
  }
  int thread_ID = 1;
  while (1) {
    int client_sockfd;
    std::string ip;
    client_sockfd = try_accept(this->my_sockfd, ip);
    if (client_sockfd == -1) {
      return -1;
    }
    struct client_info clientInfo;
    clientInfo.socket_fd = client_sockfd;
    clientInfo.IP = ip;
    clientInfo.ID = thread_ID;
    clientInfo.cache_p = cache;
    pthread_t thread;
    pthread_create(&thread, NULL, process, &clientInfo);
    thread_ID++;
  }
}

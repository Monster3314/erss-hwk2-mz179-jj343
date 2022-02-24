//
// Created by Zhong on 2/17/22.
//

#ifndef ERSS_HWK2_MZ179_JJ343_SERVICE_H
#define ERSS_HWK2_MZ179_JJ343_SERVICE_H

#include <sys/socket.h>
#include <unistd.h>

#include <fstream>

#include "Cache.h"
#include "my_time.hpp"
#include "request.hpp"
#include "response.hpp"

class Service {
 public:
  Request * request;
  Response * response;
  int clientFd;
  int serverFd;
  bool cached;
  pthread_mutex_t * mutex_p;
  std::ofstream * ofs_p;
  std::string user_ip;
  int ID;
  My_time my_time;
  Cache * cache_p;
  pthread_mutex_t * c_mutex_p;

 public:
  Service(int fd,
          pthread_mutex_t * mutex_p,
          std::ofstream * ofs_p,
          std::string user_ip,
          int ID,
          Cache * cache_p,
          pthread_mutex_t * c_mutex_p) :
      request(nullptr),
      response(nullptr),
      clientFd(fd),
      serverFd(-1),
      cached(false),
      mutex_p(mutex_p),
      ofs_p(ofs_p),
      user_ip(user_ip),
      ID(ID),
      cache_p(cache_p),
      c_mutex_p(c_mutex_p) {}
  ~Service() {
    delete request;
    if (!cached) {
      delete response;
    }

    close(clientFd);
    if (serverFd != -1) {
      close(serverFd);
    }
  }
  void process();
  void recvRequest();
  void sendRequest();
  void recvResponse();
  void sendResponse();
  bool try_connect_origin();
  void try_cache_response();
  void handle_connect();
  void send400();
  void send502();
  bool revalidate();
  void handle_chunk();
};
#endif  //ERSS_HWK2_MZ179_JJ343_SERVICE_H

#ifndef ERSS_HWK2_MZ179_JJ343_PROXY_H
#define ERSS_HWK2_MZ179_JJ343_PROXY_H
#include "Cache.h"

class Proxy {
 private:
  int my_sockfd;
  Cache * cache;

 public:
  Proxy() : my_sockfd(-1), cache(new Cache(10)) {}
  //build server and set up my_sockfd
  bool try_build_server(const char * PORT);
  int try_accept(int sockfd, std::string & ip);  //return -1 if fail
  int run();
  ~Proxy() { delete cache; }
};
#endif  //ERSS_HWK2_MZ179_JJ343_PROXY_H

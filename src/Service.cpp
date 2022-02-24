#include "Service.h"

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#define BUFFER_UNIT_SIZE 10000

class my_exception : public std::exception {
 private:
  std::string msg;

 public:
  my_exception(std::string msg) : msg(msg) {}
  virtual const char * what() const throw() { return msg.c_str(); }
};

std::vector<char> receive2(int fd) {
  std::vector<char> buffer(BUFFER_UNIT_SIZE);
  int bytes;
  int size = 0;
  while ((bytes = recv(fd, &buffer.data()[size], BUFFER_UNIT_SIZE, 0))) {
    if (bytes == -1) {
      throw my_exception("ERROR tcp receive failed");
    }
    size += bytes;
    if (bytes < BUFFER_UNIT_SIZE) {
      break;
    }
    buffer.resize(size + BUFFER_UNIT_SIZE);
  }
  buffer.resize(size);
  return buffer;
}

void getinfo(const std::vector<char> & vec, size_t * content_len, size_t * body_start) {
  std::string s(vec.begin(), vec.end());

  size_t start;
  size_t end;
  if ((start = s.find("\r\n\r\n", 0)) != std::string::npos) {
    *body_start = start + 4;
    if ((start = s.find("Content-Length: ", 0)) != std::string::npos) {
      end = s.find("\r\n", start);
      *content_len = atoi((s.substr(start + 16, end - start - 16)).c_str());
    }
    else {
      return;
    }
  }
}

std::vector<char> receive(int fd) {
  std::vector<char> buffer(BUFFER_UNIT_SIZE);
  int bytes;
  size_t size = 0;
  size_t body_start = 0;
  size_t content_len = 0;
  size_t read_size = BUFFER_UNIT_SIZE;

  while ((bytes = recv(fd, &buffer.data()[size], read_size, 0))) {
    if (bytes == -1) {
      throw my_exception("ERROR tcp receive failed");
    }
    size += bytes;
    if (!body_start) {
      getinfo(buffer, &content_len, &body_start);
    }
    if (body_start) {
      if (content_len) {
        read_size = content_len - (size - body_start);

        buffer.resize(size + read_size);
        if (read_size <= 0) {
          return buffer;
        }
        continue;
      }
      else {
        break;
      }
    }
  }
  buffer.resize(size);
  return buffer;
}

void send(int fd, std::vector<char> & msg) {
  int sent = 0;
  int size = msg.size();
  while (sent < size) {
    int bytes = send(fd, &msg.data()[sent], size - sent, 0);
    if (bytes == -1) {
      throw my_exception("ERROR tcp send failed");
    }
    sent += bytes;
  }
}

void Service::send400() {
  std::string msg = "HTTP/1.1 400 Bad Request\r\n\r\n";
  send(clientFd, msg.c_str(), msg.size(), 0);
  throw my_exception("Responding \"HTTP/1.1 400 Bad Request\"");
}

void Service::send502() {
  std::string msg = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
  send(clientFd, msg.c_str(), msg.size(), 0);
  throw my_exception("Responding \"HTTP/1.1 502 Bad Gateway\"");
}

void Service::recvRequest() {
  request = new Request(receive(clientFd));
  try {
    request->set_fields();
  }
  catch (std::exception & e) {
    send400();
  }
}

void Service::recvResponse() {
  response = new Response(receive(serverFd));
  try {
    response->set_fields();

    response->set_first_line();
  }
  catch (std::exception & e) {
    send502();
  }
}

void Service::sendRequest() {
  send(serverFd, request->vec);
}

void Service::sendResponse() {
  send(clientFd, response->vec);
}

bool Service::try_connect_origin() {
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo * host_info_list;
  const char * hostname = this->request->hostname.c_str();
  const char * port = this->request->port.c_str();

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    pthread_mutex_lock(mutex_p);
    (*ofs_p) << "Error: cannot get address info for host" << std::endl;
    (*ofs_p) << "  (" << hostname << "," << port << ")" << std::endl;
    pthread_mutex_unlock(mutex_p);
    return false;
  }  //if

  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    pthread_mutex_lock(mutex_p);
    (*ofs_p) << "Error: cannot create socket" << std::endl;
    (*ofs_p) << "  (" << hostname << "," << port << ")" << std::endl;
    pthread_mutex_unlock(mutex_p);
    return false;
  }  //if

  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    pthread_mutex_lock(mutex_p);
    (*ofs_p) << "Error: cannot connect to socket" << std::endl;
    (*ofs_p) << "  (" << hostname << "," << port << ")" << std::endl;
    pthread_mutex_unlock(mutex_p);
    return false;
  }  //if

  freeaddrinfo(host_info_list);
  serverFd = socket_fd;

  return true;
}

//implement it!!!
void Service::try_cache_response() {
  if (!cached && request->method == "GET" && response->first_line == "HTTP/1.1 200 OK") {
    if (response->is_private || response->nostore || !response->max_age) {
      pthread_mutex_lock(mutex_p);
      (*ofs_p) << ID
               << ": not cacheable because private/no-store/max-age=0 appears in cache "
                  "control field"
               << std::endl;
      pthread_mutex_unlock(mutex_p);
      return;
    }

    pthread_mutex_lock(mutex_p);
    if (response->nocache) {
      (*ofs_p) << ID << ": cached, but requires re-validation" << std::endl;
    }
    else {
      (*ofs_p) << ID << ": cached";
      if (response->expire_time) {
        (*ofs_p) << ", expires at " << my_time.get_expire_time(response->expire_time);
      }
      else {
        (*ofs_p) << std::endl;
      }
    }
    pthread_mutex_unlock(mutex_p);

    pthread_mutex_lock(c_mutex_p);
    cache_p->put(request->URI, response);
    pthread_mutex_unlock(c_mutex_p);
    cached = true;
  }
}

void Service::handle_connect() {
  send(clientFd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
  pthread_mutex_lock(mutex_p);
  (*ofs_p) << ID << ": Responding \"HTTP/1.1 200 OK\"" << std::endl;
  pthread_mutex_unlock(mutex_p);
  fd_set readfds;
  int nfds = std::max(clientFd, serverFd) + 1;
  while (1) {
    FD_ZERO(&readfds);
    FD_SET(serverFd, &readfds);
    FD_SET(clientFd, &readfds);

    select(nfds, &readfds, NULL, NULL, NULL);
    int fd[2] = {serverFd, clientFd};

    for (int i = 0; i < 2; i++) {
      if (FD_ISSET(fd[i], &readfds)) {
        std::vector<char> buffer = receive2(fd[i]);
        if (buffer.size() <= 0) {
          pthread_mutex_lock(mutex_p);
          (*ofs_p) << ID << ": Tunnel closed" << std::endl;
          pthread_mutex_unlock(mutex_p);
          return;
        }
        send(fd[1 - i], buffer);
      }
    }
  }
}

bool Service::revalidate() {
  //need revalidation
  bool expired = !my_time.is_late_than_currtime(response->expire_time);
  if (request->nocache || expired) {
    pthread_mutex_lock(mutex_p);
    if (expired) {
      (*ofs_p) << ID << ": in cache, but expired at "
               << my_time.get_expire_time(response->expire_time);
    }
    else {
      (*ofs_p) << ID << ": in cache, requires validation" << std::endl;
    }
    pthread_mutex_unlock(mutex_p);
    //send revalidation
    if (response->etag == "" && response->last_modified == "") {
      response = nullptr;
      return false;
    }
    std::string revalid_info(request->first_line);
    revalid_info.append("Host: ");
    revalid_info.append(request->hostname);
    revalid_info.append(":");
    revalid_info.append(request->port);
    revalid_info.append("\r\n");
    if (response->etag != "") {
      revalid_info.append("If-None-Match: ");
      revalid_info.append(response->etag);
      revalid_info.append("\r\n");
    }
    if (response->last_modified != "") {
      revalid_info.append("If-Modified-Since: ");
      revalid_info.append(response->last_modified);
      revalid_info.append("\r\n");
    }
    revalid_info.append("\r\n");
    if (!try_connect_origin()) {
      //fail to connect orgin : error handle
      send400();
    }
    std::vector<char> revalid(revalid_info.begin(), revalid_info.end());
    send(serverFd, revalid);
    Response * temp_res = new Response(receive(serverFd));
    //success
    temp_res->set_first_line();
    temp_res->set_fields();
    if (temp_res->first_line == "HTTP/1.1 200 OK") {
      pthread_mutex_lock(c_mutex_p);
      cache_p->put(request->URI, temp_res);
      pthread_mutex_unlock(c_mutex_p);
      response = temp_res;
    }
    else {
      delete temp_res;
    }
    return true;
  }
  pthread_mutex_lock(mutex_p);
  (*ofs_p) << ID << ": in cache, valid" << std::endl;
  pthread_mutex_unlock(mutex_p);
  return true;
}

void Service::handle_chunk() {
  pthread_mutex_lock(mutex_p);
  (*ofs_p) << ID << ": not cacheable because it is chunked" << std::endl;
  pthread_mutex_unlock(mutex_p);
  while (1) {  //receive and send remaining message
    std::vector<char> buffer = receive2(serverFd);

    if (buffer.size() <= 0) {
      return;
    }
    send(clientFd, buffer);
  }
}

void Service::process() {
  recvRequest();
  if (!request->is_valid()) {
    send400();
  }
  pthread_mutex_lock(mutex_p);
  (*ofs_p) << ID << ": " << '\"' << request->first_line << '\"' << " from " << user_ip
           << " @ " << my_time.get_curr_time();
  pthread_mutex_unlock(mutex_p);
  if (request->method == "GET") {
    //check if we can get response from cache
    pthread_mutex_lock(c_mutex_p);
    response = cache_p->get(request->URI);
    pthread_mutex_unlock(c_mutex_p);

    if (response) {
      cached = revalidate();
    }
    else {
      pthread_mutex_lock(mutex_p);
      (*ofs_p) << ID << ": not in cache" << std::endl;
      pthread_mutex_unlock(mutex_p);
    }
  }
  if (!cached) {
    if (!try_connect_origin()) {
      //fail to connect orgin : error handle
      send400();
    }

    if (request->method == "CONNECT") {
      handle_connect();
      return;
    }
    else {
      //POST
      sendRequest();
      pthread_mutex_lock(mutex_p);
      (*ofs_p) << ID << ": Requesting \"" << request->first_line << "\" from "
               << request->hostname << std::endl;
      pthread_mutex_unlock(mutex_p);

      recvResponse();

      if (response->is_chunk) {
        pthread_mutex_lock(mutex_p);
        (*ofs_p) << ID << ": Received \"" << response->first_line << "\" from "
                 << request->hostname << std::endl;
        pthread_mutex_unlock(mutex_p);
        sendResponse();
        handle_chunk();
        return;
      }
      pthread_mutex_lock(mutex_p);
      (*ofs_p) << ID << ": Received \"" << response->first_line << "\" from "
               << request->hostname << std::endl;
      pthread_mutex_unlock(mutex_p);
      if (!response->is_valid()) {
        send502();
      }
    }
  }

  try_cache_response();
  sendResponse();
  pthread_mutex_lock(mutex_p);
  (*ofs_p) << ID << ": Responding \"" << response->first_line << "\"" << std::endl;
  pthread_mutex_unlock(mutex_p);
}

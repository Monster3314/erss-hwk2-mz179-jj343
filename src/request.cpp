#include "request.hpp"

#include <string>
#include <vector>
void Request::set_fields() {
  std::string helper(cstr);
  size_t start = 0;
  size_t end;
  //set method
  if ((end = helper.find(' ', start)) != std::string::npos) {
    method = helper.substr(start, end - start);
  }
  //set first_line
  if ((end = helper.find("\r\n", start)) != std::string::npos) {
    first_line = helper.substr(start, end - start);
  }
  //set hostname and port
  if ((start = helper.find("Host", start)) != std::string::npos) {
    end = helper.find("\r\n", start);
    std::string host_line = helper.substr(start + 6, end - start - 6);
    size_t port_start;
    if ((port_start = host_line.find(':', 0)) != std::string::npos) {
      hostname = host_line.substr(0, port_start);
      end = host_line.find("\r\n", port_start);
      port = host_line.substr(port_start + 1, end - port_start - 1);
    }
    else {
      hostname = helper.substr(start + 6, end - start - 6);
      port = "80";
    }
  }
  //if method is GET set URI
  if (method == "GET") {
    size_t URI_start;
    size_t URI_end;
    URI_start = helper.find("h", 0);
    URI_end = helper.find(" ", URI_start);
    URI = helper.substr(URI_start, URI_end - URI_start);
  }
  else {
    URI = "";
  }
  //set nocache
  if ((start = helper.find("no-cache")) != std::string::npos) {
    nocache = true;
  }
  else {
    nocache = false;
  }
}
bool Request::is_valid() {
  if (method != "GET" && method != "POST" && method != "CONNECT") {
    return false;
  }
  return true;
}

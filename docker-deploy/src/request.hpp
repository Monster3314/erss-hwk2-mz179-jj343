#ifndef ERSS_HWK2_MZ179_JJ343_REQUEST_H
#define ERSS_HWK2_MZ179_JJ343_REQUEST_H
#include <string>
#include <vector>

class Request {
 public:
  std::vector<char> vec;
  const char * cstr;
  std::string method;
  std::string hostname;
  std::string port;
  std::string first_line;
  std::string URI;
  bool nocache;

 public:
  Request(const std::vector<char> & init_vec) : vec(init_vec), cstr(vec.data()) {}
  void set_fields();
  bool is_valid();
};
#endif  //ERSS_HWK2_MZ179_JJ343_REQUEST_H

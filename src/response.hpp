#ifndef ERSS_HWK2_MZ179_JJ343_RESPONSE_H
#define ERSS_HWK2_MZ179_JJ343_RESPONSE_H
#include <string>
#include <vector>
class Response {
 public:
  std::vector<char> vec;
  const char * cstr;
  std::string first_line;

  //head fields
  std::string res_time;
  int max_age;
  std::string Expires;
  bool nocache;
  bool nostore;
  bool is_private;
  std::string etag;
  std::string last_modified;
  int expire_time;
  bool is_chunk;

 public:
  Response(const std::vector<char> & init_vec) :
      vec(init_vec),
      cstr(vec.data()),
      res_time(""),
      max_age(0),
      Expires(""),
      nocache(false),
      nostore(false),
      is_private(false),
      etag(""),
      last_modified(""),
      expire_time(0),
      is_chunk(false) {}
  void set_first_line();
  void set_fields();
  bool is_valid();
};
#endif  //ERSS_HWK2_MZ179_JJ343_RESPONSE_H

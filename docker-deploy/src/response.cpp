#include "response.hpp"

#include "my_time.hpp"
void Response::set_first_line() {
  for (size_t i = 0; i < vec.size(); i++) {
    if (vec[i] != '\r') {
      first_line.push_back(vec[i]);
    }
    else {
      return;
    }
  }
}

int set_real_expire_time(int max_age, std::string Expires, std::string res_time) {
  My_time my_time;
  std::string ans;
  if (max_age > 0) {
    return max_age + my_time.transfer_to_seconds(res_time);
  }
  else if (Expires != "") {
    return my_time.transfer_to_seconds(Expires);
  }
  else {
    return 0;
  }
}

void Response::set_fields() {
  std::string helper(cstr);
  size_t start;
  size_t end;

  if ((start = helper.find("Date", 0)) != std::string::npos) {
    end = helper.find("\r\n", start);
    res_time = helper.substr(start + 6, end - start - 6);
  }

  if ((start = helper.find("max-age", 0)) != std::string::npos) {
    end = helper.find("\r\n", start);
    max_age = atoi((helper.substr(start + 8, end - start - 8)).c_str());
  }

  if ((start = helper.find("Expires", 0)) != std::string::npos) {
    end = helper.find("\r\n", start);
    Expires = helper.substr(start + 9, end - start - 9);
    if (Expires == "-1") {
      Expires = "";
    }
  }

  if ((start = helper.find("no-cache", 0)) != std::string::npos) {
    nocache = true;
  }

  if ((start = helper.find("no-store", 0)) != std::string::npos) {
    nostore = true;
  }

  if ((start = helper.find("private", 0)) != std::string::npos) {
    is_private = true;
  }

  if ((start = helper.find("ETag :\"", 0)) != std::string::npos) {
    end = helper.find("\r\n", start);
    etag = helper.substr(start + 6, end - start - 6);
  }

  if ((start = helper.find("Last-Modified", 0)) != std::string::npos) {
    end = helper.find("\r\n", start);
    last_modified = helper.substr(start + +15, end - start - 15);
  }

  if ((start = helper.find("chunked", 0)) != std::string::npos) {
    is_chunk = true;
  }

  expire_time = set_real_expire_time(max_age, Expires, res_time);
}
bool Response::is_valid() {
  std::string helper(cstr);
  if (helper.find("\r\n\r\n") == std::string::npos) {
    return false;
  }
  return true;
}

#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
class My_time {
 private:
  //Www Mmm dd hh:mm:ss yyyy
  //{ Mon, Tue, Wed, Thu, Fri, Sat, Sun } weekday;
  // { Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec } month;
  std::unordered_map<std::string, int> weekday;
  std::unordered_map<std::string, int> month;

 public:
  My_time() {
    weekday.insert(std::make_pair<std::string, int>("Mon", 1));
    weekday.insert(std::make_pair<std::string, int>("Tue", 2));
    weekday.insert(std::make_pair<std::string, int>("Wed", 3));
    weekday.insert(std::make_pair<std::string, int>("Thu", 4));
    weekday.insert(std::make_pair<std::string, int>("Fri", 5));
    weekday.insert(std::make_pair<std::string, int>("Sat", 6));
    weekday.insert(std::make_pair<std::string, int>("Sun", 0));

    month.insert(std::make_pair<std::string, int>("Jan", 0));
    month.insert(std::make_pair<std::string, int>("Feb", 1));
    month.insert(std::make_pair<std::string, int>("Mar", 2));
    month.insert(std::make_pair<std::string, int>("Apr", 3));
    month.insert(std::make_pair<std::string, int>("May", 4));
    month.insert(std::make_pair<std::string, int>("Jun", 5));
    month.insert(std::make_pair<std::string, int>("Jul", 6));
    month.insert(std::make_pair<std::string, int>("Agu", 7));
    month.insert(std::make_pair<std::string, int>("Sep", 8));
    month.insert(std::make_pair<std::string, int>("Oct", 9));
    month.insert(std::make_pair<std::string, int>("Nov", 10));
    month.insert(std::make_pair<std::string, int>("Dec", 11));
  }

  char * get_curr_time() {
    time_t rawtime;
    struct tm * ptm;
    time(&rawtime);
    ptm = gmtime(&rawtime);
    return asctime(ptm);
  }

  std::string transfer_time(const std::string & origin_time) {
    std::vector<std::string> vec;
    vec.resize(7);
    std::string ans;
    //Wed, 21 Oct 2015 07:28:00
    vec[0] = origin_time.substr(0, 3);
    vec[1] = origin_time.substr(5, 2);
    vec[2] = origin_time.substr(8, 3);
    vec[3] = origin_time.substr(12, 4);
    vec[4] = origin_time.substr(17, 2);
    vec[5] = origin_time.substr(20, 2);
    vec[6] = origin_time.substr(23, 2);
    ans.append(vec[0]);
    ans.append(" ");
    ans.append(vec[2]);
    ans.append(" ");
    ans.append(vec[1]);
    ans.append(" ");
    ans.append(vec[4]);
    ans.append(":");
    ans.append(vec[5]);
    ans.append(":");
    ans.append(vec[6]);
    ans.append(" ");
    ans.append(vec[3]);
    return ans;
  }

  time_t transfer_to_seconds(const std::string & origin_time) {
    std::vector<std::string> vec;
    vec.resize(7);
    std::string ans;
    //Wed, 21 Oct 2015 07:28:00
    vec[0] = origin_time.substr(0, 3);   //weekday
    vec[1] = origin_time.substr(5, 2);   //day
    vec[2] = origin_time.substr(8, 3);   //month
    vec[3] = origin_time.substr(12, 4);  //year
    vec[4] = origin_time.substr(17, 2);  //hour
    vec[5] = origin_time.substr(20, 2);  //min
    vec[6] = origin_time.substr(23, 2);  //second
    struct tm y2k;
    time_t seconds;
    y2k.tm_hour = atoi((vec[4]).c_str());

    y2k.tm_min = atoi((vec[5]).c_str());

    y2k.tm_sec = atoi((vec[6]).c_str());

    y2k.tm_year = atoi((vec[3]).c_str()) - 1900;

    y2k.tm_mon = month[vec[2]];

    y2k.tm_mday = atoi((vec[1]).c_str());

    //y2k.tm_wday = weekday[vec[0]];
    y2k.tm_isdst = 1;
    seconds = mktime(&y2k);
    return seconds;
  }

  bool is_late_than_currtime(int in_seconds) {
    //curr_time
    if (in_seconds == 0) {
      return true;
    }
    time_t timer;
    time_t seconds;
    struct tm * temp;

    time(&timer); /* get current time; same as: timer = time(NULL)  */
    temp = gmtime(&timer);
    timer = mktime(temp);
    seconds = difftime(timer, in_seconds);
    if (seconds > 0) {
      return false;
    }
    else {
      return true;
    }
  }
  char * get_expire_time(time_t rawtime) {
    struct tm * ptm;
    ptm = gmtime(&rawtime);
    return asctime(ptm);
  }
};

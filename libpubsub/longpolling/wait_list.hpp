#pragma once

#include <string>
#include <map>
#include <set>
#include <vector>
#include <ctime>

namespace wfc{ namespace pubsub{

class wait_list
{
public:
  explicit wait_list(time_t timeout_s);

  void wait(const std::vector<std::string>& cl);
  void ready(const std::string& name);
  void ready(const std::vector<std::string>& cl);
  size_t pop_outdated(std::set<std::string>* cl);
  void clear();
  size_t size() const;

private:
  typedef std::map<std::string, time_t> channel_map_t;
  typedef std::set< std::pair<time_t, std::string> > time_set_t;

  time_t _timeout_s = 10;
  channel_map_t _channel_map;
  time_set_t _time_set;
};

}}

#pragma once

#include <pubsub/channel_map.hpp>
#include <pubsub/pubsub_options.hpp>
#include <pubsub/api/types.hpp>
#include <pubsub/api/subscribe_params.hpp>
#include <functional>
#include <atomic>
#include <mutex>
#include <list>
#include <map>
#include <set>

namespace wfc{ namespace pubsub{

class multi_rocksdb;

class pubsub
{
public:

  typedef std::shared_ptr<multi_rocksdb> rocksdb_ptr;

  typedef channel_map::message_list_t message_list_t;
  typedef std::function<void(const std::string& channel, const message& msg)> handler_fun_t;

  pubsub();

  void reconfigure(rocksdb_ptr db, const pubsub_options& opt);

  void publish(const std::string& channel, message&& msg);

  void get_messages(message_list_t* ml, const subscribe_params& params);

  subscriber_id_t subscribe(message_list_t* ml, const subscribe_params& params);

  void subscribe( message_list_t* ml, subscriber_id_t id, const subscribe_params& params);

  size_t describe(subscriber_id_t id);

  bool describe(subscriber_id_t id, const std::string& channel);

  size_t subscriber_count() const;

  size_t size(size_t* count) const;

  size_t get_removed(bool reset) const;

  size_t empty_count() const;

  void reset();

  void write_log() const;

private:

  bool subscribe_(subscriber_id_t id, const std::string& channel, const handler_fun_t& handler);
  bool describe_(subscriber_id_t id, const std::string& channel);
  void fire_(const std::string& channel, const message& msg) const;

private:
  typedef std::pair<subscriber_id_t, std::string> subscriber_channel_t;
  typedef std::pair<std::string, subscriber_id_t> channel_subscriber_t;
  typedef std::map<channel_subscriber_t, handler_fun_t> handler_map_t;
  typedef std::set<subscriber_channel_t> subscriber_set_t;

  static std::atomic<subscriber_id_t> _id_counter;
  channel_map _channel_map;
  handler_map_t _handler_map;
  subscriber_set_t _subscriber_set;
  rocksdb_ptr _rocksdb;
  pubsub_options _opt;
};

}}

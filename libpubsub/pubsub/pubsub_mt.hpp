#pragma once

#include <pubsub/pubsub.hpp>
#include <pubsub/pubsub_options.hpp>
#include <pubsub/rocksdb/rocksdb_options.hpp>

namespace wfc{ namespace pubsub{

class multi_rocksdb;

class pubsub_mt
{
  typedef std::shared_ptr<multi_rocksdb> rocksdb_ptr;

public:

  typedef pubsub::message_list_t message_list_t;

  pubsub_mt(const pubsub_options& opt, const rocksdb_options& ropt);

  void reconfigure(const pubsub_options& opt, const rocksdb_options& ropt);

  void close();

  void get_messages(message_list_t* ml, const subscribe_params& params);

  void publish(const std::string& channel, message&& msg);

  subscriber_id_t subscribe(message_list_t* ml, const subscribe_params& params);

  void subscribe( message_list_t* ml, subscriber_id_t id, const subscribe_params& params);

  size_t describe(subscriber_id_t id);

  bool describe(subscriber_id_t id, const std::string& channel);

  size_t subscriber_count() const;

  /**
   * @param count [out] - общее количестов сообщений
   * @return количество каналов
   */
  size_t size(size_t* count) const;

  size_t get_removed(bool reset) const;
  size_t empty_count() const;
  void write_log() const;


  void reset();
private:
  size_t index_( const std::string& channel ) const;
private:
  typedef std::mutex mutex_type;
  size_t _size = 0;
  mutable std::vector<mutex_type> _mutext_list;
  std::vector<pubsub> _pubsub_list;
  rocksdb_ptr _rocksdb;
};

}}

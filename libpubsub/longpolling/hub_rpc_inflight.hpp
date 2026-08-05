//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <atomic>
#include <cstddef>
#include <ctime>
#include <deque>
#include <limits>

namespace wfc{ namespace pubsub{

/**
 * @brief Лимит одновременных subscribe/describe RPC domain → hub
 *        (отдельно от inflight внутри subscribe_manager).
 */
class hub_rpc_inflight
{
public:
  void configure(size_t subscribe_max, size_t describe_max, time_t timeout_s);
  void reset();
  void recover();

  bool subscribe_full() const;
  bool describe_full() const;

  size_t subscribe_count() const;
  size_t describe_count() const;
  size_t subscribe_max() const;
  size_t describe_max() const;

  void note_subscribe();
  void note_describe();
  void ack_subscribe();
  void ack_describe();

private:
  size_t _subscribe_max = std::numeric_limits<size_t>::max();
  size_t _describe_max = std::numeric_limits<size_t>::max();
  time_t _timeout_s = 10;
  std::atomic<size_t> _subscribe{0};
  std::atomic<size_t> _describe{0};
  std::deque<time_t> _subscribe_deadline;
  std::deque<time_t> _describe_deadline;
};

}}

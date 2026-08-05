#pragma once

#include <cstddef>

namespace wfc{ namespace pubsub{

struct hub_queue_stat
{
  size_t counter_map = 0;
  size_t wait_subscribe = 0;
  size_t wait_describe = 0;
  size_t describe_delay = 0;
  size_t inflight_subscribe = 0;
  size_t inflight_describe = 0;

  hub_queue_stat& operator += (const hub_queue_stat& other)
  {
    this->counter_map += other.counter_map;
    this->wait_subscribe += other.wait_subscribe;
    this->wait_describe += other.wait_describe;
    this->describe_delay += other.describe_delay;
    this->inflight_subscribe += other.inflight_subscribe;
    this->inflight_describe += other.inflight_describe;
    return *this;
  }
};

}}

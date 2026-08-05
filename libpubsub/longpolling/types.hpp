#pragma once
#include <vector>
#include <message_queue/topic.hpp>
#include <longpolling/channel_params.hpp>
#include <functional>

namespace wfc{ namespace pubsub{

typedef std::vector<topic> topic_list_t;
typedef std::vector<channel_params> channel_list_t;
typedef std::function<void(const topic_list_t&)> longpoll_hundler_t;

}}

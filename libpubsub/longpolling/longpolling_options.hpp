#pragma once

#include <longpolling/agent_map_options.hpp>

namespace wfc{ namespace pubsub{

struct longpolling_options
  : agent_map_options
{
  // время жизни канала, с момента ухода последнего подписчика, после чего будет отписка с хаба
  time_t describe_delay_s = 3600;
  // для отслеживания потерянных сообщений о подписке с хабом
  time_t lost_delay_s = 10;

  // Отключить отписку от хаба
  bool describe_suspend = false;
};

}}


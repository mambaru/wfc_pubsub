#include <fas/testing.hpp>
#include <pubsub/api/message.hpp>
#include <pubsub/stored_message.hpp>
#include <pubsub/message_queue.hpp>

namespace{
  UNIT(message, "")
  {
    using namespace fas::testing;
    using namespace wfc;

    pubsub::message m;
    m.key = 1;
    m.identity = "identity1";
    m.content = std::make_unique<pubsub::content_t>("content1");
    m.lifetime = 10;
    pubsub::stored_message sm( std::move(m) );
    t << equal<expect, pubsub::actions>(sm.action(), pubsub::actions::publish) << FAS_FL;
    t << equal<expect, time_t>(sm.birthtime(), 0 ) << FAS_FL;
    time_t now = time(nullptr);
    sm.reborn(now);
    t << equal<expect, time_t>(sm.birthtime(), now ) << FAS_FL;
    t << equal<expect, time_t>(sm.deathtime(), now + 10) << FAS_FL;
  }

  UNIT(message_queue, "")
  {
    using namespace fas::testing;
    using namespace wfc;
    pubsub::message_queue mq;
    for (int i = 0; i < 10; ++i)
    {
      pubsub::message m;
      m.key = 1;
      m.identity = "identity" + std::to_string(i);
      m.content = std::make_unique<pubsub::content_t>( "content" + std::to_string(i) );
      m.lifetime = 10;
      if ( i % 2 == 0 )
        m.birthtime = time(nullptr) - 30;
      mq.push( std::move(m) );
    }
    t << nothing;
  }
}

BEGIN_SUITE(message_suite, "")
  ADD_UNIT(message)
  ADD_UNIT(message_queue)

END_SUITE(message_suite)

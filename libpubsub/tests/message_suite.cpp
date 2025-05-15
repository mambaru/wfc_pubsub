#include <fas/testing.hpp>
#include <message_queue/message.hpp>
#include <message_queue/message_queue.hpp>
#include <message_queue/channel_map.hpp>
#include <message_queue/stored_message.hpp>

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

  UNIT(channel_map, "")
  {
    using namespace fas::testing;
    using namespace wfc;

    pubsub::channel_map cm;

    pubsub::message m;
    m.limit = 10;
    m.lifetime = 10;
    m.birthtime = time(nullptr);

    m.identity="id1.1";
    cm.push("ch1", m);
    m.identity="id1.2";
    cm.push("ch1", m);
    m.identity="id1.3";
    cm.push("ch1", m);

    m.identity="id2.1";
    cm.push("ch2", m);
    m.identity="id2.2";
    cm.push("ch2", m);
    m.identity="id2.3";
    cm.push("ch2", m);

    size_t s1 = 0, s2 = 0;
    s1 = cm.size(&s2);
    t << equal<expect, size_t>(s1, 2 ) << FAS_FL;
    t << equal<expect, size_t>(s2, 6 ) << FAS_FL;


    pubsub::channel_map::topic_list_t tl;
    size_t count = cm.takeaway(4, &tl);
    t << equal<expect, size_t>(count, 4 ) << FAS_FL;
    t << equal<expect, size_t>(tl.size(), 4 ) << FAS_FL;
    tl.clear();
    count = cm.takeaway(4, &tl);
    t << equal<expect, size_t>(count, 2 ) << FAS_FL;
    t << equal<expect, size_t>(tl.size(), 2 ) << FAS_FL;

    s2 = 0;
    s1 = cm.size(&s2);
    t << equal<expect, size_t>(s1, 2 ) << FAS_FL;
    t << equal<expect, size_t>(s2, 0 ) << FAS_FL;
  }

}

BEGIN_SUITE(message_suite, "")
  ADD_UNIT(message)
  ADD_UNIT(message_queue)
  ADD_UNIT(channel_map)
END_SUITE(message_suite)

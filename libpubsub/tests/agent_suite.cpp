#include <fas/testing.hpp>
#include <longpolling/agent.hpp>
#include <longpolling/agent_map_mt.hpp>
#include <longpolling/longpolling.hpp>
namespace{
  UNIT(agent, "")
  {
    using namespace fas::testing;
    using namespace wfc;

    pubsub::agent_options opt;
    opt.longpoll_limit = 20;
    opt.prio_limit = 5;
    pubsub::agent a(opt);

    std::vector<std::string> chs={"ch4", "ch3", "ch1","ch2", "ch5"};
    for (auto ch: chs)
      a.open({pubsub::channel_params{ch}});

    a.close({"ch5"});
    for (auto ch: chs)
    {
      for (int i = 0; i < 10; ++i)
      {
        pubsub::message m;
        m.identity = ch + "-"+ std::to_string(i) + "-pri";
        a.push(ch, m);
        m.identity = ch + "-"+ std::to_string(i);
        m.lifetime = 10;
        m.limit = 20;
        a.push(ch, m);
      }
    }
    a.close({"ch4"});

    pubsub::topic_list_t topics;
    pubsub::longpoll_hundler_t poll_handler = [&topics](const pubsub::topic_list_t& tl) noexcept -> void
    {
      std::transform(tl.begin(), tl.end(), std::back_inserter(topics), [](const pubsub::topic& ct) -> pubsub::topic
      {
        pubsub::topic tp;
        tp.channel = ct.channel;
        static_cast<pubsub::message&>(tp) = pubsub::stored_message(ct).get();
        return tp;
      });
    };


    a.longpoll(poll_handler);
    a.fire(nullptr);
    t << equal<assert, size_t>(topics.size(), 20 ) << FAS_FL;
    t << flush;

    t << equal_str<expect>(topics[0].identity,  "ch2-5-pri" ) << FAS_FL;
    t << equal_str<expect>(topics[4].identity,  "ch2-9-pri" ) << FAS_FL;
    t << equal_str<expect>(topics[5].identity,  "ch1-0" ) << FAS_FL;
    t << equal_str<expect>(topics[14].identity, "ch1-9" ) << FAS_FL;
    t << equal_str<expect>(topics[15].identity, "ch2-0" ) << FAS_FL;
    t << equal_str<expect>(topics[19].identity, "ch2-4" ) << FAS_FL;

    // Повтор без confirm
    topics.clear();
    a.longpoll(poll_handler);
    a.fire(nullptr);
    t << equal<assert, size_t>(topics.size(), 20 ) << FAS_FL;
    t << flush;

    t << equal_str<expect>(topics[0].identity,  "ch2-5-pri" ) << FAS_FL;
    t << equal_str<expect>(topics[4].identity,  "ch2-9-pri" ) << FAS_FL;
    t << equal_str<expect>(topics[5].identity,  "ch1-0" ) << FAS_FL;
    t << equal_str<expect>(topics[14].identity, "ch1-9" ) << FAS_FL;
    t << equal_str<expect>(topics[15].identity, "ch2-0" ) << FAS_FL;
    t << equal_str<expect>(topics[19].identity, "ch2-4" ) << FAS_FL;

    topics.clear();
    a.confirm();
    a.longpoll(poll_handler);
    a.fire(nullptr);
    t << equal<assert, size_t>(topics.size(), 15 ) << FAS_FL;
    t << flush;

    t << equal_str<expect>(topics[0].identity, "ch2-5" ) << FAS_FL;
    t << equal_str<expect>(topics[4].identity, "ch2-9" ) << FAS_FL;
    t << equal_str<expect>(topics[5].identity, "ch3-0" ) << FAS_FL;
    t << equal_str<expect>(topics[9].identity, "ch3-4" ) << FAS_FL;
    t << equal_str<expect>(topics[14].identity,"ch3-9" ) << FAS_FL;
  }

  UNIT(agent_map, "")
  {
    using namespace fas::testing;
    using namespace wfc;

    pubsub::agent_map_options amopt;
    pubsub::agent_map_mt am(amopt);
    std::string u1 = pubsub::agent::create_uuid();
    std::string u2 = pubsub::agent::create_uuid();
    t << message("u1=") << u1;
    t << message("u2=") << u2;
    t << not_equal< assert, std::string >(u1, u2) << FAS_FL;
    am.create(pubsub::agent_options{u1});
    am.create(pubsub::agent_options{u2});
    am.open( pubsub::agent_params{u1},
             { pubsub::channel_params{"channel1"}, pubsub::channel_params{"channel2"}});
    am.open( pubsub::agent_params{u2},
             { pubsub::channel_params{"channel2"}, pubsub::channel_params{"channel3"}});

    pubsub::message m{pubsub::actions::publish, size_t(10), time_t(10)};
    am.push("channel1", m);
    am.push("channel2", m);
    am.push("channel2", m);
    am.push("channel3", m);
    am.push("channel4", m);

    size_t c1 = 0, c2 = 0;
    auto h1 = [&c1, &t](const pubsub::topic_list_t& tl)
    {
      t << equal<assert>(tl.size(), 3) << FAS_FL;
      t << stop;
      t << equal_str<expect>(tl[0].channel, "channel1") << FAS_FL;
      t << equal_str<expect>(tl[1].channel, "channel2") << FAS_FL;
      t << equal_str<expect>(tl[2].channel, "channel2") << FAS_FL;
      c1+=tl.size();
    };

    auto h2 = [&c2, &t](const pubsub::topic_list_t& tl)
    {
      t << equal<assert>(tl.size(), 3) << FAS_FL;
      t << stop;
      t << equal_str<expect>(tl[0].channel, "channel2") << FAS_FL;
      t << equal_str<expect>(tl[1].channel, "channel2") << FAS_FL;
      t << equal_str<expect>(tl[2].channel, "channel3") << FAS_FL;
      c2+=tl.size();
    };

    am.longpoll(pubsub::agent_params{u1}, h1);
    am.longpoll(pubsub::agent_params{u2}, h2);

    pubsub::fire_stat stat;
    am.fire(&stat, nullptr);

    t << message("Second fire!");

    am.longpoll(pubsub::agent_params{u1,0,true}, h1);
    am.longpoll(pubsub::agent_params{u2,0,true}, h2);
    am.fire(&stat, nullptr);

    t << equal<expect>(c1, 3) << FAS_FL;
    t << equal<expect>(c2, 3) << FAS_FL;
  }
}

BEGIN_SUITE(agent_suite, "")
  ADD_UNIT(agent)
  ADD_UNIT(agent_map)
END_SUITE(agent_suite)


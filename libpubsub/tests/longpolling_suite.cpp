#include <fas/testing.hpp>
#include <longpolling/longpolling.hpp>

namespace{


  UNIT(longpolling1, "")
  {
    using namespace fas::testing;
    using namespace wfc;

    pubsub::longpolling_options amopt;
    amopt.lost_delay_s = 1;
    pubsub::longpolling lp(amopt);
    std::string u1 = pubsub::agent::create_uuid();
    std::string u2 = pubsub::agent::create_uuid();
    t << message("u1=") << u1;
    t << message("u2=") << u2;
    t << not_equal< assert, std::string >(u1, u2) << FAS_FL;
    lp.create(pubsub::agent_options{u1});
    lp.create(pubsub::agent_options{u2});
    lp.open( pubsub::agent_params{u1},
             { pubsub::channel_params{"channel1"}, pubsub::channel_params{"channel2"}});
    lp.open( pubsub::agent_params{u2},
             { pubsub::channel_params{"channel2"}, pubsub::channel_params{"channel3"}});


    for (int i =0 ; i < 5; ++i)
    {
      std::vector<std::string> ch, channels;
      while ( lp.pop_for_subscribe(&ch, 1) )
      {
        t << equal<assert>(ch.size(), 1) << FAS_FL;
        t << stop;
        channels.push_back(ch[0]);
        ch.clear();
      }
      t << equal<expect>(channels.size(), 3) << FAS_FL;
      lp.rollback_for_subscribe(channels);
    }

    std::vector<std::string> channels;
    lp.pop_for_subscribe(&channels, 100);
    t << equal<assert>(channels.size(), 3) << FAS_FL;
    t << equal<assert>(channels[1], "channel2") << FAS_FL;
    channels.clear();
    lp.pop_for_subscribe(&channels, 100);
    t << equal<assert>(channels.size(), 0) << FAS_FL;

    sleep(1);  //  Имитация потери
    channels.clear();
    lp.pop_for_subscribe(&channels, 100);  // Для повторного запроса
    t << equal<assert>(channels.size(), 3) << FAS_FL;
    t << equal<assert>(channels[1], "channel2") << FAS_FL;

    sleep(1);  //  Имитация потери
    lp.confirm_for_subscribe(channels); // Подтверждение срабатывает и после таймаута
    channels.clear();
    lp.pop_for_subscribe(&channels, 100);
    t << equal<assert>(channels.size(), 0) << FAS_FL;
  }

  UNIT(longpolling2, "")
  {
    using namespace fas::testing;
    using namespace wfc;

    pubsub::longpolling_options amopt;
    amopt.describe_delay_s = 1;
    pubsub::longpolling lp(amopt);
    std::string u1 = pubsub::agent::create_uuid();
    t << message("u1=") << u1;
    pubsub::agent_options ao;
    ao.uuid = u1;
    ao.agent_lifetime = 1;
    lp.create(ao);
    lp.open(pubsub::agent_params{u1}, { pubsub::channel_params{"channel1"} });
    std::vector<std::string> ch;
    lp.pop_for_subscribe(&ch, 100);
    t << equal<assert>(ch.size(), 1) << FAS_FL;
    lp.confirm_for_subscribe(ch);
    lp.close(u1, {"channel1"});
    ch.clear();

    sleep(3);
    pubsub::fire_stat fs;
    lp.fire(&fs);
    t << message("stat: ") << fs.active_agents << ", " << fs.deleted_agents;
    lp.pop_for_describe(&ch, 100);
    t << equal<assert>(ch.size(), 1) << FAS_FL;
    lp.confirm_for_describe(ch);
    ch.clear();

    t << message("reopen");
    t << stop;
    sleep(1);
    fs = pubsub::fire_stat();
    lp.fire(&fs);
    t << equal<assert>(fs.subs_stat.counter_map, 0) << FAS_FL;


    lp.create(ao);
    lp.open(pubsub::agent_params{u1}, { pubsub::channel_params{"channel1"} });
    lp.pop_for_subscribe(&ch, 100);
    t << equal<assert>(ch.size(), 1) << FAS_FL;
    lp.confirm_for_subscribe(ch);
    lp.fire(&fs);
  }

  UNIT(longpolling3, "")
  {
    using namespace fas::testing;
    using namespace wfc;

    pubsub::longpolling_options amopt;
    amopt.describe_delay_s = 1;
    pubsub::longpolling lp(amopt);

    std::string u1 = pubsub::agent::create_uuid();
    pubsub::agent_options ao;
    ao.uuid = u1;
    ao.agent_lifetime = 1;
    lp.create(ao);
    lp.open(pubsub::agent_params{u1}, { pubsub::channel_params{"channel1"} });
    lp.open(pubsub::agent_params{u1}, { pubsub::channel_params{"channel2"} });
    lp.open(pubsub::agent_params{u1}, { pubsub::channel_params{"channel3"} });

    std::vector<std::string> ch_list;
    lp.pop_for_subscribe(&ch_list, 100);
    t << equal<expect>(ch_list.size(), 3) << FAS_FL;
    lp.confirm_for_subscribe(ch_list);
    sleep(1);
    pubsub::fire_stat fs;
    lp.fire(&fs);
    t << equal<expect>(fs.active_agents, 0) << FAS_FL;
    t << equal<expect>(fs.deleted_agents, 1) << FAS_FL;
    t << equal<expect>(fs.subs_stat.describe_delay, 3) << FAS_FL;

    sleep(1);
    fs = pubsub::fire_stat();
    lp.fire(&fs);

    ch_list.clear();
    lp.pop_for_describe(&ch_list, 100);
    t << equal<expect>(ch_list.size(), 3) << FAS_FL;
    lp.confirm_for_describe(ch_list);

    fs = pubsub::fire_stat();
    lp.fire(&fs);

    t << equal<expect>(fs.subs_stat.counter_map, 0) << FAS_FL;

  }
}

BEGIN_SUITE(longpolling_suite, "")
  ADD_UNIT(longpolling1)
  ADD_UNIT(longpolling2)
  ADD_UNIT(longpolling3)
END_SUITE(longpolling_suite)




#include <fas/testing.hpp>
#include <pubsub/domain/pubsub_domain.hpp>
#include <longpolling/domain/longpolling_domain.hpp>
#include <wfc/testing/testing_domain.hpp>
#include "wjson/_json.hpp"

namespace{

UNIT(longpolling1, "")
{
  using namespace fas::testing;
  using namespace wfc;
  using namespace wjson::literals;

  t << nothing;

  auto ptest = std::make_shared<wfc::testing_domain>();

  pubsub::pubsub_domain::domain_config pubsub_conf;
  pubsub_conf.name = "pubsub1";
  pubsub_conf.rocksdb_disabled = true;
  auto ppubsub = ptest->create<pubsub::pubsub_domain>(pubsub_conf);
  // ppubsub->configure();

  pubsub::longpolling_domain::domain_config longpolling_conf;
  longpolling_conf.name = "longpolling1";
  longpolling_conf.target = "pubsub1";
  longpolling_conf.fire_timer_ms = 10;
  auto plongpolling = ptest->create<pubsub::longpolling_domain>(longpolling_conf);
  // plongpolling->configure();

  ptest->initialize();
  ptest->start();

  auto pub = std::make_unique< pubsub::request::publish >();
  pub->messages.resize(1);
  pub->messages.back().channel = "test1";
  pub->messages.back().limit = 10;
  pub->messages.back().lifetime = 10;
  ppubsub->publish(std::move(pub), nullptr );

  auto cre = std::make_unique< pubsub::request::create >();
  cre->channels.resize(1);
  cre->channels.back().channel = "test1";

  std::string uuid;
  plongpolling->create1(std::move(cre), [&uuid](pubsub::response::create::ptr res){uuid=res->uuid;} );
  t << not_equal<assert, size_t>(uuid.size(), 0) << FAS_FL;
  t << message("uuid:") << uuid;


  auto pol = std::make_unique<pubsub::request::longpoll>();
  pol->uuid = uuid;

  std::string channel_name;
  plongpolling->longpoll(std::move(pol), [&t, &channel_name, ptest](pubsub::response::longpoll::ptr res)
  {
    channel_name=res->messages.front().channel;
    ptest->global()->io_context.stop();
    t << message("READY:") << channel_name << " size: " << res->messages.size();
  } );

  ptest->global()->io_context.run();
  /*
  std::this_thread::sleep_for( std::chrono::milliseconds(100) );
  ptest->global()->io_context.poll_one();
  std::this_thread::sleep_for( std::chrono::milliseconds(100) );
  ptest->global()->io_context.poll_one();
  */

  t << not_equal<assert, size_t>(channel_name.size(), 0) << FAS_FL;
  t << message("channel:") << channel_name;

  ptest->stop();
  ptest.reset();
  plongpolling.reset();

}

}

BEGIN_SUITE(longpoll, "")
  ADD_UNIT(longpolling1)
END_SUITE(longpoll)

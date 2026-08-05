#include <fas/testing.hpp>
#include <pubsub/domain/pubsub_domain.hpp>
#include <longpolling/domain/longpolling_domain.hpp>
#include <wfc/testing/testing_domain.hpp>
#include "wjson/_json.hpp"

#include <chrono>
#include <thread>

namespace{

void drain_io(const std::shared_ptr<wfc::testing_domain>& ptest, std::chrono::milliseconds ms)
{
  ptest->global()->io_context.run_for(ms);
  ptest->global()->io_context.restart();
}

void shutdown_longpoll(const std::shared_ptr<wfc::testing_domain>& ptest)
{
  ptest->stop();
  drain_io(ptest, std::chrono::milliseconds(300));
}

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
  plongpolling->create1(std::move(cre), [&uuid](pubsub::response::create::ptr res) noexcept {uuid=res->uuid;} );
  t << not_equal<assert, size_t>(uuid.size(), 0) << FAS_FL;
  t << message("uuid:") << uuid;


  auto pol = std::make_unique<pubsub::request::longpoll>();
  pol->uuid = uuid;

  std::string channel_name;
  plongpolling->longpoll(std::move(pol), [&t, &channel_name, ptest](pubsub::response::longpoll::ptr res) noexcept
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

  shutdown_longpoll(ptest);
  ptest.reset();
  plongpolling.reset();

}

UNIT(longpolling_auth, "")
{
  using namespace fas::testing;
  using namespace wfc;

  auto ptest = std::make_shared<wfc::testing_domain>();

  pubsub::pubsub_domain::domain_config pubsub_conf;
  pubsub_conf.name = "pubsub1";
  pubsub_conf.rocksdb_disabled = true;
  auto ppubsub = ptest->create<pubsub::pubsub_domain>(pubsub_conf);

  pubsub::longpolling_domain::domain_config longpolling_conf;
  longpolling_conf.name = "longpolling1";
  longpolling_conf.target = "pubsub1";
  longpolling_conf.fire_timer_ms = 10;
  longpolling_conf.auth.disabled = false;
  longpolling_conf.auth.channel = "auth.sessions";
  longpolling_conf.auth.ttl_s = 3600;
  auto plongpolling = ptest->create<pubsub::longpolling_domain>(longpolling_conf);

  ptest->initialize();
  ptest->start();

  // Публикуем сессию до подписки — она будет доставлена при subscribe
  auto auth_pub = std::make_unique<pubsub::request::publish>();
  auth_pub->messages.resize(1);
  auth_pub->messages.back().channel = "auth.sessions";
  auth_pub->messages.back().limit = 1000;
  auth_pub->messages.back().lifetime = 3600;
  auth_pub->messages.back().content = std::make_unique<pubsub::content_t>(
    R"({"oid":1001,"sid":"sess1"})"
  );
  ppubsub->publish(std::move(auth_pub), nullptr);

  // Ждём подписку longpolling на auth.sessions и обработку сообщений
  drain_io(ptest, std::chrono::milliseconds(200));

  // allow_anonymous=false (по умолчанию): без авторизации create отклоняется
  auto cre_anon = std::make_unique<pubsub::request::create>();
  cre_anon->channels.resize(1);
  cre_anon->channels.back().channel = "test1";
  bool create_rejected = false;
  plongpolling->create1(std::move(cre_anon), [&create_rejected](pubsub::response::create::ptr res) noexcept{
    create_rejected = res == nullptr;
  });
  t << equal<assert, bool>(create_rejected, true) << FAS_FL;

  // С авторизацией create проходит
  auto cre = std::make_unique<pubsub::request::create>();
  cre->oid = 1001;
  cre->sid = "sess1";
  cre->channels.resize(1);
  cre->channels.back().channel = "test1";
  std::string uuid;
  plongpolling->create1(std::move(cre), [&uuid](pubsub::response::create::ptr res) noexcept{
    if ( res != nullptr )
      uuid = res->uuid;
  });
  t << not_equal<assert, size_t>(uuid.size(), 0) << FAS_FL;

  shutdown_longpoll(ptest);
}

UNIT(longpolling_auth_suspend, "")
{
  using namespace fas::testing;
  using namespace wfc;

  auto ptest = std::make_shared<wfc::testing_domain>();

  pubsub::pubsub_domain::domain_config pubsub_conf;
  pubsub_conf.name = "pubsub1";
  pubsub_conf.rocksdb_disabled = true;
  ptest->create<pubsub::pubsub_domain>(pubsub_conf);

  pubsub::longpolling_domain::domain_config longpolling_conf;
  longpolling_conf.name = "longpolling1";
  longpolling_conf.target = "pubsub1";
  longpolling_conf.fire_timer_ms = 10;
  longpolling_conf.auth.disabled = false;
  longpolling_conf.auth.suspend = true;
  longpolling_conf.auth.channel = "auth.sessions";
  longpolling_conf.auth.ttl_s = 3600;
  longpolling_conf.auth.allow_anonymous = false;
  auto plongpolling = ptest->create<pubsub::longpolling_domain>(longpolling_conf);

  ptest->initialize();
  ptest->start();
  drain_io(ptest, std::chrono::milliseconds(100));

  // Без сессии: лог unauthorized, но create проходит (suspend)
  auto cre = std::make_unique<pubsub::request::create>();
  cre->oid = 1001;
  cre->sid = "bad-sid";
  cre->channels.resize(1);
  cre->channels.back().channel = "test1";
  std::string uuid;
  plongpolling->create1(std::move(cre), [&uuid](pubsub::response::create::ptr res) noexcept{
    if ( res != nullptr )
      uuid = res->uuid;
  });
  t << not_equal<assert, size_t>(uuid.size(), 0) << FAS_FL;

  // open/longpoll тоже не режутся
  auto op = std::make_unique<pubsub::request::open>();
  op->uuid = uuid;
  op->oid = 1001;
  op->sid = "bad-sid";
  op->channels.resize(1);
  op->channels.back().channel = "test2";
  bool open_ok = false;
  plongpolling->open(std::move(op), [&open_ok](pubsub::response::open::ptr res) noexcept{
    open_ok = res != nullptr;
  });
  t << equal<assert, bool>(open_ok, true) << FAS_FL;

  shutdown_longpoll(ptest);
}

UNIT(longpolling_auth_public_only, "")
{
  using namespace fas::testing;
  using namespace wfc;

  auto ptest = std::make_shared<wfc::testing_domain>();

  pubsub::pubsub_domain::domain_config pubsub_conf;
  pubsub_conf.name = "pubsub1";
  pubsub_conf.rocksdb_disabled = true;
  auto ppubsub = ptest->create<pubsub::pubsub_domain>(pubsub_conf);

  pubsub::longpolling_domain::domain_config longpolling_conf;
  longpolling_conf.name = "longpolling1";
  longpolling_conf.target = "pubsub1";
  longpolling_conf.fire_timer_ms = 10;
  longpolling_conf.auth.disabled = false;
  longpolling_conf.auth.channel = "auth.sessions";
  longpolling_conf.auth.ttl_s = 3600;
  longpolling_conf.auth.allow_anonymous = true;
  auto plongpolling = ptest->create<pubsub::longpolling_domain>(longpolling_conf);

  ptest->initialize();
  ptest->start();

  auto auth_pub = std::make_unique<pubsub::request::publish>();
  auth_pub->messages.resize(1);
  auth_pub->messages.back().channel = "auth.sessions";
  auth_pub->messages.back().limit = 1000;
  auth_pub->messages.back().lifetime = 3600;
  auth_pub->messages.back().content = std::make_unique<pubsub::content_t>(
    R"({"oid":1001,"sid":"sess1"})"
  );
  ppubsub->publish(std::move(auth_pub), nullptr);
  drain_io(ptest, std::chrono::milliseconds(200));

  // Анонимный агент: приватные не накапливаются (key=0)
  auto cre_anon = std::make_unique<pubsub::request::create>();
  cre_anon->channels.resize(1);
  cre_anon->channels.back().channel = "test1";
  std::string anon_uuid;
  plongpolling->create1(std::move(cre_anon), [&anon_uuid](pubsub::response::create::ptr res) noexcept{
    if ( res != nullptr )
      anon_uuid = res->uuid;
  });
  t << not_equal<assert, size_t>(anon_uuid.size(), 0) << FAS_FL;
  drain_io(ptest, std::chrono::milliseconds(100));

  auto pub_msg = std::make_unique<pubsub::request::publish>();
  pub_msg->messages.resize(1);
  pub_msg->messages.back().channel = "test1";
  pub_msg->messages.back().limit = 10;
  pub_msg->messages.back().lifetime = 10;
  pub_msg->messages.back().identity = "public";
  pub_msg->messages.back().key = 0;
  ppubsub->publish(std::move(pub_msg), nullptr);

  auto priv_msg = std::make_unique<pubsub::request::publish>();
  priv_msg->messages.resize(1);
  priv_msg->messages.back().channel = "test1";
  priv_msg->messages.back().limit = 10;
  priv_msg->messages.back().lifetime = 10;
  priv_msg->messages.back().identity = "private";
  priv_msg->messages.back().key = 1001;
  ppubsub->publish(std::move(priv_msg), nullptr);
  drain_io(ptest, std::chrono::milliseconds(200));

  size_t anon_count = 0;
  std::string anon_identity;
  auto pol_anon = std::make_unique<pubsub::request::longpoll>();
  pol_anon->uuid = anon_uuid;
  plongpolling->longpoll(std::move(pol_anon), [&anon_count, &anon_identity](pubsub::response::longpoll::ptr res) noexcept{
    if ( res != nullptr )
    {
      anon_count = res->messages.size();
      if ( !res->messages.empty() )
        anon_identity = res->messages.front().identity;
    }
  });
  drain_io(ptest, std::chrono::seconds(2));
  t << equal<assert, size_t>(anon_count, 1) << FAS_FL;
  t << equal<assert, std::string>(anon_identity, "public") << FAS_FL;

  // Авторизованный агент: longpoll без oid/sid запрещён
  auto cre = std::make_unique<pubsub::request::create>();
  cre->oid = 1001;
  cre->sid = "sess1";
  cre->channels.resize(1);
  cre->channels.back().channel = "test2";
  std::string uuid;
  plongpolling->create1(std::move(cre), [&uuid](pubsub::response::create::ptr res) noexcept{
    if ( res != nullptr )
      uuid = res->uuid;
  });
  t << not_equal<assert, size_t>(uuid.size(), 0) << FAS_FL;

  auto priv_msg2 = std::make_unique<pubsub::request::publish>();
  priv_msg2->messages.resize(1);
  priv_msg2->messages.back().channel = "test2";
  priv_msg2->messages.back().limit = 10;
  priv_msg2->messages.back().lifetime = 10;
  priv_msg2->messages.back().identity = "private";
  priv_msg2->messages.back().key = 1001;
  ppubsub->publish(std::move(priv_msg2), nullptr);
  drain_io(ptest, std::chrono::milliseconds(200));

  bool anon_longpoll_rejected = false;
  auto pol_bad = std::make_unique<pubsub::request::longpoll>();
  pol_bad->uuid = uuid;
  plongpolling->longpoll(std::move(pol_bad), [&anon_longpoll_rejected](pubsub::response::longpoll::ptr res) noexcept{
    anon_longpoll_rejected = res == nullptr;
  });
  drain_io(ptest, std::chrono::milliseconds(200));
  t << equal<assert, bool>(anon_longpoll_rejected, true) << FAS_FL;

  size_t auth_count = 0;
  std::string auth_identity;
  auto pol_auth = std::make_unique<pubsub::request::longpoll>();
  pol_auth->uuid = uuid;
  pol_auth->oid = 1001;
  pol_auth->sid = "sess1";
  plongpolling->longpoll(std::move(pol_auth), [&auth_count, &auth_identity](pubsub::response::longpoll::ptr res) noexcept{
    if ( res != nullptr )
    {
      auth_count = res->messages.size();
      if ( !res->messages.empty() )
        auth_identity = res->messages.front().identity;
    }
  });
  drain_io(ptest, std::chrono::seconds(2));
  t << equal<assert, size_t>(auth_count, 1) << FAS_FL;
  t << equal<assert, std::string>(auth_identity, "private") << FAS_FL;

  shutdown_longpoll(ptest);
}

UNIT(longpolling_auth_grace, "")
{
  using namespace fas::testing;
  using namespace wfc;

  auto ptest = std::make_shared<wfc::testing_domain>();

  pubsub::pubsub_domain::domain_config pubsub_conf;
  pubsub_conf.name = "pubsub1";
  pubsub_conf.rocksdb_disabled = true;
  auto ppubsub = ptest->create<pubsub::pubsub_domain>(pubsub_conf);

  pubsub::longpolling_domain::domain_config longpolling_conf;
  longpolling_conf.name = "longpolling1";
  longpolling_conf.target = "pubsub1";
  longpolling_conf.fire_timer_ms = 10;
  longpolling_conf.auth.disabled = false;
  longpolling_conf.auth.channel = "auth.sessions";
  longpolling_conf.auth.ttl_s = 3600;
  longpolling_conf.auth.grace_period_s = 2;
  longpolling_conf.auth.allow_anonymous = false;
  auto plongpolling = ptest->create<pubsub::longpolling_domain>(longpolling_conf);

  ptest->initialize();
  ptest->start();

  drain_io(ptest, std::chrono::milliseconds(50));

  // В grace: анонимный create отклоняется (allow_anonymous=false)
  auto cre_anon = std::make_unique<pubsub::request::create>();
  cre_anon->channels.resize(1);
  cre_anon->channels.back().channel = "test1";
  bool anon_rejected = false;
  plongpolling->create1(std::move(cre_anon), [&anon_rejected](pubsub::response::create::ptr res) noexcept{
    anon_rejected = res == nullptr;
  });
  t << equal<assert, bool>(anon_rejected, true) << FAS_FL;

  // В grace: oid/sid ещё не в store — create проходит
  auto cre_grace = std::make_unique<pubsub::request::create>();
  cre_grace->oid = 1001;
  cre_grace->sid = "sess1";
  cre_grace->channels.resize(1);
  cre_grace->channels.back().channel = "test1";
  std::string uuid;
  plongpolling->create1(std::move(cre_grace), [&uuid](pubsub::response::create::ptr res) noexcept{
    if ( res != nullptr )
      uuid = res->uuid;
  });
  t << not_equal<assert, size_t>(uuid.size(), 0) << FAS_FL;

  std::this_thread::sleep_for(std::chrono::seconds(3));

  // После grace: та же пара, но ещё не в store — отклоняется
  auto cre_late = std::make_unique<pubsub::request::create>();
  cre_late->oid = 1002;
  cre_late->sid = "sess2";
  cre_late->channels.resize(1);
  cre_late->channels.back().channel = "test2";
  bool late_rejected = false;
  plongpolling->create1(std::move(cre_late), [&late_rejected](pubsub::response::create::ptr res) noexcept{
    late_rejected = res == nullptr;
  });
  t << equal<assert, bool>(late_rejected, true) << FAS_FL;

  shutdown_longpoll(ptest);
}

UNIT(longpolling_auth_grace_mismatch, "")
{
  using namespace fas::testing;
  using namespace wfc;

  auto ptest = std::make_shared<wfc::testing_domain>();

  pubsub::pubsub_domain::domain_config pubsub_conf;
  pubsub_conf.name = "pubsub1";
  pubsub_conf.rocksdb_disabled = true;
  auto ppubsub = ptest->create<pubsub::pubsub_domain>(pubsub_conf);

  pubsub::longpolling_domain::domain_config longpolling_conf;
  longpolling_conf.name = "longpolling1";
  longpolling_conf.target = "pubsub1";
  longpolling_conf.fire_timer_ms = 10;
  longpolling_conf.auth.disabled = false;
  longpolling_conf.auth.channel = "auth.sessions";
  longpolling_conf.auth.ttl_s = 3600;
  longpolling_conf.auth.grace_period_s = 10;
  auto plongpolling = ptest->create<pubsub::longpolling_domain>(longpolling_conf);

  ptest->initialize();
  ptest->start();

  auto auth_pub = std::make_unique<pubsub::request::publish>();
  auth_pub->messages.resize(1);
  auth_pub->messages.back().channel = "auth.sessions";
  auth_pub->messages.back().limit = 1000;
  auth_pub->messages.back().lifetime = 3600;
  auth_pub->messages.back().content = std::make_unique<pubsub::content_t>(
    R"({"oid":1001,"sid":"sess1"})"
  );
  ppubsub->publish(std::move(auth_pub), nullptr);

  auto auth_pub2 = std::make_unique<pubsub::request::publish>();
  auth_pub2->messages.resize(1);
  auth_pub2->messages.back().channel = "auth.sessions";
  auth_pub2->messages.back().limit = 1000;
  auth_pub2->messages.back().lifetime = 3600;
  auth_pub2->messages.back().content = std::make_unique<pubsub::content_t>(
    R"({"oid":1002,"sid":"sess2"})"
  );
  ppubsub->publish(std::move(auth_pub2), nullptr);
  drain_io(ptest, std::chrono::milliseconds(200));

  auto cre = std::make_unique<pubsub::request::create>();
  cre->oid = 1001;
  cre->sid = "sess1";
  cre->channels.resize(1);
  cre->channels.back().channel = "test1";
  std::string uuid;
  plongpolling->create1(std::move(cre), [&uuid](pubsub::response::create::ptr res) noexcept{
    if ( res != nullptr )
      uuid = res->uuid;
  });
  t << not_equal<assert, size_t>(uuid.size(), 0) << FAS_FL;

  // В grace: валидная сессия, но oid не совпадает с агентом — отклоняется
  bool longpoll_rejected = false;
  auto pol = std::make_unique<pubsub::request::longpoll>();
  pol->uuid = uuid;
  pol->oid = 1002;
  pol->sid = "sess2";
  plongpolling->longpoll(std::move(pol), [&longpoll_rejected](pubsub::response::longpoll::ptr res) noexcept{
    longpoll_rejected = res == nullptr;
  });
  drain_io(ptest, std::chrono::milliseconds(200));
  t << equal<assert, bool>(longpoll_rejected, true) << FAS_FL;

  // В grace: пара в store, но sid неверен — отклоняется
  bool bad_sid_rejected = false;
  auto cre_bad = std::make_unique<pubsub::request::create>();
  cre_bad->oid = 1001;
  cre_bad->sid = "bad-sid";
  cre_bad->channels.resize(1);
  cre_bad->channels.back().channel = "test2";
  plongpolling->create1(std::move(cre_bad), [&bad_sid_rejected](pubsub::response::create::ptr res) noexcept{
    bad_sid_rejected = res == nullptr;
  });
  t << equal<assert, bool>(bad_sid_rejected, true) << FAS_FL;

  shutdown_longpoll(ptest);
}

UNIT(longpolling_auth_grace_anonymous, "")
{
  using namespace fas::testing;
  using namespace wfc;

  auto ptest = std::make_shared<wfc::testing_domain>();

  pubsub::pubsub_domain::domain_config pubsub_conf;
  pubsub_conf.name = "pubsub1";
  pubsub_conf.rocksdb_disabled = true;
  auto ppubsub = ptest->create<pubsub::pubsub_domain>(pubsub_conf);

  pubsub::longpolling_domain::domain_config longpolling_conf;
  longpolling_conf.name = "longpolling1";
  longpolling_conf.target = "pubsub1";
  longpolling_conf.fire_timer_ms = 10;
  longpolling_conf.auth.disabled = false;
  longpolling_conf.auth.channel = "auth.sessions";
  longpolling_conf.auth.ttl_s = 3600;
  longpolling_conf.auth.grace_period_s = 2;
  longpolling_conf.auth.allow_anonymous = true;
  auto plongpolling = ptest->create<pubsub::longpolling_domain>(longpolling_conf);

  ptest->initialize();
  ptest->start();

  drain_io(ptest, std::chrono::milliseconds(50));

  std::string anon_uuid;
  auto cre_anon = std::make_unique<pubsub::request::create>();
  cre_anon->channels.resize(1);
  cre_anon->channels.back().channel = "test1";
  plongpolling->create1(std::move(cre_anon), [&anon_uuid](pubsub::response::create::ptr res) noexcept{
    if ( res != nullptr )
      anon_uuid = res->uuid;
  });
  t << not_equal<assert, size_t>(anon_uuid.size(), 0) << FAS_FL;

  auto cre_grace = std::make_unique<pubsub::request::create>();
  cre_grace->oid = 1001;
  cre_grace->sid = "sess1";
  cre_grace->channels.resize(1);
  cre_grace->channels.back().channel = "test1";
  std::string uuid;
  plongpolling->create1(std::move(cre_grace), [&uuid](pubsub::response::create::ptr res) noexcept{
    if ( res != nullptr )
      uuid = res->uuid;
  });
  t << not_equal<assert, size_t>(uuid.size(), 0) << FAS_FL;

  std::this_thread::sleep_for(std::chrono::seconds(3));

  bool late_rejected = false;
  auto cre_late = std::make_unique<pubsub::request::create>();
  cre_late->oid = 1002;
  cre_late->sid = "sess2";
  cre_late->channels.resize(1);
  cre_late->channels.back().channel = "test2";
  plongpolling->create1(std::move(cre_late), [&late_rejected](pubsub::response::create::ptr res) noexcept{
    late_rejected = res == nullptr;
  });
  t << equal<assert, bool>(late_rejected, true) << FAS_FL;

  shutdown_longpoll(ptest);
}

UNIT(longpolling_publish_prefix, "")
{
  using namespace fas::testing;
  using namespace wfc;

  auto ptest = std::make_shared<wfc::testing_domain>();

  pubsub::pubsub_domain::domain_config pubsub_conf;
  pubsub_conf.name = "pubsub1";
  pubsub_conf.rocksdb_disabled = true;
  auto ppubsub = ptest->create<pubsub::pubsub_domain>(pubsub_conf);

  pubsub::longpolling_domain::domain_config default_conf;
  default_conf.name = "longpolling-default";
  default_conf.target = "pubsub1";
  default_conf.fire_timer_ms = 10;
  auto plp_default = ptest->create<pubsub::longpolling_domain>(default_conf);

  pubsub::longpolling_domain::domain_config wildcard_conf;
  wildcard_conf.name = "longpolling-wildcard";
  wildcard_conf.target = "pubsub1";
  wildcard_conf.fire_timer_ms = 10;
  wildcard_conf.publish.enabled = true;
  wildcard_conf.publish.channel_prefixes = {"*"};
  auto plp_wildcard = ptest->create<pubsub::longpolling_domain>(wildcard_conf);

  pubsub::longpolling_domain::domain_config prefix_conf;
  prefix_conf.name = "longpolling-prefix";
  prefix_conf.target = "pubsub1";
  prefix_conf.fire_timer_ms = 10;
  prefix_conf.publish.enabled = true;
  prefix_conf.publish.channel_prefixes = {"client."};
  auto plp_prefix = ptest->create<pubsub::longpolling_domain>(prefix_conf);

  pubsub::longpolling_domain::domain_config disabled_conf;
  disabled_conf.name = "longpolling-disabled";
  disabled_conf.target = "pubsub1";
  disabled_conf.fire_timer_ms = 10;
  disabled_conf.publish.enabled = false;
  disabled_conf.publish.channel_prefixes = {"*"};
  auto plp_disabled = ptest->create<pubsub::longpolling_domain>(disabled_conf);

  ptest->initialize();
  ptest->start();

  bool default_denied = false;
  auto default_pub = std::make_unique<pubsub::request::publish>();
  default_pub->messages.resize(1);
  default_pub->messages.back().channel = "any.channel";
  default_pub->messages.back().limit = 10;
  default_pub->messages.back().lifetime = 10;
  plp_default->publish(
    std::move(default_pub),
    [&default_denied](pubsub::response::publish::ptr res) noexcept { default_denied = res == nullptr; }
  );
  t << equal<assert, bool>(default_denied, true) << FAS_FL;

  bool flag_denied = false;
  auto flag_pub = std::make_unique<pubsub::request::publish>();
  flag_pub->messages.resize(1);
  flag_pub->messages.back().channel = "any.channel";
  flag_pub->messages.back().limit = 10;
  flag_pub->messages.back().lifetime = 10;
  plp_disabled->publish(
    std::move(flag_pub),
    [&flag_denied](pubsub::response::publish::ptr res) noexcept { flag_denied = res == nullptr; }
  );
  t << equal<assert, bool>(flag_denied, true) << FAS_FL;

  bool wildcard_ok = false;
  auto wildcard_pub = std::make_unique<pubsub::request::publish>();
  wildcard_pub->messages.resize(1);
  wildcard_pub->messages.back().channel = "any.channel";
  wildcard_pub->messages.back().limit = 10;
  wildcard_pub->messages.back().lifetime = 10;
  plp_wildcard->publish(
    std::move(wildcard_pub),
    [&wildcard_ok](pubsub::response::publish::ptr res) noexcept { wildcard_ok = res != nullptr; }
  );
  t << equal<expect, bool>(wildcard_ok, true) << FAS_FL;

  bool rejected = false;
  auto bad_pub = std::make_unique<pubsub::request::publish>();
  bad_pub->messages.resize(1);
  bad_pub->messages.back().channel = "test1";
  bad_pub->messages.back().limit = 10;
  bad_pub->messages.back().lifetime = 10;
  plp_prefix->publish(
    std::move(bad_pub),
    [&rejected](pubsub::response::publish::ptr res) noexcept { rejected = res == nullptr; }
  );
  t << equal<assert, bool>(rejected, true) << FAS_FL;

  size_t ok_count = 0;
  auto good_pub = std::make_unique<pubsub::request::publish>();
  good_pub->messages.resize(1);
  good_pub->messages.back().channel = "client.test1";
  good_pub->messages.back().limit = 10;
  good_pub->messages.back().lifetime = 10;
  plp_prefix->publish(
    std::move(good_pub),
    [&ok_count](pubsub::response::publish::ptr res) noexcept { if ( res != nullptr ) ++ok_count; }
  );
  t << equal<expect, size_t>(ok_count, 1) << FAS_FL;

  shutdown_longpoll(ptest);
}

UNIT(longpolling_open_flow, "")
{
  using namespace fas::testing;
  using namespace wfc;

  auto ptest = std::make_shared<wfc::testing_domain>();

  pubsub::pubsub_domain::domain_config pubsub_conf;
  pubsub_conf.name = "pubsub1";
  pubsub_conf.rocksdb_disabled = true;
  auto ppubsub = ptest->create<pubsub::pubsub_domain>(pubsub_conf);

  pubsub::longpolling_domain::domain_config longpolling_conf;
  longpolling_conf.name = "longpolling1";
  longpolling_conf.target = "pubsub1";
  longpolling_conf.fire_timer_ms = 10;
  longpolling_conf.auth.disabled = false;
  longpolling_conf.auth.channel = "auth.sessions";
  longpolling_conf.auth.ttl_s = 3600;
  auto plongpolling = ptest->create<pubsub::longpolling_domain>(longpolling_conf);

  ptest->initialize();
  ptest->start();

  auto auth_pub = std::make_unique<pubsub::request::publish>();
  auth_pub->messages.resize(1);
  auth_pub->messages.back().channel = "auth.sessions";
  auth_pub->messages.back().limit = 1000;
  auth_pub->messages.back().lifetime = 3600;
  auth_pub->messages.back().content = std::make_unique<pubsub::content_t>(
    R"({"oid":1001,"sid":"sess1"})"
  );
  ppubsub->publish(std::move(auth_pub), nullptr);
  drain_io(ptest, std::chrono::milliseconds(200));

  // Реальный клиент: create без channels, затем open.
  auto cre = std::make_unique<pubsub::request::create>();
  cre->oid = 1001;
  cre->sid = "sess1";
  cre->key = 1001;
  cre->agent_lifetime = 630;
  cre->longpoll_timeout = 600;
  std::string uuid;
  plongpolling->create1(std::move(cre), [&uuid](pubsub::response::create::ptr res) noexcept{
    if ( res != nullptr )
      uuid = res->uuid;
  });
  t << not_equal<assert, size_t>(uuid.size(), 0) << FAS_FL;

  auto opn = std::make_unique<pubsub::request::open>();
  opn->uuid = uuid;
  opn->oid = 1001;
  opn->sid = "sess1";
  opn->key = 1001;
  opn->channels.resize(1);
  opn->channels.back().channel = "mamba.test.channel.1001";
  opn->channels.back().cursor = 0;
  opn->channels.back().min_size = 1;
  opn->channels.back().max_size = 10;
  bool open_ok = false;
  plongpolling->open(std::move(opn), [&open_ok](pubsub::response::open::ptr res) noexcept{
    open_ok = res != nullptr;
  });
  t << equal<assert, bool>(open_ok, true) << FAS_FL;

  drain_io(ptest, std::chrono::milliseconds(200));

  auto priv_msg = std::make_unique<pubsub::request::publish>();
  priv_msg->messages.resize(1);
  priv_msg->messages.back().channel = "mamba.test.channel.1001";
  priv_msg->messages.back().limit = 10;
  priv_msg->messages.back().lifetime = 60;
  priv_msg->messages.back().key = 1001;
  priv_msg->messages.back().content = std::make_unique<pubsub::content_t>("payload");
  ppubsub->publish(std::move(priv_msg), nullptr);
  drain_io(ptest, std::chrono::milliseconds(200));

  size_t msg_count = 0;
  auto pol = std::make_unique<pubsub::request::longpoll>();
  pol->uuid = uuid;
  pol->oid = 1001;
  pol->sid = "sess1";
  plongpolling->longpoll(std::move(pol), [&msg_count](pubsub::response::longpoll::ptr res) noexcept{
    if ( res != nullptr )
      msg_count = res->messages.size();
  });
  drain_io(ptest, std::chrono::seconds(2));
  t << equal<assert, size_t>(msg_count, 1) << FAS_FL;

  shutdown_longpoll(ptest);
}

}

BEGIN_SUITE(longpoll, "")
  ADD_UNIT(longpolling1)
  ADD_UNIT(longpolling_auth)
  ADD_UNIT(longpolling_auth_suspend)
  ADD_UNIT(longpolling_auth_public_only)
  ADD_UNIT(longpolling_auth_grace)
  ADD_UNIT(longpolling_auth_grace_mismatch)
  ADD_UNIT(longpolling_auth_grace_anonymous)
  ADD_UNIT(longpolling_publish_prefix)
  ADD_UNIT(longpolling_open_flow)
END_SUITE(longpoll)

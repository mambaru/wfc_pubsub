#include <fas/testing.hpp>
#include <http/domain/http_domain.hpp>
#include <pubsub/domain/pubsub_domain.hpp>
#include <longpolling/domain/longpolling_domain.hpp>
#include <longpolling/service/longpolling_service.hpp>
#include <wfc/testing/testing_domain.hpp>
#include "wjson/_json.hpp"

#include <chrono>
#include <sstream>

namespace{

std::unique_ptr<iow::io::data_type> make_buf(const std::string& str)
{
  return std::make_unique<iow::io::data_type>( str.begin(), str.end());
}

std::string make_http_post(const std::string& path, const std::string& query, const std::string& body)
{
  std::string target = path;
  if ( !query.empty() )
    target += "?" + query;
  std::ostringstream req;
  req << "POST " << target << " HTTP/1.1\r\n"
      << "Host: localhost\r\n"
      << "Content-Type: application/json\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "\r\n"
      << body;
  return req.str();
}

std::string http_body(const std::string& raw)
{
  const auto pos = raw.find("\r\n\r\n");
  if ( pos == std::string::npos )
    return std::string();
  return raw.substr(pos + 4);
}

int http_status_code(const std::string& raw)
{
  const auto end = raw.find("\r\n");
  if ( end == std::string::npos )
    return 0;
  const std::string status_line = raw.substr(0, end);
  const auto sp1 = status_line.find(' ');
  if ( sp1 == std::string::npos )
    return 0;
  const auto sp2 = status_line.find(' ', sp1 + 1);
  if ( sp2 == std::string::npos )
    return 0;
  return std::stoi(status_line.substr(sp1 + 1, sp2 - sp1 - 1));
}

std::string json_field(const std::string& json, const char* field)
{
  const std::string key = std::string("\"") + field + "\":\"";
  const auto beg = json.find(key);
  if ( beg == std::string::npos )
    return std::string();
  const auto start = beg + key.size();
  const auto end = json.find('"', start);
  if ( end == std::string::npos )
    return std::string();
  return json.substr(start, end - start);
}

void drain_io(const std::shared_ptr<wfc::testing_domain>& ptest, std::chrono::milliseconds ms)
{
  ptest->global()->io_context.run_for(ms);
  ptest->global()->io_context.restart();
}

void shutdown_http_chain(
  const std::shared_ptr<wfc::testing_domain>& ptest,
  const std::shared_ptr<wfc::http_domain>& phttp,
  wfc::iinstance::io_id_t io_id)
{
  drain_io(ptest, std::chrono::milliseconds(200));
  if ( phttp != nullptr )
    phttp->unreg_io(io_id);
  ptest->stop();
}

UNIT(pubsub1, "")
{
  using namespace fas::testing;
  using namespace wfc::pubsub;

  t << nothing;

  auto ptest = std::make_shared<wfc::testing_domain>();
  pubsub_domain::domain_config conf;
  conf.name = "pubsub1";
  conf.rocksdb_disabled = true;
  auto ppubsub = ptest->create<pubsub_domain>(conf);

  ptest->initialize();
  ptest->start();

  size_t subscribe_count = 0;
  size_t publish_count = 0;

  auto sub = std::make_unique<request::subscribe>();
  sub->channels.resize(1);
  sub->channels.back().channel = "test.channel";
  ppubsub->subscribe(
    std::move(sub),
    [&subscribe_count](response::subscribe::ptr) noexcept { ++subscribe_count; },
    0,
    std::weak_ptr<isubscriber>()
  );

  auto pub = std::make_unique<request::publish>();
  pub->messages.resize(1);
  pub->messages.back().channel = "test.channel";
  pub->messages.back().content = std::make_unique<content_t>("test content");
  ppubsub->publish(
    std::move(pub),
    [&publish_count](response::publish::ptr) noexcept { ++publish_count; }
  );

  t << equal<expect, size_t>(subscribe_count, 1) << FAS_FL;
  t << equal<expect, size_t>(publish_count, 1) << FAS_FL;

  ptest->stop();

  ptest.reset();
  ppubsub.reset();
}


UNIT(http1, "")
{
  using namespace fas::testing;
  using namespace wfc;
  using namespace wjson::literals;
  //typedef iinstance::data_type data_type;
  typedef iinstance::data_ptr data_ptr;


  auto ptest = std::make_shared<wfc::testing_domain>();

  http_domain::domain_config conf;
  conf.name = "http1";
  auto phttp = ptest->create<http_domain>(conf);

  ptest->initialize();
  ptest->start();

  phttp->reg_io(1, std::weak_ptr<iinterface>() );

  auto handler = [](data_ptr) noexcept{};
  phttp->perform_io(make_buf("GET /index.ph"), 1, handler);
  phttp->perform_io(make_buf("p?val=1&val=2 HT"), 1, handler);
  phttp->perform_io(make_buf("TP/1.1\r\n"), 1, handler);
  phttp->perform_io(make_buf("Host: example.com\r\n"), 1, handler);
  phttp->perform_io(make_buf("\r\n"), 1, handler);

  ptest->stop();
  phttp->stop();

  t << nothing;

  ptest.reset();
  phttp.reset();
}

UNIT(http_longpoll_chain, "")
{
  using namespace fas::testing;
  using namespace wfc;
  using namespace wjson::literals;

  const iinstance::io_id_t io_id = 1;

  auto ptest = std::make_shared<wfc::testing_domain>();

  pubsub::pubsub_domain::domain_config pubsub_conf;
  pubsub_conf.name = "pubsub1";
  pubsub_conf.rocksdb_disabled = true;
  auto ppubsub = ptest->create<pubsub::pubsub_domain>(pubsub_conf);

  pubsub::longpolling_domain::domain_config longpolling_conf;
  longpolling_conf.name = "longpolling1";
  longpolling_conf.target = "pubsub1";
  longpolling_conf.fire_timer_ms = 10;
  auto plongpolling = ptest->create<pubsub::longpolling_domain>(longpolling_conf);

  typedef wfc::jsonrpc::service<pubsub::longpolling_service_method_list> longpolling_service;
  longpolling_service::domain_config lp_serv_conf;
  lp_serv_conf.name = "longpolling-service1";
  lp_serv_conf.target_name = "longpolling1";
  auto plp_service = ptest->create<longpolling_service>(lp_serv_conf);

  http_domain::domain_config http_conf;
  http_conf.name = "http1";
  http_conf.target = "longpolling-service1";
  http_conf.jsonrpc_path = "/comet";
  auto phttp = ptest->create<http_domain>(http_conf);

  ptest->initialize();
  ptest->start();

  phttp->reg_io(io_id, std::weak_ptr<iinterface>());

  auto pub = std::make_unique<pubsub::request::publish>();
  pub->messages.resize(1);
  pub->messages.back().channel = "test1";
  pub->messages.back().limit = 10;
  pub->messages.back().lifetime = 10;
  ppubsub->publish(std::move(pub), nullptr);

  const std::string create_body =
    "{'method':'create','params':{'channels':[{'channel':'test1'}]},'id':1}"_json;
  std::string create_resp;
  phttp->perform_io(
    make_buf(make_http_post("/comet", "", create_body)),
    io_id,
    [&create_resp](iinstance::data_ptr d) noexcept
    {
      if ( d != nullptr )
        create_resp.assign(d->begin(), d->end());
    }
  );
  drain_io(ptest, std::chrono::milliseconds(500));

  const std::string uuid = json_field(http_body(create_resp), "uuid");
  t << not_equal<assert, size_t>(uuid.size(), 0) << FAS_FL;
  t << message("uuid:") << uuid;

  const std::string longpoll_body =
    "{'method':'longpoll','params':{},'id':2}"_json;
  const std::string longpoll_query = "uuid=" + uuid;
  std::string longpoll_resp;
  phttp->perform_io(
    make_buf(make_http_post("/comet", longpoll_query, longpoll_body)),
    io_id,
    [&longpoll_resp](iinstance::data_ptr d) noexcept
    {
      if ( d != nullptr )
        longpoll_resp.assign(d->begin(), d->end());
    }
  );
  drain_io(ptest, std::chrono::seconds(2));

  const std::string channel = json_field(http_body(longpoll_resp), "channel");
  t << equal<assert, std::string>(channel, "test1") << FAS_FL;

  shutdown_http_chain(ptest, phttp, io_id);

  ptest.reset();
  ppubsub.reset();
  plongpolling.reset();
  plp_service.reset();
  phttp.reset();
}

UNIT(http_longpoll_auth, "")
{
  using namespace fas::testing;
  using namespace wfc;
  using namespace wjson::literals;

  const iinstance::io_id_t io_id = 1;

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

  typedef wfc::jsonrpc::service<pubsub::longpolling_service_method_list> longpolling_service;
  longpolling_service::domain_config lp_serv_conf;
  lp_serv_conf.name = "longpolling-service1";
  lp_serv_conf.target_name = "longpolling1";
  auto plp_service = ptest->create<longpolling_service>(lp_serv_conf);

  http_domain::domain_config http_conf;
  http_conf.name = "http1";
  http_conf.target = "longpolling-service1";
  http_conf.jsonrpc_path = "/comet";
  auto phttp = ptest->create<http_domain>(http_conf);

  ptest->initialize();
  ptest->start();

  phttp->reg_io(io_id, std::weak_ptr<iinterface>());

  const std::string create_body =
    "{'method':'create','params':{'channels':[{'channel':'test1'}]},'id':1}"_json;
  std::string create_denied_resp;
  phttp->perform_io(
    make_buf(make_http_post("/comet", "", create_body)),
    io_id,
    [&create_denied_resp](iinstance::data_ptr d) noexcept
    {
      if ( d != nullptr )
        create_denied_resp.assign(d->begin(), d->end());
    }
  );
  drain_io(ptest, std::chrono::milliseconds(500));
  t << equal<assert, int>(http_status_code(create_denied_resp), 403) << FAS_FL;

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

  auto data_pub = std::make_unique<pubsub::request::publish>();
  data_pub->messages.resize(1);
  data_pub->messages.back().channel = "test1";
  data_pub->messages.back().limit = 10;
  data_pub->messages.back().lifetime = 10;
  ppubsub->publish(std::move(data_pub), nullptr);

  const std::string auth_query = "oid=1001&sid=sess1";
  std::string create_ok_resp;
  phttp->perform_io(
    make_buf(make_http_post("/comet", auth_query, create_body)),
    io_id,
    [&create_ok_resp](iinstance::data_ptr d) noexcept
    {
      if ( d != nullptr )
        create_ok_resp.assign(d->begin(), d->end());
    }
  );
  drain_io(ptest, std::chrono::milliseconds(500));

  const std::string uuid = json_field(http_body(create_ok_resp), "uuid");
  t << not_equal<assert, size_t>(uuid.size(), 0) << FAS_FL;

  const std::string longpoll_body =
    "{'method':'longpoll','params':{},'id':2}"_json;
  const std::string longpoll_query = auth_query + "&uuid=" + uuid;
  std::string longpoll_resp;
  phttp->perform_io(
    make_buf(make_http_post("/comet", longpoll_query, longpoll_body)),
    io_id,
    [&longpoll_resp](iinstance::data_ptr d) noexcept
    {
      if ( d != nullptr )
        longpoll_resp.assign(d->begin(), d->end());
    }
  );
  drain_io(ptest, std::chrono::seconds(2));

  const std::string channel = json_field(http_body(longpoll_resp), "channel");
  t << equal<assert, std::string>(channel, "test1") << FAS_FL;

  shutdown_http_chain(ptest, phttp, io_id);

  ptest.reset();
  ppubsub.reset();
  plongpolling.reset();
  plp_service.reset();
  phttp.reset();
}


}

BEGIN_SUITE(package, "")
  ADD_UNIT(pubsub1)
  ADD_UNIT(http1)
  ADD_UNIT(http_longpoll_chain)
  ADD_UNIT(http_longpoll_auth)
END_SUITE(package)

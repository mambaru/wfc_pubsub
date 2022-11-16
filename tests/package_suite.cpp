#include <fas/testing.hpp>
#include <pubsub/domain/pubsub_domain.hpp>
#include <pubsub/service/pubsub_service.hpp>
#include <comet_adapter/domain/comet_adapter_domain.hpp>
#include <comet_adapter/service/comet_adapter_service.hpp>
#include <wfc/testing/testing_domain.hpp>
#include "wjson/_json.hpp"

namespace{

UNIT(pubsub1, "")
{
  using namespace fas::testing;
  using namespace wfc::pubsub;
  using namespace wjson::literals;

  t << nothing;

  auto ptest = std::make_shared<wfc::testing_domain>();
  pubsub_domain::domain_config conf;
  conf.name = "pubsub1";
  auto ppubsub = ptest->create<pubsub_domain>(conf);

  typedef wfc::jsonrpc::service<service_method_list, pubsub_interface> pubsub_service;
  pubsub_service::domain_config serv_conf;

  serv_conf.name = "pubsub-service1";
  serv_conf.target_name = "pubsub1";
  auto pservice = ptest->create<pubsub_service>(serv_conf);

  ptest->initialize();
  ptest->start();

  typedef ipubsub::data_type data_type;
  typedef ipubsub::data_ptr data_ptr;

  size_t subscribe_count = 0;
  size_t publish_count = 0;

  std::string req_str="{'method':'subscribe','params':{'channels':[{'channel':'test.channel'}]},'id':1}"_json;

  pservice->perform_io(
    std::make_unique<data_type>(req_str.begin(), req_str.end()),
    1,
    [&](data_ptr d) noexcept
    {
      t << fas::testing::message("JSONRPC Response (subscribe):") << d;
      ++subscribe_count;
    }
  );

  req_str="{'method':'publish','params':{'channel':'test.channel', 'content':'test content'},'id':1}"_json;

  pservice->perform_io(
    std::make_unique<data_type>(req_str.begin(), req_str.end()),
    1,
    [&](data_ptr d) noexcept
    {
      t << fas::testing::message("JSONRPC Response (publish:") << d;
      ++publish_count;
    }
  );

  t << equal<expect, size_t>(subscribe_count, 1) << "TODO: реальзовать JSON для pubsub (пустая заглушка)" << FAS_FL;
  t << equal<expect, size_t>(publish_count, 1) << FAS_FL;

  ptest->stop();
  ppubsub->stop();

  ptest.reset();
  ppubsub.reset();
}

UNIT(pubsub2, "")
{
  using namespace fas::testing;
  using namespace wfc;
  using namespace wjson::literals;

  t << nothing;

  auto ptest = std::make_shared<wfc::testing_domain>();

  pubsub::pubsub_domain::domain_config conf;
  conf.name = "pubsub1";
  auto ppubsub = ptest->create<pubsub::pubsub_domain>(conf);

  comet_adapter::comet_adapter_domain::domain_config conf2;
  conf2.name = "adapter1";
  conf2.target = "pubsub1";
  auto padapter = ptest->create<comet_adapter::comet_adapter_domain>(conf2);

  typedef wfc::jsonrpc::service<comet_adapter::service_method_list, comet_adapter::comet_adapter_interface> comet_adapter_service;
  comet_adapter_service::domain_config serv_conf;

  serv_conf.name = "pubsub-service1";
  serv_conf.target_name = "adapter1";
  auto pservice = ptest->create<comet_adapter_service>(serv_conf);

  ptest->initialize();
  ptest->start();


  typedef pubsub::ipubsub::data_type data_type;
  typedef pubsub::ipubsub::data_ptr data_ptr;

  size_t subscribe_count = 0;
  size_t publish_count = 0;

  std::string req_str="{'method':'subscribe','params':'test.channel','id':1}"_json;

  //pservice->reg_io(1, std::weak_ptr<wfc::iinterface>() );
  pservice->perform_io(
    std::make_unique<data_type>(req_str.begin(), req_str.end()),
    1,
    [&](data_ptr d) noexcept
    {
      t << fas::testing::message("JSONRPC Response (subscribe):") << d;
      ++subscribe_count;
    }
  );

  req_str="{'method':'publish','params':{'channel':'test.channel', 'content':'test content'},'id':1}"_json;

  pservice->perform_io(
    std::make_unique<data_type>(req_str.begin(), req_str.end()),
    1,
    [&](data_ptr d) noexcept
    {
      t << fas::testing::message("JSONRPC Response: (publish)") << d;
      ++publish_count;
    }
  );


  t << equal<expect, size_t>(subscribe_count, 1) << FAS_FL;
  t << equal<expect, size_t>(publish_count, 1) << FAS_FL;

  ptest->stop();
  ppubsub->stop();

  ptest.reset();
  ppubsub.reset();
}

}

BEGIN_SUITE(package, "")
  ADD_UNIT(pubsub1)
  ADD_UNIT(pubsub2)
END_SUITE(package)

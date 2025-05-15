#include <iostream>
#include <longpolling/longpolling.hpp>

using namespace ::wfc::pubsub;

int main(int /*argc*/, char* /*argv*/[])
{
  {
    const int AGENT_TEST = 1000;
    const int CHANNEL_TEST = 100;
    const int MESSAGE_TEST = 100;

    longpolling_options lpo;
    longpolling lp(lpo);

    for (int i = 0; i < AGENT_TEST; ++i)
    {
      agent_options ao;
      ao.uuid = "UUID" + std::to_string(i);
      if ( !lp.create(ao) )
        abort();

      agent_params ap;
      ap.uuid = ao.uuid;
      channel_list_t chl;
      for ( int j = 0 ; j < CHANNEL_TEST ; ++j)
      {
        channel_params chp;
        chp.max_size = MESSAGE_TEST;
        chp.channel = "channel"+std::to_string(j);
        chl.push_back(chp);
      }
      if ( !lp.open(ap, chl) )
        abort();
    }

    std::cout << "Наполнение сообщениями" << std::endl;

    for (int i = 0; i < MESSAGE_TEST; ++i)
    {
      std::cout << i << " \r";
      std::cout.flush();
      for ( int j = 0 ; j < CHANNEL_TEST ; ++j)
      {
        std::string channel = "channel" + std::to_string(j);
        message m;
//        m.content = std::make_unique<content_t>("message" + std::to_string(j));
        m.limit = 1000000000;
        m.lifetime = 10000000;
        lp.push(channel, m);
      }
    }

    std::cout << "Enter для очистки " << std::endl;
    std::cin.get();
    lp.clear();
    std::cout << "Enter для удаления" << std::endl;
    std::cin.get();
  }
  std::cout << "Enter для выхода" << std::endl;
  std::cin.get();
  return 0;
}

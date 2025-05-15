#include <iostream>
#include <longpolling/longpolling.hpp>

using namespace ::wfc::pubsub;

int main(int /*argc*/, char* /*argv*/[])
{
  {
    const int AGENT_TEST = 2000;
    const int CHANNEL_TEST = 100 * AGENT_TEST;
    const int MESSAGE_TEST = 100;

    message_queue lp;

    std::cout << "Наполнение сообщениями" << std::endl;

    for (int i = 0; i < MESSAGE_TEST; ++i)
    {
      std::cout << i << " \r";
      std::cout.flush();
      for ( int j = 0 ; j < CHANNEL_TEST ; ++j)
      {
        message m;
        //m.content = std::make_unique<content_t>("message" + std::to_string(j));
        m.limit = 1000000000;
        m.lifetime = 10000000;
        lp.push( std::move(m) );
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


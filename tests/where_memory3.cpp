#include <iostream>
#include <longpolling/longpolling.hpp>

using namespace ::wfc::pubsub;



/*
 *
 *
 */
int main(int /*argc*/, char* /*argv*/[])
{
  if (true)
  {
    typedef std::vector< std::unique_ptr<uint64_t> > queue_type;

    const uint64_t AGENT_TEST = 2000;
    const uint64_t CHANNEL_TEST = 100 * AGENT_TEST;
    const uint64_t MESSAGE_TEST = 100;

    queue_type lp;

    std::cout << "Наполнение сообщениями" << std::endl;

    for (uint64_t i = 0; i < MESSAGE_TEST; ++i)
    {
      std::cout << i << " \r";
      std::cout.flush();
      for ( uint64_t j = 0 ; j < CHANNEL_TEST ; ++j)
      {

        lp.push_back( std::make_unique<uint64_t>(i*j) );
      }
    }

    std::cout << "Enter для очистки " << std::endl;
    std::cin.get();
    for (auto& m : lp)
      m = nullptr;
    lp.clear();
    lp.shrink_to_fit();
    std::cout << "Enter для удаления" << std::endl;
    std::cin.get();
  }

  {
    typedef std::vector< std::shared_ptr<uint64_t> > queue_type;

    const uint64_t AGENT_TEST = 2000;
    const uint64_t CHANNEL_TEST = 100 * AGENT_TEST;
    const uint64_t MESSAGE_TEST = 100;

    queue_type lp;

    std::cout << "Наполнение сообщениями" << std::endl;

    for (uint64_t i = 0; i < MESSAGE_TEST; ++i)
    {
      std::cout << i << " \r";
      std::cout.flush();
      for ( uint64_t j = 0 ; j < CHANNEL_TEST ; ++j)
      {

        lp.push_back( std::make_shared<uint64_t>(i*j) );
      }
    }

    std::cout << "Enter для очистки " << std::endl;
    std::cin.get();
    for (auto& m : lp)
      m = nullptr;
    lp.clear();
    lp.shrink_to_fit();
    std::cout << "Enter для удаления" << std::endl;
    std::cin.get();
  }

  std::cout << "Enter для выхода" << std::endl;
  std::cin.get();
  return 0;
}



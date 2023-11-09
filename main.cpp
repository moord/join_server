#include <iostream>
#include "async_server.h"

int main(int argc, char* argv[]) {

 try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: join_server <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    server server(io_context, std::atoi(argv[1]));

    auto n = std::thread::hardware_concurrency();

    std::vector<std::thread> m_threads;
    m_threads.reserve( n);

    auto run = [&](){
        io_context.run();
    };

    std::generate_n( std::back_inserter(m_threads), n,  [&](){ return std::thread{run}; });

    for (auto& t : m_threads)
    {
        if (t.joinable())
            t.join();
    }

  }
  catch (const std::exception& ex)
  {
    std::cerr << "Exception: " << ex.what() << "\n";
  }

    return 0;
}

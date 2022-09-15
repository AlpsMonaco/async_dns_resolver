#include <iostream>
#include <thread>
#include "dns_query/dns_query.h"

int main(int argc, char** argv)
{
    dns::DNSQuery query;
    query.AsyncResolve("www.baidu.com", [&](const dns::Result& result) -> void
                       {
                           if (result.HasError())
                           {
                               std::cout << result.Error() << std::endl;
                               return;
                           }
                           std::cout << result.Name() << std::endl;
                           for (const auto& v : result)
                           {
                               std::cout << v << std::endl;
                           }
                       });
    std::thread t(
        [&]() -> void
        {
            query.Run(false);
        });
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "next" << std::endl;
    query.AsyncResolve("www.baidu.com", [&](const dns::Result& result) -> void
                       {
                           if (result.HasError())
                           {
                               std::cout << result.Error() << std::endl;
                               return;
                           }
                           std::cout << result.Name() << std::endl;
                           for (const auto& v : result)
                           {
                               std::cout << v << std::endl;
                           }
                       });
    std::this_thread::sleep_for(std::chrono::seconds(3));
    query.Stop();
    t.join();
}
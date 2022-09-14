#include <iostream>
#include <thread>
#include "dns_query/dns_query.h"

int main(int argc, char **argv)
{
    dns::DNSQuery query;
    query.Add("www.baidu.com");
    query.Add("www.google.com");
    auto err = query.Start([](const dns::Result &result) -> void
                           {
                               if (result.HasError())
                               {
                                   std::cout << result.Error() << " " << result.Name() << std::endl;
                                   return;
                               }
                               std::cout << result.Name() << std::endl;
                               for (const auto &v : result)
                               {
                                   std::cout << v << std::endl;
                               } });
    if (err)
    {
        std::cout << err << std::endl;
    }
}
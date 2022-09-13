#include <iostream>
#include <thread>
#include "dns_query/dns_query.h"

int main(int argc, char **argv)
{
    dns::DNSQuery query;
    query.Add("www.baidu.com");
    query.Add("www.google.com");
    auto err = query.Start([](const dns::Record &record, const dns::Error &err) -> void
                           {
                                if (err)
                                {
                                    std::cout << err << " " << record.Name() << std::endl ;
                                    return;
                                }
                                std::cout << record.Name() << std::endl;
                                for (const auto &addr : record.AddrList())
                                {
                                    std::cout << addr << std::endl;
                                } });

    if (err)
    {
        std::cout << err << std::endl;
    }
}
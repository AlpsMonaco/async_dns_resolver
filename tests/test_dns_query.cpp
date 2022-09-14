#include <iostream>
#include <thread>
#include "ares_setup.h"
#include "dns_query/dns_query.h"

int main(int argc, char **argv)
{
#ifdef USE_WINSOCK
    WORD wVersionRequested = MAKEWORD(USE_WINSOCK, USE_WINSOCK);
    WSADATA wsaData;
    WSAStartup(wVersionRequested, &wsaData);
#endif

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

#ifdef USE_WINSOCK
    WSACleanup();
#endif
}
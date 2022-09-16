#include <iostream>
#include "dns_query/dns_query.h"

int main(int argc, char** argv)
{
    dns::DNSQuery query;
    query.AsyncResolve("www.google.com", [&](const dns::Result& result) -> void
                       {
                           if (result.HasError())
                           {
                               std::cout << result.Error() << std::endl;
                           }
                           else
                           {
                               std::cout << result.Name() << std::endl;
                               for (const auto& v : result)
                               {
                                   std::cout << v << std::endl;
                               }
                           }
                           query.AsyncResolve("www.youtube.com", [&](const dns::Result& result) -> void
                                              {
                                                  if (result.HasError())
                                                  {
                                                      std::cout << result.Error() << std::endl;
                                                  }
                                                  else
                                                  {
                                                      std::cout << result.Name() << std::endl;
                                                      for (const auto& v : result)
                                                      {
                                                          std::cout << v << std::endl;
                                                      }
                                                  }
                                              });
                       });
    // Error DNSQuery::Run(bool stop_at_end)
    // Run will return if all async task is finished;
    // pass false to Run() if you need it to keep running and resolve
    // more dns in the future.
    query.Run(true);
}
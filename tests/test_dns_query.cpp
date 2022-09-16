#include "dns_query/dns_query.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

template <typename T>
void Println(T&& t)
{
  std::stringstream ss;
  ss << t << std::endl;
  std::cout << ss.str();
}

void MultiThreadTest()
{
  std::vector<std::thread> thread_list;
  for (size_t i = 0; i < 10; i++)
    thread_list.emplace_back([]() -> void {
      dnsresolve::Resolver resolver;
      resolver.AsyncResolve("www.google.com", [&](const dnsresolve::Result& result) -> void {
        if (result.HasError()) {
          Println(result.Error());
        } else {
          Println(result.Name());
          for (const auto& v : result) {
            Println(v);
          }
        }
        resolver.AsyncResolve("www.youtube.com", [&](const dnsresolve::Result& result) -> void {
          if (result.HasError()) {
            Println(result.Error());
          } else {
            Println(result.Name());
            for (const auto& v : result) {
              Println(v);
            }
          }
        });
      });
      resolver.Run(true);
    });
  for (auto& t : thread_list)
    t.join();
}

void SingleThreadTest()
{
  dnsresolve::Resolver resolver;
  resolver.AsyncResolve("www.google.com", [&](const dnsresolve::Result& result) -> void {
    if (result.HasError()) {
      Println(result.Error());
    } else {
      Println(result.Name());
      for (const auto& v : result) {
        Println(v);
      }
    }
    resolver.AsyncResolve("www.domain_could_not_be_exists.com", [&](const dnsresolve::Result& result) -> void {
      if (result.HasError()) {
        Println(result.Error());
      } else {
        Println(result.Name());
        for (const auto& v : result) {
          Println(v);
        }
      }
    });
  });
  resolver.Run(true);
}

void SimpleTest()
{
  dnsresolve::Resolver resolver;
  resolver.AsyncResolve("www.google.com", [&](const dnsresolve::Result& result) -> void {
    if (result.HasError()) {
      Println(result.Error());
    } else {
      Println(result.Name());
      for (const auto& v : result) {
        Println(v);
      }
    }
  });
  resolver.Run(true);
}

int main(int argc, char** argv)
{
  SimpleTest();
  SingleThreadTest();
  MultiThreadTest();
}
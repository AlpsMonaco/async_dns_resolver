# async_dns_resolver
async_dns_resolver is c++ async dns resolve library which provides simple API and designs for  
high performace and high concurrency scene.Also runtime and thread safe.

## Quick Start
```c++
#include <iostream>
#include "dns_resolve/dns_resolve.h"

int main(int argc,char**argv)
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
  resolver.Run();
}
```

## Introduction
* High performance based on c-ares,a C language dns async resolve library.
* Provide C++ style and simple API,also runtime safe.


## Usage

1. Init submodule,run  
   ```bash
   git submodule update --init
   ```
2. Use CMake to build library.
3. Include to your project.
for CMake user,use
```cmake
add_subdirectory(/path/to/dns_query)

target_include_directories(your_project
    PRIVATE
        /path/to/dns_query/include
)

target_link_libraries(your_project
    PRIVATE
        ...
        dns_query::lib
    )
```

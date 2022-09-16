# dns_query
C++ async dns resolve library.Provide asio-callback-like API.


## Quick Start
```c++
#include <iostream>
#include "dns_query/dns_query.h"

int main(int argc,char**argv)
{
  dns::Resolver resolver;
  resolver.AsyncResolve("www.google.com", [&](const dns::Result& result) -> void {
    if (result.HasError()) {
      std::cout << result.Error() << std::endl;
    } else {
      std::cout << result.Name() << std::endl;
      for (const auto& v : result) {
        std::cout << v << std::endl;
      }
    }
  });
  resolver.Run(true);
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

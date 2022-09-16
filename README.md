# async_dns_resolver
async_dns_resolver is c++ cross-platform async dns resolve library which provides simple API and designs for  
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
      std::cout <<result.Error() << std::endl;
    } else {
      std::cout <<result.Name() << std::endl;
      for (const auto& v : result) {
        std::cout << v << std::endl;
      }
    }
  });
  resolver.Run();
}
```

## Include

## CMake

It is recommend to use CMake to include this library.

1. Initialize git submodule,run  
```bash
git submodule update --init
```

2. (Optional) Use Cmake to build this library. 
```bash
mkdir -p build && cd build && cmake .. && make
```

for windows user,use `Visual Studio` to compile static library.

3. Include to your CMakeLists.txt
```cmake
add_subdirectory(/path/to/async_dns_resolver)

target_link_libraries(your_project
    PRIVATE
        ...
        dns_resolve::lib
    )
```
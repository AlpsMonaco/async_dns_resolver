#include <ares.h>
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>

#include <thread>
#include <vector>

static void callback(void* arg, int status, int timeouts, struct hostent* host)
{
  char** p;
  if (status != ARES_SUCCESS) {
    fprintf(stderr, "%s: %s\n", (char*)arg, ares_strerror(status));
    return;
  }
  for (p = host->h_addr_list; *p; p++) {
    char addr_buf[46] = "??";
    ares_inet_ntop(host->h_addrtype, *p, addr_buf, sizeof(addr_buf));
    printf("%-32s\t%s", host->h_name, addr_buf);
    puts("");
  }
}

struct GlobalAresMgr {
  static int Init()
  {
    GlobalAresMgr mgr;
    return mgr.status_;
  }

private:
  int status_;
  GlobalAresMgr()
  {
    status_ = ares_library_init(ARES_LIB_INIT_ALL);
  }

  ~GlobalAresMgr()
  {
    ares_library_cleanup();
  }
};

int CreateCAresTask()
{
  int status = GlobalAresMgr::Init();
  if (status != ARES_SUCCESS) {
    std::cout << ares_strerror(status) << std::endl;
    return 1;
  }
  ares_channel channel;
  struct ares_options options;
  int optmask = 0;
  memset(&options, 0, sizeof(options));
  status = ares_init_options(&channel, &options, optmask);
  if (status != ARES_SUCCESS) {
    std::cout << ares_strerror(status) << std::endl;
    return 1;
  }
  const char* domain = "www.baidu.com";
  for (size_t i = 0; i < 100; i++)
    ares_gethostbyname(channel, domain, AF_INET, callback, (void*)domain);
  timeval tm { 1, 0 };
  fd_set read_fds;
  fd_set write_fds;
  int nfds;
  for (;;) {
    int res;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    nfds = ares_fds(channel, &read_fds, &write_fds);
    if (nfds == 0)
      break;
    res = select(nfds, &read_fds, &write_fds, NULL, &tm);
    if (-1 == res)
      break;
    ares_process(channel, &read_fds, &write_fds);
  }
  ares_destroy(channel);
  return 0;
}

int main(int argc, char** argv)
{
  std::vector<std::thread> thread_list;
  for (size_t i = 0; i < 10; i++)
    thread_list.emplace_back([]() -> void { CreateCAresTask(); });
  for (auto& v : thread_list)
    v.join();
}
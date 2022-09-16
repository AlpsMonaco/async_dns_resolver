#include "dns_resolve.h"

#include <ares.h>
#include <string>

#ifdef _WIN32

#else
#include <string.h>
#endif

namespace dnsresolve {

struct QueryTask {
  std::string domain;
  Resolver::Callback callback;

  QueryTask(const std::string_view& target_domain, const Resolver::Callback& target_callback)
      : domain(target_domain)
      , callback(target_callback)
  {
  }
};

std::ostream& operator<<(std::ostream& os, const Error& error)
{
  os << error.Code() << " " << error.Message();
  return os;
}

Error::Error(int code)
    : code_(code)
{
}

Error::~Error()
{
}

int Error::Code() const
{
  return code_;
}

std::string_view Error::Message() const
{
  return ares_strerror(code_);
}

Error::operator bool() const
{
  return code_ != ARES_SUCCESS;
}

int Error::Code()
{
  return const_cast<const Error&>(*this).Code();
}

std::string_view Error::Message()
{
  return const_cast<const Error&>(*this).Message();
}

Error::operator bool()
{
  return const_cast<const Error&>(*this);
}

Result::Result(const std::string_view& domain, int code, hostent* hostent_ptr)
    : name_(domain)
    , code_(code)
    , hostent_ptr_(hostent_ptr)
{
}

Result::~Result()
{
}

const std::string_view& Result::Name() const
{
  return name_;
}

bool Result::HasError() const
{
  return code_ != 0;
}

Error Result::Error() const
{
  return dnsresolve::Error(code_);
}

Result::Iterator::Iterator(int addr_type, const char* const* addr_list)
    : addr_type_(addr_type)
    , p_(addr_list)
{
}

Result::Iterator::Iterator(const char* const* addr_list)
    : addr_type_()
    , p_(addr_list)
{
}

Result::Iterator Result::Iterator::operator++(int)
{
  Iterator it(addr_type_, p_);
  p_ += 1;
  return it;
}

bool Result::Iterator::operator!=(const Result::Iterator& rhs)
{
  return *rhs.p_ != *p_;
}

bool Result::Iterator::operator==(const Result::Iterator& rhs)
{
  return *rhs.p_ == *p_;
}

Result::Iterator Result::Begin() const
{
  return Iterator{hostent_ptr_->h_addrtype, hostent_ptr_->h_addr_list};
}

Result::Iterator Result::End() const
{
  return iterator_end;
}

Result::Iterator Result::begin() const
{
  return Begin();
}

Result::Iterator Result::end() const
{
  return End();
}

Result::Iterator& Result::Iterator::operator++()
{
  p_++;
  return *this;
}

std::string_view& Result::Iterator::operator*()
{
  UpdatePointer();
  return ptr_;
}

std::string_view* Result::Iterator::operator->()
{
  UpdatePointer();
  return &ptr_;
}

void Result::Iterator::UpdatePointer()
{
  buf_[0] = '?';
  buf_[1] = '0';
  ares_inet_ntop(addr_type_, *p_, buf_, sizeof(buf_));
  ptr_ = buf_;
}

class AresMgr {
public:
  static int Init()
  {
    static AresMgr mgr;
    return mgr.status_;
  }

  static void Callback(void* arg, int status, int timeouts, hostent* hostent_ptr)
  {
    auto query_task_ptr = reinterpret_cast<QueryTask*>(arg);
    query_task_ptr->callback(Result(query_task_ptr->domain, status, hostent_ptr));
    delete query_task_ptr;
  }

protected:
  int status_;

  AresMgr()
  {
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
#endif
    status_ = ares_library_init(ARES_LIB_INIT_ALL);
  }

  ~AresMgr()
  {
    if (status_ == ARES_SUCCESS)
      ares_library_cleanup();
#ifdef _WIN32
    WSACleanup();
#endif
  }
};

class Resolver::Impl {
public:
  Impl()
      : channel_()
      , status_(0)
  {
    status_ = AresMgr::Init();
    if (status_ != ARES_SUCCESS)
      return;
    struct ares_options options;
    int optmask = 0;
    memset(&options, 0, sizeof(options));
    status_ = ares_init_options(&channel_, &options, optmask);
    if (status_ != ARES_SUCCESS)
      return;
  }

  ~Impl()
  {
  }

  void AsyncResolve(const std::string_view& domain, const Callback& callback)
  {
    QueryTask* query_task_ptr = new QueryTask{domain, callback};
    ares_gethostbyname(channel_, domain.data(), AF_INET, AresMgr::Callback, query_task_ptr);
  }

  Error Run(bool stop_at_end)
  {
    is_stop_ = stop_at_end;
    if (status_ != ARES_SUCCESS)
      return status_;
    int nfds, addr_family = AF_INET;
    fd_set read_fds, write_fds;
    timeval tv{1, 0};
    for (;;) {
      int res;
      FD_ZERO(&read_fds);
      FD_ZERO(&write_fds);
      nfds = ares_fds(channel_, &read_fds, &write_fds);
      if (nfds == 0 && is_stop_)
        break;
      res = select(nfds, &read_fds, &write_fds, NULL, &tv);
      if (-1 == res && is_stop_)
        break;
      ares_process(channel_, &read_fds, &write_fds);
    }
    return ARES_SUCCESS;
  }

  void SetStop()
  {
    is_stop_ = true;
  }

protected:
  ares_channel channel_;
  int status_;
  bool is_stop_;
};

Resolver::Resolver()
{
  pimpl_ = std::make_unique<Impl>();
}

Resolver::~Resolver()
{
}

void Resolver::AsyncResolve(const std::string_view& domain, const Callback& callback)
{
  pimpl_->AsyncResolve(domain, callback);
}

Error Resolver::Run(bool stop_at_end)
{
  return pimpl_->Run(stop_at_end);
}

void Resolver::Stop()
{
  pimpl_->SetStop();
}

} // namespace dns
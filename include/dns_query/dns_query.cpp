#include "dns_query.h"
#include <ares.h>
#include <string>

#ifdef _WIN32

#else
#include <string.h>
#endif

namespace dns
{
    class DNSQueryImpl
    {
    public:
        struct QueryTask
        {
            std::string domain;
            DNSQuery::Callback callback;
        };

        static void Add(const std::string_view& domain, const DNSQuery::Callback& callback)
        {
            auto query_task_ptr = new QueryTask{std::string(domain), callback};
            ares_gethostbyname(Instance().channel_, domain.data(), AF_INET, Callback, query_task_ptr);
        }

        static void SetStop()
        {
            Instance().is_stop_ = true;
        }

        static Error Run(bool stop_at_end)
        {
            Instance().is_stop_ = stop_at_end;
            if (Instance().init_status_ != ARES_SUCCESS) return Instance().init_status_;
            int nfds, addr_family = AF_INET;
            fd_set read_fds, write_fds;
            struct timeval *tvp, tv;
            for (;;)
            {
                int res;
                FD_ZERO(&read_fds);
                FD_ZERO(&write_fds);
                nfds = ares_fds(Instance().channel_, &read_fds, &write_fds);
                if (nfds == 0 && Instance().is_stop_) break;
                tvp = ares_timeout(Instance().channel_, NULL, &tv);
                res = select(nfds, &read_fds, &write_fds, NULL, tvp);
                if (-1 == res && Instance().is_stop_) break;
                ares_process(Instance().channel_, &read_fds, &write_fds);
            }
            return ARES_SUCCESS;
        }

        ~DNSQueryImpl()
        {
            ares_destroy(Instance().channel_);
            ares_library_cleanup();
        }

    protected:
        static DNSQueryImpl& Instance()
        {
            static DNSQueryImpl ares;
            return ares;
        }

        static void Callback(void* arg, int status, int timeouts, struct hostent* host)
        {
            auto task_ptr = reinterpret_cast<QueryTask*>(arg);
            task_ptr->callback(Result(task_ptr->domain, status, host));
            delete task_ptr;
        }

        DNSQueryImpl()
            : init_status_(ARES_SUCCESS)
        {
            struct ares_options options;
            int optmask = 0;
            memset(&options, 0, sizeof(options));
            init_status_ = ares_library_init(ARES_LIB_INIT_ALL);
            if (init_status_ != ARES_SUCCESS)
                return;
            init_status_ = ares_init_options(&channel_, &options, optmask);
        }

        ares_channel channel_;
        int init_status_;
        bool is_stop_;
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
        : name_(domain),
          code_(code),
          hostent_ptr_(hostent_ptr)
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
        return dns::Error(code_);
    }

    Result::Iterator::Iterator(int addr_type, const char* const* addr_list)
        : addr_type_(addr_type),
          p_(addr_list)
    {
    }

    Result::Iterator::Iterator(const char* const* addr_list)
        : addr_type_(),
          p_(addr_list)
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

    DNSQuery::DNSQuery()
    {
#ifdef _WIN32
        WORD wVersionRequested;
        WSADATA wsaData;
        wVersionRequested = MAKEWORD(2, 2);
        WSAStartup(wVersionRequested, &wsaData);
#endif
    }

    DNSQuery::~DNSQuery()
    {
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void DNSQuery::AsyncResolve(const std::string_view& domain, const Callback& callback)
    {
        DNSQueryImpl::Add(domain, callback);
    }

    Error DNSQuery::Run(bool stop_at_end)
    {
        return DNSQueryImpl::Run(stop_at_end);
    }

    void DNSQuery::Stop()
    {
        DNSQueryImpl::SetStop();
    }
} // namespace dns
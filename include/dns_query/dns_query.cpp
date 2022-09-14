#include "dns_query.h"
#include <ares.h>

#ifdef _WIN32

#else
#include <string.h>

#endif

namespace dns
{
    std::ostream &operator<<(std::ostream &os, const Error &error)
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
        return const_cast<const Error &>(*this).Code();
    }

    std::string_view Error::Message()
    {
        return const_cast<const Error &>(*this).Message();
    }

    Error::operator bool()
    {
        return const_cast<const Error &>(*this);
    }

    Result::Result(const std::string_view &domain, int code, hostent *hostent_ptr)
        : name_(domain),
          code_(code),
          hostent_ptr_(hostent_ptr)
    {
    }

    Result::~Result()
    {
    }

    const std::string_view &Result::Name() const
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

    Result::Iterator::Iterator(int addr_type, const char *const *addr_list)
        : addr_type_(addr_type),
          p_(addr_list)
    {
    }

    Result::Iterator Result::Iterator::operator++(int)
    {
        Iterator it(addr_type_, p_);
        p_ += 1;
        return it;
    }

    bool Result::Iterator::operator!=(const Result::Iterator &rhs)
    {
        return *rhs.p_ != *p_;
    }

    bool Result::Iterator::operator==(const Result::Iterator &rhs)
    {
        return *rhs.p_ == *p_;
    }

    Result::Iterator Result::Begin() const
    {
        return Iterator{hostent_ptr_->h_addrtype, hostent_ptr_->h_addr_list};
    }

    Result::Iterator Result::End() const
    {
        return Iterator{hostent_ptr_->h_addrtype, Iterator::end};
    }

    Result::Iterator Result::begin() const
    {
        return Begin();
    }

    Result::Iterator Result::end() const
    {
        return End();
    }

    Result::Iterator &Result::Iterator::operator++()
    {
        p_++;
        return *this;
    }

    std::string_view &Result::Iterator::operator*()
    {
        UpdatePointer();
        return ptr_;
    }

    std::string_view *Result::Iterator::operator->()
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
        : domain_list_()
    {
    }

    DNSQuery::~DNSQuery()
    {
    }

    void DNSQuery::Add(const std::string_view &domain)
    {
        domain_list_.emplace_back(domain);
    }

    class Ares
    {
    public:
        static void Callback(void *arg, int status, int timeouts, struct hostent *host)
        {
            const std::string &domain = *reinterpret_cast<std::string *>(arg);
            Instance().callback_(Result{domain, status, host});
        }

        static void SetCallback(const DNSQuery::Callback &callback)
        {
            Instance().callback_ = callback;
        }

        static void SetDomainList(std::vector<std::string> &domain_list)
        {
            Instance().domain_list_ptr_ = &domain_list;
        }

        static Error Start()
        {
            Instance().index_ = 0;
            struct ares_options options;
            int optmask = 0;
            int status, nfds, addr_family = AF_INET;
            fd_set read_fds, write_fds;
            struct timeval *tvp, tv;
            memset(&options, 0, sizeof(options));
            status = ares_library_init(ARES_LIB_INIT_ALL);
            if (status != ARES_SUCCESS)
                return status;
            status = ares_init_options(&Instance().channel_, &options, optmask);
            if (status != ARES_SUCCESS)
                return status;
            for (auto &domain : *(Instance().domain_list_ptr_))
            {
                ares_gethostbyname(Instance().channel_, domain.c_str(), addr_family, Ares::Callback, &domain);
            }
            for (;;)
            {
                int res;
                FD_ZERO(&read_fds);
                FD_ZERO(&write_fds);
                nfds = ares_fds(Instance().channel_, &read_fds, &write_fds);
                if (nfds == 0)
                    break;
                tvp = ares_timeout(Instance().channel_, NULL, &tv);
                res = select(nfds, &read_fds, &write_fds, NULL, tvp);
                if (-1 == res)
                    break;
                ares_process(Instance().channel_, &read_fds, &write_fds);
            }
            ares_destroy(Instance().channel_);
            ares_library_cleanup();
            return status;
        }
        ~Ares() {}

    protected:
        static Ares &Instance()
        {
            static Ares ares;
            return ares;
        }

        Ares()
            : callback_(),
              domain_list_ptr_(nullptr),
              index_(0)
        {
        }

        DNSQuery::Callback callback_;
        std::vector<std::string> *domain_list_ptr_;
        size_t index_;
        ares_channel channel_;
    };

    Error DNSQuery::Start(const Callback &callback)
    {
        Ares::SetCallback(callback);
        Ares::SetDomainList(domain_list_);
        return Ares::Start();
    }
}
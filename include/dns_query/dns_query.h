#include <string_view>
#include <string>
#include <vector>
#include <ostream>
#include <functional>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <netdb.h>
#endif

namespace dns
{
    class Error
    {
    public:
        Error(int code);
        ~Error();

        operator bool();
        int Code();
        std::string_view Message();
        friend std::ostream &operator<<(std::ostream &os, const Error &error);

        operator bool() const;
        int Code() const;
        std::string_view Message() const;

    protected:
        int code_;
    };

    class Record
    {
    public:
        Record(const std::string_view &domain, hostent *host);
        ~Record();

        size_t AddrLength();
        std::string_view Name();
        std::vector<std::string> AddrList();
        hostent *Raw();

        size_t AddrLength() const;
        std::string_view Name() const;
        std::vector<std::string> AddrList() const;
        const hostent *Raw() const;

    protected:
        std::string_view name_;
        hostent *hostent_ptr_;
    };

    class DNSQuery
    {
    public:
        using Callback = std::function<void(const Record &record, const Error &error)>;
        DNSQuery();
        ~DNSQuery();

        void Add(const std::string_view &domain);
        Error Start(const Callback &callback);

    protected:
        std::vector<std::string> domain_list_;
    };
}

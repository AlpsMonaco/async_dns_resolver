#include <string_view>
#include <string>
#include <vector>
#include <ostream>
#include <array>
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

    class Result
    {
    public:
        Result(const std::string_view &domain, int code, hostent *hostent_ptr);
        ~Result();

        struct Iterator
        {
            Iterator(int addr_type, const char *const *addr_list);
            Iterator operator++(int);
            Iterator &operator++();
            std::string_view &operator*();
            std::string_view *operator->();
            bool operator!=(const Iterator &rhs);
            bool operator==(const Iterator &rhs);

            static constexpr char *end[1] = {NULL};

        private:
            void UpdatePointer();
            int addr_type_;
            const char *const *p_;
            char buf_[46];
            std::string_view ptr_;
        };

        const std::string_view &Name() const;
        bool HasError() const;
        Error Error() const;
        Iterator Begin() const;
        Iterator End() const;
        Iterator begin() const;
        Iterator end() const;

    protected:
        std::string_view name_;
        int code_;
        hostent *hostent_ptr_;
    };

    class DNSQuery
    {
    public:
        using Callback = std::function<void(const Result &result)>;
        DNSQuery();
        ~DNSQuery();

        void Add(const std::string_view &domain);
        Error Start(const Callback &callback);

    protected:
        std::vector<std::string> domain_list_;
    };
}

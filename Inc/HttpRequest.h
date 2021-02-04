#pragma once

#include <iostream>
#include <unordered_map>
#include <string>

class HttpRequest;
std::ostream &operator<<(std::ostream &, const HttpRequest &);

struct HttpRequest
{
    friend std::ostream &operator<<(std::ostream &, const HttpRequest &);
    enum HTTP_VERSION
    {
        HTTP_10 = 0,
        HTTP_11,
        VERSION_NOT_SUPPORT
    };
    enum HTTP_METHOD
    {
        GET = 0,
        POST,
        PUT,
        DELETE,
        METHOD_NOT_SUPPORT
    };
    enum HTTP_HEADER
    {
        HOST = 0,
        USER_AGENT,
        CONNECTION,
        ACCEPT_ENCODING,
        ACCEPT_LANGUAGE,
        ACCEPT,
        CACHE_CONTROL,
        UPGRADE_INSECURE_REQUESTS
    };
    struct EnumClassHash
    {
        template <typename T>
        std::size_t operator()(T t) const
        {
            return static_cast<std::size_t>(t);
        }
    };

    HttpRequest(std::string url = std::string(""), HTTP_METHOD method = METHOD_NOT_SUPPORT,
                HTTP_VERSION version = VERSION_NOT_SUPPORT) : _method(method), _version(version), _url(url), _content(nullptr),
                                                              _headers(std::unordered_map<HTTP_HEADER, std::string, EnumClassHash>()){};
    //TODO 析构函数
    HTTP_METHOD _method;
    HTTP_VERSION _version;
    std::string _url;

    char *_content; //TODO 不一样的地方
    std::unordered_map<HTTP_HEADER, std::string, EnumClassHash> _headers;
    static std::unordered_map<std::string, HTTP_HEADER> headerMap;
};
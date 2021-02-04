#include "HttpRequest.h"

std::ostream &operator<<(std::ostream &os, const HttpRequest &request)
{
    os << "method: " << request._method << std::endl;
    os << "url: " << request._url << std::endl;
    os << "version: " << request._version << std::endl;

    for (auto iter = request._headers.begin(); iter != request._headers.end(); iter++)
    {
        os << iter->first << ":" << iter->second << std::endl;
    }
    return os;
}

//std::unordered_map<std::string, HttpRequest::HTTP_HEADER> HttpRequest::headerMap;
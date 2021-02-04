#pragma once

#include <string>
#include <unordered_map>
#include "HttpRequest.h"

struct MimeType
{
    MimeType(const std::string &str) : type(str) {}
    MimeType(const char *str) : type(str) {}
    std::string type;
};

extern std::unordered_map<std::string, MimeType> MimeMap;

class HttpResponse
{
public:
    enum HttpStatusCode
    {
        Unknow,
        k200OK = 200,
        k403Forbidden = 403,
        k404NotFound = 404
    };
    explicit HttpResponse(bool keep = true) : _statusCode(Unknow), _keepAlive(keep), _mime("text/html"),
                                              _content(nullptr), _version(HttpRequest::HTTP_10) {}
    void setStatusCode(HttpStatusCode code)
    {
        _statusCode = code;
    }

    HttpStatusCode getStatusCode() const
    {
        return _statusCode;
    }
    void setBody(const char *buf)
    {
        _content = buf;
    }

    void setContentLength(int len)
    {
        _contentLength = len;
    }

    void setVersion(HttpRequest::HTTP_VERSION version)
    {
        _version = version;
    }

    HttpRequest::HTTP_VERSION getVersion() const
    {
        return _version;
    }

    void setStatusMsg(const std::string &msg)
    {
        _statusMsg = msg;
    }

    const std::string getStatusMsg() const
    {
        return _statusMsg;
    }

    void setFilePath(std::string &filepath)
    {
        _filePath = filepath;
    }

    const std::string getFilePath() const
    {
        return _filePath;
    }

    void setMime(const MimeType &mime)
    {
        _mime = mime;
    }

    void setKeepAlive(bool isAlive)
    {
        _keepAlive = isAlive;
    }

    bool isKeepAlive() const
    {
        return _keepAlive;
    }

    void addHeader(const std::string &key, const std::string &value)
    {
        _headers[key] = value;
    }

    void constructBuffer(char *) const;

    ~HttpResponse()
    {
        if (_content != nullptr)
        {
            delete[] _content;
        }
    }

private:
    HttpStatusCode _statusCode;
    std::string _statusMsg;
    HttpRequest::HTTP_VERSION _version;
    bool _keepAlive;
    MimeType _mime;
    const char *_content;
    int _contentLength;
    std::string _filePath;
    std::unordered_map<std::string, std::string> _headers;
};
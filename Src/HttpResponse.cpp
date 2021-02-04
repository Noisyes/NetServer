#include "HttpResponse.h"

std::unordered_map<std::string, MimeType> MimeMap = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/msword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"},
    {"", "text/plain"}, //默认
    {"default", "text/plain"}};

void HttpResponse::constructBuffer(char *buffer) const
{
    if (_version == HttpRequest::HTTP_10)
    {
        sprintf(buffer, "HTTP/1.0 %d %s\r\n", _statusCode, _statusMsg.c_str());
    }
    else
    {
        sprintf(buffer, "HTTP/1.1 %d %s\r\n", _statusCode, _statusMsg.c_str());
    }

    for (auto iter = _headers.begin(); iter != _headers.end(); iter++)
    {
        sprintf(buffer, "%s%s:%s\r\n", buffer, iter->first.c_str(), iter->second.c_str());
    }

    if (_keepAlive)
    {
        sprintf(buffer, "%sConnection: Keep-Alive\r\n", buffer);
    }
    else
    {
        sprintf(buffer, "%sConnection: close\r\n", buffer);
    }
}
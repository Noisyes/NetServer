#include "HttpParse.h"

#include "HttpRequest.h"
#include "Utils.h"
#include <string.h>
#include <algorithm>

std::unordered_map<std::string, HttpRequest::HTTP_HEADER> HttpRequest::headerMap = {
    {"HOST", HttpRequest::HOST},
    {"USER_AGENT", HttpRequest::USER_AGENT},
    {"CONNECTION", HttpRequest::CONNECTION},
    {"ACCEPT-ENCODING", HttpRequest::ACCEPT_ENCODING},
    {"ACCEPT-LANGUAGE", HttpRequest::ACCEPT_LANGUAGE},
    {"ACCEPT", HttpRequest::ACCEPT},
    {"CACHE-CONTROL", HttpRequest::CACHE_CONTROL},
    {"UPGREADE-INSECURE-REQUESTS", HttpRequest::UPGRADE_INSECURE_REQUESTS}};

HttpRequestParser::LINE_STATE
HttpRequestParser::parse_line(char *buffer, int &checked_index, int &read_index)
{
    char temp;
    for (; checked_index < read_index; checked_index++)
    {
        temp = buffer[checked_index];
        if (temp == CR)
        {
            if (checked_index + 1 == read_index)
                return LINE_MORE;
            if (buffer[checked_index + 1] == LF)
            {
                buffer[checked_index++] = LINE_END;
                buffer[checked_index++] = LINE_END;
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_MORE;
}

HttpRequestParser::HTTP_CODE
HttpRequestParser::parse_requestline(char *line, PARSE_STATE &parse_state, HttpRequest &request)
{
    char *url = strpbrk(line, " \t");
    if (!url)
    {
        return BAD_REQUEST;
    }
    *url++ = LINE_END;
    char *method = line;
    if (strcasecmp(method, "GET") == 0)
    {
        request._method = HttpRequest::GET;
    }
    else if (strcasecmp(method, "POST") == 0)
    {
        request._method = HttpRequest::POST;
    }
    else if (strcasecmp(method, "PUT") == 0)
    {
        request._method = HttpRequest::PUT;
    }
    else
    {
        return BAD_REQUEST;
    }

    url += strspn(url, " \t");
    char *version = strpbrk(url, " \t");
    if (!version)
    {
        return BAD_REQUEST;
    }
    *version++ = LINE_END;
    version += strspn(version, " \t");
    if (strncasecmp("HTTP/1.1", version, 8) == 0)
    {
        request._version = HttpRequest::HTTP_11;
    }
    else if (strncasecmp("HTTP/1.0", version, 8) == 0)
    {
        request._version = HttpRequest::HTTP_10;
    }
    else
    {
        return BAD_REQUEST;
    }
    if (strncasecmp(url, "http://", 7) == 0)
    {
        url += 7;
        url = strchr(url, '/');
    }
    else if (strncasecmp(url, "/", 1) == 0)
    {
        PASS;
    }
    else
        return BAD_REQUEST;

    if (!url || *url != '/')
        return BAD_REQUEST;
    request._url = std::string(url);

    parse_state = PARSE_HEADER;
    return NO_REQUEST;
}

HttpRequestParser::HTTP_CODE
HttpRequestParser::parse_headers(char *line, PARSE_STATE &parse_state, HttpRequest &request)
{
    if (*line == '\0')
    {
        if (request._method == HttpRequest::GET)
        {
            return GET_REQUEST;
        }
        parse_state = PARSE_BODY;
        return NO_REQUEST;
    }

    char key[100], value[300];
    sscanf(line, "%[^:]:%[^:]", key, value);
    decltype(HttpRequest::headerMap)::iterator iter;
    std::string _key(key);
    std::transform(_key.begin(), _key.end(), _key.begin(), ::toupper);
    std::string _value(value);
    if (((iter = HttpRequest::headerMap.find(trim(_key))) != HttpRequest::headerMap.end()))
    {
        request._headers[iter->second] = trim(_value);
    }
    else
    {
        //return BAD_REQUEST;
    }

    return NO_REQUEST;
}

HttpRequestParser::HTTP_CODE
HttpRequestParser::parse_body(char *body, PARSE_STATE &parse_state, HttpRequest &request)
{
    strcpy(request._content, body);
    return GET_REQUEST;
}

HttpRequestParser::HTTP_CODE
HttpRequestParser::parse_content(char *buffer, int &check_index, int &read_index,
                                 PARSE_STATE &parse_state, int &start_line, HttpRequest &request)
{
    LINE_STATE line_state = LINE_OK;
    HTTP_CODE retcode = NO_REQUEST;
    while ((line_state = parse_line(buffer, check_index, read_index)) == LINE_OK)
    {
        char *tmp = buffer + start_line;
        start_line = check_index;
        switch (parse_state)
        {
        case PARSE_REQUESTLINE:
            retcode = parse_requestline(tmp, parse_state, request);
            if (retcode == BAD_REQUEST)
                return BAD_REQUEST;
            break;
        case PARSE_HEADER:
            retcode = parse_headers(tmp, parse_state, request);
            if (retcode == BAD_REQUEST)
                return BAD_REQUEST;
            else if (retcode == GET_REQUEST)
            {
                return retcode;
            }
            break;
        case PARSE_BODY:
            retcode = parse_body(tmp, parse_state, request);
            if (retcode == GET_REQUEST)
                return retcode;
            return BAD_REQUEST;
        default:
            return INTERAL_ERROR;
        }
    }
    if (line_state == LINE_MORE)
        return NO_REQUEST;
    else if (line_state = LINE_BAD)
        return BAD_REQUEST;
}
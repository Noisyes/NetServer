#include "HttpServer.h"
#include "ThreadPool.h"
#include "Epoll.h"
#include "Utils.h"
#include "HttpParse.h"
#include "CommonHead.h"
#include <string>
#include <iostream>
#include <sstream>

char NOT_FOUND_PAGE[] = "<html>\n"
                        "<head><title>404 Not Found</title></head>\n"
                        "<body bgcolor=\"white\">\n"
                        "<center><h1>404 Not Found</h1></center>\n"
                        "<hr><center>NetServer/0.3 (Linux)</center>\n"
                        "</body>\n"
                        "</html>";

char FORBIDDEN_PAGE[] = "<html>\n"
                        "<head><title>403 Forbidden</title></head>\n"
                        "<body bgcolor=\"white\">\n"
                        "<center><h1>403 Forbidden</h1></center>\n"
                        "<hr><center>NetServer/0.3 (Linux)</center>\n"
                        "</body>\n"
                        "</html>";

char INDEX_PAGE[] = "<!DOCTYPE html>\n"
                    "<html>\n"
                    "<head>\n"
                    "    <title>Welcome to NetServer!</title>\n"
                    "    <style>\n"
                    "        body {\n"
                    "            width: 35em;\n"
                    "            margin: 0 auto;\n"
                    "            font-family: Tahoma, Verdana, Arial, sans-serif;\n"
                    "        }\n"
                    "    </style>\n"
                    "</head>\n"
                    "<body>\n"
                    "<h1>Welcome to NetServer!</h1>\n"
                    "<p>If you see this page, the webserver is successfully installed and\n"
                    "    working. </p>\n"
                    "\n"
                    "<p>For online documentation and support please refer to\n"
                    "    <a href=\"https://github.com/noisyes\">Noisyes</a>.<br/>\n"
                    "\n"
                    "<p><em>Thank you for using Noisyes WebServer.</em></p>\n"
                    "</body>\n"
                    "</html>";

char TEST[] = "HELLO WORLD";

void request_test(std::shared_ptr<void> arg)
{
    char buffer[BUFFER_SIZE];
    std::shared_ptr<HttpData> httpData = std::static_pointer_cast<HttpData>(arg);
    int ret = recv(httpData->clientSocket->clientfd, buffer, sizeof(buffer), 0);
    buffer[ret] = '\0';
    std::string test(buffer);
    std::cout << test << std::endl;
}

extern std::string basePath;

void HttpServer::run(int thread_num, int max_request)
{
    ThreadPool pool(thread_num, max_request);
    int epoll_fd = Epoll::init(MAX_EVENT_NUMBER);

    serverSocket._epollfd = epoll_fd;
    spHttpData httpData(new HttpData);
    httpData->epoll_fd = epoll_fd;
    __uint32_t event = (EPOLLIN | EPOLLET);
    Epoll::addFD(epoll_fd, serverSocket._listenfd, event, httpData);

    while (true)
    {
        std::vector<std::shared_ptr<HttpData>> events = Epoll::poll(serverSocket, MAX_EVENT_NUMBER, -1);
        for (auto &req : events)
        {
            pool.append(req, bind(&HttpServer::do_request, this, std::placeholders::_1));
        }
        Epoll::timerManager.handle_expired_event();
    }
}

void HttpServer::do_request(std::shared_ptr<void> arg)
{
    spHttpData sharedHttpData = std::static_pointer_cast<HttpData>(arg);
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int checked_index = 0, start_line = 0, read_index = 0;
    ssize_t recv_data;
    HttpRequestParser::PARSE_STATE parse_state = HttpRequestParser::PARSE_REQUESTLINE;
    while (true)
    {
        recv_data = recv(sharedHttpData->clientSocket->clientfd, buffer + read_index, BUFFER_SIZE - read_index, 0);
        std::cout << buffer << std::endl;
        if (recv_data < 0)
        {
            if ((errno == EAGAIN || (errno == EWOULDBLOCK)))
            {
                break;
            }
            std::cout << "reading failed" << std::endl;
            return;
        }
        else if (recv_data == 0)
        {
            std::cout << "Connection Close by Client" << std::endl;
            break;
        }
        read_index += recv_data;
        HttpRequestParser::HTTP_CODE retcode = HttpRequestParser::parse_content(buffer, checked_index, read_index, parse_state, start_line, *sharedHttpData->httpRequest);
        if (retcode == HttpRequestParser::NO_REQUEST)
        {
            continue;
        }
        if (retcode == HttpRequestParser::GET_REQUEST)
        {
            auto iter = sharedHttpData->httpRequest->_headers.find(HttpRequest::CONNECTION);
            if (iter != sharedHttpData->httpRequest->_headers.end())
            {
                if (iter->second == "keep-alive")
                {
                    sharedHttpData->httpResponse->setKeepAlive(true);
                    std::ostringstream osstr;
                    osstr << "timeout=" << TIME_OUT;
                    sharedHttpData->httpResponse->addHeader("Keep-Alive", osstr.str());
                }
                else
                {
                    sharedHttpData->httpResponse->setKeepAlive(false);
                }
            }
            header(sharedHttpData);
            getMime(sharedHttpData);

            FileStatus fileState = static_file(sharedHttpData, basePath.c_str());
            send(sharedHttpData, fileState);

            if (sharedHttpData->httpResponse->isKeepAlive())
            {
                Epoll::modFD(sharedHttpData->epoll_fd, sharedHttpData->clientSocket->clientfd, Epoll::DEFAULT_EVENT, sharedHttpData);
                Epoll::timerManager.addTimer(sharedHttpData, TimerManager::DEFAULT_TIME_OUT);
            }
        }
        else
        {
            std::cout << "Bad Request" << std::endl;
        }
    }
}

void HttpServer::header(spHttpData httpData)
{
    if (httpData->httpRequest->HTTP_10)
    {
        httpData->httpResponse->setVersion(HttpRequest::HTTP_10);
    }
    else
    {
        httpData->httpResponse->setVersion(HttpRequest::HTTP_11);
    }
    httpData->httpResponse->addHeader("Server", "My Server");
}

HttpServer::FileStatus
HttpServer::static_file(spHttpData spData, const char *basepath)
{
    struct stat file_stat;
    char *file = new char[strlen(basepath) + strlen(spData->httpResponse->getFilePath().c_str()) + 1];
    strcpy(file, basepath);
    strcpy(file + strlen(basepath), spData->httpResponse->getFilePath().c_str());
    std::cout << "===============================" << std::endl;
    std::cout << file << std::endl;
    std::cout << "===============================" << std::endl;

    if (spData->httpResponse->getFilePath() == "/" || stat(file, &file_stat) < 0)
    {
        spData->httpResponse->setMime(MimeType("text/html"));
        if (spData->httpResponse->getFilePath() == "/")
        {
            spData->httpResponse->setStatusCode(HttpResponse::k200OK);
            spData->httpResponse->setStatusMsg("OK");
        }
        else
        {
            spData->httpResponse->setStatusCode(HttpResponse::k404NotFound);
            spData->httpResponse->setStatusMsg("Not Found");
        }
        return FILE_NOT_FOUND;
    }
    if (!S_ISREG(file_stat.st_mode))
    {
        spData->httpResponse->setMime(MimeType("text/html"));
        spData->httpResponse->setStatusCode(HttpResponse::k403Forbidden);
        spData->httpResponse->setStatusMsg("ForBidden");
        std::cout << "not normal file" << std::endl;
        return FILE_FORBIDEN;
    }
    spData->httpResponse->setStatusCode(HttpResponse::k200OK);
    spData->httpResponse->setStatusMsg("OK");
    std::string path(file);
    spData->httpResponse->setFilePath(path);
    delete[] file;
    return FILE_OK;
}

void HttpServer::getMime(spHttpData httpData)
{
    std::string filepath = httpData->httpRequest->_url;
    std::cout << "===============================" << std::endl;
    std::cout << filepath << std::endl;
    std::cout << "===============================" << std::endl;
    std::string mime;
    int pos;
    if ((pos = filepath.rfind('?')) != std::string::npos)
    {
        filepath.erase(filepath.rfind('?'));
    }

    if (filepath.rfind('.') != std::string::npos)
    {
        mime = filepath.substr(filepath.rfind('.'));
    }
    decltype(MimeMap)::iterator iter = MimeMap.find(mime);
    if (iter != MimeMap.end())
    {
        httpData->httpResponse->setMime(iter->second);
    }
    else
    {
        httpData->httpResponse->setMime(MimeMap.find("default")->second);
    }
    httpData->httpResponse->setFilePath(filepath);
}

void HttpServer::send(spHttpData httpData, FileStatus fileState)
{
    char header[BUFFER_SIZE];
    memset(header, '\0', BUFFER_SIZE);
    const char *internal_error = "Internal Error";
    struct stat file_stat;
    httpData->httpResponse->constructBuffer(header);

    if (fileState == FILE_NOT_FOUND)
    {
        if (httpData->httpResponse->getFilePath() == std::string("/"))
        {
            sprintf(header, "%sContent-length: %ld\r\n\r\n", header, strlen(INDEX_PAGE));
            sprintf(header, "%s%s", header, INDEX_PAGE);
        }
        else
        {
            sprintf(header, "%sContent-length: %ld\r\n\r\n", header, strlen(NOT_FOUND_PAGE));
            sprintf(header, "%s%s", header, NOT_FOUND_PAGE);
        }
        ::send(httpData->clientSocket->clientfd, header, strlen(header), 0);
        return;
    }
    if (fileState == FILE_FORBIDEN)
    {
        sprintf(header, "%sContent-length: %ld\r\n\r\n", header, strlen(FORBIDDEN_PAGE));
        sprintf(header, "%s%s", header, FORBIDDEN_PAGE);
        ::send(httpData->clientSocket->clientfd, header, strlen(header), 0);
        return;
    }
    if (stat(httpData->httpResponse->getFilePath().c_str(), &file_stat) < 0)
    {
        sprintf(header, "%sContent-length: %ld\r\n\r\n", header, strlen(internal_error));
        sprintf(header, "%s%s", header, internal_error);
        ::send(httpData->clientSocket->clientfd, header, strlen(header), 0);
        return;
    }

    int filefd = ::open(httpData->httpResponse->getFilePath().c_str(), O_RDONLY);
    if (filefd < 0)
    {
        std::cout << "打开文件失败" << std::endl;
        sprintf(header, "%sContent-length: %ld\r\n\r\n", header, strlen(internal_error));
        sprintf(header, "%s%s", header, internal_error);
        ::send(httpData->clientSocket->clientfd, header, strlen(header), 0);
        return;
    }

    sprintf(header, "%sContent-length: %ld\r\n\r\n", header, file_stat.st_size);
    ::send(httpData->clientSocket->clientfd, header, strlen(header), 0);
    printf("%s\n", header);
    sendfile(httpData->clientSocket->clientfd, filefd, NULL, file_stat.st_size);
    close(filefd);
    return;
}

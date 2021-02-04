#pragma once

bool setReusePort(int fd);

int setnonblocking(int fd);

std::string &ltrim(std::string &str);

std::string &rtrim(std::string &str);

std::string &trim(std::string &str);
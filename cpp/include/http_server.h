#pragma once

#include <cstdint>
#include <functional>
#include <string>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
};

using RequestHandler = std::function<std::string(const HttpRequest &request)>;

class HttpServer {
public:
    explicit HttpServer(RequestHandler handler);

    bool start(uint16_t port);
    std::string handle(const HttpRequest &request) const;

private:
    RequestHandler handler_;
};

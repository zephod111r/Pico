#include "http_server.h"

#include <cstdlib>
#include <cstring>

#include "lwip/tcp.h"

namespace {
struct ConnectionState {
    char buffer[2048];
    size_t length = 0;
};

HttpServer *g_server = nullptr;

bool parse_request(const char *data, size_t length, HttpRequest &request) {
    std::string text(data, length);
    size_t line_end = text.find("\r\n");
    if (line_end == std::string::npos) {
        return false;
    }

    std::string request_line = text.substr(0, line_end);
    size_t method_end = request_line.find(' ');
    size_t path_end = request_line.find(' ', method_end + 1);
    if (method_end == std::string::npos || path_end == std::string::npos) {
        return false;
    }

    request.method = request_line.substr(0, method_end);
    request.path = request_line.substr(method_end + 1, path_end - method_end - 1);

    size_t header_end = text.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return false;
    }

    request.body = text.substr(header_end + 4);
    return true;
}

err_t on_receive(void *arg, tcp_pcb *tpcb, pbuf *p, err_t err) {
    ConnectionState *state = static_cast<ConnectionState *>(arg);
    if (!p) {
        tcp_close(tpcb);
        delete state;
        return ERR_OK;
    }

    if (err != ERR_OK) {
        pbuf_free(p);
        tcp_close(tpcb);
        delete state;
        return err;
    }

    tcp_recved(tpcb, p->tot_len);

    if (state->length + p->tot_len >= sizeof(state->buffer)) {
        pbuf_free(p);
        tcp_close(tpcb);
        delete state;
        return ERR_OK;
    }

    pbuf_copy_partial(p, state->buffer + state->length, p->tot_len, 0);
    state->length += p->tot_len;
    pbuf_free(p);

    std::string text(state->buffer, state->length);
    size_t header_end = text.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return ERR_OK;
    }

    size_t content_length = 0;
    size_t headers_end = header_end + 4;
    std::string headers = text.substr(0, header_end);
    size_t pos = headers.find("Content-Length:");
    if (pos != std::string::npos) {
        size_t value_start = headers.find(':', pos);
        if (value_start != std::string::npos) {
            content_length = static_cast<size_t>(std::atoi(headers.c_str() + value_start + 1));
        }
    }

    if (text.size() - headers_end < content_length) {
        return ERR_OK;
    }

    std::string response;
    HttpRequest request;
    if (parse_request(state->buffer, state->length, request)) {
        if (g_server) {
            response = g_server->handle(request);
        }
    } else {
        response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
    }

    if (!response.empty()) {
        tcp_write(tpcb, response.c_str(), response.size(), TCP_WRITE_FLAG_COPY);
    }

    tcp_close(tpcb);
    delete state;
    return ERR_OK;
}

err_t on_accept(void *arg, tcp_pcb *new_pcb, err_t err) {
    if (err != ERR_OK || !new_pcb) {
        return ERR_VAL;
    }

    auto *state = new ConnectionState();
    tcp_arg(new_pcb, state);
    tcp_recv(new_pcb, on_receive);
    return ERR_OK;
}
}  // namespace

HttpServer::HttpServer(RequestHandler handler) : handler_(handler) {
    g_server = this;
}

std::string HttpServer::handle(const HttpRequest &request) const {
    if (!handler_) {
        return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
    }
    return handler_(request);
}

bool HttpServer::start(uint16_t port) {
    tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        return false;
    }

    err_t err = tcp_bind(pcb, IP_ADDR_ANY, port);
    if (err != ERR_OK) {
        tcp_close(pcb);
        return false;
    }

    tcp_pcb *listen_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!listen_pcb) {
        tcp_close(pcb);
        return false;
    }

    tcp_accept(listen_pcb, on_accept);
    return true;
}

/*
MIT License

Copyright (c) 2024 Lauterbach GmbH

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "comm.hpp"

mcd_exception::mcd_exception(const mcd_error_info_st &error_info)
    : error_info{error_info}
{
}

const char *mcd_exception::what() { return error_info.error_str; }

#if defined(WIN32)
int MCDServer::winsock_connections {0};
#endif

MCDServer::MCDServer(const std::string &host, int port)
    : host{host}, port{port}, msg_buf{buf + sizeof(uint32_t)}
{
#if defined(WIN32)
    if (MCDServer::winsock_connections == 0) {
        WSADATA d;
        if (WSAStartup(MAKEWORD(2, 2), &d)) {
            throw mcd_exception{mcd_error_info_st{
                .return_status{MCD_RET_ACT_HANDLE_ERROR},
                .error_code{MCD_ERR_CONNECTION},
                .error_events{MCD_ERR_EVT_NONE},
                .error_str{"winsock initialization failed"},
            }};
        }
    }
#endif

    this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (!ISVALIDSOCKET(this->socket_fd)) {
#if defined(WIN32)
        if (MCDServer::winsock_connections == 0) {
            WSACleanup();
        }
#endif
        throw mcd_exception{mcd_error_info_st{
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_CONNECTION},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"socket creation failed"},
        }};
    }

#if defined(WIN32)
    MCDServer::winsock_connections++;
#endif
}

MCDServer::MCDServer(MCDServer &&other)
    : msg_buf{this->buf + sizeof(uint32_t)},
      socket_fd{other.socket_fd},
      connected{other.connected},
      host{other.host},
      port{other.port}
{
    other.socket_fd = 0;
    other.connected = false;
}

MCDServer::~MCDServer()
{
    if (this->socket_fd) {
        shutdown(this->socket_fd, SHUTDOWN_ALL);
        CLOSESOCKET(this->socket_fd);
#if defined(WIN32)
        winsock_connections--;
        if (MCDServer::winsock_connections <= 0) {
            MCDServer::winsock_connections = 0;
            WSACleanup();
        }
#endif
    }
}

MCDServer &MCDServer::operator=(MCDServer &&other)
{
    socket_fd = other.socket_fd;
    connected = other.connected;
    host = other.host;
    port = other.port;
    other.socket_fd = 0;
    other.connected = false;
    other.host.clear();
    other.port = 0;
    return *this;
}

MCDServer MCDServer::Open(const std::string &host, int port)
{
    MCDServer server{host, port};
    mcd_error_info_st error;

    if (server.connect_to_target(error) != MCD_RET_ACT_NONE) {
        throw mcd_exception{error};
    }

    return server;
}

mcd_return_et MCDServer::connect_to_target(mcd_error_info_st &error)
{
    struct sockaddr_in server_address {
        .sin_family{AF_INET}, .sin_port{htons((u_short)this->port)},
    };
    inet_pton(AF_INET, this->host.c_str(), &server_address.sin_addr.s_addr);

    if (connect(this->socket_fd, (struct sockaddr *)&server_address,
                sizeof(server_address)) != 0) {
        error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_CONNECTION},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"TCP connection failed"},
        };
        return error.return_status;
    }

    this->connected = true;
    return MCD_RET_ACT_NONE;
}

mcd_return_et MCDServer::request(uint32_t request_size,
                                 mcd_error_info_st &error)
{
    if (!this->connected) {
        if (this->socket_fd) {
            CLOSESOCKET(this->socket_fd);
        }

        this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (!ISVALIDSOCKET(this->socket_fd)) {
            this->socket_fd = 0;
            error = {mcd_error_info_st{
                .return_status{MCD_RET_ACT_HANDLE_ERROR},
                .error_code{MCD_ERR_CONNECTION},
                .error_events{MCD_ERR_EVT_NONE},
                .error_str{"socket creation failed while reconnecting"},
            }};
            return error.return_status;
        }

        int optval{TRUE};
        setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval,
                   sizeof(int));
        if (this->connect_to_target(error) != MCD_RET_ACT_NONE) {
            error = {
                .return_status{MCD_RET_ACT_HANDLE_ERROR},
                .error_code{MCD_ERR_CONNECTION},
                .error_events{MCD_ERR_EVT_NONE},
                .error_str{"server reconnection failed"},
            };
            return error.return_status;
        };
    }

    request_size += marshal_uint32_t(request_size, this->buf);
    if (send(this->socket_fd, (char *)this->buf, (int)request_size, 0) ==
        SOCKET_ERROR) {
        this->connected = false;
        error = {
            .return_status{MCD_RET_ACT_HANDLE_EVENT},
            .error_code{MCD_ERR_CONNECTION},
            .error_events{MCD_ERR_EVT_PWRDN},
            .error_str{"sending request failed (socket error)"},
        };
        return error.return_status;
    }

    return MCD_RET_ACT_NONE;
}

mcd_return_et MCDServer::request_response(uint32_t request_size,
                                          mcd_error_info_st &error)
{
    mcd_return_et ret{this->request(request_size, error)};
    if (ret != MCD_RET_ACT_NONE) {
        return ret;
    }

    uint32_t length;
    long int result{recv(this->socket_fd, (char *)&length, sizeof(length), 0)};
    if (result != sizeof(length)) {
        this->connected = false;
        error = {
            .return_status{MCD_RET_ACT_HANDLE_EVENT},
            .error_code{MCD_ERR_CONNECTION},
            .error_events{MCD_ERR_EVT_PWRDN},
            .error_str{"receiving response failed (socket error)"},
        };
        return error.return_status;
    }

    if (length > MCD_MAX_PACKET_LENGTH) {
        error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_CONNECTION},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"receiving response failed (length exceeds limit)"},
        };

        return error.return_status;
    } else {
        uint32_t received_bytes = 0;
        do {
            received_bytes +=
                recv(this->socket_fd, (char *)this->msg_buf + received_bytes,
                     length, 0);
        } while (received_bytes < length);

        if (received_bytes != length) {
            error = {
                .return_status{MCD_RET_ACT_HANDLE_ERROR},
                .error_code{MCD_ERR_CONNECTION},
                .error_events{MCD_ERR_EVT_NONE},
                .error_str{"receiving response failed (lengths do not match)"},
            };
            return error.return_status;
        }
    }

    return MCD_RET_ACT_NONE;
}

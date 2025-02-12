/*
 * MIT License
 *
 * Copyright (c) 2025 Lauterbach GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "comm.hpp"

mcd_exception::mcd_exception(const mcd_error_info_st &error_info)
    : error_info{error_info}
{
}

const char *mcd_exception::what() { return error_info.error_str; }

#if defined(WIN32)
int MCDServer::winsock_connections{0};
#endif

MCDServer::MCDServer(const std::string &host, int port)
    : host{host}, port{port}, msg_buf{buf}
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
    : msg_buf{this->buf},
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
    struct addrinfo *servinfo, *a;
    struct addrinfo hints{
        .ai_family{AF_UNSPEC},
        .ai_socktype{SOCK_STREAM},
    };

    std::string port_s{std::to_string(this->port)};
    int gai_ret{
        getaddrinfo(this->host.c_str(), port_s.c_str(), &hints, &servinfo)};
    if (gai_ret != 0) {
        error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_CONNECTION},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{""},
        };
        snprintf(error.error_str, MCD_INFO_STR_LEN,
                 "TCP connection failed (%s)", gai_strerror(gai_ret));
        return error.return_status;
    }

    for (a = servinfo; a; a = a->ai_next) {
        if (connect(this->socket_fd, a->ai_addr, (int)a->ai_addrlen) != 0) {
            error = {
                .return_status{MCD_RET_ACT_HANDLE_ERROR},
                .error_code{MCD_ERR_CONNECTION},
                .error_events{MCD_ERR_EVT_NONE},
                .error_str{},
            };
            snprintf(error.error_str, MCD_INFO_STR_LEN,
                     "TCP connection failed (%d)", GETSOCKETERRNO());
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (!a) {
        /* failed to connect */
        return error.return_status;
    }

    this->connected = true;
    return MCD_RET_ACT_NONE;
}

mcd_return_et MCDServer::send_message(uint32_t request_size,
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

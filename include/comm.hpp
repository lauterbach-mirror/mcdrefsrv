/*
MIT License

Copyright (c) 2025 Lauterbach GmbH

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

#pragma once

#include <string>

#include "mcd_api.h"
#include "mcd_rpc.h"

#if defined(WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())
#define SHUTDOWN_ALL SD_BOTH
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define SOCKET int
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define GETSOCKETERRNO() (errno)
#define SHUTDOWN_ALL SHUT_RDWR
#define SOCKET_ERROR (-1)
#endif

#define LOCALHOST "127.0.0.1"
#define MCD_DEFAULT_TCP_PORT 1235
#define MCD_MAX_PACKET_LENGTH 65535

/**
 * \brief Custom MCD exception type with error info encoded as
 * \c mcd_error_info_st.
 */
struct mcd_exception : public std::exception {
    const mcd_error_info_st error_info;
    mcd_exception(const mcd_error_info_st &error_info);
    const char *what();
};

/** \brief Provides a communication channel with the MCD server.
 */
class MCDServer
{
#if defined(WIN32)
    static int winsock_connections;
#endif
    MCDServer(const std::string &host, int port);
    mcd_return_et connect_to_target(mcd_error_info_st &error);
    std::string host;
    int port;
    SOCKET socket_fd;
    bool connected;
    char buf[MCD_MAX_PACKET_LENGTH];

public:
    uint32_t server_uid;
    char *const msg_buf;

    /**
     * \brief Initializes a new TCP connection to a MCD server.
     *
     * @throws \c mcd_exception
     *
     * @param host Host name of the server socket.
     * @param port TCP port number of the server socket.
     */
    static MCDServer Open(const std::string &host, int port);

    /**
     * \brief Checks whether the server is currently in the "connected" state.
     */
    bool is_connected()
    {
        return this->connected;
    }

    /**
     * \brief Sends a message to the MCD server.
     *
     * When the method is called, the message is expected to be at the
     * beginning of msg_buf.
     *
     * @throws \c mcd_exception
     *
     * @param len Message length in bytes.
     * @param error Error information in case of failure.
     *
     * @returns Return code as defined in MCD API.
     */
    mcd_return_et send_message(uint32_t len, mcd_error_info_st &error);

    /**
     * \brief Receives messages from the server.
     *
     * On success, the messages will be at the beginning of msg_buf.
     *
     * When using a protocol like QMP, the server might also send messages that
     * are not sent as a response to a RPC request. For that reason, the
     * constraint that one request message precedes exactly one response message
     * does not hold anymore.
     *
     * @param error Error information in case of failure.
     *
     * @returns Return code as defined in MCD API.
     */
    mcd_return_et receive_messages(mcd_error_info_st &error);

    MCDServer(MCDServer &) = delete;
    MCDServer &operator=(MCDServer &other) = delete;
    MCDServer(MCDServer &&);
    MCDServer &operator=(MCDServer &&other);

    /**
     * \brief Closes the TCP connection.
     */
    ~MCDServer();
};

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

#define TIMEOUT_SECONDS 5

mcd_return_et MCDServer::receive_messages(mcd_error_info_st &error)
{
    fd_set readfds;
    struct timeval tv{
        .tv_sec{TIMEOUT_SECONDS},
    };

    static constexpr char DELIMITER = '\n';
    uint32_t received_bytes{0};
    do {
        FD_ZERO(&readfds);
        FD_SET(this->socket_fd, &readfds);
        select((int)this->socket_fd + 1, &readfds, NULL, NULL, &tv);
        if (!FD_ISSET(this->socket_fd, &readfds)) {
            this->connected = false;
            error = {
                .return_status{MCD_RET_ACT_HANDLE_ERROR},
                .error_code{MCD_ERR_TIMED_OUT},
                .error_events{MCD_ERR_EVT_NONE},
                .error_str{"receiving response failed (timeout)"},
            };
            return error.return_status;
        }

        long int num_bytes{recv(this->socket_fd,
                                (char *)this->buf + received_bytes,
                                MCD_MAX_PACKET_LENGTH - received_bytes, 0)};

        if (num_bytes == 0) {
            this->connected = false;
            error = {
                .return_status{MCD_RET_ACT_HANDLE_EVENT},
                .error_code{MCD_ERR_CONNECTION},
                .error_events{MCD_ERR_EVT_PWRDN},
                .error_str{"receiving response failed (connection closed)"},
            };
            return error.return_status;
        } else if (num_bytes < 0) {
            error = {
                .return_status{MCD_RET_ACT_HANDLE_EVENT},
                .error_code{MCD_ERR_CONNECTION},
                .error_events{MCD_ERR_EVT_NONE},
                .error_str{""},
            };
            snprintf(error.error_str, MCD_INFO_STR_LEN,
                     "receiving response failed (%d)", GETSOCKETERRNO());
            return error.return_status;
        }

        received_bytes += num_bytes;
    } while (received_bytes < MCD_MAX_PACKET_LENGTH &&
             this->buf[received_bytes - 1] != DELIMITER);

    if (this->buf[received_bytes - 1] != DELIMITER) {
        error = {
            .return_status{MCD_RET_ACT_HANDLE_EVENT},
            .error_code{MCD_ERR_CONNECTION},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"receiving response failed (overflow)"},
        };
        /* TODO: handle overflow (clear buffer) */
        return error.return_status;
    }

    this->buf[received_bytes] = '\0';
    return MCD_RET_ACT_NONE;
}

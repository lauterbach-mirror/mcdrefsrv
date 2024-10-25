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

#include <cassert>
#include <cstring>
#include <optional>

#include "mcd_api.h"

/* Use custom serial protocol */
#include "mcd_rpc.h"

/* TODO: Add QMP support*/

#include "comm.hpp"
#include "adapter.hpp"

/*
 * The Client Stub realizes two functions:
 *   1. It serves as a communication channel between client and server.
 *   2. It serves as an adapter between client and server.
 *
 * Client         Client Stub         Server
 *      |                             |
 *      |     Communication Channel   |     The client stub sends API calls from
 *      |                             |     the client as requests to the 
 *      | --------- Request --------> |     server. In most cases, a response
 *      | <------- [Response] ------- |     from the server is expected.
 *      |                             |
 *      |                             |
 *      |                             |
 *      |           Adapter           |     The client might expect a different
 *      |                             |     representation of core information
 *      |      -----------------      |     (like memory spaces and registers)
 *      |     | Memory Spaces   |     |     than the server provides, e.g.
 *      | <---| Register Groups |---- |     - memory spaces with different names
 *      |     | Registers       |     |     - registers with different addresses
 *      |      -----------------      |     - additional (redundant) registers
 *      |      -----------------      |
 *      | ----| Transactions    |---> |     Then, when the client accesses
 *      |      -----------------      |     the converted memory / registers,
 *      |                             |     the transaction has to be converted
 *      |                             |     before sending the request to and
 *      |                             |     after receiving the response from
 *      |                             |     the server.
 */

#define MCD_LIB_VENDOR "Lauterbach Engineering"
const uint16_t MCD_LIB_BUILD{(uint16_t)strtoul(REVISION_STRING + 0, 0, 0)};
#define MCD_LIB_DATE __DATE__

/* Common Error Scenarios */

const mcd_error_info_st MCD_ERROR_NONE{
    .return_status{MCD_RET_ACT_NONE},
    .error_code{MCD_ERR_NONE},
    .error_events{MCD_ERR_EVT_NONE},
    .error_str{},
};

const mcd_error_info_st MCD_ERROR_NOT_IMPLEMENTED{
    .return_status{MCD_RET_ACT_HANDLE_ERROR},
    .error_code{MCD_ERR_FN_UNIMPLEMENTED},
    .error_events{MCD_ERR_EVT_NONE},
    .error_str{},
};

const mcd_error_info_st MCD_ERROR_MARSHAL{
    .return_status{MCD_RET_ACT_HANDLE_ERROR},
    .error_code{MCD_ERR_RPC_MARSHAL},
    .error_events{MCD_ERR_EVT_NONE},
    .error_str{"error during argument marshalling"},
};

const mcd_error_info_st MCD_ERROR_UNMARSHAL{
    .return_status{MCD_RET_ACT_HANDLE_ERROR},
    .error_code{MCD_ERR_RPC_UNMARSHAL},
    .error_events{MCD_ERR_EVT_NONE},
    .error_str{"error during argument unmarshalling"},
};

const mcd_error_info_st MCD_ERROR_SERVER_NOT_OPEN{
    .return_status{MCD_RET_ACT_HANDLE_ERROR},
    .error_code{MCD_ERR_CONNECTION},
    .error_events{MCD_ERR_EVT_NONE},
    .error_str{"server not open"},
};

const mcd_error_info_st MCD_ERROR_UNKNOWN_SERVER{
    .return_status{MCD_RET_ACT_HANDLE_ERROR},
    .error_code{MCD_ERR_USAGE},
    .error_events{MCD_ERR_EVT_NONE},
    .error_str{"server unknown to client stub"},
};

const mcd_error_info_st MCD_ERROR_INVALID_NULL_PARAM{
    .return_status{MCD_RET_ACT_HANDLE_ERROR},
    .error_code{MCD_ERR_PARAM},
    .error_events{MCD_ERR_EVT_NONE},
    .error_str{"null was invalidly passed as a parameter"},
};

/* Reserves memory for special error scenarios */
static mcd_error_info_st custom_mcd_error{};

/* Indicates that the error information is stored on server side */
const mcd_error_info_st MCD_ERROR_ASK_SERVER{};

const mcd_error_info_st *last_error{&MCD_ERROR_NONE};

static std::optional<MCDServer> g_mcd_server{};

mcd_return_et mcd_initialize_f(const mcd_api_version_st *version_req,
                               mcd_impl_version_info_st *impl_info)
{
    if (!version_req || !impl_info) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    mcd_api_version_st api_version{
        .v_api_major{MCD_API_VER_MAJOR},
        .v_api_minor{MCD_API_VER_MINOR},
        .author{MCD_API_VER_AUTHOR},
    };

    *impl_info = {
        .v_api{api_version},
        .v_imp_major{MCD_API_VER_MAJOR},
        .v_imp_minor{MCD_API_VER_MINOR},
        .v_imp_build{MCD_LIB_BUILD},
        .vendor{MCD_LIB_VENDOR},
        .date{MCD_LIB_DATE},
    };

    /* For now, only allow exact version matches */
    if (version_req->v_api_major == api_version.v_api_major &&
        version_req->v_api_minor == api_version.v_api_minor) {
        last_error = &MCD_ERROR_NONE;
    } else {
        custom_mcd_error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_GENERAL},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{},
        };
        last_error = &custom_mcd_error;
    }

    return last_error->return_status;
}

void mcd_exit_f(void)
{
    if (!g_mcd_server) {
        /* no server connection active */
        last_error = &MCD_ERROR_NONE;
        return;
    }

    marshal_uint8_t(UID_MCD_EXIT, g_mcd_server->msg_buf);
    if (g_mcd_server->request(1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
    } else {
        last_error = &MCD_ERROR_NONE;
    }

    g_mcd_server = std::nullopt;
}

mcd_return_et mcd_qry_servers_f(const mcd_char_t *host, mcd_bool_t running,
                                uint32_t start_index, uint32_t *num_servers,
                                mcd_server_info_st *server_info)
{
    last_error = &MCD_ERROR_NOT_IMPLEMENTED;
    return last_error->return_status;
}

mcd_return_et mcd_open_server_f(const mcd_char_t *system_key,
                                const mcd_char_t *config_string,
                                mcd_server_st **server)
{
    if (!server || !system_key || !config_string) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (g_mcd_server.has_value()) {
        custom_mcd_error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_CONNECTION},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"server already open: close first"},
        };
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    std::string host{LOCALHOST};
    int port{MCD_DEFAULT_TCP_PORT};
    if (config_string && strlen(config_string)) {
        /* expected format: <hostname>:<port> */
        std::string s{config_string};
        size_t i{s.find_first_of(':')};
        if (i == 0 || i == std::string::npos || i != s.find_last_of(':')) {
            custom_mcd_error = {
                .return_status{MCD_RET_ACT_HANDLE_ERROR},
                .error_code{MCD_ERR_PARAM},
                .error_events{MCD_ERR_EVT_NONE},
                .error_str{"ill-formed config_string, expected: "
                           "<hostname>:<port>"},
            };
            last_error = &custom_mcd_error;
            return last_error->return_status;
        }

        host = {s, 0, i};
        std::string port_s{s, i + 1, std::string::npos};

        try {
            port = std::stoi(port_s);
        } catch (std::exception const &) {
            custom_mcd_error = {
                .return_status{MCD_RET_ACT_HANDLE_ERROR},
                .error_code{MCD_ERR_PARAM},
                .error_events{MCD_ERR_EVT_NONE},
                .error_str{"ill-formed config_string, expected: "
                           "<hostname>:<port>"},
            };
            last_error = &custom_mcd_error;
            return last_error->return_status;
        }
    }

    try {
        g_mcd_server = MCDServer::Open(host, port);
    } catch (const mcd_exception &ex) {
        custom_mcd_error = ex.error_info;
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_open_server_args args{
        .system_key_len{(uint32_t)strlen(system_key)},
        .system_key{(mcd_char_t *)system_key},
        .config_string_len{(uint32_t)strlen(config_string)},
        .config_string{(mcd_char_t *)config_string},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_OPEN_SERVER, buf);
    uint32_t req_len{marshal_mcd_open_server_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_open_server_result res;
    if (unmarshal_mcd_open_server_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        g_mcd_server->server_uid = res.server_uid;
        *server = new mcd_server_st{
            .instance{&(*g_mcd_server)},
            .host{new mcd_char_t[]{LOCALHOST}},
        };
        last_error = &MCD_ERROR_NONE;
    } else {
        custom_mcd_error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_CONNECTION},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"opening the connection failed on server side"},
        };
        last_error = &custom_mcd_error;
    }

    free_mcd_open_server_result(&res);
    return last_error->return_status;
}

mcd_return_et mcd_close_server_f(const mcd_server_st *server)
{
    if (!server) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server.has_value() || server->instance != &(*g_mcd_server)) {
        custom_mcd_error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_USAGE},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"wrong server would be closed"},
        };
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_close_server_args args{
        .server_uid = g_mcd_server->server_uid,
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_CLOSE_SERVER, buf);
    uint32_t req_len{marshal_mcd_close_server_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_close_server_result res;
    if (unmarshal_mcd_close_server_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        delete[] server->host;
        delete server;
        g_mcd_server = std::nullopt;
        last_error = &MCD_ERROR_NONE;
    } else {
        custom_mcd_error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_CONNECTION},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"closing the connection failed on server side"},
        };
        last_error = &custom_mcd_error;
    }

    return last_error->return_status;
}

mcd_return_et mcd_set_server_config_f(const mcd_server_st *server,
                                      const mcd_char_t *config_string)
{
    if (!server) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server.has_value() || server->instance != &(*g_mcd_server)) {
        last_error = &MCD_ERROR_UNKNOWN_SERVER;
        return last_error->return_status;
    }

    mcd_set_server_config_args args{
        .server_uid{g_mcd_server->server_uid},
        .config_string_len{(uint32_t)strlen(config_string)},
        .config_string{(mcd_char_t *)config_string},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_SET_SERVER_CONFIG, buf);
    uint32_t req_len{marshal_mcd_set_server_config_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_set_server_config_result res;
    if (unmarshal_mcd_set_server_config_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_server_config_f(const mcd_server_st *server,
                                      uint32_t *max_len,
                                      mcd_char_t *config_string)
{
    if (!server || !max_len || !config_string) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server.has_value() || server->instance != &(*g_mcd_server)) {
        last_error = &MCD_ERROR_UNKNOWN_SERVER;
        return last_error->return_status;
    }

    mcd_qry_server_config_args args{
        .server_uid{g_mcd_server->server_uid},
        .max_len{*max_len},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_SERVER_CONFIG, buf);
    uint32_t req_len{marshal_mcd_qry_server_config_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_server_config_result res;
    if (unmarshal_mcd_qry_server_config_result(g_mcd_server->msg_buf, &res) ==
        0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        std::string{res.config_string}.copy(config_string,
                                            res.config_string_len, 0);
        *max_len = res.max_len;
    }

    free_mcd_qry_server_config_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_systems_f(uint32_t start_index, uint32_t *num_systems,
                                mcd_core_con_info_st *system_con_info)
{
    if (!num_systems || (*num_systems && !system_con_info)) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    mcd_qry_systems_args args{
        .start_index{start_index},
        .num_systems{*num_systems},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_SYSTEMS, buf);
    uint32_t req_len{marshal_mcd_qry_systems_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_systems_result res;
    if (unmarshal_mcd_qry_systems_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_systems = res.num_systems;
        for (uint32_t i = 0; i < res.system_con_info_len; i++) {
            system_con_info[i] = res.system_con_info[i];
        }
    }

    free_mcd_qry_systems_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_devices_f(const mcd_core_con_info_st *system_con_info,
                                uint32_t start_index, uint32_t *num_devices,
                                mcd_core_con_info_st *device_con_info)
{
    if (!system_con_info || !num_devices ||
        (*num_devices && !device_con_info)) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    mcd_qry_devices_args args{
        .system_con_info{*system_con_info},
        .start_index{start_index},
        .num_devices{*num_devices},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_DEVICES, buf);
    uint32_t req_len{marshal_mcd_qry_devices_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_devices_result res;
    if (unmarshal_mcd_qry_devices_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_devices = res.num_devices;
        for (uint32_t i = 0; i < res.device_con_info_len; i++) {
            device_con_info[i] = res.device_con_info[i];
        }
    }

    free_mcd_qry_devices_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_cores_f(const mcd_core_con_info_st *connection_info,
                              uint32_t start_index, uint32_t *num_cores,
                              mcd_core_con_info_st *core_con_info)
{
    if (!num_cores || (*num_cores && !core_con_info)) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    mcd_qry_cores_args args{
        .connection_info{*connection_info},
        .start_index{start_index},
        .num_cores{*num_cores},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_CORES, buf);
    uint32_t req_len{marshal_mcd_qry_cores_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_cores_result res;
    if (unmarshal_mcd_qry_cores_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_cores = res.num_cores;
        for (uint32_t i = 0; i < res.core_con_info_len; i++) {
            core_con_info[i] = res.core_con_info[i];
        }
    }

    free_mcd_qry_cores_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_core_modes_f(const mcd_core_st *core,
                                   uint32_t start_index, uint32_t *num_modes,
                                   mcd_core_mode_info_st *core_mode_info)
{
    if (!core || !core->instance || !num_modes ||
        (*num_modes && !core_mode_info)) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_core_modes_args args{
        .core_uid{adapter->core_uid},
        .start_index{start_index},
        .num_modes{*num_modes},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_CORE_MODES, buf);
    uint32_t req_len{marshal_mcd_qry_core_modes_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_core_modes_result res;
    if (unmarshal_mcd_qry_core_modes_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_modes = res.num_modes;
        for (uint32_t i = 0; i < res.core_mode_info_len; i++) {
            core_mode_info[i] = res.core_mode_info[i];
        }
    }

    free_mcd_qry_core_modes_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_open_core_f(const mcd_core_con_info_st *core_con_info,
                              mcd_core_st **core)
{
    if (!core_con_info || !core) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    mcd_open_core_args args{
        .core_con_info{*core_con_info},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_OPEN_CORE, buf);
    uint32_t req_len{marshal_mcd_open_core_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_open_core_result res;
    if (unmarshal_mcd_open_core_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret != MCD_RET_ACT_NONE) {
        last_error = &MCD_ERROR_ASK_SERVER;
        return res.ret;
    }

    Core *adapter{new Core{res.core_con_info, res.core_uid}};
    *core = new mcd_core_st{
        .instance{adapter},
        .core_con_info{new mcd_core_con_info_st{res.core_con_info}},
    };

    if (adapter->update_core_database(custom_mcd_error) != MCD_RET_ACT_NONE) {
        mcd_return_et ret{mcd_close_core_f(*core)};
        assert(ret == MCD_RET_ACT_NONE);
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_NONE;
    return last_error->return_status;
}

mcd_return_et mcd_close_core_f(const mcd_core_st *core)
{
    if (!core || !core->instance) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_close_core_args args{
        .core_uid{adapter->core_uid},
    };

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_CLOSE_CORE, buf);
    uint32_t req_len{marshal_mcd_close_core_args(&args, buf)};

    /*
     * After passing through the mcd_close_core_f request,
     * the following errors might occur:
     * 1. The server connection is not established anymore (might be because
     * server is down)
     * 2. Error during transmission of request or response
     * 3. Error in mcd_close_core_f of server
     */

    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        if ((custom_mcd_error.return_status == MCD_RET_ACT_HANDLE_EVENT) &&
            (custom_mcd_error.error_events & MCD_ERR_EVT_PWRDN)) {
            delete adapter;
            delete core->core_con_info;
            delete core;

            /* since target is powered down, we did everything we could */
            last_error = &MCD_ERROR_NONE;
        } else {
            /* error during transmission */
            last_error = &custom_mcd_error;
        }

        return last_error->return_status;
    }

    mcd_close_core_result res;
    if (unmarshal_mcd_close_core_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        delete adapter;
        delete core->core_con_info;
        delete core;
    }
    /*
     * else, don't free
     * we might need to keep the information for another try
     */

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

void mcd_qry_error_info_f(const mcd_core_st *core,
                          mcd_error_info_st *error_info)
{
    if (!error_info) {
        return;
    }

    if (last_error != &MCD_ERROR_ASK_SERVER) {
        /* todo: core specific */
        *error_info = *last_error;
        return;
    }

    /*
     * We need to request the error info from the server
     * If an error occurs during transmission, return this error instead
     * (mcd_qry_error_info should not fail in normal operation)
     */

    if (!g_mcd_server) {
        *error_info = MCD_ERROR_SERVER_NOT_OPEN;
        return;
    }

    mcd_qry_error_info_args args{
        .has_core_uid{core && core->instance},
    };

    if (args.has_core_uid) {
        Core *adapter{(Core *)core->instance};
        args.core_uid = adapter->core_uid;
    }

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_ERROR_INFO, buf);
    uint32_t req_len{marshal_mcd_qry_error_info_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        *error_info = custom_mcd_error;
        return;
    }

    mcd_qry_error_info_result res;
    if (unmarshal_mcd_qry_error_info_result(g_mcd_server->msg_buf, &res) == 0) {
        *error_info = MCD_ERROR_UNMARSHAL;
        return;
    }

    *error_info = res.error_info;
}

mcd_return_et mcd_qry_device_description_f(const mcd_core_st *core,
                                           mcd_char_t *url,
                                           uint32_t *url_length)
{
    if (!core || !core->instance || !url_length) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_device_description_args args{
        .core_uid{adapter->core_uid},
        .url_length{*url_length},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_DEVICE_DESCRIPTION, buf);
    uint32_t req_len{marshal_mcd_qry_device_description_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_device_description_result res;
    if (unmarshal_mcd_qry_device_description_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        if (url) {
            std::string{res.url}.copy(url, res.url_len, 0);
        }
        *url_length = res.url_length;
    }

    free_mcd_qry_device_description_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_max_payload_size_f(const mcd_core_st *core,
                                         uint32_t *max_payload)
{
    if (!core || !core->instance || !max_payload) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_max_payload_size_args args{
        .core_uid{adapter->core_uid},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_MAX_PAYLOAD_SIZE, buf);
    uint32_t req_len{marshal_mcd_qry_max_payload_size_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_max_payload_size_result res;
    if (unmarshal_mcd_qry_max_payload_size_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *max_payload = res.max_payload;
    }

    last_error = &MCD_ERROR_NONE;
    return last_error->return_status;
}

mcd_return_et mcd_qry_input_handle_f(const mcd_core_st *core,
                                     uint32_t *input_handle)
{
    if (!core || !core->instance || !input_handle) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_input_handle_args args{
        .core_uid{adapter->core_uid},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_INPUT_HANDLE, buf);
    uint32_t req_len{marshal_mcd_qry_input_handle_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_input_handle_result res;
    if (unmarshal_mcd_qry_input_handle_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *input_handle = res.input_handle;
    }

    last_error = &MCD_ERROR_NONE;
    return last_error->return_status;
}

mcd_return_et mcd_qry_mem_spaces_f(const mcd_core_st *core,
                                   uint32_t start_index,
                                   uint32_t *num_mem_spaces,
                                   mcd_memspace_st *mem_spaces)
{
    if (!core || !core->instance || !num_mem_spaces) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};

    if (adapter->core_database_updated()) {
        if (adapter->query_mem_spaces(start_index, num_mem_spaces, mem_spaces,
                                      custom_mcd_error) != MCD_RET_ACT_NONE) {
            last_error = &custom_mcd_error;
        } else {
            last_error = &MCD_ERROR_NONE;
        }
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    mcd_qry_mem_spaces_args args{
        .core_uid{adapter->core_uid},
        .start_index{start_index},
        .num_mem_spaces{*num_mem_spaces},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_MEM_SPACES, buf);
    uint32_t req_len{marshal_mcd_qry_mem_spaces_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_mem_spaces_result res;
    if (unmarshal_mcd_qry_mem_spaces_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_mem_spaces = res.num_mem_spaces;
        for (uint32_t i = 0; i < res.mem_spaces_len; i++) {
            mem_spaces[i] = res.mem_spaces[i];
        }
    }

    free_mcd_qry_mem_spaces_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_mem_blocks_f(const mcd_core_st *core,
                                   uint32_t mem_space_id, uint32_t start_index,
                                   uint32_t *num_mem_blocks,
                                   mcd_memblock_st *mem_blocks)
{
    if (!core || !core->instance || !num_mem_blocks ||
        (*num_mem_blocks && !mem_blocks)) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_mem_blocks_args args{
        .core_uid{adapter->core_uid},
        .mem_space_id{mem_space_id},
        .start_index{start_index},
        .num_mem_blocks{*num_mem_blocks},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_MEM_BLOCKS, buf);
    uint32_t req_len{marshal_mcd_qry_mem_blocks_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_mem_blocks_result res;
    if (unmarshal_mcd_qry_mem_blocks_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_mem_blocks = res.num_mem_blocks;
        for (uint32_t i = 0; i < res.mem_blocks_len; i++) {
            mem_blocks[i] = res.mem_blocks[i];
        }
    }

    free_mcd_qry_mem_blocks_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_active_overlays_f(const mcd_core_st *core,
                                        uint32_t start_index,
                                        uint32_t *num_active_overlays,
                                        uint32_t *active_overlays)
{
    if (!core || !core->instance || !num_active_overlays ||
        (*num_active_overlays && !active_overlays)) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }
    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_active_overlays_args args{
        .core_uid{adapter->core_uid},
        .start_index{start_index},
        .num_active_overlays{*num_active_overlays},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_ACTIVE_OVERLAYS, buf);
    uint32_t req_len{marshal_mcd_qry_active_overlays_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_active_overlays_result res;
    if (unmarshal_mcd_qry_active_overlays_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_active_overlays = res.num_active_overlays;
        for (uint32_t i = 0; i < res.active_overlays_len; i++) {
            active_overlays[i] = res.active_overlays[i];
        }
    }

    free_mcd_qry_active_overlays_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_reg_groups_f(const mcd_core_st *core,
                                   uint32_t start_index,
                                   uint32_t *num_reg_groups,
                                   mcd_register_group_st *reg_groups)
{
    if (!core || !core->instance || !num_reg_groups) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};

    if (adapter->core_database_updated()) {
        if (adapter->query_reg_groups(start_index, num_reg_groups, reg_groups,
                                      custom_mcd_error) != MCD_RET_ACT_NONE) {
            last_error = &custom_mcd_error;
        } else {
            last_error = &MCD_ERROR_NONE;
        }
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    mcd_qry_reg_groups_args args{
        .core_uid{adapter->core_uid},
        .start_index{start_index},
        .num_reg_groups{*num_reg_groups},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_REG_GROUPS, buf);
    uint32_t req_len{marshal_mcd_qry_reg_groups_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_reg_groups_result res;
    if (unmarshal_mcd_qry_reg_groups_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_reg_groups = res.num_reg_groups;
        for (uint32_t i = 0; i < res.reg_groups_len; i++) {
            reg_groups[i] = res.reg_groups[i];
        }
    }

    free_mcd_qry_reg_groups_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_reg_map_f(const mcd_core_st *core, uint32_t reg_group_id,
                                uint32_t start_index, uint32_t *num_regs,
                                mcd_register_info_st *reg_info)
{
    if (!core || !core->instance || !num_regs) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};

    if (adapter->core_database_updated()) {
        if (adapter->query_reg_map(reg_group_id, start_index, num_regs,
                                   reg_info,
                                   custom_mcd_error) != MCD_RET_ACT_NONE) {
            last_error = &custom_mcd_error;
            return last_error->return_status;
        }

        last_error = &MCD_ERROR_NONE;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    mcd_qry_reg_map_args args{
        .core_uid{adapter->core_uid},
        .reg_group_id{reg_group_id},
        .start_index{start_index},
        .num_regs{*num_regs},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_REG_MAP, buf);
    uint32_t req_len{marshal_mcd_qry_reg_map_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_reg_map_result res;
    if (unmarshal_mcd_qry_reg_map_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_regs = res.num_regs;
        for (uint32_t i = 0; i < res.reg_info_len; i++) {
            reg_info[i] = res.reg_info[i];
        }
    }

    free_mcd_qry_reg_map_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_reg_compound_f(const mcd_core_st *core,
                                     uint32_t compound_reg_id,
                                     uint32_t start_index,
                                     uint32_t *num_reg_ids,
                                     uint32_t *reg_id_array)
{
    if (!core || !core->instance || !num_reg_ids ||
        (*num_reg_ids && !reg_id_array)) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_reg_compound_args args{
        .core_uid{adapter->core_uid},
        .compound_reg_id{compound_reg_id},
        .start_index{start_index},
        .num_reg_ids{*num_reg_ids},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_REG_COMPOUND, buf);
    uint32_t req_len{marshal_mcd_qry_reg_compound_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_reg_compound_result res;
    if (unmarshal_mcd_qry_reg_compound_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_reg_ids = res.num_reg_ids;
        for (uint32_t i = 0; i < res.reg_id_array_len; i++) {
            reg_id_array[i] = res.reg_id_array[i];
        }
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_trig_info_f(const mcd_core_st *core,
                                  mcd_trig_info_st *trig_info)
{
    if (!core || !core->instance) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_trig_info_args args{
        .core_uid{adapter->core_uid},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_TRIG_INFO, buf);
    uint32_t req_len{marshal_mcd_qry_trig_info_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_trig_info_result res;
    if (unmarshal_mcd_qry_trig_info_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *trig_info = res.trig_info;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_ctrigs_f(const mcd_core_st *core, uint32_t start_index,
                               uint32_t *num_ctrigs,
                               mcd_ctrig_info_st *ctrig_info)
{
    if (!core || !core->instance || !num_ctrigs) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_ctrigs_args args{
        .core_uid{adapter->core_uid},
        .start_index{start_index},
        .num_ctrigs{*num_ctrigs},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_CTRIGS, buf);
    uint32_t req_len{marshal_mcd_qry_ctrigs_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_ctrigs_result res;
    if (unmarshal_mcd_qry_ctrigs_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_ctrigs = res.num_ctrigs;
        for (uint32_t i = 0; i < res.ctrig_info_len; i++) {
            ctrig_info[i] = res.ctrig_info[i];
        }
    }

    free_mcd_qry_ctrigs_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_create_trig_f(const mcd_core_st *core, void *trig,
                                uint32_t *trig_id)
{
    if (!core || !core->instance || !trig || !trig_id) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    uint32_t trig_struct_size{*(uint32_t *)trig};
    mcd_create_trig_args args{
        .core_uid{adapter->core_uid},
        .trig{
            .is_complex_core{trig_struct_size ==
                             sizeof(mcd_trig_complex_core_st)},
            .is_simple_core{trig_struct_size ==
                            sizeof(mcd_trig_simple_core_st)},
            .is_trig_bus{trig_struct_size == sizeof(mcd_trig_trig_bus_st)},
            .is_counter{trig_struct_size == sizeof(mcd_trig_counter_st)},
            .is_custom{trig_struct_size == sizeof(mcd_trig_custom_st)},
        }};

    if (args.trig.is_simple_core) {
        args.trig.simple_core = *(mcd_trig_simple_core_st *)trig;
    } else if (args.trig.is_complex_core) {
        args.trig.complex_core = *(mcd_trig_complex_core_st *)trig;
    } else if (args.trig.is_trig_bus) {
        args.trig.trig_bus = *(mcd_trig_trig_bus_st *)trig;
    } else if (args.trig.is_counter) {
        args.trig.counter = *(mcd_trig_counter_st *)trig;
    } else if (args.trig.is_custom) {
        args.trig.custom = *(mcd_trig_custom_st *)trig;
    } else {
        custom_mcd_error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_TRIG_CREATE},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"unknown trigger size"},
        };
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_CREATE_TRIG, buf);
    uint32_t req_len{marshal_mcd_create_trig_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_create_trig_result res;
    if (unmarshal_mcd_create_trig_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *trig_id = res.trig_id;

        if (res.trig_modified) {
            if (res.trig.is_simple_core && args.trig.is_simple_core) {
                *(mcd_trig_simple_core_st *)trig = res.trig.simple_core;
            } else if (res.trig.is_complex_core && args.trig.is_complex_core) {
                *(mcd_trig_complex_core_st *)trig = res.trig.complex_core;
            } else if (res.trig.is_trig_bus && args.trig.is_trig_bus) {
                *(mcd_trig_trig_bus_st *)trig = res.trig.trig_bus;
            } else if (res.trig.is_counter && args.trig.is_counter) {
                *(mcd_trig_counter_st *)trig = res.trig.counter;
            } else if (res.trig.is_custom && args.trig.is_custom) {
                *(mcd_trig_custom_st *)trig = res.trig.custom;
            } else {
                last_error = &MCD_ERROR_UNMARSHAL;
                return last_error->return_status;
            }
        }
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_trig_f(const mcd_core_st *core, uint32_t trig_id,
                             uint32_t max_trig_size, void *trig)
{
    if (!core || !core->instance || !trig) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_trig_args args{
        .core_uid{adapter->core_uid},
        .trig_id{trig_id},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_TRIG, buf);
    uint32_t req_len{marshal_mcd_qry_trig_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_trig_result res;
    if (unmarshal_mcd_qry_trig_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        if (res.trig.is_simple_core &&
            max_trig_size >= sizeof(mcd_trig_simple_core_st)) {
            *(mcd_trig_simple_core_st *)trig = res.trig.simple_core;
        } else if (res.trig.is_complex_core &&
                   max_trig_size >= sizeof(mcd_trig_complex_core_st)) {
            *(mcd_trig_complex_core_st *)trig = res.trig.complex_core;
        } else if (res.trig.is_trig_bus &&
                   max_trig_size >= sizeof(mcd_trig_trig_bus_st)) {
            *(mcd_trig_trig_bus_st *)trig = res.trig.trig_bus;
        } else if (res.trig.is_counter &&
                   max_trig_size >= sizeof(mcd_trig_counter_st)) {
            *(mcd_trig_counter_st *)trig = res.trig.counter;
        } else if (res.trig.is_custom &&
                   max_trig_size >= sizeof(mcd_trig_custom_st)) {
            *(mcd_trig_custom_st *)trig = res.trig.custom;
        } else {
            last_error = &MCD_ERROR_UNMARSHAL;
            return last_error->return_status;
        }
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_remove_trig_f(const mcd_core_st *core, uint32_t trig_id)
{
    if (!core || !core->instance) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_remove_trig_args args{
        .core_uid{adapter->core_uid},
        .trig_id{trig_id},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_REMOVE_TRIG, buf);
    uint32_t req_len{marshal_mcd_remove_trig_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_remove_trig_result res;
    if (unmarshal_mcd_remove_trig_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_trig_state_f(const mcd_core_st *core, uint32_t trig_id,
                                   mcd_trig_state_st *trig_state)
{
    if (!core || !core->instance | !trig_state) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_trig_state_args args{
        .core_uid{adapter->core_uid},
        .trig_id{trig_id},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_TRIG_STATE, buf);
    uint32_t req_len{marshal_mcd_qry_trig_state_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_trig_state_result res;
    if (unmarshal_mcd_qry_trig_state_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *trig_state = res.trig_state;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_activate_trig_set_f(const mcd_core_st *core)
{
    if (!core || !core->instance) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_activate_trig_set_args args{
        .core_uid{adapter->core_uid},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_ACTIVATE_TRIG_SET, buf);
    uint32_t req_len{marshal_mcd_activate_trig_set_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_activate_trig_set_result res;
    if (unmarshal_mcd_activate_trig_set_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_remove_trig_set_f(const mcd_core_st *core)
{
    if (!core) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_remove_trig_set_args args{
        .core_uid{adapter->core_uid},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_REMOVE_TRIG_SET, buf);
    uint32_t req_len{marshal_mcd_remove_trig_set_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_remove_trig_set_result res;
    if (unmarshal_mcd_remove_trig_set_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_trig_set_f(const mcd_core_st *core, uint32_t start_index,
                                 uint32_t *num_trigs, uint32_t *trig_ids)
{
    if (!core || !core->instance || !num_trigs || (*num_trigs && !trig_ids)) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_trig_set_args args{
        .core_uid{adapter->core_uid},
        .start_index{start_index},
        .num_trigs{*num_trigs},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_TRIG_SET, buf);
    uint32_t req_len{marshal_mcd_qry_trig_set_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_trig_set_result res;
    if (unmarshal_mcd_qry_trig_set_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_trigs = res.num_trigs;
        for (uint32_t i = 0; i < res.trig_ids_len; i++) {
            trig_ids[i] = res.trig_ids[i];
        }
    }

    free_mcd_qry_trig_set_result(&res);
    last_error = &MCD_ERROR_NONE;
    return last_error->return_status;
}

mcd_return_et mcd_qry_trig_set_state_f(const mcd_core_st *core,
                                       mcd_trig_set_state_st *trig_state)
{
    if (!core || !core->instance || !trig_state) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_trig_set_state_args args{
        .core_uid{adapter->core_uid},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_TRIG_SET_STATE, buf);
    uint32_t req_len{marshal_mcd_qry_trig_set_state_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_trig_set_state_result res;
    if (unmarshal_mcd_qry_trig_set_state_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *trig_state = res.trig_state;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_execute_txlist_f(const mcd_core_st *core,
                                   mcd_txlist_st *txlist)
{
    if (!core || !core->instance || !txlist) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (txlist->num_tx == 0) {
        last_error = &MCD_ERROR_NONE;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};

    if (!adapter->core_database_updated()) {
        custom_mcd_error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_GENERAL},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"core database not updated"},
        };
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    /* we only have a limited buffer size to communicate with the server */
    if (txlist->num_tx > 1) {
        txlist->num_tx_ok = 0;
        for (uint32_t i = 0; i < txlist->num_tx; i++) {
            mcd_txlist_st tx{
                .tx{txlist->tx + i},
                .num_tx{1},
                .num_tx_ok{0},
            };

            mcd_return_et ret{mcd_execute_txlist_f(core, &tx)};
            if (ret == MCD_RET_ACT_NONE) {
                txlist->num_tx_ok++;
            } else {
                return ret;
            }
        }

        last_error = &MCD_ERROR_NONE;
        return last_error->return_status;
    }

    mcd_tx_st &client_tx{txlist->tx[0]};
    TxAdapter *tx_adapter;
    if (adapter->get_tx_adapter(client_tx, &tx_adapter, custom_mcd_error)) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_txlist_st server_request;
    tx_adapter->grant_server_access(core);
    if (tx_adapter->yield_server_request(
            client_tx, server_request, custom_mcd_error) != MCD_RET_ACT_NONE) {
        /* skip transaction */
        txlist->tx[0].num_bytes_ok = 0;
        txlist->num_tx_ok = 1;
        last_error = &MCD_ERROR_NONE;
        return last_error->return_status;
    }

    mcd_execute_txlist_args args{
        .core_uid{adapter->core_uid},
        .txlist{server_request},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_EXECUTE_TXLIST, buf);
    uint32_t req_len{marshal_mcd_execute_txlist_args(&args, buf)};

    /*
     * Since the server request has been serialized into the buffer,
     * its object is no longer required
     */
    tx_adapter->free_server_request(std::move(server_request));

    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_execute_txlist_result res;
    if (unmarshal_mcd_execute_txlist_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret != MCD_RET_ACT_NONE) {
        free_mcd_execute_txlist_result(&res);
        last_error = &MCD_ERROR_ASK_SERVER;
        return res.ret;
    }

    if (tx_adapter->collect_client_response(
            client_tx, res.txlist, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
    } else {
        txlist->num_tx_ok++;
        last_error = &MCD_ERROR_NONE;
    }

    free_mcd_execute_txlist_result(&res);
    return last_error->return_status;
}

mcd_return_et mcd_run_f(const mcd_core_st *core, mcd_bool_t global)
{
    if (!core || !core->instance) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_run_args args{
        .core_uid{adapter->core_uid},
        .global{global},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_RUN, buf);
    uint32_t req_len{marshal_mcd_run_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_run_result res;
    if (unmarshal_mcd_run_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_stop_f(const mcd_core_st *core, mcd_bool_t global)
{
    if (!core || !core->instance) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_stop_args args{
        .core_uid{adapter->core_uid},
        /* TODO currently, only global stops are implemented */
        .global{TRUE},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_STOP, buf);
    uint32_t req_len{marshal_mcd_stop_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_stop_result res;
    if (unmarshal_mcd_stop_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_run_until_f(const mcd_core_st *core, mcd_bool_t global,
                              mcd_bool_t absolute_time, uint64_t run_until_time)
{
    if (!core || !core->instance) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_run_until_args args{
        .core_uid{adapter->core_uid},
        .global{global},
        .absolute_time{absolute_time},
        .run_until_time{run_until_time},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_RUN_UNTIL, buf);
    uint32_t req_len{marshal_mcd_run_until_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_run_until_result res;
    if (unmarshal_mcd_run_until_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_current_time_f(const mcd_core_st *core,
                                     uint64_t *current_time)
{
    if (!core || !core->instance) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_current_time_args args{
        .core_uid{adapter->core_uid},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_CURRENT_TIME, buf);
    uint32_t req_len{marshal_mcd_qry_current_time_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_current_time_result res;
    if (unmarshal_mcd_qry_current_time_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *current_time = res.current_time;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_step_f(const mcd_core_st *core, mcd_bool_t global,
                         mcd_core_step_type_et step_type, uint32_t n_steps)
{
    if (!core || !core->instance) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_step_args args{
        .core_uid{adapter->core_uid},
        .global{global},
        .step_type{step_type},
        .n_steps{n_steps},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_STEP, buf);
    uint32_t req_len{marshal_mcd_step_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_step_result res;
    if (unmarshal_mcd_step_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_set_global_f(const mcd_core_st *core, mcd_bool_t enable)
{
    if (!core || !core->instance) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_set_global_args args{
        .core_uid{adapter->core_uid},
        .enable{enable},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_SET_GLOBAL, buf);
    uint32_t req_len{marshal_mcd_set_global_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_set_global_result res;
    if (unmarshal_mcd_set_global_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_state_f(const mcd_core_st *core, mcd_core_state_st *state)
{
    if (!core || !core->instance || !state) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_state_args args{
        .core_uid{adapter->core_uid},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_STATE, buf);
    uint32_t req_len{marshal_mcd_qry_state_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_state_result res;
    if (unmarshal_mcd_qry_state_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *state = res.state;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_execute_command_f(const mcd_core_st *core,
                                    const mcd_char_t *command_string,
                                    uint32_t result_string_size,
                                    mcd_char_t *result_string)
{
    if (!core || !core->instance || !command_string ||
        (result_string_size && !result_string)) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_execute_command_args args{
        .core_uid{adapter->core_uid},
        .command_string_len{(uint32_t)strlen(command_string)},
        .command_string{(mcd_char_t *)command_string},
        .result_string_size{result_string_size},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_EXECUTE_COMMAND, buf);
    uint32_t req_len{marshal_mcd_execute_command_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_execute_command_result res;
    if (unmarshal_mcd_execute_command_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        (result_string, result_string_size, res.result_string,
         res.result_string_len);
    }

    free_mcd_execute_command_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_rst_classes_f(const mcd_core_st *core,
                                    uint32_t *rst_class_vector)
{
    if (!core || !core->instance || !rst_class_vector) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_rst_classes_args args{
        .core_uid{adapter->core_uid},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_RST_CLASSES, buf);
    uint32_t req_len{marshal_mcd_qry_rst_classes_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_rst_classes_result res;
    if (unmarshal_mcd_qry_rst_classes_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *rst_class_vector = res.rst_class_vector;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_rst_class_info_f(const mcd_core_st *core,
                                       uint8_t rst_class,
                                       mcd_rst_info_st *rst_info)
{
    if (!core || !rst_info) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_rst_class_info_args args{
        .core_uid{adapter->core_uid},
        .rst_class{rst_class},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_RST_CLASS_INFO, buf);
    uint32_t req_len{marshal_mcd_qry_rst_class_info_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_rst_class_info_result res;
    if (unmarshal_mcd_qry_rst_class_info_result(
            g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *rst_info = res.rst_info;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_rst_f(const mcd_core_st *core, uint32_t rst_class_vector,
                        mcd_bool_t rst_and_halt)
{
    if (!core) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_rst_args args{
        .core_uid{adapter->core_uid},
        .rst_class_vector{rst_class_vector},
        .rst_and_halt{rst_and_halt},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_RST, buf);
    uint32_t req_len{marshal_mcd_rst_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_rst_result res;
    if (unmarshal_mcd_rst_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_chl_open_f(const mcd_core_st *core, mcd_chl_st *channel)
{
    if (!core || !core->instance || !channel) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_chl_open_args args{
        .core_uid{adapter->core_uid},
        .channel{*channel},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_CHL_OPEN, buf);
    uint32_t req_len{marshal_mcd_chl_open_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_chl_open_result res;
    if (unmarshal_mcd_chl_open_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *channel = res.channel;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_send_msg_f(const mcd_core_st *core, const mcd_chl_st *channel,
                             uint32_t msg_len, const uint8_t *msg)
{
    if (!core || !core->instance || !channel || !msg) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_send_msg_args args{
        .core_uid{adapter->core_uid},
        .channel{*channel},
        .msg_len{msg_len},
        .msg{(uint8_t *)msg},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_SEND_MSG, buf);
    uint32_t req_len{marshal_mcd_send_msg_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_send_msg_result res;
    if (unmarshal_mcd_send_msg_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_receive_msg_f(const mcd_core_st *core,
                                const mcd_chl_st *channel, uint32_t timeout,
                                uint32_t *msg_len, uint8_t *msg)
{
    if (!core || !core->instance || !channel || !msg_len ||
        (*msg_len && !msg)) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_receive_msg_args args{
        .core_uid{adapter->core_uid},
        .channel{*channel},
        .timeout{timeout},
        .msg_len{*msg_len},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_RECEIVE_MSG, buf);
    uint32_t req_len{marshal_mcd_receive_msg_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_receive_msg_result res;
    if (unmarshal_mcd_receive_msg_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *msg_len = res.msg_len;
        for (uint32_t i = 0; i < res.msg_len; i++) {
            msg[i] = res.msg[i];
        }
    }

    free_mcd_receive_msg_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_chl_reset_f(const mcd_core_st *core,
                              const mcd_chl_st *channel)
{
    if (!core || !core->instance || !channel) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_chl_reset_args args{
        .core_uid{adapter->core_uid},
        .channel{*channel},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_CHL_RESET, buf);
    uint32_t req_len{marshal_mcd_chl_reset_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_chl_reset_result res;
    if (unmarshal_mcd_chl_reset_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_chl_close_f(const mcd_core_st *core,
                              const mcd_chl_st *channel)
{
    if (!core || !core->instance || !channel) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_chl_close_args args{
        .core_uid{adapter->core_uid},
        .channel{*channel},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_CHL_CLOSE, buf);
    uint32_t req_len{marshal_mcd_chl_close_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_chl_close_result res;
    if (unmarshal_mcd_chl_close_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_traces_f(const mcd_core_st *core, uint32_t start_index,
                               uint32_t *num_traces,
                               mcd_trace_info_st *trace_info)
{
    if (!core || !core->instance || !num_traces ||
        (*num_traces && !trace_info)) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_traces_args args{
        .core_uid{adapter->core_uid},
        .start_index{start_index},
        .num_traces{*num_traces},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_TRACES, buf);
    uint32_t req_len{marshal_mcd_qry_traces_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_traces_result res;
    if (unmarshal_mcd_qry_traces_result(g_mcd_server->msg_buf, &res) == 0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *num_traces = res.num_traces;
        for (uint32_t i = 0; i < res.trace_info_len; i++) {
            trace_info[i] = res.trace_info[i];
        }
    }

    free_mcd_qry_traces_result(&res);
    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_qry_trace_state_f(const mcd_core_st *core, uint32_t trace_id,
                                    mcd_trace_state_st *state)
{
    if (!core || !core->instance || !state) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_qry_trace_state_args args{
        .core_uid{adapter->core_uid},
        .trace_id{trace_id},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_QRY_TRACE_STATE, buf);
    uint32_t req_len{marshal_mcd_qry_trace_state_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_qry_trace_state_result res;
    if (unmarshal_mcd_qry_trace_state_result(g_mcd_server->msg_buf, &res) ==
        0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *state = res.state;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_set_trace_state_f(const mcd_core_st *core, uint32_t trace_id,
                                    mcd_trace_state_st *state)
{
    if (!core || !core->instance || !state) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_set_trace_state_args args{
        .core_uid{adapter->core_uid},
        .trace_id{trace_id},
        .state{*state},
    };

    uint8_t *buf{g_mcd_server->msg_buf};
    buf += marshal_uint8_t(UID_MCD_SET_TRACE_STATE, buf);
    uint32_t req_len{marshal_mcd_set_trace_state_args(&args, buf)};
    if (g_mcd_server->request_response(
            req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    mcd_set_trace_state_result res;
    if (unmarshal_mcd_set_trace_state_result(g_mcd_server->msg_buf, &res) ==
        0) {
        last_error = &MCD_ERROR_UNMARSHAL;
        return last_error->return_status;
    }

    if (res.ret == MCD_RET_ACT_NONE) {
        *state = res.state;
    }

    last_error = &MCD_ERROR_ASK_SERVER;
    return res.ret;
}

mcd_return_et mcd_read_trace_f(const mcd_core_st *core, uint32_t trace_id,
                               uint64_t start_index, uint32_t *num_frames,
                               uint32_t trace_data_size, void *trace_data)
{
    /* TODO: Use trace_buf for unmarshalling */
    // static mcd_trace_data_core_st trace_buf[100];

    if (!core || !core->instance || !num_frames ||
        (*num_frames && !trace_data)) {
        last_error = &MCD_ERROR_INVALID_NULL_PARAM;
        return last_error->return_status;
    }

    if (!g_mcd_server) {
        last_error = &MCD_ERROR_SERVER_NOT_OPEN;
        return last_error->return_status;
    }

    Core *adapter{(Core *)core->instance};
    mcd_read_trace_args args{
        .core_uid{adapter->core_uid},
        .trace_id{trace_id},
        .start_index{0},
        .num_frames{0},
        .is_trace_data_core{false},
        .is_trace_data_event{false},
        .is_trace_data_stat{false},
    };

    if (trace_data_size == sizeof(mcd_trace_data_core_st)) {
        args.is_trace_data_core = true;
    } else if (trace_data_size == sizeof(mcd_trace_data_event_st)) {
        args.is_trace_data_event = true;
    } else if (trace_data_size == sizeof(mcd_trace_data_stat_st)) {
        args.is_trace_data_stat = true;
    } else {
        custom_mcd_error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_PARAM},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"unknown trace_data_size"},
        };
        last_error = &custom_mcd_error;
        return last_error->return_status;
    }

    /*
     * Partition the request since we only have a limited buffer size
     */

    uint32_t num_req{0};
    do {
        uint32_t cnt{*num_frames - num_req};
        if (cnt > 100) {
            cnt = 100;
        }

        args.start_index = start_index + num_req;
        args.num_frames = cnt;

        uint8_t *buf{g_mcd_server->msg_buf};
        buf += marshal_uint8_t(UID_MCD_READ_TRACE, buf);
        uint32_t req_len{marshal_mcd_read_trace_args(&args, buf)};
        if (g_mcd_server->request_response(
                req_len + 1, custom_mcd_error) != MCD_RET_ACT_NONE) {
            last_error = &custom_mcd_error;
            return last_error->return_status;
        }

        mcd_read_trace_result res;

        if (unmarshal_mcd_read_trace_result(g_mcd_server->msg_buf, &res) == 0) {
            last_error = &MCD_ERROR_UNMARSHAL;
            return last_error->return_status;
        }

        if (res.ret != MCD_RET_ACT_NONE) {
            last_error = &MCD_ERROR_ASK_SERVER;
            return res.ret;
        }

        if (res.is_trace_data_core && args.is_trace_data_core) {
            for (uint32_t i = 0; i < res.trace_data_len; i++) {
                ((mcd_trace_data_core_st *)trace_data)[num_req + i] =
                    res.trace_data_core[i];
            }
        } else if (res.is_trace_data_event && args.is_trace_data_event) {
            for (uint32_t i = 0; i < res.trace_data_len; i++) {
                ((mcd_trace_data_event_st *)trace_data)[num_req + i] =
                    res.trace_data_event[i];
            }
        } else if (res.is_trace_data_stat && args.is_trace_data_stat) {
            for (uint32_t i = 0; i < res.trace_data_len; i++) {
                ((mcd_trace_data_stat_st *)trace_data)[num_req + i] =
                    res.trace_data_stat[i];
            }
        } else {
            last_error = &MCD_ERROR_UNMARSHAL;
            return last_error->return_status;
        }

        num_req += res.num_frames;

        if (res.num_frames < cnt) {
            /*
             * The number of frames have been set down by the server.
             * We interpret this as having finished reading all frames.
             */
            *num_frames = num_req;
        } else if (res.num_frames > cnt) {
            custom_mcd_error = {
                .return_status{MCD_RET_ACT_HANDLE_ERROR},
                .error_code{MCD_ERR_GENERAL},
                .error_events{MCD_ERR_EVT_NONE},
                .error_str{"server returned more frames than requested"},
            };
            last_error = &custom_mcd_error;
            return last_error->return_status;
        }
    } while (num_req < *num_frames);

    last_error = &MCD_ERROR_NONE;
    return last_error->return_status;
}

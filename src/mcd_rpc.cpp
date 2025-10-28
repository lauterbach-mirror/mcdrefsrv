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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "mcd_rpc.h"
#include "mcd_api.h"

#include <stdio.h>

#if defined __BYTE_ORDER__
static const bool HOST_BIG_ENDIAN = __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__;
#elif defined _MSC_VER
static const mcd_bool_t HOST_BIG_ENDIAN = FALSE;
#endif

typedef uint8_t mcd_f_uid;
enum {
    UID_MCD_INITIALIZE = 1,
    UID_MCD_EXIT = 2,
    UID_MCD_QRY_SERVERS = 3,
    UID_MCD_OPEN_SERVER = 4,
    UID_MCD_CLOSE_SERVER = 5,
    UID_MCD_SET_SERVER_CONFIG = 6,
    UID_MCD_QRY_SERVER_CONFIG = 7,
    UID_MCD_QRY_SYSTEMS = 8,
    UID_MCD_QRY_DEVICES = 9,
    UID_MCD_QRY_CORES = 10,
    UID_MCD_QRY_CORE_MODES = 11,
    UID_MCD_OPEN_CORE = 12,
    UID_MCD_CLOSE_CORE = 13,
    UID_MCD_QRY_ERROR_INFO = 14,
    UID_MCD_QRY_DEVICE_DESCRIPTION = 15,
    UID_MCD_QRY_MAX_PAYLOAD_SIZE = 16,
    UID_MCD_QRY_INPUT_HANDLE = 17,
    UID_MCD_QRY_MEM_SPACES = 18,
    UID_MCD_QRY_MEM_BLOCKS = 19,
    UID_MCD_QRY_ACTIVE_OVERLAYS = 20,
    UID_MCD_QRY_REG_GROUPS = 21,
    UID_MCD_QRY_REG_MAP = 22,
    UID_MCD_QRY_REG_COMPOUND = 23,
    UID_MCD_QRY_TRIG_INFO = 24,
    UID_MCD_QRY_CTRIGS = 25,
    UID_MCD_CREATE_TRIG = 26,
    UID_MCD_QRY_TRIG = 27,
    UID_MCD_REMOVE_TRIG = 28,
    UID_MCD_QRY_TRIG_STATE = 29,
    UID_MCD_ACTIVATE_TRIG_SET = 30,
    UID_MCD_REMOVE_TRIG_SET = 31,
    UID_MCD_QRY_TRIG_SET = 32,
    UID_MCD_QRY_TRIG_SET_STATE = 33,
    UID_MCD_EXECUTE_TXLIST = 34,
    UID_MCD_RUN = 35,
    UID_MCD_STOP = 36,
    UID_MCD_RUN_UNTIL = 37,
    UID_MCD_QRY_CURRENT_TIME = 38,
    UID_MCD_STEP = 39,
    UID_MCD_SET_GLOBAL = 40,
    UID_MCD_QRY_STATE = 41,
    UID_MCD_EXECUTE_COMMAND = 42,
    UID_MCD_QRY_RST_CLASSES = 43,
    UID_MCD_QRY_RST_CLASS_INFO = 44,
    UID_MCD_RST = 45,
    UID_MCD_CHL_OPEN = 46,
    UID_MCD_SEND_MSG = 47,
    UID_MCD_RECEIVE_MSG = 48,
    UID_MCD_CHL_RESET = 49,
    UID_MCD_CHL_CLOSE = 50,
    UID_MCD_QRY_TRACES = 51,
    UID_MCD_QRY_TRACE_STATE = 52,
    UID_MCD_SET_TRACE_STATE = 53,
    UID_MCD_READ_TRACE = 54,
};

#define DEFINE_PRIMITIVE(type)                                               \
    static int marshal_##type(type obj, char *buf)                           \
    {                                                                        \
        const int BYTES = sizeof(type);                                      \
        if constexpr (HOST_BIG_ENDIAN && BYTES > 1)                          \
            for (int i = BYTES - 1; i >= 0; i--) *buf++ = ((char *)&obj)[i]; \
        else                                                                 \
            *(type *)buf = obj;                                              \
        return BYTES;                                                        \
    }                                                                        \
    static int unmarshal_##type(const char *buf, type *obj)                  \
    {                                                                        \
        const int BYTES = sizeof(type);                                      \
        if constexpr (HOST_BIG_ENDIAN && BYTES > 1)                          \
            for (int i = 0; i < BYTES; i++)                                  \
                ((char *)obj)[i] = buf[BYTES - i - 1];                       \
        else                                                                 \
            *obj = *(const type *)buf;                                       \
        return BYTES;                                                        \
    }

DEFINE_PRIMITIVE(mcd_addr_space_type_et)
DEFINE_PRIMITIVE(mcd_char_t)
DEFINE_PRIMITIVE(mcd_chl_attributes_et)
DEFINE_PRIMITIVE(mcd_chl_type_et)
DEFINE_PRIMITIVE(mcd_core_event_et)
DEFINE_PRIMITIVE(mcd_core_state_et)
DEFINE_PRIMITIVE(mcd_core_step_type_et)
DEFINE_PRIMITIVE(mcd_endian_et)
DEFINE_PRIMITIVE(mcd_error_code_et)
DEFINE_PRIMITIVE(mcd_error_event_et)
DEFINE_PRIMITIVE(mcd_mem_type_et)
DEFINE_PRIMITIVE(mcd_reg_type_et)
DEFINE_PRIMITIVE(mcd_return_et)
DEFINE_PRIMITIVE(mcd_trace_cycle_et)
DEFINE_PRIMITIVE(mcd_trace_format_et)
DEFINE_PRIMITIVE(mcd_trace_marker_et)
DEFINE_PRIMITIVE(mcd_trace_mode_et)
DEFINE_PRIMITIVE(mcd_trace_state_et)
DEFINE_PRIMITIVE(mcd_trace_type_et)
DEFINE_PRIMITIVE(mcd_trig_action_et)
DEFINE_PRIMITIVE(mcd_trig_opt_et)
DEFINE_PRIMITIVE(mcd_trig_type_et)
DEFINE_PRIMITIVE(mcd_tx_access_opt_et)
DEFINE_PRIMITIVE(mcd_tx_access_type_et)
DEFINE_PRIMITIVE(uint16_t)
DEFINE_PRIMITIVE(uint32_t)
DEFINE_PRIMITIVE(uint64_t)
DEFINE_PRIMITIVE(uint8_t)

static int marshal_mcd_bool_t(mcd_bool_t obj, char *buf)
{
    uint8_t b = !!obj;
    return marshal_uint8_t(b, buf);
}

static int unmarshal_mcd_bool_t(const char *buf, mcd_bool_t *obj)
{
    uint8_t b;
    int l = unmarshal_uint8_t(buf, &b);
    *obj = b ? TRUE : FALSE;
    return l;
}

/* marshal / unmarshal complex data types */

static uint32_t marshal_mcd_core_con_info_st(const mcd_core_con_info_st *obj,
                                             char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t((uint32_t)MCD_HOSTNAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t)MCD_HOSTNAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->host[i], tail);
    }

    tail += marshal_uint32_t(obj->server_port, tail);

    tail += marshal_uint32_t((uint32_t)MCD_KEY_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t)MCD_KEY_LEN; i++) {
        tail += marshal_mcd_char_t(obj->server_key[i], tail);
    }

    tail += marshal_uint32_t((uint32_t)MCD_KEY_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t)MCD_KEY_LEN; i++) {
        tail += marshal_mcd_char_t(obj->system_key[i], tail);
    }

    tail += marshal_uint32_t((uint32_t)MCD_KEY_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t)MCD_KEY_LEN; i++) {
        tail += marshal_mcd_char_t(obj->device_key[i], tail);
    }

    tail += marshal_uint32_t((uint32_t)MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t)MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->system[i], tail);
    }

    tail += marshal_uint32_t((uint32_t)MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t)MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->system_instance[i], tail);
    }

    tail += marshal_uint32_t((uint32_t)MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t)MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->acc_hw[i], tail);
    }

    tail += marshal_uint32_t(obj->device_type, tail);

    tail += marshal_uint32_t((uint32_t)MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t)MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->device[i], tail);
    }

    tail += marshal_uint32_t(obj->device_id, tail);

    tail += marshal_uint32_t((uint32_t)MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t)MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->core[i], tail);
    }

    tail += marshal_uint32_t(obj->core_type, tail);

    tail += marshal_uint32_t(obj->core_id, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t unmarshal_mcd_core_con_info_st(const char *buf,
                                               mcd_core_con_info_st *obj)
{
    const char *head = buf;

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->host + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->server_port);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->server_key + i);
        }
    }

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->system_key + i);
        }
    }

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->device_key + i);
        }
    }

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->system + i);
        }
    }

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->system_instance + i);
        }
    }

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->acc_hw + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->device_type);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->device + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->device_id);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->core + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->core_type);

    head += unmarshal_uint32_t(head, &obj->core_id);

    return (uint32_t)(head - buf);
}

static uint32_t unmarshal_mcd_error_info_st(const char *buf,
                                            mcd_error_info_st *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    head += unmarshal_mcd_error_code_et(head, &obj->error_code);

    head += unmarshal_mcd_error_event_et(head, &obj->error_events);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->error_str + i);
        }
    }

    return (uint32_t)(head - buf);
}

uint32_t unmarshal_mcd_memspace_st(const char *buf, mcd_memspace_st *obj)
{
    const char *head = buf;

    head += unmarshal_uint32_t(head, &obj->mem_space_id);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->mem_space_name + i);
        }
    }

    head += unmarshal_mcd_mem_type_et(head, &obj->mem_type);

    head += unmarshal_uint32_t(head, &obj->bits_per_mau);

    head += unmarshal_uint8_t(head, &obj->invariance);

    head += unmarshal_mcd_endian_et(head, &obj->endian);

    head += unmarshal_uint64_t(head, &obj->min_addr);

    head += unmarshal_uint64_t(head, &obj->max_addr);

    head += unmarshal_uint32_t(head, &obj->num_mem_blocks);

    head +=
        unmarshal_mcd_tx_access_opt_et(head, &obj->supported_access_options);

    head += unmarshal_uint32_t(head, &obj->core_mode_mask_read);

    head += unmarshal_uint32_t(head, &obj->core_mode_mask_write);

    return (uint32_t)(head - buf);
}

static uint32_t unmarshal_mcd_register_group_st(const char *buf,
                                                mcd_register_group_st *obj)
{
    const char *head = buf;

    head += unmarshal_uint32_t(head, &obj->reg_group_id);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->reg_group_name + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->n_registers);

    return (uint32_t)(head - buf);
}

static uint32_t marshal_mcd_addr_st(const mcd_addr_st *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint64_t(obj->address, tail);

    tail += marshal_uint32_t(obj->mem_space_id, tail);

    tail += marshal_uint32_t(obj->addr_space_id, tail);

    tail += marshal_mcd_addr_space_type_et(obj->addr_space_type, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t unmarshal_mcd_addr_st(const char *buf, mcd_addr_st *obj)
{
    const char *head = buf;

    head += unmarshal_uint64_t(head, &obj->address);

    head += unmarshal_uint32_t(head, &obj->mem_space_id);

    head += unmarshal_uint32_t(head, &obj->addr_space_id);

    head += unmarshal_mcd_addr_space_type_et(head, &obj->addr_space_type);

    return (uint32_t)(head - buf);
}

static uint32_t unmarshal_mcd_register_info_st(const char *buf,
                                               mcd_register_info_st *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_addr_st(head, &obj->addr);

    head += unmarshal_uint32_t(head, &obj->reg_group_id);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->regname + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->regsize);

    head += unmarshal_uint32_t(head, &obj->core_mode_mask_read);

    head += unmarshal_uint32_t(head, &obj->core_mode_mask_write);

    head += unmarshal_mcd_bool_t(head, &obj->has_side_effects_read);

    head += unmarshal_mcd_bool_t(head, &obj->has_side_effects_write);

    head += unmarshal_mcd_reg_type_et(head, &obj->reg_type);

    head += unmarshal_uint32_t(head, &obj->hw_thread_id);

    return (uint32_t)(head - buf);
}

static uint32_t marshal_mcd_tx_st(const mcd_tx_st *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_mcd_addr_st(&obj->addr, tail);

    tail += marshal_mcd_tx_access_type_et(obj->access_type, tail);

    tail += marshal_mcd_tx_access_opt_et(obj->options, tail);

    tail += marshal_uint8_t(obj->access_width, tail);

    tail += marshal_uint8_t(obj->core_mode, tail);

    tail += marshal_uint32_t((uint32_t)obj->num_bytes, tail);
    for (uint32_t i = 0; i < (uint32_t)obj->num_bytes; i++) {
        tail += marshal_uint8_t(obj->data[i], tail);
    }

    tail += marshal_uint32_t(obj->num_bytes, tail);

    tail += marshal_uint32_t(obj->num_bytes_ok, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t unmarshal_mcd_tx_st(const char *buf, mcd_tx_st *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_addr_st(head, &obj->addr);

    head += unmarshal_mcd_tx_access_type_et(head, &obj->access_type);

    head += unmarshal_mcd_tx_access_opt_et(head, &obj->options);

    head += unmarshal_uint8_t(head, &obj->access_width);

    head += unmarshal_uint8_t(head, &obj->core_mode);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);
        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_uint8_t(head, obj->data + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->num_bytes);

    head += unmarshal_uint32_t(head, &obj->num_bytes_ok);

    return (uint32_t)(head - buf);
}

static uint32_t marshal_mcd_txlist_st(const mcd_txlist_st *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t((uint32_t)obj->num_tx, tail);
    for (uint32_t i = 0; i < (uint32_t)obj->num_tx; i++) {
        tail += marshal_mcd_tx_st(obj->tx + i, tail);
    }

    tail += marshal_uint32_t(obj->num_tx, tail);

    tail += marshal_uint32_t(obj->num_tx_ok, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t unmarshal_mcd_txlist_st(const char *buf, mcd_txlist_st *obj)
{
    const char *head = buf;

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);
        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_tx_st(head, obj->tx + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->num_tx);

    head += unmarshal_uint32_t(head, &obj->num_tx_ok);

    return (uint32_t)(head - buf);
}

static uint32_t unmarshal_mcd_trig_info_st(const char *buf,
                                           mcd_trig_info_st *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_trig_type_et(head, &obj->type);

    head += unmarshal_mcd_trig_opt_et(head, &obj->option);

    head += unmarshal_mcd_trig_action_et(head, &obj->action);

    head += unmarshal_uint32_t(head, &obj->trig_number);

    head += unmarshal_uint32_t(head, &obj->state_number);

    head += unmarshal_uint32_t(head, &obj->counter_number);

    head += unmarshal_mcd_bool_t(head, &obj->sw_breakpoints);

    return (uint32_t)(head - buf);
}

static uint32_t unmarshal_mcd_trig_state_st(const char *buf,
                                            mcd_trig_state_st *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_bool_t(head, &obj->active);

    head += unmarshal_mcd_bool_t(head, &obj->captured);

    head += unmarshal_mcd_bool_t(head, &obj->captured_valid);

    head += unmarshal_uint64_t(head, &obj->count_value);

    head += unmarshal_mcd_bool_t(head, &obj->count_valid);

    return (uint32_t)(head - buf);
}

static uint32_t unmarshal_mcd_trig_set_state_st(const char *buf,
                                                mcd_trig_set_state_st *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_bool_t(head, &obj->active);

    head += unmarshal_uint32_t(head, &obj->state);

    head += unmarshal_mcd_bool_t(head, &obj->state_valid);

    head += unmarshal_uint32_t(head, &obj->trig_bus);

    head += unmarshal_mcd_bool_t(head, &obj->trig_bus_valid);

    head += unmarshal_mcd_bool_t(head, &obj->trace);

    head += unmarshal_mcd_bool_t(head, &obj->trace_valid);

    head += unmarshal_mcd_bool_t(head, &obj->analysis);

    head += unmarshal_mcd_bool_t(head, &obj->analysis_valid);

    return (uint32_t)(head - buf);
}

static uint32_t unmarshal_mcd_core_state_st(const char *buf,
                                            mcd_core_state_st *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_core_state_et(head, &obj->state);

    head += unmarshal_mcd_core_event_et(head, &obj->event);

    head += unmarshal_uint32_t(head, &obj->hw_thread_id);

    head += unmarshal_uint32_t(head, &obj->trig_id);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->stop_str + i);
        }
    }

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->info_str + i);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t unmarshal_mcd_rst_info_st(const char *buf, mcd_rst_info_st *obj)
{
    const char *head = buf;

    head += unmarshal_uint32_t(head, &obj->class_vector);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->info_str + i);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_open_server_args(
    const mcd_open_server_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->system_key_len, tail);

    tail += marshal_uint32_t((uint32_t)obj->system_key_len, tail);
    for (uint32_t i = 0; i < (uint32_t)obj->system_key_len; i++) {
        tail += marshal_mcd_char_t(obj->system_key[i], tail);
    }

    tail += marshal_uint32_t(obj->config_string_len, tail);

    tail += marshal_uint32_t((uint32_t)obj->config_string_len, tail);
    for (uint32_t i = 0; i < (uint32_t)obj->config_string_len; i++) {
        tail += marshal_mcd_char_t(obj->config_string[i], tail);
    }

    return (uint32_t)(tail - buf);
}

static uint32_t unmarshal_mcd_ctrig_info_st(const char *buf,
                                            mcd_ctrig_info_st *obj)
{
    const char *head = buf;

    head += unmarshal_uint32_t(head, &obj->ctrig_id);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->info_str + i);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t marshal_mcd_trig_simple_core_st(
    const mcd_trig_simple_core_st *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->struct_size, tail);

    tail += marshal_mcd_trig_type_et(obj->type, tail);

    tail += marshal_mcd_trig_opt_et(obj->option, tail);

    tail += marshal_mcd_trig_action_et(obj->action, tail);

    tail += marshal_uint32_t(obj->action_param, tail);

    tail += marshal_mcd_bool_t(obj->modified, tail);

    tail += marshal_uint32_t(obj->state_mask, tail);

    tail += marshal_mcd_addr_st(&obj->addr_start, tail);

    tail += marshal_uint64_t(obj->addr_range, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t unmarshal_mcd_trig_simple_core_st(const char *buf,
                                                  mcd_trig_simple_core_st *obj)
{
    const char *head = buf;

    head += unmarshal_uint32_t(head, &obj->struct_size);

    head += unmarshal_mcd_trig_type_et(head, &obj->type);

    head += unmarshal_mcd_trig_opt_et(head, &obj->option);

    head += unmarshal_mcd_trig_action_et(head, &obj->action);

    head += unmarshal_uint32_t(head, &obj->action_param);

    head += unmarshal_mcd_bool_t(head, &obj->modified);

    head += unmarshal_uint32_t(head, &obj->state_mask);

    head += unmarshal_mcd_addr_st(head, &obj->addr_start);

    head += unmarshal_uint64_t(head, &obj->addr_range);

    return (uint32_t)(head - buf);
}

static uint32_t marshal_mcd_trig_complex_core_st(
    const mcd_trig_complex_core_st *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->struct_size, tail);

    tail += marshal_mcd_trig_type_et(obj->type, tail);

    tail += marshal_mcd_trig_opt_et(obj->option, tail);

    tail += marshal_mcd_trig_action_et(obj->action, tail);

    tail += marshal_uint32_t(obj->action_param, tail);

    tail += marshal_mcd_bool_t(obj->modified, tail);

    tail += marshal_uint32_t(obj->state_mask, tail);

    tail += marshal_mcd_addr_st(&obj->addr_start, tail);

    tail += marshal_uint64_t(obj->addr_range, tail);

    tail += marshal_uint64_t(obj->data_start, tail);

    tail += marshal_uint64_t(obj->data_range, tail);

    tail += marshal_uint64_t(obj->data_mask, tail);

    tail += marshal_uint32_t(obj->data_size, tail);

    tail += marshal_uint32_t(obj->hw_thread_id, tail);

    tail += marshal_uint64_t(obj->sw_thread_id, tail);

    tail += marshal_uint32_t(obj->core_mode_mask, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t unmarshal_mcd_trig_complex_core_st(
    const char *buf, mcd_trig_complex_core_st *obj)
{
    const char *head = buf;

    head += unmarshal_uint32_t(head, &obj->struct_size);

    head += unmarshal_mcd_trig_type_et(head, &obj->type);

    head += unmarshal_mcd_trig_opt_et(head, &obj->option);

    head += unmarshal_mcd_trig_action_et(head, &obj->action);

    head += unmarshal_uint32_t(head, &obj->action_param);

    head += unmarshal_mcd_bool_t(head, &obj->modified);

    head += unmarshal_uint32_t(head, &obj->state_mask);

    head += unmarshal_mcd_addr_st(head, &obj->addr_start);

    head += unmarshal_uint64_t(head, &obj->addr_range);

    head += unmarshal_uint64_t(head, &obj->data_start);

    head += unmarshal_uint64_t(head, &obj->data_range);

    head += unmarshal_uint64_t(head, &obj->data_mask);

    head += unmarshal_uint32_t(head, &obj->data_size);

    head += unmarshal_uint32_t(head, &obj->hw_thread_id);

    head += unmarshal_uint64_t(head, &obj->sw_thread_id);

    head += unmarshal_uint32_t(head, &obj->core_mode_mask);

    return (uint32_t)(head - buf);
}

static uint32_t marshal_mcd_rpc_trig_st(const mcd_rpc_trig_st *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint8_t(obj->is_complex_core, tail);

    tail += marshal_uint8_t(obj->is_complex_core, tail);
    if (obj->is_complex_core) {
        tail += marshal_mcd_trig_complex_core_st(obj->complex_core, tail);
    }

    tail += marshal_uint8_t(obj->is_simple_core, tail);

    tail += marshal_uint8_t(obj->is_simple_core, tail);
    if (obj->is_simple_core) {
        tail += marshal_mcd_trig_simple_core_st(obj->simple_core, tail);
    }

    /* backwards compatibility */

    /* is_trig_bus */
    tail += marshal_uint8_t(FALSE, tail);
    tail += marshal_uint8_t(FALSE, tail);

    /* is_counter */
    tail += marshal_uint8_t(FALSE, tail);
    tail += marshal_uint8_t(FALSE, tail);

    /* is_custom */
    tail += marshal_uint8_t(FALSE, tail);
    tail += marshal_uint8_t(FALSE, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t unmarshal_mcd_rpc_trig_st(const char *buf, mcd_rpc_trig_st *obj)
{
    const char *head = buf;

    head += unmarshal_uint8_t(head, (uint8_t *)&obj->is_complex_core);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_complex_core_st(head, obj->complex_core);
        }
    }

    head += unmarshal_uint8_t(head, (uint8_t *)&obj->is_simple_core);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_simple_core_st(head, obj->simple_core);
        }
    }

    /* backwards compatibility:
     * is_trig_bus, is_counter, is_custom */

    return (uint32_t)(head - buf) + 3 * 2 * sizeof(uint8_t);
}

static uint32_t rpc_unmarshal_mcd_open_server_result(
    const char *buf, mcd_open_server_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->server.server_uid);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->server.host_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                mcd_char_t *host;
                if (len) {
                    host = new mcd_char_t[len];
                } else {
                    host = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_char_t(head, host + i);
                }

                obj->server.host = host;
            }
        } else {
            obj->server.host = 0;
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->server.config_string_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                mcd_char_t *config_string;
                if (len) {
                    config_string = new mcd_char_t[len];
                } else {
                    config_string = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_char_t(head, config_string + i);
                }

                obj->server.config_string = config_string;
            }
        } else {
            obj->server.config_string = 0;
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_close_server_args(
    const mcd_close_server_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->server_uid, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_close_server_result(
    const char *buf, mcd_close_server_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_systems_args(
    const mcd_qry_systems_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_systems, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_systems_result(
    const char *buf, mcd_qry_systems_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, obj->num_systems);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->system_con_info_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);
                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_core_con_info_st(
                        head, obj->system_con_info + i);
                }
            }
        } else {
            obj->system_con_info = 0;
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_devices_args(
    const mcd_qry_devices_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_mcd_core_con_info_st(obj->system_con_info, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_devices, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_devices_result(
    const char *buf, mcd_qry_devices_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, obj->num_devices);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->device_con_info_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);
                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_core_con_info_st(
                        head, obj->device_con_info + i);
                }
            }
        } else {
            obj->device_con_info = 0;
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_cores_args(const mcd_qry_cores_args *obj,
                                               char *buf)
{
    char *tail = buf;

    tail += marshal_mcd_core_con_info_st(obj->connection_info, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_cores, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_cores_result(const char *buf,
                                                   mcd_qry_cores_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, obj->num_cores);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->core_con_info_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);
                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_core_con_info_st(
                        head, obj->core_con_info + i);
                }
            }
        } else {
            obj->core_con_info = 0;
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_open_core_args(const mcd_open_core_args *obj,
                                               char *buf)
{
    char *tail = buf;

    tail += marshal_mcd_core_con_info_st(obj->core_con_info, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_open_core_result(const char *buf,
                                                   mcd_open_core_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->core.core_uid);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            obj->core.core_con_info = new mcd_core_con_info_st{};
            head +=
                unmarshal_mcd_core_con_info_st(head, obj->core.core_con_info);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_close_core_args(const mcd_close_core_args *obj,
                                                char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_close_core_result(const char *buf,
                                                    mcd_close_core_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_error_info_args(
    const mcd_qry_error_info_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_mcd_bool_t(obj->has_core_uid, tail);

    tail += marshal_uint8_t(obj->has_core_uid != 0, tail);
    if (obj->has_core_uid != 0) {
        tail += marshal_uint32_t(obj->core_uid, tail);
    }

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_error_info_result(
    const char *buf, mcd_qry_error_info_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_error_info_st(head, obj->error_info);

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_mem_spaces_args(
    const mcd_qry_mem_spaces_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_mem_spaces, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_mem_spaces_result(
    const char *buf, mcd_qry_mem_spaces_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, obj->num_mem_spaces);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->mem_spaces_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);
                for (uint32_t i = 0; i < len; i++) {
                    head +=
                        unmarshal_mcd_memspace_st(head, obj->mem_spaces + i);
                }
            }
        } else {
            obj->mem_spaces = 0;
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_reg_groups_args(
    const mcd_qry_reg_groups_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_reg_groups, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_reg_groups_result(
    const char *buf, mcd_qry_reg_groups_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, obj->num_reg_groups);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->reg_groups_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);
                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_register_group_st(
                        head, obj->reg_groups + i);
                }
            }
        } else {
            obj->reg_groups = 0;
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_reg_map_args(
    const mcd_qry_reg_map_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->reg_group_id, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_regs, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_reg_map_result(
    const char *buf, mcd_qry_reg_map_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, obj->num_regs);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->reg_info_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);
                for (uint32_t i = 0; i < len; i++) {
                    head +=
                        unmarshal_mcd_register_info_st(head, obj->reg_info + i);
                }
            }
        } else {
            obj->reg_info = 0;
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_execute_txlist_args(
    const mcd_execute_txlist_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_txlist_st(obj->txlist, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_execute_txlist_result(
    const char *buf, mcd_execute_txlist_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_txlist_st(head, obj->txlist);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_trig_info_args(
    const mcd_qry_trig_info_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_trig_info_result(
    const char *buf, mcd_qry_trig_info_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_info_st(head, obj->trig_info);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_ctrigs_args(const mcd_qry_ctrigs_args *obj,
                                                char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_ctrigs, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_ctrigs_result(const char *buf,
                                                    mcd_qry_ctrigs_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, obj->num_ctrigs);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->ctrig_info_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);
                for (uint32_t i = 0; i < len; i++) {
                    head +=
                        unmarshal_mcd_ctrig_info_st(head, obj->ctrig_info + i);
                }
            }
        } else {
            obj->ctrig_info = 0;
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_create_trig_args(
    const mcd_create_trig_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_rpc_trig_st(obj->trig, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_create_trig_result(
    const char *buf, mcd_create_trig_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint8_t(head, &obj->trig_modified);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_rpc_trig_st(head, obj->trig);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, obj->trig_id);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_trig_args(const mcd_qry_trig_args *obj,
                                              char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->trig_id, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_trig_result(const char *buf,
                                                  mcd_qry_trig_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_rpc_trig_st(head, obj->trig);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_remove_trig_args(
    const mcd_remove_trig_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->trig_id, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_remove_trig_result(
    const char *buf, mcd_remove_trig_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_trig_state_args(
    const mcd_qry_trig_state_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->trig_id, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_trig_state_result(
    const char *buf, mcd_qry_trig_state_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_state_st(head, obj->trig_state);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_activate_trig_set_args(
    const mcd_activate_trig_set_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_activate_trig_set_result(
    const char *buf, mcd_activate_trig_set_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_remove_trig_set_args(
    const mcd_remove_trig_set_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_remove_trig_set_result(
    const char *buf, mcd_remove_trig_set_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_trig_set_args(
    const mcd_qry_trig_set_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_trigs, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_trig_set_result(
    const char *buf, mcd_qry_trig_set_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, obj->num_trigs);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->trig_ids_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);
                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_uint32_t(head, obj->trig_ids + i);
                }
            }
        } else {
            obj->trig_ids = 0;
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_trig_set_state_args(
    const mcd_qry_trig_set_state_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_trig_set_state_result(
    const char *buf, mcd_qry_trig_set_state_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_set_state_st(head, obj->trig_state);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_run_args(const mcd_run_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_bool_t(obj->global, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_run_result(const char *buf,
                                             mcd_run_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_stop_args(const mcd_stop_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_bool_t(obj->global, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_stop_result(const char *buf,
                                              mcd_stop_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_step_args(const mcd_step_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_bool_t(obj->global, tail);

    tail += marshal_mcd_core_step_type_et(obj->step_type, tail);

    tail += marshal_uint32_t(obj->n_steps, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_step_result(const char *buf,
                                              mcd_step_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_set_global_args(const mcd_set_global_args *obj,
                                                char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_bool_t(obj->enable, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_set_global_result(const char *buf,
                                                    mcd_set_global_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_state_args(const mcd_qry_state_args *obj,
                                               char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_state_result(const char *buf,
                                                   mcd_qry_state_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_core_state_st(head, obj->state);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_rst_classes_args(
    const mcd_qry_rst_classes_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_rst_classes_result(
    const char *buf, mcd_qry_rst_classes_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, obj->rst_class_vector);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_qry_rst_class_info_args(
    const mcd_qry_rst_class_info_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint8_t(obj->rst_class, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_qry_rst_class_info_result(
    const char *buf, mcd_qry_rst_class_info_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_rst_info_st(head, obj->rst_info);
        }
    }

    return (uint32_t)(head - buf);
}

static uint32_t rpc_marshal_mcd_rst_args(const mcd_rst_args *obj, char *buf)
{
    char *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->rst_class_vector, tail);

    tail += marshal_mcd_bool_t(obj->rst_and_halt, tail);

    return (uint32_t)(tail - buf);
}

static uint32_t rpc_unmarshal_mcd_rst_result(const char *buf,
                                             mcd_rst_result *obj)
{
    const char *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->return_status);

    return (uint32_t)(head - buf);
}

uint32_t marshal_mcd_exit(char *buf, size_t buf_size)
{
    return marshal_uint8_t(UID_MCD_EXIT, buf);
}

#define DEFINE_RPC(function, uid)                                              \
    uint32_t marshal_##function##_args(function##_args const *args,            \
                                       char *buf, size_t buf_size)             \
    {                                                                          \
        char *marsh = buf + sizeof(uint32_t); /* reserve space for length */   \
        char *tail = marsh;                                                    \
        tail += marshal_uint8_t(uid, tail);                                    \
        tail += rpc_marshal_##function##_args(args, tail);                     \
        *(uint32_t *)buf = (uint32_t)(tail - marsh);                           \
        return (uint32_t)(tail - buf);                                         \
    }                                                                          \
    mcd_return_et unmarshal_##function##_result(char const *buf,               \
                                                function##_result *res,        \
                                                mcd_error_info_st *error_info) \
    {                                                                          \
        uint32_t length;                                                       \
        buf += unmarshal_uint32_t(buf, &length);                               \
        uint32_t actual_length = rpc_unmarshal_##function##_result(buf, res);  \
        if (actual_length != length) {                                         \
            *error_info = {                                                    \
                .return_status = MCD_RET_ACT_HANDLE_ERROR,                     \
                .error_code = MCD_ERR_CONNECTION,                              \
                .error_events = MCD_ERR_EVT_NONE,                              \
                .error_str = "",                                               \
            };                                                                 \
            snprintf(error_info->error_str, MCD_INFO_STR_LEN,                  \
                     "RPC error: unmarshalled length does not match expected " \
                     "length (%d vs. %d)",                                     \
                     actual_length, length);                                   \
            return error_info->return_status;                                  \
        }                                                                      \
        return MCD_RET_ACT_NONE;                                               \
    }

DEFINE_RPC(mcd_open_server, UID_MCD_OPEN_SERVER)
DEFINE_RPC(mcd_close_server, UID_MCD_CLOSE_SERVER)
DEFINE_RPC(mcd_qry_systems, UID_MCD_QRY_SYSTEMS)
DEFINE_RPC(mcd_qry_devices, UID_MCD_QRY_DEVICES)
DEFINE_RPC(mcd_qry_cores, UID_MCD_QRY_CORES)
DEFINE_RPC(mcd_open_core, UID_MCD_OPEN_CORE)
DEFINE_RPC(mcd_close_core, UID_MCD_CLOSE_CORE)
DEFINE_RPC(mcd_qry_error_info, UID_MCD_QRY_ERROR_INFO)
DEFINE_RPC(mcd_qry_mem_spaces, UID_MCD_QRY_MEM_SPACES)
DEFINE_RPC(mcd_qry_reg_groups, UID_MCD_QRY_REG_GROUPS)
DEFINE_RPC(mcd_qry_reg_map, UID_MCD_QRY_REG_MAP)
DEFINE_RPC(mcd_execute_txlist, UID_MCD_EXECUTE_TXLIST)
DEFINE_RPC(mcd_qry_trig_info, UID_MCD_QRY_TRIG_INFO)
DEFINE_RPC(mcd_qry_ctrigs, UID_MCD_QRY_CTRIGS)
DEFINE_RPC(mcd_create_trig, UID_MCD_CREATE_TRIG)
DEFINE_RPC(mcd_qry_trig, UID_MCD_QRY_TRIG)
DEFINE_RPC(mcd_remove_trig, UID_MCD_REMOVE_TRIG)
DEFINE_RPC(mcd_qry_trig_state, UID_MCD_QRY_TRIG_STATE)
DEFINE_RPC(mcd_activate_trig_set, UID_MCD_ACTIVATE_TRIG_SET)
DEFINE_RPC(mcd_remove_trig_set, UID_MCD_REMOVE_TRIG_SET)
DEFINE_RPC(mcd_qry_trig_set, UID_MCD_QRY_TRIG_SET)
DEFINE_RPC(mcd_qry_trig_set_state, UID_MCD_QRY_TRIG_SET_STATE)
DEFINE_RPC(mcd_run, UID_MCD_RUN)
DEFINE_RPC(mcd_stop, UID_MCD_STOP)
DEFINE_RPC(mcd_step, UID_MCD_STEP)
DEFINE_RPC(mcd_set_global, UID_MCD_SET_GLOBAL)
DEFINE_RPC(mcd_qry_state, UID_MCD_QRY_STATE)
DEFINE_RPC(mcd_qry_rst_classes, UID_MCD_QRY_RST_CLASSES)
DEFINE_RPC(mcd_qry_rst_class_info, UID_MCD_QRY_RST_CLASS_INFO)
DEFINE_RPC(mcd_rst, UID_MCD_RST)

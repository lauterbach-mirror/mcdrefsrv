/*
 * Remote Procedure Call (RPC) support for Multi-Core Debug API (MCD)
 *
 * Copyright (c) 2024 Lauterbach GmbH
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-2.0+
 */

#ifndef MCD_RPC_H
#define MCD_RPC_H

#include "mcd_api.h"
#include "stdlib.h"

#if defined __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-function"
#elif defined _MSC_VER
#       pragma warning(disable: 4505) // disable warning C4505
#endif

#if defined __BYTE_ORDER__
static constexpr bool HOST_BIG_ENDIAN { __BYTE_ORDER__!=__ORDER_LITTLE_ENDIAN__ };
#elif defined _MSC_VER
static constexpr bool HOST_BIG_ENDIAN { false };
#endif

/* custom memory space types */
const mcd_mem_type_et MCD_MEM_SPACE_IS_SECURE = 0x00010000;

/* custom error codes */
const mcd_error_code_et MCD_ERR_RPC_MARSHAL = 0x20000000;
const mcd_error_code_et MCD_ERR_RPC_UNMARSHAL = 0x20000001;

#define MCD_MAX_PACKET_LENGTH 65535

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

#define MAX_NUM_UID_MCD 256

typedef struct {
    mcd_api_version_st version_req;
} mcd_initialize_args;

typedef struct {
    mcd_return_et ret;
    mcd_impl_version_info_st impl_info;
} mcd_initialize_result;

typedef struct {
    uint32_t host_len;
    mcd_char_t *host;
    mcd_bool_t running;
    uint32_t start_index;
    uint32_t num_servers;
} mcd_qry_servers_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_servers;
    /* performance boost since server_info_len != num_servers
     * if num_servers were 0 on request */
    uint32_t server_info_len;
    mcd_server_info_st *server_info;
} mcd_qry_servers_result;

typedef struct {
    uint32_t system_key_len;
    mcd_char_t *system_key;
    uint32_t config_string_len;
    mcd_char_t *config_string;
} mcd_open_server_args;

typedef struct {
    mcd_return_et ret;
    uint32_t server_uid;
    uint32_t host_len;
    mcd_char_t *host;
    uint32_t config_string_len;
    mcd_char_t *config_string;
} mcd_open_server_result;

typedef struct {
    uint32_t server_uid;
} mcd_close_server_args;

typedef struct {
    mcd_return_et ret;
} mcd_close_server_result;

typedef struct {
    uint32_t server_uid;
    uint32_t config_string_len;
    mcd_char_t *config_string;
} mcd_set_server_config_args;

typedef struct {
    mcd_return_et ret;
} mcd_set_server_config_result;

typedef struct {
    uint32_t server_uid;
    uint32_t max_len;
} mcd_qry_server_config_args;

typedef struct {
    mcd_return_et ret;
    uint32_t max_len;
    uint32_t config_string_len;
    mcd_char_t *config_string;
} mcd_qry_server_config_result;

typedef struct {
    uint32_t start_index;
    uint32_t num_systems;
} mcd_qry_systems_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_systems;
    uint32_t system_con_info_len;
    mcd_core_con_info_st *system_con_info;
} mcd_qry_systems_result;

typedef struct {
    mcd_core_con_info_st system_con_info;
    uint32_t start_index;
    uint32_t num_devices;
} mcd_qry_devices_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_devices;
    uint32_t device_con_info_len;
    mcd_core_con_info_st *device_con_info;
} mcd_qry_devices_result;

typedef struct {
    mcd_core_con_info_st connection_info;
    uint32_t start_index;
    uint32_t num_cores;
} mcd_qry_cores_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_cores;
    uint32_t core_con_info_len;
    mcd_core_con_info_st *core_con_info;
} mcd_qry_cores_result;


typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_modes;
} mcd_qry_core_modes_args;

typedef struct {
    mcd_return_et ret;
    uint32_t core_uid;
    uint32_t num_modes;
    uint32_t core_mode_info_len;
    mcd_core_mode_info_st *core_mode_info;
} mcd_qry_core_modes_result;

typedef struct {
    mcd_core_con_info_st core_con_info;
} mcd_open_core_args;

typedef struct {
    mcd_return_et ret;
    uint32_t core_uid;
    mcd_core_con_info_st core_con_info;
} mcd_open_core_result;

typedef struct {
    uint32_t core_uid;
} mcd_close_core_args;

typedef struct {
    mcd_return_et ret;
} mcd_close_core_result;

typedef struct {
    mcd_bool_t has_core_uid;
    uint32_t core_uid;
} mcd_qry_error_info_args;

typedef struct {
    mcd_error_info_st error_info;
} mcd_qry_error_info_result;

typedef struct {
    uint32_t core_uid;
    uint32_t url_length;
} mcd_qry_device_description_args;

typedef struct {
    mcd_return_et ret;
    uint32_t url_length;
    uint32_t url_len;
    mcd_char_t *url;
} mcd_qry_device_description_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_max_payload_size_args;

typedef struct {
    mcd_return_et ret;
    uint32_t max_payload;
} mcd_qry_max_payload_size_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_input_handle_args;

typedef struct {
    mcd_return_et ret;
    uint32_t input_handle;
} mcd_qry_input_handle_result;

typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_mem_spaces;
} mcd_qry_mem_spaces_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_mem_spaces;
    uint32_t mem_spaces_len;
    mcd_memspace_st *mem_spaces;
} mcd_qry_mem_spaces_result;

typedef struct {
    uint32_t core_uid;
    uint32_t mem_space_id;
    uint32_t start_index;
    uint32_t num_mem_blocks;
} mcd_qry_mem_blocks_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_mem_blocks;
    uint32_t mem_blocks_len;
    mcd_memblock_st *mem_blocks;
} mcd_qry_mem_blocks_result;

typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_active_overlays;
} mcd_qry_active_overlays_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_active_overlays;
    uint32_t active_overlays_len;
    uint32_t *active_overlays;
} mcd_qry_active_overlays_result;

typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_reg_groups;
} mcd_qry_reg_groups_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_reg_groups;
    uint32_t reg_groups_len;
    mcd_register_group_st *reg_groups;
} mcd_qry_reg_groups_result;

typedef struct {
    uint32_t core_uid;
    uint32_t reg_group_id;
    uint32_t start_index;
    uint32_t num_regs;
} mcd_qry_reg_map_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_regs;
    uint32_t reg_info_len;
    mcd_register_info_st *reg_info;
} mcd_qry_reg_map_result;

typedef struct {
    uint32_t core_uid;
    uint32_t compound_reg_id;
    uint32_t start_index;
    uint32_t num_reg_ids;
} mcd_qry_reg_compound_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_reg_ids;
    uint32_t reg_id_array_len;
    uint32_t *reg_id_array;
} mcd_qry_reg_compound_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_trig_info_args;

typedef struct {
    mcd_return_et ret;
    mcd_trig_info_st trig_info;
} mcd_qry_trig_info_result;

typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_ctrigs;
} mcd_qry_ctrigs_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_ctrigs;
    uint32_t ctrig_info_len;
    mcd_ctrig_info_st *ctrig_info;
} mcd_qry_ctrigs_result;

typedef struct {
    uint8_t is_complex_core;
    mcd_trig_complex_core_st complex_core;
    uint8_t is_simple_core;
    mcd_trig_simple_core_st simple_core;
    uint8_t is_trig_bus;
    mcd_trig_trig_bus_st trig_bus;
    uint8_t is_counter;
    mcd_trig_counter_st counter;
    uint8_t is_custom;
    mcd_trig_custom_st custom;
} mcd_rpc_trig_st;

typedef struct {
    uint32_t core_uid;
    mcd_rpc_trig_st trig;
} mcd_create_trig_args;

typedef struct {
    mcd_return_et ret;
    uint8_t trig_modified;
    mcd_rpc_trig_st trig;
    uint32_t trig_id;
} mcd_create_trig_result;

typedef struct {
    uint32_t core_uid;
    uint32_t trig_id;
} mcd_qry_trig_args;

typedef struct {
    mcd_return_et ret;
    mcd_rpc_trig_st trig;
} mcd_qry_trig_result;

typedef struct {
    uint32_t core_uid;
    uint32_t trig_id;
} mcd_remove_trig_args;

typedef struct {
    mcd_return_et ret;
} mcd_remove_trig_result;

typedef struct {
    uint32_t core_uid;
    uint32_t trig_id;
} mcd_qry_trig_state_args;

typedef struct {
    mcd_return_et ret;
    mcd_trig_state_st trig_state;
} mcd_qry_trig_state_result;

typedef struct {
    uint32_t core_uid;
} mcd_activate_trig_set_args;

typedef struct {
    mcd_return_et ret;
} mcd_activate_trig_set_result;

typedef struct {
    uint32_t core_uid;
} mcd_remove_trig_set_args;

typedef struct {
    mcd_return_et ret;;
} mcd_remove_trig_set_result;

typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_trigs;
} mcd_qry_trig_set_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_trigs;
    uint32_t trig_ids_len;
    uint32_t *trig_ids;
} mcd_qry_trig_set_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_trig_set_state_args;

typedef struct {
    mcd_return_et ret;
    mcd_trig_set_state_st trig_state;
} mcd_qry_trig_set_state_result;

typedef struct {
    uint32_t core_uid;
    mcd_txlist_st txlist;
} mcd_execute_txlist_args;

typedef struct {
    mcd_return_et ret;
    mcd_txlist_st txlist;
} mcd_execute_txlist_result;

typedef struct {
    uint32_t core_uid;
    mcd_bool_t global;
} mcd_run_args;

typedef struct {
    mcd_return_et ret;
} mcd_run_result;

typedef struct {
    uint32_t core_uid;
    mcd_bool_t global;
} mcd_stop_args;

typedef struct {
    mcd_return_et ret;
} mcd_stop_result;

typedef struct {
    uint32_t core_uid;
    mcd_bool_t global;
    mcd_bool_t absolute_time;
    uint64_t run_until_time;
} mcd_run_until_args;

typedef struct {
    mcd_return_et ret;
} mcd_run_until_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_current_time_args;

typedef struct {
    mcd_return_et ret;
    uint64_t current_time;
} mcd_qry_current_time_result;

typedef struct {
    uint32_t core_uid;
    mcd_bool_t global;
    mcd_core_step_type_et step_type;
    uint32_t n_steps;
} mcd_step_args;

typedef struct {
    mcd_return_et ret;
} mcd_step_result;

typedef struct {
    uint32_t core_uid;
    mcd_bool_t enable;
} mcd_set_global_args;

typedef struct {
    mcd_return_et ret;
} mcd_set_global_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_state_args;

typedef struct {
    mcd_return_et ret;
    mcd_core_state_st state;
} mcd_qry_state_result;

typedef struct {
    uint32_t core_uid;
    uint32_t command_string_len;
    mcd_char_t *command_string;
    uint32_t result_string_size;
} mcd_execute_command_args;

typedef struct {
    mcd_return_et ret;
    uint32_t result_string_len;
    mcd_char_t *result_string;
} mcd_execute_command_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_rst_classes_args;

typedef struct {
    mcd_return_et ret;
    uint32_t rst_class_vector;
} mcd_qry_rst_classes_result;

typedef struct {
    uint32_t core_uid;
    uint8_t rst_class;
} mcd_qry_rst_class_info_args;

typedef struct {
    mcd_return_et ret;
    mcd_rst_info_st rst_info;
} mcd_qry_rst_class_info_result;

typedef struct {
    uint32_t core_uid;
    uint32_t rst_class_vector;
    mcd_bool_t rst_and_halt;
} mcd_rst_args;

typedef struct {
    mcd_return_et ret;
} mcd_rst_result;

typedef struct {
    uint32_t core_uid;
    mcd_chl_st channel;
} mcd_chl_open_args;

typedef struct {
    mcd_return_et ret;
    mcd_chl_st channel;
} mcd_chl_open_result;

typedef struct {
    uint32_t core_uid;
    mcd_chl_st channel;
    uint32_t msg_len;
    uint8_t *msg;
} mcd_send_msg_args;

typedef struct {
    mcd_return_et ret;
} mcd_send_msg_result;

typedef struct {
    uint32_t core_uid;
    mcd_chl_st channel;
    uint32_t timeout;
    uint32_t msg_len;
} mcd_receive_msg_args;

typedef struct {
    mcd_return_et ret;
    uint32_t msg_len;
    uint8_t *msg;
} mcd_receive_msg_result;

typedef struct {
    uint32_t core_uid;
    mcd_chl_st channel;
} mcd_chl_reset_args;

typedef struct {
    mcd_return_et ret;
} mcd_chl_reset_result;

typedef struct {
    uint32_t core_uid;
    mcd_chl_st channel;
} mcd_chl_close_args;

typedef struct {
    mcd_return_et ret;
} mcd_chl_close_result;

typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_traces;
} mcd_qry_traces_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_traces;
    uint32_t trace_info_len;
    mcd_trace_info_st *trace_info;
} mcd_qry_traces_result;

typedef struct {
    uint32_t core_uid;
    uint32_t trace_id;
} mcd_qry_trace_state_args;

typedef struct {
    mcd_return_et ret;
    mcd_trace_state_st state;
} mcd_qry_trace_state_result;

typedef struct {
    uint32_t core_uid;
    uint32_t trace_id;
    mcd_trace_state_st state;
} mcd_set_trace_state_args;

typedef struct {
    mcd_return_et ret;
    uint8_t state_modified;
    mcd_trace_state_st state;
} mcd_set_trace_state_result;

typedef struct {
    uint32_t trace_data_struct_size;
    mcd_trace_data_core_st core_trace;
    mcd_trace_data_event_st event_trace;
    mcd_trace_data_stat_st stat_trace;
} mcd_rpc_trace_data_st;

typedef struct {
    uint32_t core_uid;
    uint32_t trace_id;
    uint64_t start_index;
    uint32_t num_frames;
    uint8_t is_trace_data_core;
    uint8_t is_trace_data_event;
    uint8_t is_trace_data_stat;
} mcd_read_trace_args;

typedef struct {
    mcd_return_et ret;
    uint32_t num_frames;
    uint32_t trace_data_len;
    uint8_t is_trace_data_core;
    mcd_trace_data_core_st *trace_data_core;
    uint8_t is_trace_data_event;
    mcd_trace_data_event_st *trace_data_event;
    uint8_t is_trace_data_stat;
    mcd_trace_data_stat_st *trace_data_stat;
} mcd_read_trace_result;

#define DEFINE_PRIMITIVE(type) \
static int marshal_##type (type obj, uint8_t *buf) \
{ \
    const int BYTES = sizeof(type); \
    if constexpr (HOST_BIG_ENDIAN && BYTES > 1) \
        for (int i = BYTES-1; i >= 0; i--) \
            *buf++ = ((uint8_t *)&obj)[i]; \
    else *(type *)buf = obj; \
    return BYTES; \
} \
static int unmarshal_##type (const uint8_t *buf, type *obj) \
{ \
    const int BYTES = sizeof(type); \
    if constexpr (HOST_BIG_ENDIAN && BYTES > 1) \
        for (int i = 0; i < BYTES; i++) \
            ((uint8_t *)obj)[i] = buf[BYTES-i-1]; \
    else *obj = *(const type *)buf; \
    return BYTES; \
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

static int marshal_mcd_bool_t (mcd_bool_t obj, uint8_t *buf)
{
    uint8_t b = !!obj;
    return marshal_uint8_t(b, buf);
}

static int unmarshal_mcd_bool_t (const uint8_t *buf, mcd_bool_t *obj)
{
    uint8_t b;
    int l = unmarshal_uint8_t(buf, &b);
    *obj = b ? TRUE : FALSE;
    return l;
}

static uint32_t marshal_mcd_activate_trig_set_args(const mcd_activate_trig_set_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_activate_trig_set_args(const uint8_t *buf, mcd_activate_trig_set_args *obj);

static uint32_t marshal_mcd_activate_trig_set_result(const mcd_activate_trig_set_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_activate_trig_set_result(const uint8_t *buf, mcd_activate_trig_set_result *obj);

static uint32_t marshal_mcd_addr_st(const mcd_addr_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_addr_st(const uint8_t *buf, mcd_addr_st *obj);

static uint32_t marshal_mcd_api_version_st(const mcd_api_version_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_api_version_st(const uint8_t *buf, mcd_api_version_st *obj);

static uint32_t marshal_mcd_chl_close_args(const mcd_chl_close_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_chl_close_args(const uint8_t *buf, mcd_chl_close_args *obj);

static uint32_t marshal_mcd_chl_close_result(const mcd_chl_close_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_chl_close_result(const uint8_t *buf, mcd_chl_close_result *obj);

static uint32_t marshal_mcd_chl_open_args(const mcd_chl_open_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_chl_open_args(const uint8_t *buf, mcd_chl_open_args *obj);

static uint32_t marshal_mcd_chl_open_result(const mcd_chl_open_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_chl_open_result(const uint8_t *buf, mcd_chl_open_result *obj);

static uint32_t marshal_mcd_chl_reset_args(const mcd_chl_reset_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_chl_reset_args(const uint8_t *buf, mcd_chl_reset_args *obj);

static uint32_t marshal_mcd_chl_reset_result(const mcd_chl_reset_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_chl_reset_result(const uint8_t *buf, mcd_chl_reset_result *obj);

static uint32_t marshal_mcd_chl_st(const mcd_chl_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_chl_st(const uint8_t *buf, mcd_chl_st *obj);

static uint32_t marshal_mcd_close_core_args(const mcd_close_core_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_close_core_args(const uint8_t *buf, mcd_close_core_args *obj);

static uint32_t marshal_mcd_close_core_result(const mcd_close_core_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_close_core_result(const uint8_t *buf, mcd_close_core_result *obj);

static uint32_t marshal_mcd_close_server_args(const mcd_close_server_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_close_server_args(const uint8_t *buf, mcd_close_server_args *obj);

static uint32_t marshal_mcd_close_server_result(const mcd_close_server_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_close_server_result(const uint8_t *buf, mcd_close_server_result *obj);

static uint32_t marshal_mcd_core_con_info_st(const mcd_core_con_info_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_core_con_info_st(const uint8_t *buf, mcd_core_con_info_st *obj);

static uint32_t marshal_mcd_core_mode_info_st(const mcd_core_mode_info_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_core_mode_info_st(const uint8_t *buf, mcd_core_mode_info_st *obj);

static uint32_t marshal_mcd_core_state_st(const mcd_core_state_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_core_state_st(const uint8_t *buf, mcd_core_state_st *obj);

static uint32_t marshal_mcd_create_trig_args(const mcd_create_trig_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_create_trig_args(const uint8_t *buf, mcd_create_trig_args *obj);

static uint32_t marshal_mcd_create_trig_result(const mcd_create_trig_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_create_trig_result(const uint8_t *buf, mcd_create_trig_result *obj);

static uint32_t marshal_mcd_ctrig_info_st(const mcd_ctrig_info_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_ctrig_info_st(const uint8_t *buf, mcd_ctrig_info_st *obj);

static uint32_t marshal_mcd_error_info_st(const mcd_error_info_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_error_info_st(const uint8_t *buf, mcd_error_info_st *obj);

static uint32_t marshal_mcd_execute_command_args(const mcd_execute_command_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_execute_command_args(const uint8_t *buf, mcd_execute_command_args *obj);
static void free_mcd_execute_command_args(mcd_execute_command_args *obj);

static uint32_t marshal_mcd_execute_command_result(const mcd_execute_command_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_execute_command_result(const uint8_t *buf, mcd_execute_command_result *obj);
static void free_mcd_execute_command_result(mcd_execute_command_result *obj);

static uint32_t marshal_mcd_execute_txlist_args(const mcd_execute_txlist_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_execute_txlist_args(const uint8_t *buf, mcd_execute_txlist_args *obj);
static void free_mcd_execute_txlist_args(mcd_execute_txlist_args *obj);

static uint32_t marshal_mcd_execute_txlist_result(const mcd_execute_txlist_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_execute_txlist_result(const uint8_t *buf, mcd_execute_txlist_result *obj);
static void free_mcd_execute_txlist_result(mcd_execute_txlist_result *obj);

static uint32_t marshal_mcd_impl_version_info_st(const mcd_impl_version_info_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_impl_version_info_st(const uint8_t *buf, mcd_impl_version_info_st *obj);

static uint32_t marshal_mcd_initialize_args(const mcd_initialize_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_initialize_args(const uint8_t *buf, mcd_initialize_args *obj);

static uint32_t marshal_mcd_initialize_result(const mcd_initialize_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_initialize_result(const uint8_t *buf, mcd_initialize_result *obj);

static uint32_t marshal_mcd_memblock_st(const mcd_memblock_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_memblock_st(const uint8_t *buf, mcd_memblock_st *obj);

static uint32_t marshal_mcd_memspace_st(const mcd_memspace_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_memspace_st(const uint8_t *buf, mcd_memspace_st *obj);

static uint32_t marshal_mcd_open_core_args(const mcd_open_core_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_open_core_args(const uint8_t *buf, mcd_open_core_args *obj);

static uint32_t marshal_mcd_open_core_result(const mcd_open_core_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_open_core_result(const uint8_t *buf, mcd_open_core_result *obj);

static uint32_t marshal_mcd_open_server_args(const mcd_open_server_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_open_server_args(const uint8_t *buf, mcd_open_server_args *obj);
static void free_mcd_open_server_args(mcd_open_server_args *obj);

static uint32_t marshal_mcd_open_server_result(const mcd_open_server_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_open_server_result(const uint8_t *buf, mcd_open_server_result *obj);
static void free_mcd_open_server_result(mcd_open_server_result *obj);

static uint32_t marshal_mcd_qry_active_overlays_args(const mcd_qry_active_overlays_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_active_overlays_args(const uint8_t *buf, mcd_qry_active_overlays_args *obj);

static uint32_t marshal_mcd_qry_active_overlays_result(const mcd_qry_active_overlays_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_active_overlays_result(const uint8_t *buf, mcd_qry_active_overlays_result *obj);
static void free_mcd_qry_active_overlays_result(mcd_qry_active_overlays_result *obj);

static uint32_t marshal_mcd_qry_core_modes_args(const mcd_qry_core_modes_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_core_modes_args(const uint8_t *buf, mcd_qry_core_modes_args *obj);

static uint32_t marshal_mcd_qry_core_modes_result(const mcd_qry_core_modes_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_core_modes_result(const uint8_t *buf, mcd_qry_core_modes_result *obj);
static void free_mcd_qry_core_modes_result(mcd_qry_core_modes_result *obj);

static uint32_t marshal_mcd_qry_cores_args(const mcd_qry_cores_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_cores_args(const uint8_t *buf, mcd_qry_cores_args *obj);

static uint32_t marshal_mcd_qry_cores_result(const mcd_qry_cores_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_cores_result(const uint8_t *buf, mcd_qry_cores_result *obj);
static void free_mcd_qry_cores_result(mcd_qry_cores_result *obj);

static uint32_t marshal_mcd_qry_ctrigs_args(const mcd_qry_ctrigs_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_ctrigs_args(const uint8_t *buf, mcd_qry_ctrigs_args *obj);

static uint32_t marshal_mcd_qry_ctrigs_result(const mcd_qry_ctrigs_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_ctrigs_result(const uint8_t *buf, mcd_qry_ctrigs_result *obj);
static void free_mcd_qry_ctrigs_result(mcd_qry_ctrigs_result *obj);

static uint32_t marshal_mcd_qry_current_time_args(const mcd_qry_current_time_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_current_time_args(const uint8_t *buf, mcd_qry_current_time_args *obj);

static uint32_t marshal_mcd_qry_current_time_result(const mcd_qry_current_time_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_current_time_result(const uint8_t *buf, mcd_qry_current_time_result *obj);

static uint32_t marshal_mcd_qry_device_description_args(const mcd_qry_device_description_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_device_description_args(const uint8_t *buf, mcd_qry_device_description_args *obj);

static uint32_t marshal_mcd_qry_device_description_result(const mcd_qry_device_description_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_device_description_result(const uint8_t *buf, mcd_qry_device_description_result *obj);
static void free_mcd_qry_device_description_result(mcd_qry_device_description_result *obj);

static uint32_t marshal_mcd_qry_devices_args(const mcd_qry_devices_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_devices_args(const uint8_t *buf, mcd_qry_devices_args *obj);

static uint32_t marshal_mcd_qry_devices_result(const mcd_qry_devices_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_devices_result(const uint8_t *buf, mcd_qry_devices_result *obj);
static void free_mcd_qry_devices_result(mcd_qry_devices_result *obj);

static uint32_t marshal_mcd_qry_error_info_args(const mcd_qry_error_info_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_error_info_args(const uint8_t *buf, mcd_qry_error_info_args *obj);

static uint32_t marshal_mcd_qry_error_info_result(const mcd_qry_error_info_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_error_info_result(const uint8_t *buf, mcd_qry_error_info_result *obj);

static uint32_t marshal_mcd_qry_input_handle_args(const mcd_qry_input_handle_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_input_handle_args(const uint8_t *buf, mcd_qry_input_handle_args *obj);

static uint32_t marshal_mcd_qry_input_handle_result(const mcd_qry_input_handle_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_input_handle_result(const uint8_t *buf, mcd_qry_input_handle_result *obj);

static uint32_t marshal_mcd_qry_max_payload_size_args(const mcd_qry_max_payload_size_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_max_payload_size_args(const uint8_t *buf, mcd_qry_max_payload_size_args *obj);

static uint32_t marshal_mcd_qry_max_payload_size_result(const mcd_qry_max_payload_size_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_max_payload_size_result(const uint8_t *buf, mcd_qry_max_payload_size_result *obj);

static uint32_t marshal_mcd_qry_mem_blocks_args(const mcd_qry_mem_blocks_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_mem_blocks_args(const uint8_t *buf, mcd_qry_mem_blocks_args *obj);

static uint32_t marshal_mcd_qry_mem_blocks_result(const mcd_qry_mem_blocks_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_mem_blocks_result(const uint8_t *buf, mcd_qry_mem_blocks_result *obj);
static void free_mcd_qry_mem_blocks_result(mcd_qry_mem_blocks_result *obj);

static uint32_t marshal_mcd_qry_mem_spaces_args(const mcd_qry_mem_spaces_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_mem_spaces_args(const uint8_t *buf, mcd_qry_mem_spaces_args *obj);

static uint32_t marshal_mcd_qry_mem_spaces_result(const mcd_qry_mem_spaces_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_mem_spaces_result(const uint8_t *buf, mcd_qry_mem_spaces_result *obj);
static void free_mcd_qry_mem_spaces_result(mcd_qry_mem_spaces_result *obj);

static uint32_t marshal_mcd_qry_reg_compound_args(const mcd_qry_reg_compound_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_reg_compound_args(const uint8_t *buf, mcd_qry_reg_compound_args *obj);

static uint32_t marshal_mcd_qry_reg_compound_result(const mcd_qry_reg_compound_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_reg_compound_result(const uint8_t *buf, mcd_qry_reg_compound_result *obj);
static void free_mcd_qry_reg_compound_result(mcd_qry_reg_compound_result *obj);

static uint32_t marshal_mcd_qry_reg_groups_args(const mcd_qry_reg_groups_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_reg_groups_args(const uint8_t *buf, mcd_qry_reg_groups_args *obj);

static uint32_t marshal_mcd_qry_reg_groups_result(const mcd_qry_reg_groups_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_reg_groups_result(const uint8_t *buf, mcd_qry_reg_groups_result *obj);
static void free_mcd_qry_reg_groups_result(mcd_qry_reg_groups_result *obj);

static uint32_t marshal_mcd_qry_reg_map_args(const mcd_qry_reg_map_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_reg_map_args(const uint8_t *buf, mcd_qry_reg_map_args *obj);

static uint32_t marshal_mcd_qry_reg_map_result(const mcd_qry_reg_map_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_reg_map_result(const uint8_t *buf, mcd_qry_reg_map_result *obj);
static void free_mcd_qry_reg_map_result(mcd_qry_reg_map_result *obj);

static uint32_t marshal_mcd_qry_rst_class_info_args(const mcd_qry_rst_class_info_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_rst_class_info_args(const uint8_t *buf, mcd_qry_rst_class_info_args *obj);

static uint32_t marshal_mcd_qry_rst_class_info_result(const mcd_qry_rst_class_info_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_rst_class_info_result(const uint8_t *buf, mcd_qry_rst_class_info_result *obj);

static uint32_t marshal_mcd_qry_rst_classes_args(const mcd_qry_rst_classes_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_rst_classes_args(const uint8_t *buf, mcd_qry_rst_classes_args *obj);

static uint32_t marshal_mcd_qry_rst_classes_result(const mcd_qry_rst_classes_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_rst_classes_result(const uint8_t *buf, mcd_qry_rst_classes_result *obj);

static uint32_t marshal_mcd_qry_server_config_args(const mcd_qry_server_config_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_server_config_args(const uint8_t *buf, mcd_qry_server_config_args *obj);

static uint32_t marshal_mcd_qry_server_config_result(const mcd_qry_server_config_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_server_config_result(const uint8_t *buf, mcd_qry_server_config_result *obj);
static void free_mcd_qry_server_config_result(mcd_qry_server_config_result *obj);

static uint32_t marshal_mcd_qry_servers_args(const mcd_qry_servers_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_servers_args(const uint8_t *buf, mcd_qry_servers_args *obj);
static void free_mcd_qry_servers_args(mcd_qry_servers_args *obj);

static uint32_t marshal_mcd_qry_servers_result(const mcd_qry_servers_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_servers_result(const uint8_t *buf, mcd_qry_servers_result *obj);
static void free_mcd_qry_servers_result(mcd_qry_servers_result *obj);

static uint32_t marshal_mcd_qry_state_args(const mcd_qry_state_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_state_args(const uint8_t *buf, mcd_qry_state_args *obj);

static uint32_t marshal_mcd_qry_state_result(const mcd_qry_state_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_state_result(const uint8_t *buf, mcd_qry_state_result *obj);

static uint32_t marshal_mcd_qry_systems_args(const mcd_qry_systems_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_systems_args(const uint8_t *buf, mcd_qry_systems_args *obj);

static uint32_t marshal_mcd_qry_systems_result(const mcd_qry_systems_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_systems_result(const uint8_t *buf, mcd_qry_systems_result *obj);
static void free_mcd_qry_systems_result(mcd_qry_systems_result *obj);

static uint32_t marshal_mcd_qry_trace_state_args(const mcd_qry_trace_state_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_trace_state_args(const uint8_t *buf, mcd_qry_trace_state_args *obj);

static uint32_t marshal_mcd_qry_trace_state_result(const mcd_qry_trace_state_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_trace_state_result(const uint8_t *buf, mcd_qry_trace_state_result *obj);

static uint32_t marshal_mcd_qry_traces_args(const mcd_qry_traces_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_traces_args(const uint8_t *buf, mcd_qry_traces_args *obj);

static uint32_t marshal_mcd_qry_traces_result(const mcd_qry_traces_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_traces_result(const uint8_t *buf, mcd_qry_traces_result *obj);
static void free_mcd_qry_traces_result(mcd_qry_traces_result *obj);

static uint32_t marshal_mcd_qry_trig_args(const mcd_qry_trig_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_trig_args(const uint8_t *buf, mcd_qry_trig_args *obj);

static uint32_t marshal_mcd_qry_trig_info_args(const mcd_qry_trig_info_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_trig_info_args(const uint8_t *buf, mcd_qry_trig_info_args *obj);

static uint32_t marshal_mcd_qry_trig_info_result(const mcd_qry_trig_info_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_trig_info_result(const uint8_t *buf, mcd_qry_trig_info_result *obj);

static uint32_t marshal_mcd_qry_trig_result(const mcd_qry_trig_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_trig_result(const uint8_t *buf, mcd_qry_trig_result *obj);

static uint32_t marshal_mcd_qry_trig_set_args(const mcd_qry_trig_set_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_trig_set_args(const uint8_t *buf, mcd_qry_trig_set_args *obj);

static uint32_t marshal_mcd_qry_trig_set_result(const mcd_qry_trig_set_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_trig_set_result(const uint8_t *buf, mcd_qry_trig_set_result *obj);
static void free_mcd_qry_trig_set_result(mcd_qry_trig_set_result *obj);

static uint32_t marshal_mcd_qry_trig_set_state_args(const mcd_qry_trig_set_state_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_trig_set_state_args(const uint8_t *buf, mcd_qry_trig_set_state_args *obj);

static uint32_t marshal_mcd_qry_trig_set_state_result(const mcd_qry_trig_set_state_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_trig_set_state_result(const uint8_t *buf, mcd_qry_trig_set_state_result *obj);

static uint32_t marshal_mcd_qry_trig_state_args(const mcd_qry_trig_state_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_trig_state_args(const uint8_t *buf, mcd_qry_trig_state_args *obj);

static uint32_t marshal_mcd_qry_trig_state_result(const mcd_qry_trig_state_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_qry_trig_state_result(const uint8_t *buf, mcd_qry_trig_state_result *obj);

static uint32_t marshal_mcd_read_trace_args(const mcd_read_trace_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_read_trace_args(const uint8_t *buf, mcd_read_trace_args *obj);

static uint32_t marshal_mcd_read_trace_result(const mcd_read_trace_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_read_trace_result(const uint8_t *buf, mcd_read_trace_result *obj);
static void free_mcd_read_trace_result(mcd_read_trace_result *obj);

static uint32_t marshal_mcd_receive_msg_args(const mcd_receive_msg_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_receive_msg_args(const uint8_t *buf, mcd_receive_msg_args *obj);

static uint32_t marshal_mcd_receive_msg_result(const mcd_receive_msg_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_receive_msg_result(const uint8_t *buf, mcd_receive_msg_result *obj);
static void free_mcd_receive_msg_result(mcd_receive_msg_result *obj);

static uint32_t marshal_mcd_register_group_st(const mcd_register_group_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_register_group_st(const uint8_t *buf, mcd_register_group_st *obj);

static uint32_t marshal_mcd_register_info_st(const mcd_register_info_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_register_info_st(const uint8_t *buf, mcd_register_info_st *obj);

static uint32_t marshal_mcd_remove_trig_args(const mcd_remove_trig_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_remove_trig_args(const uint8_t *buf, mcd_remove_trig_args *obj);

static uint32_t marshal_mcd_remove_trig_result(const mcd_remove_trig_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_remove_trig_result(const uint8_t *buf, mcd_remove_trig_result *obj);

static uint32_t marshal_mcd_remove_trig_set_args(const mcd_remove_trig_set_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_remove_trig_set_args(const uint8_t *buf, mcd_remove_trig_set_args *obj);

static uint32_t marshal_mcd_remove_trig_set_result(const mcd_remove_trig_set_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_remove_trig_set_result(const uint8_t *buf, mcd_remove_trig_set_result *obj);

static uint32_t marshal_mcd_rpc_trace_data_st(const mcd_rpc_trace_data_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_rpc_trace_data_st(const uint8_t *buf, mcd_rpc_trace_data_st *obj);

static uint32_t marshal_mcd_rpc_trig_st(const mcd_rpc_trig_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_rpc_trig_st(const uint8_t *buf, mcd_rpc_trig_st *obj);

static uint32_t marshal_mcd_rst_args(const mcd_rst_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_rst_args(const uint8_t *buf, mcd_rst_args *obj);

static uint32_t marshal_mcd_rst_info_st(const mcd_rst_info_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_rst_info_st(const uint8_t *buf, mcd_rst_info_st *obj);

static uint32_t marshal_mcd_rst_result(const mcd_rst_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_rst_result(const uint8_t *buf, mcd_rst_result *obj);

static uint32_t marshal_mcd_run_args(const mcd_run_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_run_args(const uint8_t *buf, mcd_run_args *obj);

static uint32_t marshal_mcd_run_result(const mcd_run_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_run_result(const uint8_t *buf, mcd_run_result *obj);

static uint32_t marshal_mcd_run_until_args(const mcd_run_until_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_run_until_args(const uint8_t *buf, mcd_run_until_args *obj);

static uint32_t marshal_mcd_run_until_result(const mcd_run_until_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_run_until_result(const uint8_t *buf, mcd_run_until_result *obj);

static uint32_t marshal_mcd_send_msg_args(const mcd_send_msg_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_send_msg_args(const uint8_t *buf, mcd_send_msg_args *obj);
static void free_mcd_send_msg_args(mcd_send_msg_args *obj);

static uint32_t marshal_mcd_send_msg_result(const mcd_send_msg_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_send_msg_result(const uint8_t *buf, mcd_send_msg_result *obj);

static uint32_t marshal_mcd_server_info_st(const mcd_server_info_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_server_info_st(const uint8_t *buf, mcd_server_info_st *obj);

static uint32_t marshal_mcd_set_global_args(const mcd_set_global_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_set_global_args(const uint8_t *buf, mcd_set_global_args *obj);

static uint32_t marshal_mcd_set_global_result(const mcd_set_global_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_set_global_result(const uint8_t *buf, mcd_set_global_result *obj);

static uint32_t marshal_mcd_set_server_config_args(const mcd_set_server_config_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_set_server_config_args(const uint8_t *buf, mcd_set_server_config_args *obj);
static void free_mcd_set_server_config_args(mcd_set_server_config_args *obj);

static uint32_t marshal_mcd_set_server_config_result(const mcd_set_server_config_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_set_server_config_result(const uint8_t *buf, mcd_set_server_config_result *obj);

static uint32_t marshal_mcd_set_trace_state_args(const mcd_set_trace_state_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_set_trace_state_args(const uint8_t *buf, mcd_set_trace_state_args *obj);

static uint32_t marshal_mcd_set_trace_state_result(const mcd_set_trace_state_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_set_trace_state_result(const uint8_t *buf, mcd_set_trace_state_result *obj);

static uint32_t marshal_mcd_step_args(const mcd_step_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_step_args(const uint8_t *buf, mcd_step_args *obj);

static uint32_t marshal_mcd_step_result(const mcd_step_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_step_result(const uint8_t *buf, mcd_step_result *obj);

static uint32_t marshal_mcd_stop_args(const mcd_stop_args *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_stop_args(const uint8_t *buf, mcd_stop_args *obj);

static uint32_t marshal_mcd_stop_result(const mcd_stop_result *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_stop_result(const uint8_t *buf, mcd_stop_result *obj);

static uint32_t marshal_mcd_trace_data_core_st(const mcd_trace_data_core_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trace_data_core_st(const uint8_t *buf, mcd_trace_data_core_st *obj);

static uint32_t marshal_mcd_trace_data_event_st(const mcd_trace_data_event_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trace_data_event_st(const uint8_t *buf, mcd_trace_data_event_st *obj);

static uint32_t marshal_mcd_trace_data_stat_st(const mcd_trace_data_stat_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trace_data_stat_st(const uint8_t *buf, mcd_trace_data_stat_st *obj);

static uint32_t marshal_mcd_trace_info_st(const mcd_trace_info_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trace_info_st(const uint8_t *buf, mcd_trace_info_st *obj);

static uint32_t marshal_mcd_trace_state_st(const mcd_trace_state_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trace_state_st(const uint8_t *buf, mcd_trace_state_st *obj);

static uint32_t marshal_mcd_trig_complex_core_st(const mcd_trig_complex_core_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trig_complex_core_st(const uint8_t *buf, mcd_trig_complex_core_st *obj);

static uint32_t marshal_mcd_trig_counter_st(const mcd_trig_counter_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trig_counter_st(const uint8_t *buf, mcd_trig_counter_st *obj);

static uint32_t marshal_mcd_trig_custom_st(const mcd_trig_custom_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trig_custom_st(const uint8_t *buf, mcd_trig_custom_st *obj);

static uint32_t marshal_mcd_trig_info_st(const mcd_trig_info_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trig_info_st(const uint8_t *buf, mcd_trig_info_st *obj);

static uint32_t marshal_mcd_trig_set_state_st(const mcd_trig_set_state_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trig_set_state_st(const uint8_t *buf, mcd_trig_set_state_st *obj);

static uint32_t marshal_mcd_trig_simple_core_st(const mcd_trig_simple_core_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trig_simple_core_st(const uint8_t *buf, mcd_trig_simple_core_st *obj);

static uint32_t marshal_mcd_trig_state_st(const mcd_trig_state_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trig_state_st(const uint8_t *buf, mcd_trig_state_st *obj);

static uint32_t marshal_mcd_trig_trig_bus_st(const mcd_trig_trig_bus_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_trig_trig_bus_st(const uint8_t *buf, mcd_trig_trig_bus_st *obj);

static uint32_t marshal_mcd_tx_st(const mcd_tx_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_tx_st(const uint8_t *buf, mcd_tx_st *obj);
static void free_mcd_tx_st(mcd_tx_st *obj);

static uint32_t marshal_mcd_txlist_st(const mcd_txlist_st *obj, uint8_t *buf);
static uint32_t unmarshal_mcd_txlist_st(const uint8_t *buf, mcd_txlist_st *obj);
static void free_mcd_txlist_st(mcd_txlist_st *obj);

static uint32_t marshal_mcd_activate_trig_set_args(const mcd_activate_trig_set_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_activate_trig_set_args(const uint8_t *buf, mcd_activate_trig_set_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_activate_trig_set_result(const mcd_activate_trig_set_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_activate_trig_set_result(const uint8_t *buf, mcd_activate_trig_set_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_addr_st(const mcd_addr_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint64_t(obj->address, tail);

    tail += marshal_uint32_t(obj->mem_space_id, tail);

    tail += marshal_uint32_t(obj->addr_space_id, tail);

    tail += marshal_mcd_addr_space_type_et(obj->addr_space_type, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_addr_st(const uint8_t *buf, mcd_addr_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint64_t(head, &obj->address);

    head += unmarshal_uint32_t(head, &obj->mem_space_id);

    head += unmarshal_uint32_t(head, &obj->addr_space_id);

    head += unmarshal_mcd_addr_space_type_et(head, &obj->addr_space_type);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_api_version_st(const mcd_api_version_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint16_t(obj->v_api_major, tail);

    tail += marshal_uint16_t(obj->v_api_minor, tail);

    tail += marshal_uint32_t((uint32_t) MCD_API_IMP_VENDOR_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_API_IMP_VENDOR_LEN; i++) {
        tail += marshal_mcd_char_t(obj->author[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_api_version_st(const uint8_t *buf, mcd_api_version_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint16_t(head, &obj->v_api_major);

    head += unmarshal_uint16_t(head, &obj->v_api_minor);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->author + i);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_chl_close_args(const mcd_chl_close_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_chl_st(&obj->channel, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_chl_close_args(const uint8_t *buf, mcd_chl_close_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_mcd_chl_st(head, &obj->channel);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_chl_close_result(const mcd_chl_close_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_chl_close_result(const uint8_t *buf, mcd_chl_close_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_chl_open_args(const mcd_chl_open_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_chl_st(&obj->channel, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_chl_open_args(const uint8_t *buf, mcd_chl_open_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_mcd_chl_st(head, &obj->channel);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_chl_open_result(const mcd_chl_open_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_mcd_chl_st(&obj->channel, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_chl_open_result(const uint8_t *buf, mcd_chl_open_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_chl_st(head, &obj->channel);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_chl_reset_args(const mcd_chl_reset_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_chl_st(&obj->channel, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_chl_reset_args(const uint8_t *buf, mcd_chl_reset_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_mcd_chl_st(head, &obj->channel);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_chl_reset_result(const mcd_chl_reset_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_chl_reset_result(const uint8_t *buf, mcd_chl_reset_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_chl_st(const mcd_chl_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->chl_id, tail);

    tail += marshal_mcd_chl_type_et(obj->type, tail);

    tail += marshal_mcd_chl_attributes_et(obj->attributes, tail);

    tail += marshal_uint32_t(obj->max_msg_len, tail);

    tail += marshal_mcd_addr_st(&obj->msg_buffer_addr, tail);

    tail += marshal_uint8_t(obj->prio, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_chl_st(const uint8_t *buf, mcd_chl_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->chl_id);

    head += unmarshal_mcd_chl_type_et(head, &obj->type);

    head += unmarshal_mcd_chl_attributes_et(head, &obj->attributes);

    head += unmarshal_uint32_t(head, &obj->max_msg_len);

    head += unmarshal_mcd_addr_st(head, &obj->msg_buffer_addr);

    head += unmarshal_uint8_t(head, &obj->prio);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_close_core_args(const mcd_close_core_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_close_core_args(const uint8_t *buf, mcd_close_core_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_close_core_result(const mcd_close_core_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_close_core_result(const uint8_t *buf, mcd_close_core_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_close_server_args(const mcd_close_server_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->server_uid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_close_server_args(const uint8_t *buf, mcd_close_server_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->server_uid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_close_server_result(const mcd_close_server_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_close_server_result(const uint8_t *buf, mcd_close_server_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_core_con_info_st(const mcd_core_con_info_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t((uint32_t) MCD_HOSTNAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_HOSTNAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->host[i], tail);
    }

    tail += marshal_uint32_t(obj->server_port, tail);

    tail += marshal_uint32_t((uint32_t) MCD_KEY_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_KEY_LEN; i++) {
        tail += marshal_mcd_char_t(obj->server_key[i], tail);
    }

    tail += marshal_uint32_t((uint32_t) MCD_KEY_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_KEY_LEN; i++) {
        tail += marshal_mcd_char_t(obj->system_key[i], tail);
    }

    tail += marshal_uint32_t((uint32_t) MCD_KEY_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_KEY_LEN; i++) {
        tail += marshal_mcd_char_t(obj->device_key[i], tail);
    }

    tail += marshal_uint32_t((uint32_t) MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->system[i], tail);
    }

    tail += marshal_uint32_t((uint32_t) MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->system_instance[i], tail);
    }

    tail += marshal_uint32_t((uint32_t) MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->acc_hw[i], tail);
    }

    tail += marshal_uint32_t(obj->device_type, tail);

    tail += marshal_uint32_t((uint32_t) MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->device[i], tail);
    }

    tail += marshal_uint32_t(obj->device_id, tail);

    tail += marshal_uint32_t((uint32_t) MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->core[i], tail);
    }

    tail += marshal_uint32_t(obj->core_type, tail);

    tail += marshal_uint32_t(obj->core_id, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_core_con_info_st(const uint8_t *buf, mcd_core_con_info_st *obj)
{
    const uint8_t *head = buf;

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

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_core_mode_info_st(const mcd_core_mode_info_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint8_t(obj->core_mode, tail);

    tail += marshal_uint32_t((uint32_t) MCD_CORE_MODE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_CORE_MODE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->name[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_core_mode_info_st(const uint8_t *buf, mcd_core_mode_info_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint8_t(head, &obj->core_mode);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->name + i);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_core_state_st(const mcd_core_state_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_core_state_et(obj->state, tail);

    tail += marshal_mcd_core_event_et(obj->event, tail);

    tail += marshal_uint32_t(obj->hw_thread_id, tail);

    tail += marshal_uint32_t(obj->trig_id, tail);

    tail += marshal_uint32_t((uint32_t) MCD_INFO_STR_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_INFO_STR_LEN; i++) {
        tail += marshal_mcd_char_t(obj->stop_str[i], tail);
    }

    tail += marshal_uint32_t((uint32_t) MCD_INFO_STR_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_INFO_STR_LEN; i++) {
        tail += marshal_mcd_char_t(obj->info_str[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_core_state_st(const uint8_t *buf, mcd_core_state_st *obj)
{
    const uint8_t *head = buf;

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

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_create_trig_args(const mcd_create_trig_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_rpc_trig_st(&obj->trig, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_create_trig_args(const uint8_t *buf, mcd_create_trig_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_mcd_rpc_trig_st(head, &obj->trig);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_create_trig_result(const mcd_create_trig_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint8_t(obj->trig_modified, tail);
    }

    tail += marshal_uint8_t((obj->ret == MCD_RET_ACT_NONE) && (obj->trig_modified != 0), tail);
    if ((obj->ret == MCD_RET_ACT_NONE) && (obj->trig_modified != 0)) {
        tail += marshal_mcd_rpc_trig_st(&obj->trig, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->trig_id, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_create_trig_result(const uint8_t *buf, mcd_create_trig_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

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
            head += unmarshal_mcd_rpc_trig_st(head, &obj->trig);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->trig_id);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_ctrig_info_st(const mcd_ctrig_info_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->ctrig_id, tail);

    tail += marshal_uint32_t((uint32_t) MCD_INFO_STR_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_INFO_STR_LEN; i++) {
        tail += marshal_mcd_char_t(obj->info_str[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_ctrig_info_st(const uint8_t *buf, mcd_ctrig_info_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->ctrig_id);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->info_str + i);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_error_info_st(const mcd_error_info_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->return_status, tail);

    tail += marshal_mcd_error_code_et(obj->error_code, tail);

    tail += marshal_mcd_error_event_et(obj->error_events, tail);

    tail += marshal_uint32_t((uint32_t) MCD_INFO_STR_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_INFO_STR_LEN; i++) {
        tail += marshal_mcd_char_t(obj->error_str[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_error_info_st(const uint8_t *buf, mcd_error_info_st *obj)
{
    const uint8_t *head = buf;

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

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_execute_command_args(const mcd_execute_command_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->command_string_len, tail);

    tail += marshal_uint32_t((uint32_t) obj->command_string_len, tail);
    for (uint32_t i = 0; i < (uint32_t) obj->command_string_len; i++) {
        tail += marshal_mcd_char_t(obj->command_string[i], tail);
    }

    tail += marshal_uint32_t(obj->result_string_size, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_execute_command_args(const uint8_t *buf, mcd_execute_command_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->command_string_len);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        if (len) {
            obj->command_string = (mcd_char_t*) malloc(sizeof(*obj->command_string) * len);
        } else {
            obj->command_string = 0;
        }

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->command_string + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->result_string_size);

    return (uint32_t) (head - buf);
}

static void free_mcd_execute_command_args(mcd_execute_command_args *obj)
{
    if (obj->command_string) {
        free(obj->command_string);
    }
}

static uint32_t marshal_mcd_execute_command_result(const mcd_execute_command_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->result_string_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->result_string_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->result_string_len; i++) {
            tail += marshal_mcd_char_t(obj->result_string[i], tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_execute_command_result(const uint8_t *buf, mcd_execute_command_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->result_string_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->result_string = (mcd_char_t*) malloc(sizeof(*obj->result_string) * len);
                } else {
                    obj->result_string = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_char_t(head, obj->result_string + i);
                }
            }
        }
        else {
            obj->result_string = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_execute_command_result(mcd_execute_command_result *obj)
{
    if (obj->result_string) {
        free(obj->result_string);
    }
}

static uint32_t marshal_mcd_execute_txlist_args(const mcd_execute_txlist_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_txlist_st(&obj->txlist, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_execute_txlist_args(const uint8_t *buf, mcd_execute_txlist_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_mcd_txlist_st(head, &obj->txlist);

    return (uint32_t) (head - buf);
}

static void free_mcd_execute_txlist_args(mcd_execute_txlist_args *obj)
{
    free_mcd_txlist_st(&obj->txlist);
}

static uint32_t marshal_mcd_execute_txlist_result(const mcd_execute_txlist_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_mcd_txlist_st(&obj->txlist, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_execute_txlist_result(const uint8_t *buf, mcd_execute_txlist_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_txlist_st(head, &obj->txlist);
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_execute_txlist_result(mcd_execute_txlist_result *obj)
{
    if (obj->ret == MCD_RET_ACT_NONE) {
        free_mcd_txlist_st(&obj->txlist);
    }
}

static uint32_t marshal_mcd_impl_version_info_st(const mcd_impl_version_info_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_api_version_st(&obj->v_api, tail);

    tail += marshal_uint16_t(obj->v_imp_major, tail);

    tail += marshal_uint16_t(obj->v_imp_minor, tail);

    tail += marshal_uint16_t(obj->v_imp_build, tail);

    tail += marshal_uint32_t((uint32_t) MCD_API_IMP_VENDOR_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_API_IMP_VENDOR_LEN; i++) {
        tail += marshal_mcd_char_t(obj->vendor[i], tail);
    }

    tail += marshal_uint32_t((uint32_t) 16, tail);
    for (uint32_t i = 0; i < (uint32_t) 16; i++) {
        tail += marshal_mcd_char_t(obj->date[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_impl_version_info_st(const uint8_t *buf, mcd_impl_version_info_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_api_version_st(head, &obj->v_api);

    head += unmarshal_uint16_t(head, &obj->v_imp_major);

    head += unmarshal_uint16_t(head, &obj->v_imp_minor);

    head += unmarshal_uint16_t(head, &obj->v_imp_build);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->vendor + i);
        }
    }

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->date + i);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_initialize_args(const mcd_initialize_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_api_version_st(&obj->version_req, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_initialize_args(const uint8_t *buf, mcd_initialize_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_api_version_st(head, &obj->version_req);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_initialize_result(const mcd_initialize_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_mcd_impl_version_info_st(&obj->impl_info, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_initialize_result(const uint8_t *buf, mcd_initialize_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_impl_version_info_st(head, &obj->impl_info);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_memblock_st(const mcd_memblock_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->mem_block_id, tail);

    tail += marshal_uint32_t((uint32_t) MCD_MEM_BLOCK_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_MEM_BLOCK_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->mem_block_name[i], tail);
    }

    tail += marshal_mcd_bool_t(obj->has_children, tail);

    tail += marshal_uint32_t(obj->parent_id, tail);

    tail += marshal_uint64_t(obj->start_addr, tail);

    tail += marshal_uint64_t(obj->end_addr, tail);

    tail += marshal_mcd_endian_et(obj->endian, tail);

    tail += marshal_uint32_t((uint32_t) MCD_MEM_AUSIZE_NUM, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_MEM_AUSIZE_NUM; i++) {
        tail += marshal_uint32_t(obj->supported_au_sizes[i], tail);
    }

    tail += marshal_mcd_tx_access_opt_et(obj->supported_access_options, tail);

    tail += marshal_uint32_t(obj->core_mode_mask_read, tail);

    tail += marshal_uint32_t(obj->core_mode_mask_write, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_memblock_st(const uint8_t *buf, mcd_memblock_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->mem_block_id);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->mem_block_name + i);
        }
    }

    head += unmarshal_mcd_bool_t(head, &obj->has_children);

    head += unmarshal_uint32_t(head, &obj->parent_id);

    head += unmarshal_uint64_t(head, &obj->start_addr);

    head += unmarshal_uint64_t(head, &obj->end_addr);

    head += unmarshal_mcd_endian_et(head, &obj->endian);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_uint32_t(head, obj->supported_au_sizes + i);
        }
    }

    head += unmarshal_mcd_tx_access_opt_et(head, &obj->supported_access_options);

    head += unmarshal_uint32_t(head, &obj->core_mode_mask_read);

    head += unmarshal_uint32_t(head, &obj->core_mode_mask_write);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_memspace_st(const mcd_memspace_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->mem_space_id, tail);

    tail += marshal_uint32_t((uint32_t) MCD_MEM_SPACE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_MEM_SPACE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->mem_space_name[i], tail);
    }

    tail += marshal_mcd_mem_type_et(obj->mem_type, tail);

    tail += marshal_uint32_t(obj->bits_per_mau, tail);

    tail += marshal_uint8_t(obj->invariance, tail);

    tail += marshal_mcd_endian_et(obj->endian, tail);

    tail += marshal_uint64_t(obj->min_addr, tail);

    tail += marshal_uint64_t(obj->max_addr, tail);

    tail += marshal_uint32_t(obj->num_mem_blocks, tail);

    tail += marshal_mcd_tx_access_opt_et(obj->supported_access_options, tail);

    tail += marshal_uint32_t(obj->core_mode_mask_read, tail);

    tail += marshal_uint32_t(obj->core_mode_mask_write, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_memspace_st(const uint8_t *buf, mcd_memspace_st *obj)
{
    const uint8_t *head = buf;

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

    head += unmarshal_mcd_tx_access_opt_et(head, &obj->supported_access_options);

    head += unmarshal_uint32_t(head, &obj->core_mode_mask_read);

    head += unmarshal_uint32_t(head, &obj->core_mode_mask_write);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_open_core_args(const mcd_open_core_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_core_con_info_st(&obj->core_con_info, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_open_core_args(const uint8_t *buf, mcd_open_core_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_core_con_info_st(head, &obj->core_con_info);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_open_core_result(const mcd_open_core_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->core_uid, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_mcd_core_con_info_st(&obj->core_con_info, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_open_core_result(const uint8_t *buf, mcd_open_core_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->core_uid);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_core_con_info_st(head, &obj->core_con_info);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_open_server_args(const mcd_open_server_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->system_key_len, tail);

    tail += marshal_uint32_t((uint32_t) obj->system_key_len, tail);
    for (uint32_t i = 0; i < (uint32_t) obj->system_key_len; i++) {
        tail += marshal_mcd_char_t(obj->system_key[i], tail);
    }

    tail += marshal_uint32_t(obj->config_string_len, tail);

    tail += marshal_uint32_t((uint32_t) obj->config_string_len, tail);
    for (uint32_t i = 0; i < (uint32_t) obj->config_string_len; i++) {
        tail += marshal_mcd_char_t(obj->config_string[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_open_server_args(const uint8_t *buf, mcd_open_server_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->system_key_len);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        if (len) {
            obj->system_key = (mcd_char_t*) malloc(sizeof(*obj->system_key) * len);
        } else {
            obj->system_key = 0;
        }

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->system_key + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->config_string_len);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        if (len) {
            obj->config_string = (mcd_char_t*) malloc(sizeof(*obj->config_string) * len);
        } else {
            obj->config_string = 0;
        }

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->config_string + i);
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_open_server_args(mcd_open_server_args *obj)
{
    if (obj->system_key) {
        free(obj->system_key);
    }
    if (obj->config_string) {
        free(obj->config_string);
    }
}

static uint32_t marshal_mcd_open_server_result(const mcd_open_server_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->server_uid, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->host_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->host_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->host_len; i++) {
            tail += marshal_mcd_char_t(obj->host[i], tail);
        }
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->config_string_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->config_string_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->config_string_len; i++) {
            tail += marshal_mcd_char_t(obj->config_string[i], tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_open_server_result(const uint8_t *buf, mcd_open_server_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->server_uid);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->host_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->host = (mcd_char_t*) malloc(sizeof(*obj->host) * len);
                } else {
                    obj->host = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_char_t(head, obj->host + i);
                }
            }
        }
        else {
            obj->host = 0;
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->config_string_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->config_string = (mcd_char_t*) malloc(sizeof(*obj->config_string) * len);
                } else {
                    obj->config_string = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_char_t(head, obj->config_string + i);
                }
            }
        }
        else {
            obj->config_string = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_open_server_result(mcd_open_server_result *obj)
{
    if (obj->host) {
        free(obj->host);
    }
    if (obj->config_string) {
        free(obj->config_string);
    }
}

static uint32_t marshal_mcd_qry_active_overlays_args(const mcd_qry_active_overlays_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_active_overlays, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_active_overlays_args(const uint8_t *buf, mcd_qry_active_overlays_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_active_overlays);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_active_overlays_result(const mcd_qry_active_overlays_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_active_overlays, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->active_overlays_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->active_overlays_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->active_overlays_len; i++) {
            tail += marshal_uint32_t(obj->active_overlays[i], tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_active_overlays_result(const uint8_t *buf, mcd_qry_active_overlays_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_active_overlays);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->active_overlays_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->active_overlays = (uint32_t*) malloc(sizeof(*obj->active_overlays) * len);
                } else {
                    obj->active_overlays = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_uint32_t(head, obj->active_overlays + i);
                }
            }
        }
        else {
            obj->active_overlays = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_active_overlays_result(mcd_qry_active_overlays_result *obj)
{
    if (obj->active_overlays) {
        free(obj->active_overlays);
    }
}

static uint32_t marshal_mcd_qry_core_modes_args(const mcd_qry_core_modes_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_modes, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_core_modes_args(const uint8_t *buf, mcd_qry_core_modes_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_modes);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_core_modes_result(const mcd_qry_core_modes_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_modes, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->core_mode_info_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->core_mode_info_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->core_mode_info_len; i++) {
            tail += marshal_mcd_core_mode_info_st(obj->core_mode_info + i, tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_core_modes_result(const uint8_t *buf, mcd_qry_core_modes_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_modes);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->core_mode_info_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->core_mode_info = (mcd_core_mode_info_st*) malloc(sizeof(*obj->core_mode_info) * len);
                } else {
                    obj->core_mode_info = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_core_mode_info_st(head, obj->core_mode_info + i);
                }
            }
        }
        else {
            obj->core_mode_info = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_core_modes_result(mcd_qry_core_modes_result *obj)
{
    if (obj->core_mode_info) {
        free(obj->core_mode_info);
    }
}

static uint32_t marshal_mcd_qry_cores_args(const mcd_qry_cores_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_core_con_info_st(&obj->connection_info, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_cores, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_cores_args(const uint8_t *buf, mcd_qry_cores_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_core_con_info_st(head, &obj->connection_info);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_cores);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_cores_result(const mcd_qry_cores_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_cores, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->core_con_info_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->core_con_info_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->core_con_info_len; i++) {
            tail += marshal_mcd_core_con_info_st(obj->core_con_info + i, tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_cores_result(const uint8_t *buf, mcd_qry_cores_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_cores);
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

                if (len) {
                    obj->core_con_info = (mcd_core_con_info_st*) malloc(sizeof(*obj->core_con_info) * len);
                } else {
                    obj->core_con_info = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_core_con_info_st(head, obj->core_con_info + i);
                }
            }
        }
        else {
            obj->core_con_info = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_cores_result(mcd_qry_cores_result *obj)
{
    if (obj->core_con_info) {
        free(obj->core_con_info);
    }
}

static uint32_t marshal_mcd_qry_ctrigs_args(const mcd_qry_ctrigs_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_ctrigs, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_ctrigs_args(const uint8_t *buf, mcd_qry_ctrigs_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_ctrigs);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_ctrigs_result(const mcd_qry_ctrigs_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_ctrigs, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->ctrig_info_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->ctrig_info_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->ctrig_info_len; i++) {
            tail += marshal_mcd_ctrig_info_st(obj->ctrig_info + i, tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_ctrigs_result(const uint8_t *buf, mcd_qry_ctrigs_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_ctrigs);
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

                if (len) {
                    obj->ctrig_info = (mcd_ctrig_info_st*) malloc(sizeof(*obj->ctrig_info) * len);
                } else {
                    obj->ctrig_info = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_ctrig_info_st(head, obj->ctrig_info + i);
                }
            }
        }
        else {
            obj->ctrig_info = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_ctrigs_result(mcd_qry_ctrigs_result *obj)
{
    if (obj->ctrig_info) {
        free(obj->ctrig_info);
    }
}

static uint32_t marshal_mcd_qry_current_time_args(const mcd_qry_current_time_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_current_time_args(const uint8_t *buf, mcd_qry_current_time_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_current_time_result(const mcd_qry_current_time_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint64_t(obj->current_time, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_current_time_result(const uint8_t *buf, mcd_qry_current_time_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint64_t(head, &obj->current_time);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_device_description_args(const mcd_qry_device_description_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->url_length, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_device_description_args(const uint8_t *buf, mcd_qry_device_description_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->url_length);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_device_description_result(const mcd_qry_device_description_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->url_length, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->url_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->url_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->url_len; i++) {
            tail += marshal_mcd_char_t(obj->url[i], tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_device_description_result(const uint8_t *buf, mcd_qry_device_description_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->url_length);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->url_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->url = (mcd_char_t*) malloc(sizeof(*obj->url) * len);
                } else {
                    obj->url = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_char_t(head, obj->url + i);
                }
            }
        }
        else {
            obj->url = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_device_description_result(mcd_qry_device_description_result *obj)
{
    if (obj->url) {
        free(obj->url);
    }
}

static uint32_t marshal_mcd_qry_devices_args(const mcd_qry_devices_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_core_con_info_st(&obj->system_con_info, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_devices, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_devices_args(const uint8_t *buf, mcd_qry_devices_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_core_con_info_st(head, &obj->system_con_info);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_devices);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_devices_result(const mcd_qry_devices_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_devices, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->device_con_info_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->device_con_info_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->device_con_info_len; i++) {
            tail += marshal_mcd_core_con_info_st(obj->device_con_info + i, tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_devices_result(const uint8_t *buf, mcd_qry_devices_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_devices);
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

                if (len) {
                    obj->device_con_info = (mcd_core_con_info_st*) malloc(sizeof(*obj->device_con_info) * len);
                } else {
                    obj->device_con_info = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_core_con_info_st(head, obj->device_con_info + i);
                }
            }
        }
        else {
            obj->device_con_info = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_devices_result(mcd_qry_devices_result *obj)
{
    if (obj->device_con_info) {
        free(obj->device_con_info);
    }
}

static uint32_t marshal_mcd_qry_error_info_args(const mcd_qry_error_info_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_bool_t(obj->has_core_uid, tail);

    tail += marshal_uint8_t(obj->has_core_uid != 0, tail);
    if (obj->has_core_uid != 0) {
        tail += marshal_uint32_t(obj->core_uid, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_error_info_args(const uint8_t *buf, mcd_qry_error_info_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_bool_t(head, &obj->has_core_uid);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->core_uid);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_error_info_result(const mcd_qry_error_info_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_error_info_st(&obj->error_info, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_error_info_result(const uint8_t *buf, mcd_qry_error_info_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_error_info_st(head, &obj->error_info);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_input_handle_args(const mcd_qry_input_handle_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_input_handle_args(const uint8_t *buf, mcd_qry_input_handle_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_input_handle_result(const mcd_qry_input_handle_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->input_handle, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_input_handle_result(const uint8_t *buf, mcd_qry_input_handle_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->input_handle);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_max_payload_size_args(const mcd_qry_max_payload_size_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_max_payload_size_args(const uint8_t *buf, mcd_qry_max_payload_size_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_max_payload_size_result(const mcd_qry_max_payload_size_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->max_payload, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_max_payload_size_result(const uint8_t *buf, mcd_qry_max_payload_size_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->max_payload);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_mem_blocks_args(const mcd_qry_mem_blocks_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->mem_space_id, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_mem_blocks, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_mem_blocks_args(const uint8_t *buf, mcd_qry_mem_blocks_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->mem_space_id);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_mem_blocks);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_mem_blocks_result(const mcd_qry_mem_blocks_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_mem_blocks, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->mem_blocks_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->mem_blocks_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->mem_blocks_len; i++) {
            tail += marshal_mcd_memblock_st(obj->mem_blocks + i, tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_mem_blocks_result(const uint8_t *buf, mcd_qry_mem_blocks_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_mem_blocks);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->mem_blocks_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->mem_blocks = (mcd_memblock_st*) malloc(sizeof(*obj->mem_blocks) * len);
                } else {
                    obj->mem_blocks = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_memblock_st(head, obj->mem_blocks + i);
                }
            }
        }
        else {
            obj->mem_blocks = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_mem_blocks_result(mcd_qry_mem_blocks_result *obj)
{
    if (obj->mem_blocks) {
        free(obj->mem_blocks);
    }
}

static uint32_t marshal_mcd_qry_mem_spaces_args(const mcd_qry_mem_spaces_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_mem_spaces, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_mem_spaces_args(const uint8_t *buf, mcd_qry_mem_spaces_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_mem_spaces);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_mem_spaces_result(const mcd_qry_mem_spaces_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_mem_spaces, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->mem_spaces_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->mem_spaces_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->mem_spaces_len; i++) {
            tail += marshal_mcd_memspace_st(obj->mem_spaces + i, tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_mem_spaces_result(const uint8_t *buf, mcd_qry_mem_spaces_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_mem_spaces);
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

                if (len) {
                    obj->mem_spaces = (mcd_memspace_st*) malloc(sizeof(*obj->mem_spaces) * len);
                } else {
                    obj->mem_spaces = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_memspace_st(head, obj->mem_spaces + i);
                }
            }
        }
        else {
            obj->mem_spaces = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_mem_spaces_result(mcd_qry_mem_spaces_result *obj)
{
    if (obj->mem_spaces) {
        free(obj->mem_spaces);
    }
}

static uint32_t marshal_mcd_qry_reg_compound_args(const mcd_qry_reg_compound_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->compound_reg_id, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_reg_ids, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_reg_compound_args(const uint8_t *buf, mcd_qry_reg_compound_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->compound_reg_id);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_reg_ids);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_reg_compound_result(const mcd_qry_reg_compound_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_reg_ids, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->reg_id_array_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->reg_id_array_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->reg_id_array_len; i++) {
            tail += marshal_uint32_t(obj->reg_id_array[i], tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_reg_compound_result(const uint8_t *buf, mcd_qry_reg_compound_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_reg_ids);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->reg_id_array_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->reg_id_array = (uint32_t*) malloc(sizeof(*obj->reg_id_array) * len);
                } else {
                    obj->reg_id_array = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_uint32_t(head, obj->reg_id_array + i);
                }
            }
        }
        else {
            obj->reg_id_array = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_reg_compound_result(mcd_qry_reg_compound_result *obj)
{
    if (obj->reg_id_array) {
        free(obj->reg_id_array);
    }
}

static uint32_t marshal_mcd_qry_reg_groups_args(const mcd_qry_reg_groups_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_reg_groups, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_reg_groups_args(const uint8_t *buf, mcd_qry_reg_groups_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_reg_groups);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_reg_groups_result(const mcd_qry_reg_groups_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_reg_groups, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->reg_groups_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->reg_groups_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->reg_groups_len; i++) {
            tail += marshal_mcd_register_group_st(obj->reg_groups + i, tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_reg_groups_result(const uint8_t *buf, mcd_qry_reg_groups_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_reg_groups);
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

                if (len) {
                    obj->reg_groups = (mcd_register_group_st*) malloc(sizeof(*obj->reg_groups) * len);
                } else {
                    obj->reg_groups = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_register_group_st(head, obj->reg_groups + i);
                }
            }
        }
        else {
            obj->reg_groups = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_reg_groups_result(mcd_qry_reg_groups_result *obj)
{
    if (obj->reg_groups) {
        free(obj->reg_groups);
    }
}

static uint32_t marshal_mcd_qry_reg_map_args(const mcd_qry_reg_map_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->reg_group_id, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_regs, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_reg_map_args(const uint8_t *buf, mcd_qry_reg_map_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->reg_group_id);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_regs);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_reg_map_result(const mcd_qry_reg_map_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_regs, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->reg_info_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->reg_info_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->reg_info_len; i++) {
            tail += marshal_mcd_register_info_st(obj->reg_info + i, tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_reg_map_result(const uint8_t *buf, mcd_qry_reg_map_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_regs);
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

                if (len) {
                    obj->reg_info = (mcd_register_info_st*) malloc(sizeof(*obj->reg_info) * len);
                } else {
                    obj->reg_info = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_register_info_st(head, obj->reg_info + i);
                }
            }
        }
        else {
            obj->reg_info = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_reg_map_result(mcd_qry_reg_map_result *obj)
{
    if (obj->reg_info) {
        free(obj->reg_info);
    }
}

static uint32_t marshal_mcd_qry_rst_class_info_args(const mcd_qry_rst_class_info_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint8_t(obj->rst_class, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_rst_class_info_args(const uint8_t *buf, mcd_qry_rst_class_info_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint8_t(head, &obj->rst_class);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_rst_class_info_result(const mcd_qry_rst_class_info_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_mcd_rst_info_st(&obj->rst_info, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_rst_class_info_result(const uint8_t *buf, mcd_qry_rst_class_info_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_rst_info_st(head, &obj->rst_info);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_rst_classes_args(const mcd_qry_rst_classes_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_rst_classes_args(const uint8_t *buf, mcd_qry_rst_classes_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_rst_classes_result(const mcd_qry_rst_classes_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->rst_class_vector, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_rst_classes_result(const uint8_t *buf, mcd_qry_rst_classes_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->rst_class_vector);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_server_config_args(const mcd_qry_server_config_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->server_uid, tail);

    tail += marshal_uint32_t(obj->max_len, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_server_config_args(const uint8_t *buf, mcd_qry_server_config_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->server_uid);

    head += unmarshal_uint32_t(head, &obj->max_len);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_server_config_result(const mcd_qry_server_config_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->max_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->config_string_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->config_string_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->config_string_len; i++) {
            tail += marshal_mcd_char_t(obj->config_string[i], tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_server_config_result(const uint8_t *buf, mcd_qry_server_config_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->max_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->config_string_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->config_string = (mcd_char_t*) malloc(sizeof(*obj->config_string) * len);
                } else {
                    obj->config_string = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_char_t(head, obj->config_string + i);
                }
            }
        }
        else {
            obj->config_string = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_server_config_result(mcd_qry_server_config_result *obj)
{
    if (obj->config_string) {
        free(obj->config_string);
    }
}

static uint32_t marshal_mcd_qry_servers_args(const mcd_qry_servers_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->host_len, tail);

    tail += marshal_uint32_t((uint32_t) obj->host_len, tail);
    for (uint32_t i = 0; i < (uint32_t) obj->host_len; i++) {
        tail += marshal_mcd_char_t(obj->host[i], tail);
    }

    tail += marshal_mcd_bool_t(obj->running, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_servers, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_servers_args(const uint8_t *buf, mcd_qry_servers_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->host_len);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        if (len) {
            obj->host = (mcd_char_t*) malloc(sizeof(*obj->host) * len);
        } else {
            obj->host = 0;
        }

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->host + i);
        }
    }

    head += unmarshal_mcd_bool_t(head, &obj->running);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_servers);

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_servers_args(mcd_qry_servers_args *obj)
{
    if (obj->host) {
        free(obj->host);
    }
}

static uint32_t marshal_mcd_qry_servers_result(const mcd_qry_servers_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_servers, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->server_info_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->server_info_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->server_info_len; i++) {
            tail += marshal_mcd_server_info_st(obj->server_info + i, tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_servers_result(const uint8_t *buf, mcd_qry_servers_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_servers);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->server_info_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->server_info = (mcd_server_info_st*) malloc(sizeof(*obj->server_info) * len);
                } else {
                    obj->server_info = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_server_info_st(head, obj->server_info + i);
                }
            }
        }
        else {
            obj->server_info = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_servers_result(mcd_qry_servers_result *obj)
{
    if (obj->server_info) {
        free(obj->server_info);
    }
}

static uint32_t marshal_mcd_qry_state_args(const mcd_qry_state_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_state_args(const uint8_t *buf, mcd_qry_state_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_state_result(const mcd_qry_state_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_mcd_core_state_st(&obj->state, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_state_result(const uint8_t *buf, mcd_qry_state_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_core_state_st(head, &obj->state);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_systems_args(const mcd_qry_systems_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_systems, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_systems_args(const uint8_t *buf, mcd_qry_systems_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_systems);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_systems_result(const mcd_qry_systems_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_systems, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->system_con_info_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->system_con_info_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->system_con_info_len; i++) {
            tail += marshal_mcd_core_con_info_st(obj->system_con_info + i, tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_systems_result(const uint8_t *buf, mcd_qry_systems_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_systems);
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

                if (len) {
                    obj->system_con_info = (mcd_core_con_info_st*) malloc(sizeof(*obj->system_con_info) * len);
                } else {
                    obj->system_con_info = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_core_con_info_st(head, obj->system_con_info + i);
                }
            }
        }
        else {
            obj->system_con_info = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_systems_result(mcd_qry_systems_result *obj)
{
    if (obj->system_con_info) {
        free(obj->system_con_info);
    }
}

static uint32_t marshal_mcd_qry_trace_state_args(const mcd_qry_trace_state_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->trace_id, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_trace_state_args(const uint8_t *buf, mcd_qry_trace_state_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->trace_id);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_trace_state_result(const mcd_qry_trace_state_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_mcd_trace_state_st(&obj->state, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_trace_state_result(const uint8_t *buf, mcd_qry_trace_state_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trace_state_st(head, &obj->state);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_traces_args(const mcd_qry_traces_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_traces, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_traces_args(const uint8_t *buf, mcd_qry_traces_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_traces);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_traces_result(const mcd_qry_traces_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_traces, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->trace_info_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->trace_info_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->trace_info_len; i++) {
            tail += marshal_mcd_trace_info_st(obj->trace_info + i, tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_traces_result(const uint8_t *buf, mcd_qry_traces_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_traces);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->trace_info_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->trace_info = (mcd_trace_info_st*) malloc(sizeof(*obj->trace_info) * len);
                } else {
                    obj->trace_info = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_trace_info_st(head, obj->trace_info + i);
                }
            }
        }
        else {
            obj->trace_info = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_traces_result(mcd_qry_traces_result *obj)
{
    if (obj->trace_info) {
        free(obj->trace_info);
    }
}

static uint32_t marshal_mcd_qry_trig_args(const mcd_qry_trig_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->trig_id, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_trig_args(const uint8_t *buf, mcd_qry_trig_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->trig_id);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_trig_info_args(const mcd_qry_trig_info_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_trig_info_args(const uint8_t *buf, mcd_qry_trig_info_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_trig_info_result(const mcd_qry_trig_info_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_mcd_trig_info_st(&obj->trig_info, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_trig_info_result(const uint8_t *buf, mcd_qry_trig_info_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_info_st(head, &obj->trig_info);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_trig_result(const mcd_qry_trig_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_mcd_rpc_trig_st(&obj->trig, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_trig_result(const uint8_t *buf, mcd_qry_trig_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_rpc_trig_st(head, &obj->trig);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_trig_set_args(const mcd_qry_trig_set_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_trigs, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_trig_set_args(const uint8_t *buf, mcd_qry_trig_set_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_trigs);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_trig_set_result(const mcd_qry_trig_set_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_trigs, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->trig_ids_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->trig_ids_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->trig_ids_len; i++) {
            tail += marshal_uint32_t(obj->trig_ids[i], tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_trig_set_result(const uint8_t *buf, mcd_qry_trig_set_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_trigs);
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

                if (len) {
                    obj->trig_ids = (uint32_t*) malloc(sizeof(*obj->trig_ids) * len);
                } else {
                    obj->trig_ids = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_uint32_t(head, obj->trig_ids + i);
                }
            }
        }
        else {
            obj->trig_ids = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_qry_trig_set_result(mcd_qry_trig_set_result *obj)
{
    if (obj->trig_ids) {
        free(obj->trig_ids);
    }
}

static uint32_t marshal_mcd_qry_trig_set_state_args(const mcd_qry_trig_set_state_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_trig_set_state_args(const uint8_t *buf, mcd_qry_trig_set_state_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_trig_set_state_result(const mcd_qry_trig_set_state_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_mcd_trig_set_state_st(&obj->trig_state, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_trig_set_state_result(const uint8_t *buf, mcd_qry_trig_set_state_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_set_state_st(head, &obj->trig_state);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_trig_state_args(const mcd_qry_trig_state_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->trig_id, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_trig_state_args(const uint8_t *buf, mcd_qry_trig_state_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->trig_id);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_qry_trig_state_result(const mcd_qry_trig_state_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_mcd_trig_state_st(&obj->trig_state, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_qry_trig_state_result(const uint8_t *buf, mcd_qry_trig_state_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_state_st(head, &obj->trig_state);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_read_trace_args(const mcd_read_trace_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->trace_id, tail);

    tail += marshal_uint64_t(obj->start_index, tail);

    tail += marshal_uint32_t(obj->num_frames, tail);

    tail += marshal_uint8_t(obj->is_trace_data_core, tail);

    tail += marshal_uint8_t(obj->is_trace_data_event, tail);

    tail += marshal_uint8_t(obj->is_trace_data_stat, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_read_trace_args(const uint8_t *buf, mcd_read_trace_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->trace_id);

    head += unmarshal_uint64_t(head, &obj->start_index);

    head += unmarshal_uint32_t(head, &obj->num_frames);

    head += unmarshal_uint8_t(head, &obj->is_trace_data_core);

    head += unmarshal_uint8_t(head, &obj->is_trace_data_event);

    head += unmarshal_uint8_t(head, &obj->is_trace_data_stat);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_read_trace_result(const mcd_read_trace_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->num_frames, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->trace_data_len, tail);
    }

    tail += marshal_uint8_t(obj->is_trace_data_core, tail);

    tail += marshal_uint8_t((obj->ret == MCD_RET_ACT_NONE) && (obj->is_trace_data_core != 0), tail);
    if ((obj->ret == MCD_RET_ACT_NONE) && (obj->is_trace_data_core != 0)) {
        tail += marshal_uint32_t((uint32_t) obj->trace_data_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->trace_data_len; i++) {
            tail += marshal_mcd_trace_data_core_st(obj->trace_data_core + i, tail);
        }
    }

    tail += marshal_uint8_t(obj->is_trace_data_event, tail);

    tail += marshal_uint8_t((obj->ret == MCD_RET_ACT_NONE) && (obj->is_trace_data_event != 0), tail);
    if ((obj->ret == MCD_RET_ACT_NONE) && (obj->is_trace_data_event != 0)) {
        tail += marshal_uint32_t((uint32_t) obj->trace_data_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->trace_data_len; i++) {
            tail += marshal_mcd_trace_data_event_st(obj->trace_data_event + i, tail);
        }
    }

    tail += marshal_uint8_t(obj->is_trace_data_stat, tail);

    tail += marshal_uint8_t((obj->ret == MCD_RET_ACT_NONE) && (obj->is_trace_data_stat != 0), tail);
    if ((obj->ret == MCD_RET_ACT_NONE) && (obj->is_trace_data_stat != 0)) {
        tail += marshal_uint32_t((uint32_t) obj->trace_data_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->trace_data_len; i++) {
            tail += marshal_mcd_trace_data_stat_st(obj->trace_data_stat + i, tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_read_trace_result(const uint8_t *buf, mcd_read_trace_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->num_frames);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->trace_data_len);
        }
    }

    head += unmarshal_uint8_t(head, &obj->is_trace_data_core);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->trace_data_core = (mcd_trace_data_core_st*) malloc(sizeof(*obj->trace_data_core) * len);
                } else {
                    obj->trace_data_core = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_trace_data_core_st(head, obj->trace_data_core + i);
                }
            }
        }
        else {
            obj->trace_data_core = 0;
        }
    }

    head += unmarshal_uint8_t(head, &obj->is_trace_data_event);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->trace_data_event = (mcd_trace_data_event_st*) malloc(sizeof(*obj->trace_data_event) * len);
                } else {
                    obj->trace_data_event = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_trace_data_event_st(head, obj->trace_data_event + i);
                }
            }
        }
        else {
            obj->trace_data_event = 0;
        }
    }

    head += unmarshal_uint8_t(head, &obj->is_trace_data_stat);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->trace_data_stat = (mcd_trace_data_stat_st*) malloc(sizeof(*obj->trace_data_stat) * len);
                } else {
                    obj->trace_data_stat = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_mcd_trace_data_stat_st(head, obj->trace_data_stat + i);
                }
            }
        }
        else {
            obj->trace_data_stat = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_read_trace_result(mcd_read_trace_result *obj)
{
    if (obj->trace_data_core) {
        free(obj->trace_data_core);
    }
    if (obj->trace_data_event) {
        free(obj->trace_data_event);
    }
    if (obj->trace_data_stat) {
        free(obj->trace_data_stat);
    }
}

static uint32_t marshal_mcd_receive_msg_args(const mcd_receive_msg_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_chl_st(&obj->channel, tail);

    tail += marshal_uint32_t(obj->timeout, tail);

    tail += marshal_uint32_t(obj->msg_len, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_receive_msg_args(const uint8_t *buf, mcd_receive_msg_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_mcd_chl_st(head, &obj->channel);

    head += unmarshal_uint32_t(head, &obj->timeout);

    head += unmarshal_uint32_t(head, &obj->msg_len);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_receive_msg_result(const mcd_receive_msg_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t(obj->msg_len, tail);
    }

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint32_t((uint32_t) obj->msg_len, tail);
        for (uint32_t i = 0; i < (uint32_t) obj->msg_len; i++) {
            tail += marshal_uint8_t(obj->msg[i], tail);
        }
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_receive_msg_result(const uint8_t *buf, mcd_receive_msg_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint32_t(head, &obj->msg_len);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            {
                uint32_t len;
                head += unmarshal_uint32_t(head, &len);

                if (len) {
                    obj->msg = (uint8_t*) malloc(sizeof(*obj->msg) * len);
                } else {
                    obj->msg = 0;
                }

                for (uint32_t i = 0; i < len; i++) {
                    head += unmarshal_uint8_t(head, obj->msg + i);
                }
            }
        }
        else {
            obj->msg = 0;
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_receive_msg_result(mcd_receive_msg_result *obj)
{
    if (obj->msg) {
        free(obj->msg);
    }
}

static uint32_t marshal_mcd_register_group_st(const mcd_register_group_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->reg_group_id, tail);

    tail += marshal_uint32_t((uint32_t) MCD_REG_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_REG_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->reg_group_name[i], tail);
    }

    tail += marshal_uint32_t(obj->n_registers, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_register_group_st(const uint8_t *buf, mcd_register_group_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->reg_group_id);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->reg_group_name + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->n_registers);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_register_info_st(const mcd_register_info_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_addr_st(&obj->addr, tail);

    tail += marshal_uint32_t(obj->reg_group_id, tail);

    tail += marshal_uint32_t((uint32_t) MCD_REG_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_REG_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->regname[i], tail);
    }

    tail += marshal_uint32_t(obj->regsize, tail);

    tail += marshal_uint32_t(obj->core_mode_mask_read, tail);

    tail += marshal_uint32_t(obj->core_mode_mask_write, tail);

    tail += marshal_mcd_bool_t(obj->has_side_effects_read, tail);

    tail += marshal_mcd_bool_t(obj->has_side_effects_write, tail);

    tail += marshal_mcd_reg_type_et(obj->reg_type, tail);

    tail += marshal_uint32_t(obj->hw_thread_id, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_register_info_st(const uint8_t *buf, mcd_register_info_st *obj)
{
    const uint8_t *head = buf;

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

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_remove_trig_args(const mcd_remove_trig_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->trig_id, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_remove_trig_args(const uint8_t *buf, mcd_remove_trig_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->trig_id);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_remove_trig_result(const mcd_remove_trig_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_remove_trig_result(const uint8_t *buf, mcd_remove_trig_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_remove_trig_set_args(const mcd_remove_trig_set_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_remove_trig_set_args(const uint8_t *buf, mcd_remove_trig_set_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_remove_trig_set_result(const mcd_remove_trig_set_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_remove_trig_set_result(const uint8_t *buf, mcd_remove_trig_set_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_rpc_trace_data_st(const mcd_rpc_trace_data_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->trace_data_struct_size, tail);

    tail += marshal_mcd_trace_data_core_st(&obj->core_trace, tail);

    tail += marshal_mcd_trace_data_event_st(&obj->event_trace, tail);

    tail += marshal_mcd_trace_data_stat_st(&obj->stat_trace, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_rpc_trace_data_st(const uint8_t *buf, mcd_rpc_trace_data_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->trace_data_struct_size);

    head += unmarshal_mcd_trace_data_core_st(head, &obj->core_trace);

    head += unmarshal_mcd_trace_data_event_st(head, &obj->event_trace);

    head += unmarshal_mcd_trace_data_stat_st(head, &obj->stat_trace);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_rpc_trig_st(const mcd_rpc_trig_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint8_t(obj->is_complex_core, tail);

    tail += marshal_uint8_t(obj->is_complex_core, tail);
    if (obj->is_complex_core) {
        tail += marshal_mcd_trig_complex_core_st(&obj->complex_core, tail);
    }

    tail += marshal_uint8_t(obj->is_simple_core, tail);

    tail += marshal_uint8_t(obj->is_simple_core, tail);
    if (obj->is_simple_core) {
        tail += marshal_mcd_trig_simple_core_st(&obj->simple_core, tail);
    }

    tail += marshal_uint8_t(obj->is_trig_bus, tail);

    tail += marshal_uint8_t(obj->is_trig_bus, tail);
    if (obj->is_trig_bus) {
        tail += marshal_mcd_trig_trig_bus_st(&obj->trig_bus, tail);
    }

    tail += marshal_uint8_t(obj->is_counter, tail);

    tail += marshal_uint8_t(obj->is_counter, tail);
    if (obj->is_counter) {
        tail += marshal_mcd_trig_counter_st(&obj->counter, tail);
    }

    tail += marshal_uint8_t(obj->is_custom, tail);

    tail += marshal_uint8_t(obj->is_custom, tail);
    if (obj->is_custom) {
        tail += marshal_mcd_trig_custom_st(&obj->custom, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_rpc_trig_st(const uint8_t *buf, mcd_rpc_trig_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint8_t(head, &obj->is_complex_core);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_complex_core_st(head, &obj->complex_core);
        }
    }

    head += unmarshal_uint8_t(head, &obj->is_simple_core);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_simple_core_st(head, &obj->simple_core);
        }
    }

    head += unmarshal_uint8_t(head, &obj->is_trig_bus);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_trig_bus_st(head, &obj->trig_bus);
        }
    }

    head += unmarshal_uint8_t(head, &obj->is_counter);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_counter_st(head, &obj->counter);
        }
    }

    head += unmarshal_uint8_t(head, &obj->is_custom);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trig_custom_st(head, &obj->custom);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_rst_args(const mcd_rst_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->rst_class_vector, tail);

    tail += marshal_mcd_bool_t(obj->rst_and_halt, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_rst_args(const uint8_t *buf, mcd_rst_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->rst_class_vector);

    head += unmarshal_mcd_bool_t(head, &obj->rst_and_halt);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_rst_info_st(const mcd_rst_info_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->class_vector, tail);

    tail += marshal_uint32_t((uint32_t) MCD_INFO_STR_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_INFO_STR_LEN; i++) {
        tail += marshal_mcd_char_t(obj->info_str[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_rst_info_st(const uint8_t *buf, mcd_rst_info_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->class_vector);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->info_str + i);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_rst_result(const mcd_rst_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_rst_result(const uint8_t *buf, mcd_rst_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_run_args(const mcd_run_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_bool_t(obj->global, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_run_args(const uint8_t *buf, mcd_run_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_mcd_bool_t(head, &obj->global);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_run_result(const mcd_run_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_run_result(const uint8_t *buf, mcd_run_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_run_until_args(const mcd_run_until_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_bool_t(obj->global, tail);

    tail += marshal_mcd_bool_t(obj->absolute_time, tail);

    tail += marshal_uint64_t(obj->run_until_time, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_run_until_args(const uint8_t *buf, mcd_run_until_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_mcd_bool_t(head, &obj->global);

    head += unmarshal_mcd_bool_t(head, &obj->absolute_time);

    head += unmarshal_uint64_t(head, &obj->run_until_time);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_run_until_result(const mcd_run_until_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_run_until_result(const uint8_t *buf, mcd_run_until_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_send_msg_args(const mcd_send_msg_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_chl_st(&obj->channel, tail);

    tail += marshal_uint32_t(obj->msg_len, tail);

    tail += marshal_uint32_t((uint32_t) obj->msg_len, tail);
    for (uint32_t i = 0; i < (uint32_t) obj->msg_len; i++) {
        tail += marshal_uint8_t(obj->msg[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_send_msg_args(const uint8_t *buf, mcd_send_msg_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_mcd_chl_st(head, &obj->channel);

    head += unmarshal_uint32_t(head, &obj->msg_len);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        if (len) {
            obj->msg = (uint8_t*) malloc(sizeof(*obj->msg) * len);
        } else {
            obj->msg = 0;
        }

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_uint8_t(head, obj->msg + i);
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_send_msg_args(mcd_send_msg_args *obj)
{
    if (obj->msg) {
        free(obj->msg);
    }
}

static uint32_t marshal_mcd_send_msg_result(const mcd_send_msg_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_send_msg_result(const uint8_t *buf, mcd_send_msg_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_server_info_st(const mcd_server_info_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t((uint32_t) MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->server[i], tail);
    }

    tail += marshal_uint32_t((uint32_t) MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->system_instance[i], tail);
    }

    tail += marshal_uint32_t((uint32_t) MCD_UNIQUE_NAME_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_UNIQUE_NAME_LEN; i++) {
        tail += marshal_mcd_char_t(obj->acc_hw[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_server_info_st(const uint8_t *buf, mcd_server_info_st *obj)
{
    const uint8_t *head = buf;

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->server + i);
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

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_set_global_args(const mcd_set_global_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_bool_t(obj->enable, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_set_global_args(const uint8_t *buf, mcd_set_global_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_mcd_bool_t(head, &obj->enable);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_set_global_result(const mcd_set_global_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_set_global_result(const uint8_t *buf, mcd_set_global_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_set_server_config_args(const mcd_set_server_config_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->server_uid, tail);

    tail += marshal_uint32_t(obj->config_string_len, tail);

    tail += marshal_uint32_t((uint32_t) obj->config_string_len, tail);
    for (uint32_t i = 0; i < (uint32_t) obj->config_string_len; i++) {
        tail += marshal_mcd_char_t(obj->config_string[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_set_server_config_args(const uint8_t *buf, mcd_set_server_config_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->server_uid);

    head += unmarshal_uint32_t(head, &obj->config_string_len);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        if (len) {
            obj->config_string = (mcd_char_t*) malloc(sizeof(*obj->config_string) * len);
        } else {
            obj->config_string = 0;
        }

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->config_string + i);
        }
    }

    return (uint32_t) (head - buf);
}

static void free_mcd_set_server_config_args(mcd_set_server_config_args *obj)
{
    if (obj->config_string) {
        free(obj->config_string);
    }
}

static uint32_t marshal_mcd_set_server_config_result(const mcd_set_server_config_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_set_server_config_result(const uint8_t *buf, mcd_set_server_config_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_set_trace_state_args(const mcd_set_trace_state_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_uint32_t(obj->trace_id, tail);

    tail += marshal_mcd_trace_state_st(&obj->state, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_set_trace_state_args(const uint8_t *buf, mcd_set_trace_state_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_uint32_t(head, &obj->trace_id);

    head += unmarshal_mcd_trace_state_st(head, &obj->state);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_set_trace_state_result(const mcd_set_trace_state_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    tail += marshal_uint8_t(obj->ret == MCD_RET_ACT_NONE, tail);
    if (obj->ret == MCD_RET_ACT_NONE) {
        tail += marshal_uint8_t(obj->state_modified, tail);
    }

    tail += marshal_uint8_t((obj->ret == MCD_RET_ACT_NONE) && (obj->state_modified != 0), tail);
    if ((obj->ret == MCD_RET_ACT_NONE) && (obj->state_modified != 0)) {
        tail += marshal_mcd_trace_state_st(&obj->state, tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_set_trace_state_result(const uint8_t *buf, mcd_set_trace_state_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_uint8_t(head, &obj->state_modified);
        }
    }

    {
        uint8_t opt;
        head += unmarshal_uint8_t(head, &opt);
        if (opt) {
            head += unmarshal_mcd_trace_state_st(head, &obj->state);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_step_args(const mcd_step_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_bool_t(obj->global, tail);

    tail += marshal_mcd_core_step_type_et(obj->step_type, tail);

    tail += marshal_uint32_t(obj->n_steps, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_step_args(const uint8_t *buf, mcd_step_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_mcd_bool_t(head, &obj->global);

    head += unmarshal_mcd_core_step_type_et(head, &obj->step_type);

    head += unmarshal_uint32_t(head, &obj->n_steps);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_step_result(const mcd_step_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_step_result(const uint8_t *buf, mcd_step_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_stop_args(const mcd_stop_args *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->core_uid, tail);

    tail += marshal_mcd_bool_t(obj->global, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_stop_args(const uint8_t *buf, mcd_stop_args *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->core_uid);

    head += unmarshal_mcd_bool_t(head, &obj->global);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_stop_result(const mcd_stop_result *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_return_et(obj->ret, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_stop_result(const uint8_t *buf, mcd_stop_result *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_return_et(head, &obj->ret);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trace_data_core_st(const mcd_trace_data_core_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint64_t(obj->timestamp, tail);

    tail += marshal_mcd_trace_marker_et(obj->marker, tail);

    tail += marshal_mcd_trace_cycle_et(obj->cycle, tail);

    tail += marshal_mcd_addr_st(&obj->addr, tail);

    tail += marshal_uint64_t(obj->data, tail);

    tail += marshal_uint8_t(obj->data_width, tail);

    tail += marshal_uint8_t(obj->data_mask, tail);

    tail += marshal_uint16_t(obj->source, tail);

    tail += marshal_uint32_t(obj->aux_info, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trace_data_core_st(const uint8_t *buf, mcd_trace_data_core_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint64_t(head, &obj->timestamp);

    head += unmarshal_mcd_trace_marker_et(head, &obj->marker);

    head += unmarshal_mcd_trace_cycle_et(head, &obj->cycle);

    head += unmarshal_mcd_addr_st(head, &obj->addr);

    head += unmarshal_uint64_t(head, &obj->data);

    head += unmarshal_uint8_t(head, &obj->data_width);

    head += unmarshal_uint8_t(head, &obj->data_mask);

    head += unmarshal_uint16_t(head, &obj->source);

    head += unmarshal_uint32_t(head, &obj->aux_info);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trace_data_event_st(const mcd_trace_data_event_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint64_t(obj->timestamp, tail);

    tail += marshal_mcd_trace_marker_et(obj->marker, tail);

    tail += marshal_uint32_t((uint32_t) 8, tail);
    for (uint32_t i = 0; i < (uint32_t) 8; i++) {
        tail += marshal_uint32_t(obj->data[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trace_data_event_st(const uint8_t *buf, mcd_trace_data_event_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint64_t(head, &obj->timestamp);

    head += unmarshal_mcd_trace_marker_et(head, &obj->marker);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_uint32_t(head, obj->data + i);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trace_data_stat_st(const mcd_trace_data_stat_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint64_t(obj->timestamp, tail);

    tail += marshal_mcd_trace_marker_et(obj->marker, tail);

    tail += marshal_uint32_t((uint32_t) 8, tail);
    for (uint32_t i = 0; i < (uint32_t) 8; i++) {
        tail += marshal_uint64_t(obj->count[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trace_data_stat_st(const uint8_t *buf, mcd_trace_data_stat_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint64_t(head, &obj->timestamp);

    head += unmarshal_mcd_trace_marker_et(head, &obj->marker);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_uint64_t(head, obj->count + i);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trace_info_st(const mcd_trace_info_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->trace_id, tail);

    tail += marshal_uint32_t((uint32_t) MCD_INFO_STR_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_INFO_STR_LEN; i++) {
        tail += marshal_mcd_char_t(obj->trace_name[i], tail);
    }

    tail += marshal_mcd_trace_type_et(obj->trace_type, tail);

    tail += marshal_mcd_trace_format_et(obj->trace_format, tail);

    tail += marshal_mcd_trace_mode_et(obj->trace_modes, tail);

    tail += marshal_mcd_bool_t(obj->trace_no_timestamps, tail);

    tail += marshal_mcd_bool_t(obj->trace_shared, tail);

    tail += marshal_mcd_bool_t(obj->trace_size_is_bytes, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trace_info_st(const uint8_t *buf, mcd_trace_info_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->trace_id);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->trace_name + i);
        }
    }

    head += unmarshal_mcd_trace_type_et(head, &obj->trace_type);

    head += unmarshal_mcd_trace_format_et(head, &obj->trace_format);

    head += unmarshal_mcd_trace_mode_et(head, &obj->trace_modes);

    head += unmarshal_mcd_bool_t(head, &obj->trace_no_timestamps);

    head += unmarshal_mcd_bool_t(head, &obj->trace_shared);

    head += unmarshal_mcd_bool_t(head, &obj->trace_size_is_bytes);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trace_state_st(const mcd_trace_state_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_trace_state_et(obj->state, tail);

    tail += marshal_mcd_trace_mode_et(obj->mode, tail);

    tail += marshal_mcd_bool_t(obj->autoarm, tail);

    tail += marshal_mcd_bool_t(obj->wraparound, tail);

    tail += marshal_uint64_t(obj->frames, tail);

    tail += marshal_uint64_t(obj->count, tail);

    tail += marshal_uint64_t(obj->size, tail);

    tail += marshal_uint64_t(obj->trigger_delay, tail);

    tail += marshal_uint8_t(obj->timestamp_accuracy, tail);

    tail += marshal_mcd_bool_t(obj->timestamp_is_time, tail);

    tail += marshal_uint32_t(obj->options, tail);

    tail += marshal_mcd_bool_t(obj->modified, tail);

    tail += marshal_uint32_t((uint32_t) MCD_INFO_STR_LEN, tail);
    for (uint32_t i = 0; i < (uint32_t) MCD_INFO_STR_LEN; i++) {
        tail += marshal_mcd_char_t(obj->info_str[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trace_state_st(const uint8_t *buf, mcd_trace_state_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_trace_state_et(head, &obj->state);

    head += unmarshal_mcd_trace_mode_et(head, &obj->mode);

    head += unmarshal_mcd_bool_t(head, &obj->autoarm);

    head += unmarshal_mcd_bool_t(head, &obj->wraparound);

    head += unmarshal_uint64_t(head, &obj->frames);

    head += unmarshal_uint64_t(head, &obj->count);

    head += unmarshal_uint64_t(head, &obj->size);

    head += unmarshal_uint64_t(head, &obj->trigger_delay);

    head += unmarshal_uint8_t(head, &obj->timestamp_accuracy);

    head += unmarshal_mcd_bool_t(head, &obj->timestamp_is_time);

    head += unmarshal_uint32_t(head, &obj->options);

    head += unmarshal_mcd_bool_t(head, &obj->modified);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_char_t(head, obj->info_str + i);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trig_complex_core_st(const mcd_trig_complex_core_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

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

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trig_complex_core_st(const uint8_t *buf, mcd_trig_complex_core_st *obj)
{
    const uint8_t *head = buf;

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

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trig_counter_st(const mcd_trig_counter_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->struct_size, tail);

    tail += marshal_mcd_trig_type_et(obj->type, tail);

    tail += marshal_mcd_trig_opt_et(obj->option, tail);

    tail += marshal_mcd_trig_action_et(obj->action, tail);

    tail += marshal_uint32_t(obj->action_param, tail);

    tail += marshal_mcd_bool_t(obj->modified, tail);

    tail += marshal_uint32_t(obj->state_mask, tail);

    tail += marshal_uint64_t(obj->count_value, tail);

    tail += marshal_uint64_t(obj->reload_value, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trig_counter_st(const uint8_t *buf, mcd_trig_counter_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->struct_size);

    head += unmarshal_mcd_trig_type_et(head, &obj->type);

    head += unmarshal_mcd_trig_opt_et(head, &obj->option);

    head += unmarshal_mcd_trig_action_et(head, &obj->action);

    head += unmarshal_uint32_t(head, &obj->action_param);

    head += unmarshal_mcd_bool_t(head, &obj->modified);

    head += unmarshal_uint32_t(head, &obj->state_mask);

    head += unmarshal_uint64_t(head, &obj->count_value);

    head += unmarshal_uint64_t(head, &obj->reload_value);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trig_custom_st(const mcd_trig_custom_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->struct_size, tail);

    tail += marshal_mcd_trig_type_et(obj->type, tail);

    tail += marshal_mcd_trig_opt_et(obj->option, tail);

    tail += marshal_mcd_trig_action_et(obj->action, tail);

    tail += marshal_uint32_t(obj->action_param, tail);

    tail += marshal_mcd_bool_t(obj->modified, tail);

    tail += marshal_uint32_t(obj->state_mask, tail);

    tail += marshal_uint32_t(obj->ctrig_id, tail);

    tail += marshal_uint32_t((uint32_t) 4, tail);
    for (uint32_t i = 0; i < (uint32_t) 4; i++) {
        tail += marshal_uint32_t(obj->ctrig_args[i], tail);
    }

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trig_custom_st(const uint8_t *buf, mcd_trig_custom_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->struct_size);

    head += unmarshal_mcd_trig_type_et(head, &obj->type);

    head += unmarshal_mcd_trig_opt_et(head, &obj->option);

    head += unmarshal_mcd_trig_action_et(head, &obj->action);

    head += unmarshal_uint32_t(head, &obj->action_param);

    head += unmarshal_mcd_bool_t(head, &obj->modified);

    head += unmarshal_uint32_t(head, &obj->state_mask);

    head += unmarshal_uint32_t(head, &obj->ctrig_id);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_uint32_t(head, obj->ctrig_args + i);
        }
    }

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trig_info_st(const mcd_trig_info_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_trig_type_et(obj->type, tail);

    tail += marshal_mcd_trig_opt_et(obj->option, tail);

    tail += marshal_mcd_trig_action_et(obj->action, tail);

    tail += marshal_uint32_t(obj->trig_number, tail);

    tail += marshal_uint32_t(obj->state_number, tail);

    tail += marshal_uint32_t(obj->counter_number, tail);

    tail += marshal_mcd_bool_t(obj->sw_breakpoints, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trig_info_st(const uint8_t *buf, mcd_trig_info_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_trig_type_et(head, &obj->type);

    head += unmarshal_mcd_trig_opt_et(head, &obj->option);

    head += unmarshal_mcd_trig_action_et(head, &obj->action);

    head += unmarshal_uint32_t(head, &obj->trig_number);

    head += unmarshal_uint32_t(head, &obj->state_number);

    head += unmarshal_uint32_t(head, &obj->counter_number);

    head += unmarshal_mcd_bool_t(head, &obj->sw_breakpoints);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trig_set_state_st(const mcd_trig_set_state_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_bool_t(obj->active, tail);

    tail += marshal_uint32_t(obj->state, tail);

    tail += marshal_mcd_bool_t(obj->state_valid, tail);

    tail += marshal_uint32_t(obj->trig_bus, tail);

    tail += marshal_mcd_bool_t(obj->trig_bus_valid, tail);

    tail += marshal_mcd_bool_t(obj->trace, tail);

    tail += marshal_mcd_bool_t(obj->trace_valid, tail);

    tail += marshal_mcd_bool_t(obj->analysis, tail);

    tail += marshal_mcd_bool_t(obj->analysis_valid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trig_set_state_st(const uint8_t *buf, mcd_trig_set_state_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_bool_t(head, &obj->active);

    head += unmarshal_uint32_t(head, &obj->state);

    head += unmarshal_mcd_bool_t(head, &obj->state_valid);

    head += unmarshal_uint32_t(head, &obj->trig_bus);

    head += unmarshal_mcd_bool_t(head, &obj->trig_bus_valid);

    head += unmarshal_mcd_bool_t(head, &obj->trace);

    head += unmarshal_mcd_bool_t(head, &obj->trace_valid);

    head += unmarshal_mcd_bool_t(head, &obj->analysis);

    head += unmarshal_mcd_bool_t(head, &obj->analysis_valid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trig_simple_core_st(const mcd_trig_simple_core_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->struct_size, tail);

    tail += marshal_mcd_trig_type_et(obj->type, tail);

    tail += marshal_mcd_trig_opt_et(obj->option, tail);

    tail += marshal_mcd_trig_action_et(obj->action, tail);

    tail += marshal_uint32_t(obj->action_param, tail);

    tail += marshal_mcd_bool_t(obj->modified, tail);

    tail += marshal_uint32_t(obj->state_mask, tail);

    tail += marshal_mcd_addr_st(&obj->addr_start, tail);

    tail += marshal_uint64_t(obj->addr_range, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trig_simple_core_st(const uint8_t *buf, mcd_trig_simple_core_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->struct_size);

    head += unmarshal_mcd_trig_type_et(head, &obj->type);

    head += unmarshal_mcd_trig_opt_et(head, &obj->option);

    head += unmarshal_mcd_trig_action_et(head, &obj->action);

    head += unmarshal_uint32_t(head, &obj->action_param);

    head += unmarshal_mcd_bool_t(head, &obj->modified);

    head += unmarshal_uint32_t(head, &obj->state_mask);

    head += unmarshal_mcd_addr_st(head, &obj->addr_start);

    head += unmarshal_uint64_t(head, &obj->addr_range);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trig_state_st(const mcd_trig_state_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_bool_t(obj->active, tail);

    tail += marshal_mcd_bool_t(obj->captured, tail);

    tail += marshal_mcd_bool_t(obj->captured_valid, tail);

    tail += marshal_uint64_t(obj->count_value, tail);

    tail += marshal_mcd_bool_t(obj->count_valid, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trig_state_st(const uint8_t *buf, mcd_trig_state_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_bool_t(head, &obj->active);

    head += unmarshal_mcd_bool_t(head, &obj->captured);

    head += unmarshal_mcd_bool_t(head, &obj->captured_valid);

    head += unmarshal_uint64_t(head, &obj->count_value);

    head += unmarshal_mcd_bool_t(head, &obj->count_valid);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_trig_trig_bus_st(const mcd_trig_trig_bus_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t(obj->struct_size, tail);

    tail += marshal_mcd_trig_type_et(obj->type, tail);

    tail += marshal_mcd_trig_opt_et(obj->option, tail);

    tail += marshal_mcd_trig_action_et(obj->action, tail);

    tail += marshal_uint32_t(obj->action_param, tail);

    tail += marshal_mcd_bool_t(obj->modified, tail);

    tail += marshal_uint32_t(obj->state_mask, tail);

    tail += marshal_uint32_t(obj->trig_bus_value, tail);

    tail += marshal_uint32_t(obj->trig_bus_mask, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_trig_trig_bus_st(const uint8_t *buf, mcd_trig_trig_bus_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_uint32_t(head, &obj->struct_size);

    head += unmarshal_mcd_trig_type_et(head, &obj->type);

    head += unmarshal_mcd_trig_opt_et(head, &obj->option);

    head += unmarshal_mcd_trig_action_et(head, &obj->action);

    head += unmarshal_uint32_t(head, &obj->action_param);

    head += unmarshal_mcd_bool_t(head, &obj->modified);

    head += unmarshal_uint32_t(head, &obj->state_mask);

    head += unmarshal_uint32_t(head, &obj->trig_bus_value);

    head += unmarshal_uint32_t(head, &obj->trig_bus_mask);

    return (uint32_t) (head - buf);
}

static uint32_t marshal_mcd_tx_st(const mcd_tx_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_mcd_addr_st(&obj->addr, tail);

    tail += marshal_mcd_tx_access_type_et(obj->access_type, tail);

    tail += marshal_mcd_tx_access_opt_et(obj->options, tail);

    tail += marshal_uint8_t(obj->access_width, tail);

    tail += marshal_uint8_t(obj->core_mode, tail);

    tail += marshal_uint32_t((uint32_t) obj->num_bytes, tail);
    for (uint32_t i = 0; i < (uint32_t) obj->num_bytes; i++) {
        tail += marshal_uint8_t(obj->data[i], tail);
    }

    tail += marshal_uint32_t(obj->num_bytes, tail);

    tail += marshal_uint32_t(obj->num_bytes_ok, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_tx_st(const uint8_t *buf, mcd_tx_st *obj)
{
    const uint8_t *head = buf;

    head += unmarshal_mcd_addr_st(head, &obj->addr);

    head += unmarshal_mcd_tx_access_type_et(head, &obj->access_type);

    head += unmarshal_mcd_tx_access_opt_et(head, &obj->options);

    head += unmarshal_uint8_t(head, &obj->access_width);

    head += unmarshal_uint8_t(head, &obj->core_mode);

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        if (len) {
            obj->data = (uint8_t*) malloc(sizeof(*obj->data) * len);
        } else {
            obj->data = 0;
        }

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_uint8_t(head, obj->data + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->num_bytes);

    head += unmarshal_uint32_t(head, &obj->num_bytes_ok);

    return (uint32_t) (head - buf);
}

static void free_mcd_tx_st(mcd_tx_st *obj)
{
    if (obj->data) {
        free(obj->data);
    }
}

static uint32_t marshal_mcd_txlist_st(const mcd_txlist_st *obj, uint8_t *buf)
{
    uint8_t *tail = buf;

    tail += marshal_uint32_t((uint32_t) obj->num_tx, tail);
    for (uint32_t i = 0; i < (uint32_t) obj->num_tx; i++) {
        tail += marshal_mcd_tx_st(obj->tx + i, tail);
    }

    tail += marshal_uint32_t(obj->num_tx, tail);

    tail += marshal_uint32_t(obj->num_tx_ok, tail);

    return (uint32_t) (tail - buf);
}

static uint32_t unmarshal_mcd_txlist_st(const uint8_t *buf, mcd_txlist_st *obj)
{
    const uint8_t *head = buf;

    {
        uint32_t len;
        head += unmarshal_uint32_t(head, &len);

        if (len) {
            obj->tx = (mcd_tx_st*) malloc(sizeof(*obj->tx) * len);
        } else {
            obj->tx = 0;
        }

        for (uint32_t i = 0; i < len; i++) {
            head += unmarshal_mcd_tx_st(head, obj->tx + i);
        }
    }

    head += unmarshal_uint32_t(head, &obj->num_tx);

    head += unmarshal_uint32_t(head, &obj->num_tx_ok);

    return (uint32_t) (head - buf);
}

static void free_mcd_txlist_st(mcd_txlist_st *obj)
{
    if (obj->tx) {
        for (uint32_t i = 0; i < (uint32_t) obj->num_tx; i++) {
            free_mcd_tx_st(obj->tx + i);
        }
        free(obj->tx);
    }
}

#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif

#endif /* MCD_RPC_H */

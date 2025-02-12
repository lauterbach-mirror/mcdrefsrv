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

#ifndef MCD_RPC_H
#define MCD_RPC_H

#include "mcd_api.h"

/*
 * Remote Procedure Call (RPC) support for the MCD API
 *
 * While the MCD API is well-suited to be implemented with RPC in principle,
 * problems arise when trying to serialize some of the functions arguments.
 *
 * Example: mcd_create_trig_f arguments
 *     const mcd_core_st * core
 *     void              * trig
 *
 * Because pointers are not serializable, the function arguments have to be
 * converted into objects of a serializable type which carry the same
 * information in the specific context.
 *
 * Example: mcd_create_trig_args
 *    uint32_t        core_uid (unique identifier instead of pointer)
 *    mcd_rpc_trig_st trig     (enumeration of all possible trigger types)
 */

/* custom error codes */
const mcd_error_code_et MCD_ERR_RPC_MARSHAL = 0x20000000;
const mcd_error_code_et MCD_ERR_RPC_UNMARSHAL = 0x20000001;

typedef struct {
    uint32_t server_uid;
    const mcd_char_t *host;
    const mcd_char_t *config_string;
    /* auxiliary */
    uint32_t host_len;
    uint32_t config_string_len;
} mcd_rpc_server_st;

typedef struct {
    bool is_complex_core;
    mcd_trig_complex_core_st *complex_core;
    bool is_simple_core;
    mcd_trig_simple_core_st *simple_core;
} mcd_rpc_trig_st;

typedef struct {
    uint32_t core_uid;
    mcd_core_con_info_st *core_con_info;
} mcd_rpc_core_st;

typedef struct {
    bool is_trace_data_core;
    mcd_trace_data_core_st *trace_data_core;
    bool is_trace_data_event;
    mcd_trace_data_event_st *trace_data_event;
    bool is_trace_data_stat;
    mcd_trace_data_stat_st *trace_data_stat;
} mcd_rpc_trace_data_st;

typedef struct {
    const mcd_api_version_st *version_req;
} mcd_initialize_args;

typedef struct {
    mcd_return_et return_status;
    mcd_impl_version_info_st *impl_info;
} mcd_initialize_result;

typedef struct {
    const mcd_char_t *system_key;
    const mcd_char_t *config_string;
    /* auxiliary */
    uint32_t system_key_len;
    uint32_t config_string_len;
} mcd_open_server_args;

typedef struct {
    mcd_return_et return_status;
    mcd_rpc_server_st server;
} mcd_open_server_result;

typedef struct {
    uint32_t server_uid;
} mcd_close_server_args;

typedef struct {
    mcd_return_et return_status;
} mcd_close_server_result;

typedef struct {
    uint32_t start_index;
    uint32_t num_systems;
} mcd_qry_systems_args;

typedef struct {
    mcd_return_et return_status;
    uint32_t *num_systems;
    mcd_core_con_info_st *system_con_info;
    /* auxiliary */
    uint32_t system_con_info_len;
} mcd_qry_systems_result;

typedef struct {
    const mcd_core_con_info_st *system_con_info;
    uint32_t start_index;
    uint32_t num_devices;
} mcd_qry_devices_args;

typedef struct {
    mcd_return_et return_status;
    uint32_t *num_devices;
    mcd_core_con_info_st *device_con_info;
    /* auxiliary */
    uint32_t device_con_info_len;
} mcd_qry_devices_result;

typedef struct {
    const mcd_core_con_info_st *connection_info;
    uint32_t start_index;
    uint32_t num_cores;
} mcd_qry_cores_args;

typedef struct {
    mcd_return_et return_status;
    uint32_t *num_cores;
    mcd_core_con_info_st *core_con_info;
    /* auxiliary */
    uint32_t core_con_info_len;
} mcd_qry_cores_result;

typedef struct {
    const mcd_core_con_info_st *core_con_info;
} mcd_open_core_args;

typedef struct {
    mcd_return_et return_status;
    mcd_rpc_core_st core;
} mcd_open_core_result;

typedef struct {
    uint32_t core_uid;
} mcd_close_core_args;

typedef struct {
    mcd_return_et return_status;
} mcd_close_core_result;

typedef struct {
    uint32_t core_uid;
    /* auxiliary */
    bool has_core_uid;
} mcd_qry_error_info_args;

typedef struct {
    mcd_error_info_st *error_info;
} mcd_qry_error_info_result;

typedef struct {
    uint32_t core_uid;
    uint32_t url_length;
} mcd_qry_device_description_args;

typedef struct {
    mcd_return_et *return_status;
    uint32_t *url_length;
    mcd_char_t *url;
    /* auxiliary */
    uint32_t url_len;
} mcd_qry_device_description_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_max_payload_size_args;

typedef struct {
    mcd_return_et *return_status;
    uint32_t *max_payload;
} mcd_qry_max_payload_size_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_input_handle_args;

typedef struct {
    mcd_return_et *return_status;
    uint32_t *input_handle;
} mcd_qry_input_handle_result;

typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_mem_spaces;
} mcd_qry_mem_spaces_args;

typedef struct {
    mcd_return_et return_status;
    uint32_t *num_mem_spaces;
    mcd_memspace_st *mem_spaces;
    /* auxiliary */
    uint32_t mem_spaces_len;
} mcd_qry_mem_spaces_result;

typedef struct {
    uint32_t core_uid;
    uint32_t mem_space_id;
    uint32_t start_index;
    uint32_t num_mem_blocks;
} mcd_qry_mem_blocks_args;

typedef struct {
    mcd_return_et *return_status;
    uint32_t *num_mem_blocks;
    mcd_memblock_st *mem_blocks;
    /* auxiliary */
    uint32_t *mem_blocks_len;
} mcd_qry_mem_blocks_result;

typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_active_overlays;
} mcd_qry_active_overlays_args;

typedef struct {
    mcd_return_et *return_status;
    uint32_t *num_active_overlays;
    uint32_t *active_overlays;
    /* auxiliary */
    uint32_t active_overlays_len;
} mcd_qry_active_overlays_result;

typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_reg_groups;
} mcd_qry_reg_groups_args;

typedef struct {
    mcd_return_et return_status;
    uint32_t *num_reg_groups;
    mcd_register_group_st *reg_groups;
    /* auxiliary */
    uint32_t reg_groups_len;
} mcd_qry_reg_groups_result;

typedef struct {
    uint32_t core_uid;
    uint32_t reg_group_id;
    uint32_t start_index;
    uint32_t num_regs;
} mcd_qry_reg_map_args;

typedef struct {
    mcd_return_et return_status;
    uint32_t *num_regs;
    mcd_register_info_st *reg_info;
    /* auxiliary */
    uint32_t reg_info_len;
} mcd_qry_reg_map_result;

typedef struct {
    uint32_t core_uid;
    uint32_t compound_reg_id;
    uint32_t start_index;
    uint32_t num_reg_ids;
} mcd_qry_reg_compound_args;

typedef struct {
    mcd_return_et *return_status;
    uint32_t *num_reg_ids;
    uint32_t *reg_id_array;
    /* auxiliary */
    uint32_t reg_id_array_len;
} mcd_qry_reg_compound_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_trig_info_args;

typedef struct {
    mcd_return_et return_status;
    mcd_trig_info_st *trig_info;
} mcd_qry_trig_info_result;

typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_ctrigs;
} mcd_qry_ctrigs_args;

typedef struct {
    mcd_return_et return_status;
    uint32_t *num_ctrigs;
    mcd_ctrig_info_st *ctrig_info;
    /* auxiliary */
    uint32_t ctrig_info_len;
} mcd_qry_ctrigs_result;

typedef struct {
    uint32_t core_uid;
    const mcd_rpc_trig_st *trig;
} mcd_create_trig_args;

typedef struct {
    mcd_return_et return_status;
    mcd_rpc_trig_st *trig;
    uint32_t *trig_id;
    /* auxiliary */
    uint8_t trig_modified;
} mcd_create_trig_result;

typedef struct {
    uint32_t core_uid;
    uint32_t trig_id;
} mcd_qry_trig_args;

typedef struct {
    mcd_return_et return_status;
    mcd_rpc_trig_st *trig;
} mcd_qry_trig_result;

typedef struct {
    uint32_t core_uid;
    uint32_t trig_id;
} mcd_remove_trig_args;

typedef struct {
    mcd_return_et return_status;
} mcd_remove_trig_result;

typedef struct {
    uint32_t core_uid;
    uint32_t trig_id;
} mcd_qry_trig_state_args;

typedef struct {
    mcd_return_et return_status;
    mcd_trig_state_st *trig_state;
} mcd_qry_trig_state_result;

typedef struct {
    uint32_t core_uid;
} mcd_activate_trig_set_args;

typedef struct {
    mcd_return_et return_status;
} mcd_activate_trig_set_result;

typedef struct {
    uint32_t core_uid;
} mcd_remove_trig_set_args;

typedef struct {
    mcd_return_et return_status;
} mcd_remove_trig_set_result;

typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_trigs;
} mcd_qry_trig_set_args;

typedef struct {
    mcd_return_et return_status;
    uint32_t *num_trigs;
    uint32_t *trig_ids;
    /* auxiliary */
    uint32_t trig_ids_len;
} mcd_qry_trig_set_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_trig_set_state_args;

typedef struct {
    mcd_return_et return_status;
    mcd_trig_set_state_st *trig_state;
} mcd_qry_trig_set_state_result;

typedef struct {
    uint32_t core_uid;
    const mcd_txlist_st *txlist;
} mcd_execute_txlist_args;

typedef struct {
    mcd_return_et return_status;
    mcd_txlist_st *txlist;
} mcd_execute_txlist_result;

typedef struct {
    uint32_t core_uid;
    bool global;
} mcd_run_args;

typedef struct {
    mcd_return_et return_status;
} mcd_run_result;

typedef struct {
    uint32_t core_uid;
    bool global;
} mcd_stop_args;

typedef struct {
    mcd_return_et return_status;
} mcd_stop_result;

typedef struct {
    uint32_t core_uid;
    bool global;
    bool absolute_time;
    uint64_t run_until_time;
} mcd_run_until_args;

typedef struct {
    mcd_return_et *return_status;
} mcd_run_until_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_current_time_args;

typedef struct {
    mcd_return_et return_status;
    uint64_t *current_time;
} mcd_qry_current_time_result;

typedef struct {
    uint32_t core_uid;
    bool global;
    mcd_core_step_type_et step_type;
    uint32_t n_steps;
} mcd_step_args;

typedef struct {
    mcd_return_et return_status;
} mcd_step_result;

typedef struct {
    uint32_t core_uid;
    bool enable;
} mcd_set_global_args;

typedef struct {
    mcd_return_et return_status;
} mcd_set_global_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_state_args;

typedef struct {
    mcd_return_et return_status;
    mcd_core_state_st *state;
} mcd_qry_state_result;

typedef struct {
    uint32_t core_uid;
    uint32_t command_string_len;
    mcd_char_t *command_string;
    uint32_t result_string_size;
} mcd_execute_command_args;

typedef struct {
    mcd_return_et *return_status;
    mcd_char_t *result_string;
    /* auxiliary */
    uint32_t result_string_len;
} mcd_execute_command_result;

typedef struct {
    uint32_t core_uid;
} mcd_qry_rst_classes_args;

typedef struct {
    mcd_return_et return_status;
    uint32_t *rst_class_vector;
} mcd_qry_rst_classes_result;

typedef struct {
    uint32_t core_uid;
    uint8_t rst_class;
} mcd_qry_rst_class_info_args;

typedef struct {
    mcd_return_et return_status;
    mcd_rst_info_st *rst_info;
} mcd_qry_rst_class_info_result;

typedef struct {
    uint32_t core_uid;
    uint32_t rst_class_vector;
    bool rst_and_halt;
} mcd_rst_args;

typedef struct {
    mcd_return_et return_status;
} mcd_rst_result;

typedef struct {
    uint32_t core_uid;
    mcd_chl_st channel;
} mcd_chl_open_args;

typedef struct {
    mcd_return_et *return_status;
    mcd_chl_st *channel;
} mcd_chl_open_result;

typedef struct {
    uint32_t core_uid;
    const mcd_chl_st *channel;
    uint32_t msg_len;
    uint8_t *msg;
} mcd_send_msg_args;

typedef struct {
    mcd_return_et *return_status;
} mcd_send_msg_result;

typedef struct {
    uint32_t core_uid;
    const mcd_chl_st *channel;
    uint32_t timeout;
    uint32_t msg_len;
} mcd_receive_msg_args;

typedef struct {
    mcd_return_et *return_status;
    uint32_t *msg_len;
    uint8_t *msg;
} mcd_receive_msg_result;

typedef struct {
    uint32_t core_uid;
    const mcd_chl_st *channel;
} mcd_chl_reset_args;

typedef struct {
    mcd_return_et *return_status;
} mcd_chl_reset_result;

typedef struct {
    uint32_t core_uid;
    const mcd_chl_st *channel;
} mcd_chl_close_args;

typedef struct {
    mcd_return_et *return_status;
} mcd_chl_close_result;

typedef struct {
    uint32_t core_uid;
    uint32_t start_index;
    uint32_t num_traces;
} mcd_qry_traces_args;

typedef struct {
    mcd_return_et *return_status;
    uint32_t *num_traces;
    mcd_trace_info_st *trace_info;
    /* auxiliary */
    uint32_t trace_info_len;
} mcd_qry_traces_result;

typedef struct {
    uint32_t core_uid;
    uint32_t trace_id;
} mcd_qry_trace_state_args;

typedef struct {
    mcd_return_et *return_status;
    mcd_trace_state_st *state;
} mcd_qry_trace_state_result;

typedef struct {
    uint32_t core_uid;
    uint32_t trace_id;
    const mcd_trace_state_st *state;
} mcd_set_trace_state_args;

typedef struct {
    mcd_return_et *return_status;
    uint32_t *num_frames;
    mcd_rpc_trace_data_st *trace_data;
    /* auxiliary */
    uint32_t trace_data_len;
} mcd_read_trace_result;

uint32_t marshal_mcd_exit(char *buf, size_t buf_size);

#define DECLARE_MARSHAL(function)                                     \
    uint32_t marshal_##function##_args(function##_args const *args,   \
                                       char *buf, size_t buf_size);   \
    mcd_return_et unmarshal_##function##_result(                      \
        char const *buf, function##_result *res,                      \
        mcd_error_info_st *error_info);

DECLARE_MARSHAL(mcd_open_server)
DECLARE_MARSHAL(mcd_close_server)
DECLARE_MARSHAL(mcd_qry_systems)
DECLARE_MARSHAL(mcd_qry_devices)
DECLARE_MARSHAL(mcd_qry_cores)
DECLARE_MARSHAL(mcd_open_core)
DECLARE_MARSHAL(mcd_close_core)
DECLARE_MARSHAL(mcd_qry_error_info)
DECLARE_MARSHAL(mcd_qry_mem_spaces)
DECLARE_MARSHAL(mcd_qry_reg_groups)
DECLARE_MARSHAL(mcd_qry_reg_map)
DECLARE_MARSHAL(mcd_execute_txlist)
DECLARE_MARSHAL(mcd_qry_trig_info)
DECLARE_MARSHAL(mcd_qry_ctrigs)
DECLARE_MARSHAL(mcd_create_trig)
DECLARE_MARSHAL(mcd_qry_trig)
DECLARE_MARSHAL(mcd_remove_trig)
DECLARE_MARSHAL(mcd_qry_trig_state)
DECLARE_MARSHAL(mcd_activate_trig_set)
DECLARE_MARSHAL(mcd_remove_trig_set)
DECLARE_MARSHAL(mcd_qry_trig_set)
DECLARE_MARSHAL(mcd_qry_trig_set_state)
DECLARE_MARSHAL(mcd_run)
DECLARE_MARSHAL(mcd_stop)
DECLARE_MARSHAL(mcd_qry_current_time)
DECLARE_MARSHAL(mcd_step)
DECLARE_MARSHAL(mcd_set_global)
DECLARE_MARSHAL(mcd_qry_state)
DECLARE_MARSHAL(mcd_qry_rst_classes)
DECLARE_MARSHAL(mcd_qry_rst_class_info)
DECLARE_MARSHAL(mcd_rst)

#endif /* MCD_RPC_H */
from dataclasses import dataclass
from primitives import *
from modifiers import *
import typing

@dataclass
class mcd_api_version_st:
    v_api_major: uint16_t = modifier()
    v_api_minor: uint16_t = modifier()
    author: mcd_char_t = modifier(fixedLen = "MCD_API_IMP_VENDOR_LEN")

@dataclass
class mcd_impl_version_info_st:
    v_api: mcd_api_version_st = modifier()
    v_imp_major: uint16_t = modifier()
    v_imp_minor: uint16_t = modifier()
    v_imp_build: uint16_t = modifier()
    vendor: mcd_char_t = modifier(fixedLen = "MCD_API_IMP_VENDOR_LEN")
    date: mcd_char_t = modifier(fixedLen = "16")

@dataclass
class mcd_error_info_st:
    return_status: mcd_return_et = modifier()
    error_code: mcd_error_code_et = modifier()
    error_events: mcd_error_event_et = modifier()
    error_str: mcd_char_t = modifier(fixedLen = "MCD_INFO_STR_LEN")

@dataclass
class mcd_server_info_st:
    server: mcd_char_t = modifier(fixedLen = "MCD_UNIQUE_NAME_LEN")
    system_instance: mcd_char_t = modifier(fixedLen = "MCD_UNIQUE_NAME_LEN")
    acc_hw: mcd_char_t = modifier(fixedLen = "MCD_UNIQUE_NAME_LEN")

@dataclass
class mcd_core_con_info_st:
    host: mcd_char_t = modifier(fixedLen = "MCD_HOSTNAME_LEN")
    server_port: uint32_t = modifier()
    server_key: mcd_char_t = modifier(fixedLen = "MCD_KEY_LEN")
    system_key: mcd_char_t = modifier(fixedLen = "MCD_KEY_LEN")
    device_key: mcd_char_t = modifier(fixedLen = "MCD_KEY_LEN")
    system: mcd_char_t = modifier(fixedLen = "MCD_UNIQUE_NAME_LEN")
    system_instance: mcd_char_t = modifier(fixedLen = "MCD_UNIQUE_NAME_LEN")
    acc_hw: mcd_char_t = modifier(fixedLen = "MCD_UNIQUE_NAME_LEN")
    device_type: uint32_t = modifier()
    device: mcd_char_t = modifier(fixedLen = "MCD_UNIQUE_NAME_LEN")
    device_id: uint32_t = modifier()
    core: mcd_char_t = modifier(fixedLen = "MCD_UNIQUE_NAME_LEN")
    core_type: uint32_t = modifier()
    core_id: uint32_t = modifier()

@dataclass
class mcd_core_mode_info_st:
    core_mode: uint8_t = modifier()
    name: mcd_char_t = modifier(fixedLen = "MCD_CORE_MODE_NAME_LEN")

@dataclass
class mcd_addr_st:
    address: uint64_t = modifier()
    mem_space_id: uint32_t = modifier()
    addr_space_id: uint32_t = modifier()
    addr_space_type: mcd_addr_space_type_et = modifier()

@dataclass
class mcd_memspace_st:
    mem_space_id: uint32_t = modifier()
    mem_space_name: mcd_char_t = modifier(fixedLen = "MCD_MEM_SPACE_NAME_LEN")
    mem_type: mcd_mem_type_et = modifier()
    bits_per_mau: uint32_t = modifier()
    invariance: uint8_t = modifier()
    endian: mcd_endian_et = modifier()
    min_addr: uint64_t = modifier()
    max_addr: uint64_t = modifier()
    num_mem_blocks: uint32_t = modifier()
    supported_access_options: mcd_tx_access_opt_et = modifier()
    core_mode_mask_read: uint32_t = modifier()
    core_mode_mask_write: uint32_t = modifier()

@dataclass
class mcd_memblock_st:
    mem_block_id: uint32_t = modifier()
    mem_block_name: mcd_char_t = modifier(fixedLen = "MCD_MEM_BLOCK_NAME_LEN")
    has_children: mcd_bool_t = modifier()
    parent_id: uint32_t = modifier()
    start_addr: uint64_t = modifier()
    end_addr: uint64_t = modifier()
    endian: mcd_endian_et = modifier()
    supported_au_sizes: uint32_t = modifier(fixedLen = "MCD_MEM_AUSIZE_NUM")
    supported_access_options: mcd_tx_access_opt_et = modifier()
    core_mode_mask_read: uint32_t = modifier()
    core_mode_mask_write: uint32_t = modifier()

@dataclass
class mcd_register_group_st:
    reg_group_id: uint32_t = modifier()
    reg_group_name: mcd_char_t = modifier(fixedLen = "MCD_REG_NAME_LEN")
    n_registers: uint32_t = modifier()

@dataclass
class mcd_register_info_st:
    addr: mcd_addr_st = modifier()
    reg_group_id: uint32_t = modifier()
    regname: mcd_char_t = modifier(fixedLen = "MCD_REG_NAME_LEN")
    regsize: uint32_t = modifier()
    core_mode_mask_read: uint32_t = modifier()
    core_mode_mask_write: uint32_t = modifier()
    has_side_effects_read: mcd_bool_t = modifier()
    has_side_effects_write: mcd_bool_t = modifier()
    reg_type: mcd_reg_type_et = modifier()
    hw_thread_id: uint32_t = modifier()

@dataclass
class mcd_trig_info_st:
    _type: mcd_trig_type_et = modifier(rename = "type")
    option: mcd_trig_opt_et = modifier()
    action: mcd_trig_action_et = modifier()
    trig_number: uint32_t = modifier()
    state_number: uint32_t = modifier()
    counter_number: uint32_t = modifier()
    sw_breakpoints: mcd_bool_t = modifier()

@dataclass
class mcd_ctrig_info_st:
    ctrig_id: uint32_t = modifier()
    info_str: mcd_char_t = modifier(fixedLen = "MCD_INFO_STR_LEN")

@dataclass
class mcd_trig_complex_core_st:
    struct_size: uint32_t = modifier()
    _type: mcd_trig_type_et = modifier(rename = "type")
    option: mcd_trig_opt_et = modifier()
    action: mcd_trig_action_et = modifier()
    action_param: uint32_t = modifier()
    modified: mcd_bool_t = modifier()
    state_mask: uint32_t = modifier()
    addr_start: mcd_addr_st = modifier()
    addr_range: uint64_t = modifier()
    data_start: uint64_t = modifier()
    data_range: uint64_t = modifier()
    data_mask: uint64_t = modifier()
    data_size: uint32_t = modifier()
    hw_thread_id: uint32_t = modifier()
    sw_thread_id: uint64_t = modifier()
    core_mode_mask: uint32_t = modifier()

@dataclass
class mcd_trig_simple_core_st:
    struct_size: uint32_t = modifier()
    _type: mcd_trig_type_et = modifier(rename = "type")
    option: mcd_trig_opt_et = modifier()
    action: mcd_trig_action_et = modifier()
    action_param: uint32_t = modifier()
    modified: mcd_bool_t = modifier()
    state_mask: uint32_t = modifier()
    addr_start: mcd_addr_st = modifier()
    addr_range: uint64_t = modifier()

@dataclass
class mcd_trig_trig_bus_st:
    struct_size: uint32_t = modifier()
    _type: mcd_trig_type_et = modifier(rename = "type")
    option: mcd_trig_opt_et = modifier()
    action: mcd_trig_action_et = modifier()
    action_param: uint32_t = modifier()
    modified: mcd_bool_t = modifier()
    state_mask: uint32_t = modifier()
    trig_bus_value: uint32_t = modifier()
    trig_bus_mask: uint32_t = modifier()

@dataclass
class mcd_trig_counter_st:
    struct_size: uint32_t = modifier()
    _type: mcd_trig_type_et = modifier(rename = "type")
    option: mcd_trig_opt_et = modifier()
    action: mcd_trig_action_et = modifier()
    action_param: uint32_t = modifier()
    modified: mcd_bool_t = modifier()
    state_mask: uint32_t = modifier()
    count_value: uint64_t = modifier()
    reload_value: uint64_t = modifier()

@dataclass
class mcd_trig_custom_st:
    struct_size: uint32_t = modifier()
    _type: mcd_trig_type_et = modifier(rename = "type")
    option: mcd_trig_opt_et = modifier()
    action: mcd_trig_action_et = modifier()
    action_param: uint32_t = modifier()
    modified: mcd_bool_t = modifier()
    state_mask: uint32_t = modifier()
    ctrig_id: uint32_t = modifier()
    ctrig_args: uint32_t = modifier(fixedLen = "4")

@dataclass
class mcd_trig_state_st:
    active: mcd_bool_t = modifier()
    captured: mcd_bool_t = modifier()
    captured_valid: mcd_bool_t = modifier()
    count_value: uint64_t = modifier()
    count_valid: mcd_bool_t = modifier()

@dataclass
class mcd_trig_set_state_st:
    active: mcd_bool_t = modifier()
    state: uint32_t = modifier()
    state_valid: mcd_bool_t = modifier()
    trig_bus: uint32_t = modifier()
    trig_bus_valid: mcd_bool_t = modifier()
    trace: mcd_bool_t = modifier()
    trace_valid: mcd_bool_t = modifier()
    analysis: mcd_bool_t = modifier()
    analysis_valid: mcd_bool_t = modifier()

@dataclass
class mcd_tx_st:
    addr: mcd_addr_st = modifier()
    access_type: mcd_tx_access_type_et = modifier()
    options: mcd_tx_access_opt_et = modifier()
    access_width: uint8_t = modifier()
    core_mode: uint8_t = modifier()
    data: uint8_t = modifier(varLen = "obj->num_bytes")
    num_bytes: uint32_t = modifier()
    num_bytes_ok: uint32_t = modifier()

@dataclass
class mcd_txlist_st:
    tx: mcd_tx_st = modifier(varLen = "obj->num_tx")
    num_tx: uint32_t = modifier()
    num_tx_ok: uint32_t = modifier()

@dataclass
class mcd_core_state_st:
    state: mcd_core_state_et = modifier()
    event: mcd_core_event_et = modifier()
    hw_thread_id: uint32_t = modifier()
    trig_id: uint32_t = modifier()
    stop_str: mcd_char_t = modifier(fixedLen = "MCD_INFO_STR_LEN")
    info_str: mcd_char_t = modifier(fixedLen = "MCD_INFO_STR_LEN")

@dataclass
class mcd_rst_info_st:
    class_vector: uint32_t = modifier()
    info_str: mcd_char_t = modifier(fixedLen = "MCD_INFO_STR_LEN")

@dataclass
class mcd_chl_st:
    chl_id: uint32_t = modifier()
    _type: mcd_chl_type_et = modifier(rename = "type")
    attributes: mcd_chl_attributes_et = modifier()
    max_msg_len: uint32_t = modifier()
    msg_buffer_addr: mcd_addr_st = modifier()
    prio: uint8_t = modifier()

@dataclass
class mcd_trace_info_st:
    trace_id: uint32_t = modifier()
    trace_name: mcd_char_t = modifier(fixedLen = "MCD_INFO_STR_LEN")
    trace_type: mcd_trace_type_et = modifier()
    trace_format: mcd_trace_format_et = modifier()
    trace_modes: mcd_trace_mode_et = modifier()
    trace_no_timestamps: mcd_bool_t = modifier()
    trace_shared: mcd_bool_t = modifier()
    trace_size_is_bytes: mcd_bool_t = modifier()

@dataclass
class mcd_trace_state_st:
    state: mcd_trace_state_et = modifier()
    mode: mcd_trace_mode_et = modifier()
    autoarm: mcd_bool_t = modifier()
    wraparound: mcd_bool_t = modifier()
    frames: uint64_t = modifier()
    count: uint64_t = modifier()
    size: uint64_t = modifier()
    trigger_delay: uint64_t = modifier()
    timestamp_accuracy: uint8_t = modifier()
    timestamp_is_time: mcd_bool_t = modifier()
    options: uint32_t = modifier()
    modified: mcd_bool_t = modifier()
    info_str: mcd_char_t = modifier(fixedLen = "MCD_INFO_STR_LEN")

@dataclass
class mcd_trace_data_core_st:
    timestamp: uint64_t = modifier()
    marker: mcd_trace_marker_et = modifier()
    cycle: mcd_trace_cycle_et = modifier()
    addr: mcd_addr_st = modifier()
    data: uint64_t = modifier()
    data_width: uint8_t = modifier()
    data_mask: uint8_t = modifier()
    source: uint16_t = modifier()
    aux_info: uint32_t = modifier()

@dataclass
class mcd_trace_data_event_st:
    timestamp: uint64_t = modifier()
    marker: mcd_trace_marker_et = modifier()
    data: uint32_t = modifier(fixedLen = "8")

@dataclass
class mcd_trace_data_stat_st:
    timestamp: uint64_t = modifier()
    marker: mcd_trace_marker_et = modifier()
    count: uint64_t = modifier(fixedLen = "8")

@dataclass
class mcd_initialize_args:
    version_req: mcd_api_version_st = modifier()

@dataclass
class mcd_initialize_result:
    ret: mcd_return_et = modifier()
    impl_info: mcd_impl_version_info_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_qry_servers_args:
    host_len: uint32_t = modifier()
    host: mcd_char_t = modifier(varLen = "obj->host_len")
    running: mcd_bool_t = modifier()
    start_index: uint32_t = modifier()
    num_servers: uint32_t = modifier()

@dataclass
class mcd_qry_servers_result:
    ret: mcd_return_et = modifier()
    num_servers: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    server_info_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    server_info: mcd_server_info_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                               varLen = "obj->server_info_len")

@dataclass
class mcd_open_server_args:
    system_key_len: uint32_t = modifier()
    system_key: mcd_char_t = modifier(varLen = "obj->system_key_len")
    config_string_len: uint32_t = modifier()
    config_string: mcd_char_t = modifier(varLen = "obj->config_string_len")

# We will use unique identifiers to reference server objects over the network

@dataclass
class mcd_open_server_result:
    ret: mcd_return_et = modifier()
    server_uid: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    host_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    host: mcd_char_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                varLen = "obj->host_len")
    config_string_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    config_string: mcd_char_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                         varLen = "obj->config_string_len")

@dataclass
class mcd_close_server_args:
    server_uid: uint32_t = modifier()

@dataclass
class mcd_close_server_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_set_server_config_args:
    server_uid: uint32_t = modifier()
    config_string_len: uint32_t = modifier()
    config_string: mcd_char_t = modifier(varLen = "obj->config_string_len")

@dataclass
class mcd_set_server_config_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_qry_server_config_args:
    server_uid: uint32_t = modifier()
    max_len: uint32_t = modifier()

# introduce config_string_len to prevent config_string marshalling when
# only max_len is being queried

@dataclass
class mcd_qry_server_config_result:
    ret: mcd_return_et = modifier()
    max_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    config_string_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    config_string: mcd_char_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                         varLen = "obj->config_string_len")

@dataclass
class mcd_qry_systems_args:
    start_index: uint32_t = modifier()
    num_systems: uint32_t = modifier()

@dataclass
class mcd_qry_systems_result:
    ret: mcd_return_et = modifier()
    num_systems: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    system_con_info_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    system_con_info: mcd_core_con_info_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                                     varLen = "obj->system_con_info_len")

@dataclass
class mcd_qry_devices_args:
    system_con_info: mcd_core_con_info_st = modifier()
    start_index: uint32_t = modifier()
    num_devices: uint32_t = modifier()

@dataclass
class mcd_qry_devices_result:
    ret: mcd_return_et = modifier()
    num_devices: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    device_con_info_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    device_con_info: mcd_core_con_info_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                                     varLen = "obj->device_con_info_len")

@dataclass
class mcd_qry_cores_args:
    connection_info: mcd_core_con_info_st = modifier()
    start_index: uint32_t = modifier()
    num_cores: uint32_t = modifier()

@dataclass
class mcd_qry_cores_result:
    ret: mcd_return_et = modifier()
    num_cores: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    core_con_info_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    core_con_info: mcd_core_con_info_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                                   varLen = "obj->core_con_info_len")

@dataclass
class mcd_qry_core_modes_args:
    core_uid: uint32_t = modifier()
    start_index: uint32_t = modifier()
    num_modes: uint32_t = modifier()

@dataclass
class mcd_qry_core_modes_result:
    ret: mcd_return_et = modifier()
    num_modes: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    core_mode_info_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    core_mode_info: mcd_core_mode_info_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                                     varLen = "obj->core_mode_info_len")

@dataclass
class mcd_open_core_args:
    core_con_info: mcd_core_con_info_st = modifier()

@dataclass
class mcd_open_core_result:
    ret: mcd_return_et = modifier()
    core_uid: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    core_con_info: mcd_core_con_info_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_close_core_args:
    core_uid: uint32_t = modifier()

@dataclass
class mcd_close_core_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_qry_error_info_args:
    has_core_uid: mcd_bool_t = modifier()
    core_uid: uint32_t = modifier(optional = "obj->has_core_uid != 0")

@dataclass
class mcd_qry_error_info_result:
    error_info: mcd_error_info_st = modifier()

@dataclass
class mcd_qry_device_description_args:
    core_uid: uint32_t = modifier()
    url_length: uint32_t = modifier()

@dataclass
class mcd_qry_device_description_result:
    ret: mcd_return_et = modifier()
    url_length: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    url_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    url: mcd_char_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                               varLen = "obj->url_len")

@dataclass
class mcd_qry_max_payload_size_args:
    core_uid: uint32_t = modifier()

@dataclass
class mcd_qry_max_payload_size_result:
    ret: mcd_return_et = modifier()
    max_payload: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_qry_input_handle_args:
    core_uid: uint32_t = modifier()

@dataclass
class mcd_qry_input_handle_result:
    ret: mcd_return_et = modifier()
    input_handle: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_qry_mem_spaces_args:
    core_uid: uint32_t = modifier()
    start_index: uint32_t = modifier()
    num_mem_spaces: uint32_t = modifier()

@dataclass
class mcd_qry_mem_spaces_result:
    ret: mcd_return_et = modifier()
    num_mem_spaces: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    mem_spaces_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    mem_spaces: mcd_memspace_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                           varLen = "obj->mem_spaces_len")

@dataclass
class mcd_qry_mem_blocks_args:
    core_uid: uint32_t = modifier()
    mem_space_id: uint32_t = modifier()
    start_index: uint32_t = modifier()
    num_mem_blocks: uint32_t = modifier()

@dataclass
class mcd_qry_mem_blocks_result:
    ret: mcd_return_et = modifier()
    num_mem_blocks: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    mem_blocks_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    mem_blocks: mcd_memblock_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                           varLen = "obj->mem_blocks_len")

@dataclass
class mcd_qry_active_overlays_args:
    core_uid: uint32_t = modifier()
    start_index: uint32_t = modifier()
    num_active_overlays: uint32_t = modifier()

@dataclass
class mcd_qry_active_overlays_result:
    ret: mcd_return_et = modifier()
    num_active_overlays: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    active_overlays_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    active_overlays: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                         varLen = "obj->active_overlays_len")

@dataclass
class mcd_qry_reg_groups_args:
    core_uid: uint32_t = modifier()
    start_index: uint32_t = modifier()
    num_reg_groups: uint32_t = modifier()

@dataclass
class mcd_qry_reg_groups_result:
    ret: mcd_return_et = modifier()
    num_reg_groups: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    reg_groups_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    reg_groups: mcd_register_group_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                                 varLen = "obj->reg_groups_len")

@dataclass
class mcd_qry_reg_map_args:
    core_uid: uint32_t = modifier()
    reg_group_id: uint32_t = modifier()
    start_index: uint32_t = modifier()
    num_regs: uint32_t = modifier()

@dataclass
class mcd_qry_reg_map_result:
    ret: mcd_return_et = modifier()
    num_regs: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    reg_info_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    reg_info: mcd_register_info_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                              varLen = "obj->reg_info_len")

@dataclass
class mcd_qry_reg_compound_args:
    core_uid: uint32_t = modifier()
    compound_reg_id: uint32_t = modifier()
    start_index: uint32_t = modifier()
    num_reg_ids: uint32_t = modifier()

@dataclass
class mcd_qry_reg_compound_result:
    ret: mcd_return_et = modifier()
    num_reg_ids: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    reg_id_array_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    reg_id_array: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                      varLen = "obj->reg_id_array_len")

@dataclass
class mcd_qry_trig_info_args:
    core_uid: uint32_t = modifier()

@dataclass
class mcd_qry_trig_info_result:
    ret: mcd_return_et = modifier()
    trig_info: mcd_trig_info_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_qry_ctrigs_args:
    core_uid: uint32_t = modifier()
    start_index: uint32_t = modifier()
    num_ctrigs: uint32_t = modifier()

@dataclass
class mcd_qry_ctrigs_result:
    ret: mcd_return_et = modifier()
    num_ctrigs: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    ctrig_info_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    ctrig_info: mcd_ctrig_info_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                             varLen = "obj->ctrig_info_len")

# void * arguments cannot be sent over the network in a good manner
# pack all possible types in a structure and indicate in a preceeding field
# whether the type is present

@dataclass
class mcd_rpc_trig_st:
    is_complex_core: uint8_t = modifier()
    complex_core: mcd_trig_complex_core_st = modifier(optional = "obj->is_complex_core")
    is_simple_core: uint8_t = modifier()
    simple_core: mcd_trig_simple_core_st = modifier(optional = "obj->is_simple_core")
    is_trig_bus: uint8_t = modifier()
    trig_bus: mcd_trig_trig_bus_st = modifier(optional = "obj->is_trig_bus")
    is_counter: uint8_t = modifier()
    counter: mcd_trig_counter_st = modifier(optional = "obj->is_counter")
    is_custom: uint8_t = modifier()
    custom: mcd_trig_custom_st = modifier(optional = "obj->is_custom")

@dataclass
class mcd_create_trig_args:
    core_uid: uint32_t = modifier()
    trig: mcd_rpc_trig_st = modifier()

@dataclass
class mcd_create_trig_result:
    ret: mcd_return_et = modifier()
    # prevents redundant information
    trig_modified: uint8_t = modifier("obj->ret == MCD_RET_ACT_NONE")
    trig: mcd_rpc_trig_st = modifier(optional = "(obj->ret == MCD_RET_ACT_NONE) && (obj->trig_modified != 0)")
    trig_id: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_qry_trig_args:
    core_uid: uint32_t = modifier()
    trig_id: uint32_t = modifier()
    # we don't send the max_trig_size parameter over the network
    # struct sizes sizes are might differ between client and server
    # max_trig_size: uint32_t = modifier()

@dataclass
class mcd_qry_trig_result:
    ret: mcd_return_et = modifier()
    trig: mcd_rpc_trig_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_remove_trig_args:
    core_uid: uint32_t = modifier()
    trig_id: uint32_t = modifier()

@dataclass
class mcd_remove_trig_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_qry_trig_state_args:
    core_uid: uint32_t = modifier()
    trig_id: uint32_t = modifier()

@dataclass
class mcd_qry_trig_state_result:
    ret: mcd_return_et = modifier()
    trig_state: mcd_trig_state_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_activate_trig_set_args:
    core_uid: uint32_t = modifier()

@dataclass
class mcd_activate_trig_set_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_remove_trig_set_args:
    core_uid: uint32_t = modifier()

@dataclass
class mcd_remove_trig_set_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_qry_trig_set_args:
    core_uid: uint32_t = modifier()
    start_index: uint32_t = modifier()
    num_trigs: uint32_t = modifier()

@dataclass
class mcd_qry_trig_set_result:
    ret: mcd_return_et = modifier()
    num_trigs: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    trig_ids_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    trig_ids: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                  varLen = "obj->trig_ids_len")

@dataclass
class mcd_qry_trig_set_state_args:
    core_uid: uint32_t = modifier()

@dataclass
class mcd_qry_trig_set_state_result:
    ret: mcd_return_et = modifier()
    trig_state: mcd_trig_set_state_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_execute_txlist_args:
    core_uid: uint32_t = modifier()
    txlist: mcd_txlist_st = modifier()

@dataclass
class mcd_execute_txlist_result:
    ret: mcd_return_et = modifier()
    txlist: mcd_txlist_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_run_args:
    core_uid: uint32_t = modifier()
    _global: mcd_bool_t = modifier(rename = "global")

@dataclass
class mcd_run_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_stop_args:
    core_uid: uint32_t = modifier()
    _global: mcd_bool_t = modifier(rename = "global")

@dataclass
class mcd_stop_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_run_until_args:
    core_uid: uint32_t = modifier()
    _global: mcd_bool_t = modifier(rename = "global")
    absolute_time: mcd_bool_t = modifier()
    run_until_time: uint64_t = modifier()

@dataclass
class mcd_run_until_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_qry_current_time_args:
    core_uid: uint32_t = modifier()

@dataclass
class mcd_qry_current_time_result:
    ret: mcd_return_et = modifier()
    current_time: uint64_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_step_args:
    core_uid: uint32_t = modifier()
    _global: mcd_bool_t = modifier(rename = "global")
    step_type: mcd_core_step_type_et = modifier()
    n_steps: uint32_t = modifier()

@dataclass
class mcd_step_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_set_global_args:
    core_uid: uint32_t = modifier()
    enable: mcd_bool_t = modifier()

@dataclass
class mcd_set_global_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_qry_state_args:
    core_uid: uint32_t = modifier()

@dataclass
class mcd_qry_state_result:
    ret: mcd_return_et = modifier()
    state: mcd_core_state_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_execute_command_args:
    core_uid: uint32_t = modifier()
    command_string_len: uint32_t = modifier()
    command_string: mcd_char_t = modifier(varLen = "obj->command_string_len")
    result_string_size: uint32_t = modifier()

@dataclass
class mcd_execute_command_result:
    ret: mcd_return_et = modifier()
    result_string_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    result_string: mcd_char_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                         varLen = "obj->result_string_len")

@dataclass
class mcd_qry_rst_classes_args:
    core_uid: uint32_t = modifier()

@dataclass
class mcd_qry_rst_classes_result:
    ret: mcd_return_et = modifier()
    rst_class_vector: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_qry_rst_class_info_args:
    core_uid: uint32_t = modifier()
    rst_class: uint8_t = modifier()

@dataclass
class mcd_qry_rst_class_info_result:
    ret: mcd_return_et = modifier()
    rst_info: mcd_rst_info_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_rst_args:
    core_uid: uint32_t = modifier()
    rst_class_vector: uint32_t = modifier()
    rst_and_halt: mcd_bool_t = modifier()

@dataclass
class mcd_rst_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_chl_open_args:
    core_uid: uint32_t = modifier()
    channel: mcd_chl_st = modifier()

@dataclass
class mcd_chl_open_result:
    ret: mcd_return_et = modifier()
    channel: mcd_chl_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_send_msg_args:
    core_uid: uint32_t = modifier()
    channel: mcd_chl_st = modifier()
    msg_len: uint32_t = modifier()
    msg: uint8_t = modifier(varLen = "obj->msg_len")

@dataclass
class mcd_send_msg_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_receive_msg_args:
    core_uid: uint32_t = modifier()
    channel: mcd_chl_st = modifier()
    timeout: uint32_t = modifier()
    msg_len: uint32_t = modifier()

@dataclass
class mcd_receive_msg_result:
    ret: mcd_return_et = modifier()
    msg_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    msg: uint8_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                            varLen = "obj->msg_len")

@dataclass
class mcd_chl_reset_args:
    core_uid: uint32_t = modifier()
    channel: mcd_chl_st = modifier()

@dataclass
class mcd_chl_reset_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_chl_close_args:
    core_uid: uint32_t = modifier()
    channel: mcd_chl_st = modifier()

@dataclass
class mcd_chl_close_result:
    ret: mcd_return_et = modifier()

@dataclass
class mcd_qry_traces_args:
    core_uid: uint32_t = modifier()
    start_index: uint32_t = modifier()
    num_traces: uint32_t = modifier()

@dataclass
class mcd_qry_traces_result:
    ret: mcd_return_et = modifier()
    num_traces: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    trace_info_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    trace_info: mcd_trace_info_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE",
                                             varLen = "obj->trace_info_len")

@dataclass
class mcd_qry_trace_state_args:
    core_uid: uint32_t = modifier()
    trace_id: uint32_t = modifier()

@dataclass
class mcd_qry_trace_state_result:
    ret: mcd_return_et = modifier()
    state: mcd_trace_state_st = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

@dataclass
class mcd_set_trace_state_args:
    core_uid: uint32_t = modifier()
    trace_id: uint32_t = modifier()
    state: mcd_trace_state_st = modifier()

@dataclass
class mcd_set_trace_state_result:
    ret: mcd_return_et = modifier()
    # prevents redundant information
    state_modified: uint8_t = modifier("obj->ret == MCD_RET_ACT_NONE")
    state: mcd_trace_state_st = modifier(optional = "(obj->ret == MCD_RET_ACT_NONE) && (obj->state_modified != 0)")

@dataclass
class mcd_rpc_trace_data_st:
    trace_data_struct_size: uint32_t = modifier()
    core_trace: mcd_trace_data_core_st = modifier()
    event_trace: mcd_trace_data_event_st = modifier()
    stat_trace: mcd_trace_data_stat_st = modifier()

@dataclass
class mcd_read_trace_args:
    core_uid: uint32_t = modifier()
    trace_id: uint32_t = modifier()
    start_index: uint64_t = modifier()
    num_frames: uint32_t = modifier()
    is_trace_data_core: uint8_t = modifier()
    is_trace_data_event: uint8_t = modifier()
    is_trace_data_stat: uint8_t = modifier()

# similar to the different triggers, multiple trace types are supported
# however, when we use the same strategy (put everything in the same structure)
# we would have an overhead (trace *array* is being transmitted)
# therefore, we don't introduce a new structure here

@dataclass
class mcd_read_trace_result:
    ret: mcd_return_et = modifier()
    num_frames: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")
    trace_data_len: uint32_t = modifier(optional = "obj->ret == MCD_RET_ACT_NONE")

    is_trace_data_core: uint8_t = modifier()
    trace_data_core: mcd_trace_data_core_st = modifier(
        optional = "(obj->ret == MCD_RET_ACT_NONE) && (obj->is_trace_data_core != 0)",
        varLen = "obj->trace_data_len")

    is_trace_data_event: uint8_t = modifier()
    trace_data_event: mcd_trace_data_event_st = modifier(
        optional = "(obj->ret == MCD_RET_ACT_NONE) && (obj->is_trace_data_event != 0)",
        varLen = "obj->trace_data_len")

    is_trace_data_stat: uint8_t = modifier()
    trace_data_stat: mcd_trace_data_stat_st = modifier(
        optional = "(obj->ret == MCD_RET_ACT_NONE) && (obj->is_trace_data_stat != 0)",
        varLen = "obj->trace_data_len")

from ctypes import*

MCD_API_IMP_VENDOR_LEN = 32
MCD_HOSTNAME_LEN = 64
MCD_KEY_LEN = 64
MCD_UNIQUE_NAME_LEN = 64
MCD_INFO_STR_LEN = 256
MCD_MEM_SPACE_NAME_LEN = 32
MCD_REG_NAME_LEN = 32

# mcd_return_et
MCD_ERR_NONE = 0
MCD_ERR_GENERAL = 0x0F00

class mcd_api_version_st(Structure):
    _fields_ = [("v_api_major", c_uint16),
                ("v_api_minor", c_uint16),
                ("author", c_char*MCD_API_IMP_VENDOR_LEN)]

class mcd_impl_version_info_st(Structure):
    _fields_ = [("v_api", mcd_api_version_st),
                ("v_imp_major", c_uint16),
                ("v_imp_minor", c_uint16),
                ("v_imp_build", c_uint16),
                ("vendor", c_char*MCD_API_IMP_VENDOR_LEN),
                ("date", c_char*16)]
    
class mcd_server_st(Structure):
    _fields_ = [("instance", c_void_p),
                ("host", c_char_p),
                ("config_string", c_char_p),]

class mcd_core_con_info_st(Structure):
    _fields_ = [("host", c_char*MCD_HOSTNAME_LEN),
                ("server_port", c_uint32),
                ("server_key", c_char*MCD_KEY_LEN),
                ("system_key", c_char*MCD_KEY_LEN),
                ("device_key", c_char*MCD_KEY_LEN),
                ("system", c_char*MCD_UNIQUE_NAME_LEN),
                ("system_instance", c_char*MCD_UNIQUE_NAME_LEN),
                ("acc_hw", c_char*MCD_UNIQUE_NAME_LEN),
                ("device_type", c_uint32),
                ("device", c_char*MCD_UNIQUE_NAME_LEN),
                ("device_id", c_uint32),
                ("core", c_char*MCD_UNIQUE_NAME_LEN),
                ("core_type", c_uint32),
                ("core_id", c_uint32),]

class mcd_core_st(Structure):
    _fields_ = [("instance", c_void_p),
                ("core_con_info", POINTER(mcd_core_con_info_st)),]

class mcd_rst_info_st(Structure):
    _fields_ = [("class_vector", c_uint32),
                ("info_str", c_char*MCD_INFO_STR_LEN),]

class mcd_trig_info_st(Structure):
    _fields_ = [("type", c_uint32),
                ("option", c_uint32),
                ("action", c_uint32),
                ("trig_number", c_uint32),
                ("state_number", c_uint32),
                ("counter_number", c_uint32),
                ("sw_breakpoints", c_bool),]

class mcd_memspace_st(Structure):
    _fields_ = [("mem_space_id", c_uint32),
                ("mem_space_name", c_char*MCD_MEM_SPACE_NAME_LEN),
                ("mem_type", c_uint32),
                ("bits_per_mau", c_uint32),
                ("invariance", c_uint8),
                ("endian", c_uint32),
                ("min_addr", c_uint64),
                ("max_addr", c_uint64),
                ("num_mem_blocks", c_uint32),
                ("supported_access_options", c_uint32),
                ("core_mode_mask_read", c_uint32),
                ("core_mode_mask_write", c_uint32),]

class mcd_register_group_st(Structure):
    _fields_ = [("reg_group_id", c_uint32),
                ("reg_group_name", c_char*MCD_REG_NAME_LEN),
                ("n_registers", c_uint32),]

class mcd_addr_st(Structure):
    _fields_ = [("address", c_uint64),
                ("mem_space_id", c_uint32),
                ("addr_space_id", c_uint32),
                ("addr_space_type", c_uint32),]

class mcd_register_info_st(Structure):
    _fields_ = [("addr", mcd_addr_st),
                ("reg_group_id", c_uint32),
                ("regname", c_char*MCD_REG_NAME_LEN),
                ("regsize", c_uint32),
                ("core_mode_mask_read", c_uint32),
                ("core_mode_mask_write", c_uint32),
                ("has_side_effects_read", c_bool),
                ("has_side_effects_write", c_bool),
                ("reg_type", c_uint32),
                ("hw_thread_id", c_uint32),]

class mcd_core_state_st(Structure):
    _fields_ = [("state", c_uint32),
                ("event", c_uint32),
                ("hw_thread_id", c_uint32),
                ("trig_id", c_uint32),
                ("stop_str", c_char*MCD_INFO_STR_LEN),
                ("info_str", c_char*MCD_INFO_STR_LEN),]

class mcd_error_info_st(Structure):
    _fields_ = [("return_status", c_uint32),
                ("error_code", c_uint32),
                ("error_events", c_uint32),
                ("error_str", c_char*MCD_INFO_STR_LEN),]

class mcd_tx_st(Structure):
    _fields_ = [("addr", mcd_addr_st),
                ("access_type", c_uint32),
                ("options", c_uint32),
                ("access_width", c_uint8),
                ("core_mode", c_uint8),
                ("data", POINTER(c_uint8)),
                ("num_bytes", c_uint32),
                ("num_bytes_ok", c_uint32),]

class mcd_txlist_st(Structure):
    _fields_ = [("tx", POINTER(mcd_tx_st)),
                ("num_tx", c_uint32),
                ("num_tx_ok", c_uint32),]

class mcd_trig_simple_core_st(Structure):
    _fields_ = [("struct_size", c_uint32),
                ("type", c_uint32),
                ("option", c_uint32),
                ("action", c_uint32),
                ("action_param", c_uint32),
                ("modified", c_bool),
                ("state_mask", c_uint32),
                ("addr_start", mcd_addr_st),
                ("addr_range", c_uint64),]

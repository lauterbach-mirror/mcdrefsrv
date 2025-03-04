import os
from ctypes import *

# Definitions of Constants

MCD_API_VER_MAJOR = 1
MCD_API_VER_MINOR = 1
MCD_API_VER_AUTHOR = "SPRINT"
MCD_HOSTNAME_LEN = 64
MCD_REG_NAME_LEN = 32
MCD_MEM_SPACE_NAME_LEN = 32
MCD_MEM_BLOCK_NAME_LEN = 32
MCD_MEM_BLOCK_NOPARENT = 0
MCD_MEM_AUSIZE_NUM = 8
MCD_INFO_STR_LEN = 256
MCD_KEY_LEN = 64
MCD_UNIQUE_NAME_LEN = 64
MCD_MAX_TRIGS = 32
MCD_API_IMP_VENDOR_LEN = 32
MCD_CHL_NUM_MAX = 32
MCD_CHL_LOWEST_PRIORITY = 15
MCD_TX_NUM_MAX = 64
MCD_GUARANTEED_MIN_PAYLOAD = 16384
MCD_CORE_MODE_NAME_LEN = 32

# Definition of Enumerations

class mcd_return_et:
    MCD_RET_ACT_NONE         = 0x00000000
    MCD_RET_ACT_AGAIN        = 0x00000001
    MCD_RET_ACT_HANDLE_EVENT = 0x00000002
    MCD_RET_ACT_HANDLE_ERROR = 0x00000003
    MCD_RET_ACT_RESERVED_LO  = 0x00000004
    MCD_RET_ACT_RESERVED_HI  = 0x00008000
    MCD_RET_ACT_CUSTOM_LO    = 0x00010000
    MCD_RET_ACT_CUSTOM_HI    = 0x40000000

class mcd_error_code_et:
    MCD_ERR_NONE                        = 0
    MCD_ERR_FN_UNIMPLEMENTED            = 0x0100
    MCD_ERR_USAGE                       = 0x0101
    MCD_ERR_PARAM                       = 0x0102
    MCD_ERR_CONNECTION                  = 0x0200
    MCD_ERR_TIMED_OUT                   = 0x0201
    MCD_ERR_GENERAL                     = 0x0F00
    MCD_ERR_COULD_NOT_START_SERVER      = 0x1100
    MCD_ERR_SERVER_LOCKED               = 0x1101
    MCD_ERR_NO_MEM_SPACES               = 0x1401
    MCD_ERR_NO_MEM_BLOCKS               = 0x1402
    MCD_ERR_MEM_SPACE_ID                = 0x1410
    MCD_ERR_NO_REG_GROUPS               = 0x1440
    MCD_ERR_REG_GROUP_ID                = 0x1441
    MCD_ERR_REG_NOT_COMPOUND            = 0x1442
    MCD_ERR_OVERLAYS                    = 0x1500
    MCD_ERR_DEVICE_ACCESS               = 0x1900
    MCD_ERR_DEVICE_LOCKED               = 0x1901
    MCD_ERR_TXLIST_READ                 = 0x2100
    MCD_ERR_TXLIST_WRITE                = 0x2101
    MCD_ERR_TXLIST_TX                   = 0x2102
    MCD_ERR_CHL_TYPE_NOT_SUPPORTED      = 0x3100
    MCD_ERR_CHL_TARGET_NOT_SUPPORTED    = 0x3101
    MCD_ERR_CHL_SETUP                   = 0x3102
    MCD_ERR_CHL_MESSAGE_FAILED          = 0x3140
    MCD_ERR_TRIG_CREATE                 = 0x3200
    MCD_ERR_TRIG_ACCESS                 = 0x3201
    MCD_ERR_CUSTOM_LO                   = 0x10000000
    MCD_ERR_CUSTOM_HI                   = 0x7FFFFFFF

class mcd_error_event_et:
    MCD_ERR_EVT_NONE         = 0x00000000
    MCD_ERR_EVT_RESET        = 0x00000001
    MCD_ERR_EVT_PWRDN        = 0x00000002
    MCD_ERR_EVT_HWFAILURE    = 0x00000004
    MCD_ERR_EVT_RESERVED_LO  = 0x00000008
    MCD_ERR_EVT_RESERVED_HI  = 0x00008000
    MCD_ERR_EVT_CUSTOM_LO    = 0x00010000
    MCD_ERR_EVT_CUSTOM_HI    = 0x40000000

class mcd_addr_space_type_et:
    MCD_NOTUSED_ID    = 0
    MCD_OVERLAY_ID    = 1
    MCD_MEMBANK_ID    = 2
    MCD_PROCESS_ID    = 3
    MCD_HW_THREAD_ID  = 4

class mcd_mem_type_et:
    MCD_MEM_SPACE_DEFAULT     = 0x00000000
    MCD_MEM_SPACE_IS_REGISTERS= 0x00000001
    MCD_MEM_SPACE_IS_PROGRAM  = 0x00000002
    MCD_MEM_SPACE_IS_VIRTUAL  = 0x00000004
    MCD_MEM_SPACE_IS_CACHE    = 0x00000008
    MCD_MEM_SPACE_IS_PHYSICAL = 0x00000010
    MCD_MEM_SPACE_IS_LOGICAL  = 0x00000020
    MCD_MEM_SPACE_RESERVED_LO = 0x00000040
    MCD_MEM_SPACE_RESERVED_HI = 0x00008000
    MCD_MEM_SPACE_CUSTOM_LO   = 0x00010000
    MCD_MEM_SPACE_CUSTOM_HI   = 0x40000000

class mcd_endian_et:
    MCD_ENDIAN_DEFAULT =  0
    MCD_ENDIAN_LITTLE  =  1
    MCD_ENDIAN_BIG     =  2

class mcd_reg_type_et:
    MCD_REG_TYPE_SIMPLE   = 0
    MCD_REG_TYPE_COMPOUND = 1
    MCD_REG_TYPE_PARTIAL  = 2

class mcd_trig_type_et:
    MCD_TRIG_TYPE_UNDEFINED = 0x00000000
    MCD_TRIG_TYPE_IP        = 0x00000001
    MCD_TRIG_TYPE_READ      = 0x00000002
    MCD_TRIG_TYPE_WRITE     = 0x00000004
    MCD_TRIG_TYPE_RW        = 0x00000008
    MCD_TRIG_TYPE_NOCYCLE   = 0x00000010
    MCD_TRIG_TYPE_TRIG_BUS  = 0x00000020
    MCD_TRIG_TYPE_COUNTER   = 0x00000040
    MCD_TRIG_TYPE_CUSTOM    = 0x00000080
    MCD_TRIG_TYPE_CUSTOM_LO = 0x00010000
    MCD_TRIG_TYPE_CUSTOM_HI = 0x40000000

class mcd_trig_opt_et:
    MCD_TRIG_OPT_DEFAULT                = 0x00000000
    MCD_TRIG_OPT_IMPL_HARDWARE          = 0x00000001
    MCD_TRIG_OPT_IMPL_SOFTWARE          = 0x00000002
    MCD_TRIG_OPT_OUT_OF_RANGE           = 0x00000004
    MCD_TRIG_OPT_DATA_IS_CONDITION      = 0x00000008
    MCD_TRIG_OPT_DATASIZE_IS_CONDITION  = 0x00000010
    MCD_TRIG_OPT_NOT_DATA               = 0x00000020
    MCD_TRIG_OPT_SIGNED_DATA            = 0x00000040
    MCD_TRIG_OPT_HW_THREAD_IS_CONDITION = 0x00000080
    MCD_TRIG_OPT_NOT_HW_THREAD          = 0x00000100
    MCD_TRIG_OPT_SW_THREAD_IS_CONDITION = 0x00000200
    MCD_TRIG_OPT_NOT_SW_THREAD          = 0x00000400
    MCD_TRIG_OPT_DATA_MUST_CHANGE       = 0x00000800
    MCD_TRIG_OPT_CORE_MODE_IS_CONDITION = 0x00020000
    MCD_TRIG_OPT_STATE_IS_CONDITION     = 0x00040000
    MCD_TRIG_OPT_NOT                    = 0x00080000
    MCD_TRIG_OPT_CUSTOM_LO              = 0x00100000
    MCD_TRIG_OPT_CUSTOM_HI              = 0x40000000

class mcd_trig_action_et:
    MCD_TRIG_ACTION_DEFAULT               = 0x00000000
    MCD_TRIG_ACTION_DBG_DEBUG             = 0x00000001
    MCD_TRIG_ACTION_DBG_GLOBAL            = 0x00000002
    MCD_TRIG_ACTION_DBG_MONITOR           = 0x00000004
    MCD_TRIG_ACTION_TRIG_BUS_EVENT        = 0x00000010
    MCD_TRIG_ACTION_TRIG_BUS_SET          = 0x00000020
    MCD_TRIG_ACTION_TRIG_BUS_CLEAR        = 0x00000040
    MCD_TRIG_ACTION_TRACE_QUALIFY         = 0x00000100
    MCD_TRIG_ACTION_TRACE_QUALIFY_PROGRAM = 0x00000200
    MCD_TRIG_ACTION_TRACE_QUALIFY_DATA    = 0x00000400
    MCD_TRIG_ACTION_TRACE_START           = 0x00000800
    MCD_TRIG_ACTION_TRACE_STOP            = 0x00001000
    MCD_TRIG_ACTION_TRACE_TRIGGER         = 0x00002000
    MCD_TRIG_ACTION_ANA_START_PERFM       = 0x00010000
    MCD_TRIG_ACTION_ANA_STOP_PERFM        = 0x00020000
    MCD_TRIG_ACTION_STATE_CHANGE          = 0x00040000
    MCD_TRIG_ACTION_COUNT_QUALIFY         = 0x00080000
    MCD_TRIG_ACTION_COUNT_START           = 0x00100000
    MCD_TRIG_ACTION_COUNT_STOP            = 0x00200000
    MCD_TRIG_ACTION_COUNT_RESTART         = 0x00400000
    MCD_TRIG_ACTION_CUSTOM_LO             = 0x01000000
    MCD_TRIG_ACTION_CUSTOM_HI             = 0x40000000

class mcd_tx_access_type_et:
    MCD_TX_AT_R    = 0x00000001
    MCD_TX_AT_W    = 0x00000002
    MCD_TX_AT_RW   = 0x00000003
    MCD_TX_AT_WR   = 0x00000004

class mcd_tx_access_opt_et:
    MCD_TX_OPT_DEFAULT            = 0x00000000
    MCD_TX_OPT_SIDE_EFFECTS       = 0x00000001
    MCD_TX_OPT_NO_SIDE_EFFECTS    = 0x00000002
    MCD_TX_OPT_BURST_ACCESSES     = 0x00000004
    MCD_TX_OPT_NO_BURST_ACCESSES  = 0x00000008
    MCD_TX_OPT_ALTERNATE_PATH     = 0x00000010
    MCD_TX_OPT_PRIORITY_ACCESS    = 0x00000020
    MCD_TX_OPT_DCACHE_WRITE_THRU  = 0x00000040
    MCD_TX_OPT_CACHE_BYPASS       = 0x00000080
    MCD_TX_OPT_NOINCREMENT        = 0x00000100
    MCD_TX_OPT_ATOMIC_WITH_NEXT   = 0x00000200
    MCD_TX_OPT_RESERVED_LO        = 0x00000400
    MCD_TX_OPT_RESERVED_HI        = 0x00008000
    MCD_TX_OPT_CUSTOM_LO          = 0x00010000
    MCD_TX_OPT_CUSTOM_HI          = 0x40000000

class mcd_core_step_type_et:
	MCD_CORE_STEP_TYPE_CYCLES      = 0x00000001
	MCD_CORE_STEP_TYPE_INSTR       = 0x00000002
	MCD_CORE_STEP_TYPE_RESERVED_LO = 0x00000004
	MCD_CORE_STEP_TYPE_RESERVED_HI = 0x000000FF
	MCD_CORE_STEP_TYPE_CUSTOM_LO   = 0x00000100
	MCD_CORE_STEP_TYPE_CUSTOM_HI   = 0x00000F00
	MCD_CORE_STEP_TYPE_MAX_TYPES   = 0x7FFFFFFF

class mcd_core_state_et:
	MCD_CORE_STATE_UNKNOWN    = 0x00000000
	MCD_CORE_STATE_RUNNING    = 0x00000001
	MCD_CORE_STATE_HALTED     = 0x00000002
	MCD_CORE_STATE_DEBUG      = 0x00000003
	MCD_CORE_STATE_CUSTOM_LO  = 0x00000100
	MCD_CORE_STATE_CUSTOM_HI  = 0x00000800
	MCD_CORE_STATE_MAX_STATES = 0x7FFFFFFF

class mcd_core_event_et:
	MCD_CORE_EVENT_NONE            = 0x00000000
	MCD_CORE_EVENT_MEMORY_CHANGE   = 0x00000001
	MCD_CORE_EVENT_REGISTER_CHANGE = 0x00000002
	MCD_CORE_EVENT_TRACE_CHANGE    = 0x00000004
	MCD_CORE_EVENT_TRIGGER_CHANGE  = 0x00000008
	MCD_CORE_EVENT_STOPPED         = 0x00000010
	MCD_CORE_EVENT_CHL_PENDING     = 0x00000020
	MCD_CORE_EVENT_CUSTOM_LO       = 0x00010000
	MCD_CORE_EVENT_CUSTOM_HI       = 0x40000000

class mcd_chl_type_et:
	MCD_CHL_TYPE_COMMON    = 0x00000001
	MCD_CHL_TYPE_CONFIG    = 0x00000002
	MCD_CHL_TYPE_APPLI     = 0x00000003
	MCD_CHL_TYPE_CUSTOM_LO = 0x00000100
	MCD_CHL_TYPE_CUSTOM_HI = 0x00000F00

class mcd_chl_attributes_et:
	MCD_CHL_AT_RCV           = 0x00000001
	MCD_CHL_AT_SND           = 0x00000002
	MCD_CHL_AT_MEM_MAPPED    = 0x00000040
	MCD_CHL_AT_HAS_PRIO      = 0x00000800

class mcd_trace_type_et:
	MCD_TRACE_TYPE_UNKNOWN    = 0x00000000
	MCD_TRACE_TYPE_CORE       = 0x00000001
	MCD_TRACE_TYPE_BUS        = 0x00000002
	MCD_TRACE_TYPE_EVENT      = 0x00000004
	MCD_TRACE_TYPE_STAT       = 0x00000008
	MCD_TRACE_TYPE_CUSTOM_LO  = 0x00000100
	MCD_TRACE_TYPE_CUSTOM_HI  = 0x40000000

class mcd_trace_format_et:
	MCD_TRACE_FORMAT_UNKNOWN              = 0x00000000
	MCD_TRACE_FORMAT_CORE_FETCH           = 0x00000001
	MCD_TRACE_FORMAT_CORE_EXECUTE         = 0x00000002
	MCD_TRACE_FORMAT_CORE_FLOW_ICOUNT     = 0x00000003
	MCD_TRACE_FORMAT_CORE_FLOW_BCOUNT     = 0x00000004
	MCD_TRACE_FORMAT_CORE_FLOW_IPREDICATE = 0x00000005
	MCD_TRACE_FORMAT_EVENT                = 0x00000010
	MCD_TRACE_FORMAT_STAT                 = 0x00000020
	MCD_TRACE_FORMAT_CUSTOM_LO            = 0x00000100
	MCD_TRACE_FORMAT_CUSTOM_HI            = 0x7FFFFFFF

class mcd_trace_mode_et:
	MCD_TRACE_MODE_NOCHANGE   = 0x00000000
	MCD_TRACE_MODE_FIFO       = 0x00000001
	MCD_TRACE_MODE_STACK      = 0x00000002
	MCD_TRACE_MODE_LEACH      = 0x00000004
	MCD_TRACE_MODE_PIPE       = 0x00000008
	MCD_TRACE_MODE_CUSTOM_LO  = 0x00000100
	MCD_TRACE_MODE_CUSTOM_HI  = 0x40000000

class mcd_trace_state_et:
	MCD_TRACE_STATE_NOCHANGE   = 0x00000000
	MCD_TRACE_STATE_DISABLE    = 0x00000001
	MCD_TRACE_STATE_OFF        = 0x00000002
	MCD_TRACE_STATE_ARM        = 0x00000003
	MCD_TRACE_STATE_TRIGGER    = 0x00000004
	MCD_TRACE_STATE_STOP       = 0x00000005
	MCD_TRACE_STATE_INIT       = 0x00000010
	MCD_TRACE_STATE_CUSTOM_LO  = 0x00000100
	MCD_TRACE_STATE_CUSTOM_HI  = 0x7FFFFFFF

class mcd_trace_marker_et:
	MCD_TRACE_MARKER_NONE      = 0x00000000
	MCD_TRACE_MARKER_RUN       = 0x00000001
	MCD_TRACE_MARKER_DEBUG     = 0x00000002
	MCD_TRACE_MARKER_START     = 0x00000004
	MCD_TRACE_MARKER_STOP      = 0x00000008
	MCD_TRACE_MARKER_ERROR     = 0x00000010
	MCD_TRACE_MARKER_GAP       = 0x00000020
	MCD_TRACE_MARKER_CUSTOM_LO = 0x00000100
	MCD_TRACE_MARKER_CUSTOM_HI = 0x40000000

class mcd_trace_cycle_et:
	MCD_TRACE_CYCLE_UNKNOWN    = 0x00000000
	MCD_TRACE_CYCLE_NONE       = 0x00000001
	MCD_TRACE_CYCLE_EXECUTE    = 0x00000002
	MCD_TRACE_CYCLE_NOTEXECUTE = 0x00000003
	MCD_TRACE_CYCLE_FETCH      = 0x00000004
	MCD_TRACE_CYCLE_READ       = 0x00000005
	MCD_TRACE_CYCLE_WRITE      = 0x00000006
	MCD_TRACE_CYCLE_OWNERSHIP  = 0x00000007
	MCD_TRACE_CYCLE_CUSTOM_LO  = 0x00000100
	MCD_TRACE_CYCLE_CUSTOM_HI  = 0x7FFFFFFF


# Definition of Structures

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
    
class mcd_error_info_st(Structure):
    _fields_ = [("return_status", c_uint32),
                ("error_code", c_uint32),
                ("error_events", c_uint32),
                ("error_str", c_char*MCD_INFO_STR_LEN),]

class mcd_server_info_st(Structure):
    _fields_ = [("server", c_char*MCD_UNIQUE_NAME_LEN),
	             ("system_instance", c_char*MCD_UNIQUE_NAME_LEN),
	             ("acc_hw", c_char*MCD_UNIQUE_NAME_LEN),]

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

class mcd_core_mode_info_st(Structure):
	_fields_ = [("core_mode", c_uint8),
                ("name", c_char*MCD_CORE_MODE_NAME_LEN),]

class mcd_addr_st(Structure):
    _fields_ = [("address", c_uint64),
                ("mem_space_id", c_uint32),
                ("addr_space_id", c_uint32),
                ("addr_space_type", c_uint32),]

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

class mcd_memblock_st(Structure):
    _fields_ = [("mem_block_id", c_uint32),
                ("mem_block_name", c_char),
                ("has_children", c_uint32),
                ("parent_id", c_uint32),
                ("start_addr", c_uint64),
                ("end_addr", c_uint64),
                ("endian", c_uint32),
                ("supported_au_sizes", c_uint32),
                ("supported_access_options", c_uint32),
                ("core_mode_mask_read", c_uint32),
                ("core_mode_mask_write", c_uint32),]

class mcd_register_group_st(Structure):
    _fields_ = [("reg_group_id", c_uint32),
                ("reg_group_name", c_char*MCD_REG_NAME_LEN),
                ("n_registers", c_uint32),]

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

class mcd_trig_info_st(Structure):
    _fields_ = [("type", c_uint32),
                ("option", c_uint32),
                ("action", c_uint32),
                ("trig_number", c_uint32),
                ("state_number", c_uint32),
                ("counter_number", c_uint32),
                ("sw_breakpoints", c_bool),]

class mcd_ctrig_info_st(Structure):
    _fields_ = [("ctrig_id", c_uint32),
                ("info_str", c_char*MCD_INFO_STR_LEN),]

class mcd_trig_complex_core_st(Structure):
    _fields_ = [("struct_size", c_uint32),
                ("type", c_uint32),
                ("option", c_uint32),
                ("action", c_uint32),
                ("action_param", c_uint32),
                ("modified", c_bool),
                ("state_mask", c_uint32),
                ("addr_start", mcd_addr_st),
                ("addr_range", c_uint64),
                ("data_start", c_uint64),
                ("data_range", c_uint64),
                ("data_mask", c_uint64),
                ("data_size", c_uint32),
                ("hw_thread_id", c_uint32),
                ("sw_thread_id", c_uint64),
                ("core_mode_mask", c_uint32),]

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

class mcd_trig_trig_bus_st(Structure):
    _fields_ = [("struct_size", c_uint32),
                ("type", c_uint32),
                ("option", c_uint32),
                ("action", c_uint32),
                ("action_param", c_uint32),
                ("modified", c_bool),
                ("state_mask", c_uint32),
                ("trig_bus_value", c_uint32),
                ("trig_bus_mask", c_uint32),]

class mcd_trig_counter_st(Structure):
    _fields_ = [("struct_size", c_uint32),
                ("type", c_uint32),
                ("option", c_uint32),
                ("action", c_uint32),
                ("action_param", c_uint32),
                ("modified", c_uint32),
                ("state_mask", c_uint32),
                ("count_value", c_uint32),
                ("reload_value", c_uint64),]

class mcd_trig_custom_st(Structure):
    _fields_ = [("struct_size", c_uint32),
                ("type", c_uint32),
                ("option", c_uint32),
                ("action", c_uint32),
                ("action_param", c_uint32),
                ("modified", c_bool),
                ("state_mask", c_uint32),
                ("ctrig_id", c_uint32),
                ("ctrig_args", c_uint32),]

class mcd_trig_state_st(Structure):
    _fields_ = [("active", c_bool),
                ("captured", c_bool),
                ("captured_valid", c_bool),
                ("count_value", c_uint64),
                ("count_valid", c_bool),]

class mcd_trig_set_state_st(Structure):
    _fields_ = [("active", c_bool),
                ("state", c_uint32),
                ("state_valid", c_bool),
                ("trig_bus", c_uint32),
                ("trig_bus_valid", c_bool),
                ("trace", c_bool),
                ("trace_valid", c_bool),
                ("analysis", c_bool),
                ("analysis_valid", c_bool),]

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

class mcd_core_state_st(Structure):
    _fields_ = [("state", c_uint32),
                ("event", c_uint32),
                ("hw_thread_id", c_uint32),
                ("trig_id", c_uint32),
                ("stop_str", c_char*MCD_INFO_STR_LEN),
                ("info_str", c_char*MCD_INFO_STR_LEN),]

class mcd_rst_info_st(Structure):
    _fields_ = [("class_vector", c_uint32),
                ("info_str", c_char*MCD_INFO_STR_LEN),]

class mcd_chl_st(Structure):
    _fields_ = [("chl_id", c_uint32),
                ("type", c_uint32),
                ("attributes", c_uint32),
                ("max_msg_len", c_uint32),
                ("msg_buffer_addr", mcd_addr_st),
                ("prio", c_uint8),]

class mcd_trace_info_st(Structure):
    _fields_ = [("trace_id", c_uint32),
                ("trace_name", c_char),
                ("trace_type", c_uint32),
                ("trace_format", c_uint32),
                ("trace_modes", c_uint32),
                ("trace_no_timestamps", c_uint32),
                ("trace_shared", c_uint32),
                ("trace_size_is_bytes", c_uint32),]

class mcd_trace_state_st(Structure):
    _fields_ = [("state", c_uint32),
                ("mode", c_uint32),
                ("autoarm", c_bool),
                ("wraparound", c_bool),
                ("frames", c_uint64),
                ("count", c_uint64),
                ("size", c_uint64),
                ("trigger_delay", c_uint64),
                ("timestamp_accuracy", c_uint8),
                ("timestamp_is_time", c_bool),
                ("options", c_uint32),
                ("modified", c_bool),
                ("info_str", c_char),]

class mcd_trace_data_event_st(Structure):
    _fields_ = [("timestamp", c_uint64),
                ("marker", c_uint32),
                ("data", c_uint32),]

class mcd_trace_data_stat_st(Structure):
    _fields_ = [("timestamp", c_uint64),
                ("marker", c_uint32),
                ("count", c_uint64),]

# Definition of Functions

__dll = None

def connect_to_dll(path):
    global __dll
    if not os.path.isfile(path):
        raise ValueError(f'Could not find DLL under {path}')
    __dll = CDLL(path)

def disconnect_from_dll():
    global __dll
    if __dll is not None:
        del __dll
        __dll = None

def mcd_initialize_f(version_req, impl_info):
    return __dll.mcd_initialize_f(version_req, impl_info)

def mcd_exit_f():
    return __dll.mcd_exit_f()

def mcd_qry_servers_f(host, running, start_index, num_servers, server_info):
    return __dll.mcd_qry_servers_f(host, running, start_index, num_servers, server_info)

def mcd_open_server_f(system_key, config_string, server):
    return __dll.mcd_open_server_f(system_key, config_string, server)

def mcd_close_server_f(server):
    return __dll.mcd_close_server_f(server)

def mcd_set_server_config_f(server, config_string):
    return __dll.mcd_set_server_config_f(server, config_string)

def mcd_qry_server_config_f(server, max_len, config_string):
    return __dll.mcd_qry_server_config_f(server, max_len, config_string)

def mcd_qry_systems_f(start_index, num_systems, system_con_info):
    return __dll.mcd_qry_systems_f(start_index, num_systems, system_con_info)

def mcd_qry_devices_f(system_con_info, start_index, num_devices, device_con_info):
    return __dll.mcd_qry_devices_f(system_con_info, start_index, num_devices, device_con_info)

def mcd_qry_cores_f(connection_info, start_index, num_cores, core_con_info):
    return __dll.mcd_qry_cores_f(connection_info, start_index, num_cores, core_con_info)

def mcd_qry_core_modes_f(core, start_index, num_modes, core_mode_info):
    return __dll.mcd_qry_core_modes_f(core, start_index, num_modes, core_mode_info)

def mcd_open_core_f(core_con_info, core):
    return __dll.mcd_open_core_f(core_con_info, core)

def mcd_close_core_f(core):
    return __dll.mcd_close_core_f(core)

def mcd_qry_error_info_f(core, error_info):
    return __dll.mcd_qry_error_info_f(core, error_info)

def mcd_qry_device_description_f(core, url, url_length):
    return __dll.mcd_qry_device_description_f(core, url, url_length)

def mcd_qry_max_payload_size_f(core, max_payload):
    return __dll.mcd_qry_max_payload_size_f(core, max_payload)

def mcd_qry_input_handle_f(core, input_handle):
    return __dll.mcd_qry_input_handle_f(core, input_handle)

def mcd_qry_mem_spaces_f(core, start_index, num_mem_spaces, mem_spaces):
    return __dll.mcd_qry_mem_spaces_f(core, start_index, num_mem_spaces, mem_spaces)

def mcd_qry_mem_blocks_f(core, mem_space_id, start_index, num_mem_blocks, mem_blocks):
    return __dll.mcd_qry_mem_blocks_f(core, mem_space_id, start_index, num_mem_blocks, mem_blocks)

def mcd_qry_active_overlays_f(core, start_index, num_active_overlays, active_overlays):
    return __dll.mcd_qry_active_overlays_f(core, start_index, num_active_overlays, active_overlays)

def mcd_qry_reg_groups_f(core, start_index, num_reg_groups, reg_groups):
    return __dll.mcd_qry_reg_groups_f(core, start_index, num_reg_groups, reg_groups)

def mcd_qry_reg_map_f(core, reg_group_id, start_index, num_regs, reg_info):
    return __dll.mcd_qry_reg_map_f(core, reg_group_id, start_index, num_regs, reg_info)

def mcd_qry_reg_compound_f(core, compound_reg_id, start_index, num_reg_ids, reg_id_array):
    return __dll.mcd_qry_reg_compound_f(core, compound_reg_id, start_index, num_reg_ids, reg_id_array)

def mcd_qry_trig_info_f(core, trig_info):
    return __dll.mcd_qry_trig_info_f(core, trig_info)

def mcd_qry_ctrigs_f(core, start_index, num_ctrigs, ctrig_info):
    return __dll.mcd_qry_ctrigs_f(core, start_index, num_ctrigs, ctrig_info)

def mcd_create_trig_f(core, trig, trig_id):
    return __dll.mcd_create_trig_f(core, trig, trig_id)

def mcd_qry_trig_f(core, trig_id, max_trig_size, trig):
    return __dll.mcd_qry_trig_f(core, trig_id, max_trig_size, trig)

def mcd_remove_trig_f(core, trig_id):
    return __dll.mcd_remove_trig_f(core, trig_id)

def mcd_qry_trig_state_f(core, trig_id, trig_state):
    return __dll.mcd_qry_trig_state_f(core, trig_id, trig_state)

def mcd_activate_trig_set_f(core):
    return __dll.mcd_activate_trig_set_f(core)

def mcd_remove_trig_set_f(core):
    return __dll.mcd_remove_trig_set_f(core)

def mcd_qry_trig_set_f(core, start_index, num_trigs, trig_ids):
    return __dll.mcd_qry_trig_set_f(core, start_index, num_trigs, trig_ids)

def mcd_qry_trig_set_state_f(core, trig_state):
    return __dll.mcd_qry_trig_set_state_f(core, trig_state)

def mcd_execute_txlist_f(core, txlist):
    return __dll.mcd_execute_txlist_f(core, txlist)

def mcd_run_f(core, _global):
    return __dll.mcd_run_f(core, _global)

def mcd_stop_f(core, _global):
    return __dll.mcd_stop_f(core, _global)

def mcd_run_until_f(core, _global, absolute_time, run_until_time):
    return __dll.mcd_run_until_f(core, _global, absolute_time, run_until_time)

def mcd_qry_current_time_f(core, current_time):
    return __dll.mcd_qry_current_time_f(core, current_time)

def mcd_step_f(core, _global, step_type, n_steps):
    return __dll.mcd_step_f(core, _global, step_type, n_steps)

def mcd_set_global_f(core, enable):
    return __dll.mcd_set_global_f(core, enable)

def mcd_qry_state_f(core, state):
    return __dll.mcd_qry_state_f(core, state)

def mcd_execute_command_f(core, command_string, result_string_size, result_string):
    return __dll.mcd_execute_command_f(core, command_string, result_string_size, result_string)

def mcd_qry_rst_classes_f(core, rst_class_vector):
    return __dll.mcd_qry_rst_classes_f(core, rst_class_vector)

def mcd_qry_rst_class_info_f(core, rst_class, rst_info):
    return __dll.mcd_qry_rst_class_info_f(core, rst_class, rst_info)

def mcd_rst_f(core, rst_class_vector, rst_and_halt):
    return __dll.mcd_rst_f(core, rst_class_vector, rst_and_halt)

def mcd_chl_open_f(core, channel):
    return __dll.mcd_chl_open_f(core, channel)

def mcd_send_msg_f(core, channel, msg_len, msg):
    return __dll.mcd_send_msg_f(core, channel, msg_len, msg)

def mcd_receive_msg_f(core, channel, timeout, msg_len, msg):
    return __dll.mcd_receive_msg_f(core, channel, timeout, msg_len, msg)

def mcd_chl_reset_f(core, channel):
    return __dll.mcd_chl_reset_f(core, channel)

def mcd_chl_close_f(core, channel):
    return __dll.mcd_chl_close_f(core, channel)

def mcd_qry_traces_f(core, start_index, num_traces, trace_info):
    return __dll.mcd_qry_traces_f(core, start_index, num_traces, trace_info)

def mcd_qry_trace_state_f(core, trace_id, state):
    return __dll.mcd_qry_trace_state_f(core, trace_id, state)

def mcd_set_trace_state_f(core, trace_id, state):
    return __dll.mcd_set_trace_state_f(core, trace_id, state)

def mcd_read_trace_f(core, trace_id, start_index, num_frames, trace_data_size, trace_data):
    return __dll.mcd_read_trace_f(core, trace_id, start_index, num_frames, trace_data_size, trace_data)
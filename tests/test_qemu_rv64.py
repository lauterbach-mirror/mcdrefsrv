from mcd_api import *
import pytest
import os
import logging
import time

LOGGER = logging.getLogger("mcd")

ACTIVE_CORE_ID = 0

# Helper function
def log_all_fields(s):
    for field_name, field_type in s._fields_:
        field = getattr(s, field_name)
        LOGGER.debug(f"{field_name}: {field}")

def assert_equal(obj1, obj2):
    if not hasattr(obj1, '_fields_'):
        assert(obj1 == obj2)
    else:
        for field_name, _ in obj1._fields_:
            assert_equal(getattr(obj1, field_name), getattr(obj2, field_name))

@pytest.fixture(scope="module")
def spawned_target(request, spawn_qemu):
    spawn_qemu(request, "qemu-system-riscv64", f"-M virt -cpu rv64")

@pytest.fixture(scope="module")
def open_core(request, open_core_with_id):
    return open_core_with_id(request, ACTIVE_CORE_ID)

@pytest.fixture(scope="module")
def logical_memspace(request, queried_memory_spaces):
    logical_memspaces = [m for m in queried_memory_spaces[0] if m.mem_type & mcd_mem_type_et.MCD_MEM_SPACE_IS_LOGICAL]
    assert(len(logical_memspaces) == 1)
    return logical_memspaces[0]

@pytest.fixture(scope="module")
def assemble(request, open_core, logical_memspace):
    def _method(address, *opcodes):
        byte_values = []
        for opcode in opcodes:
            byte_values.extend(int(opcode).to_bytes(4, byteorder='little' if logical_memspace.endian == mcd_endian_et.MCD_ENDIAN_LITTLE else 'big'))
        data = (c_uint8*len(byte_values))(*byte_values)
        tx = mcd_tx_st(
            mcd_addr_st(address, logical_memspace.mem_space_id, 0, 0),
            mcd_tx_access_type_et.MCD_TX_AT_W, 0, 0, 0, data, len(byte_values), 0
        )
        txlist = mcd_txlist_st(pointer(tx), 1, 0)
        ret = mcd_execute_txlist_f(open_core, byref(txlist))
        assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    return _method

@pytest.fixture(scope="module")
def read_pc(request, open_core, queried_registers):
    pc_addr_candidates = [r.addr for r in queried_registers[0] if r.regname.decode() == "pc"]
    assert(len(pc_addr_candidates) == 1)
    pc_addr = pc_addr_candidates[0]
    def _method():
        data = (c_uint8*8)()
        tx = mcd_tx_st(pc_addr, mcd_tx_access_type_et.MCD_TX_AT_R, 0, 0, 0, data, 8, 0)
        txlist = mcd_txlist_st(pointer(tx), 1, 0)
        ret = mcd_execute_txlist_f(open_core, byref(txlist))
        assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
        pc = int.from_bytes(list(data), byteorder='little')
        return pc
    return _method

@pytest.fixture(scope="module")
def pc(request, queried_registers):
    pc_candidates = [r for r in queried_registers[0] if r.regname.decode() == "pc"]
    assert(len(pc_candidates) == 1)
    return pc_candidates[0]

@pytest.fixture(scope="module")
def set_pc(request, open_core, pc):
    r = pc
    def _method(address):
        size = r.regsize // 8
        b = list(int(address).to_bytes(size, byteorder='little'))
        data = (c_uint8*size)(*b)
        tx = mcd_tx_st(pc.addr, mcd_tx_access_type_et.MCD_TX_AT_W, 0, 0, 0, data, size, 0)
        txlist = mcd_txlist_st(pointer(tx), 1, 0)
        ret = mcd_execute_txlist_f(open_core, byref(txlist))
        assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    return _method

@pytest.fixture(scope="module")
def read_pc(request, open_core, pc):
    r = pc
    def _method():
        size = r.regsize // 8
        data = (c_uint8*size)()
        tx = mcd_tx_st(r.addr, mcd_tx_access_type_et.MCD_TX_AT_R, 0, 0, 0, data, size, 0)
        txlist = mcd_txlist_st(pointer(tx), 1, 0)
        ret = mcd_execute_txlist_f(open_core, byref(txlist))
        assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
        pc = int.from_bytes(list(data), byteorder='little')
        return pc
    return _method

def test_open_server(connected_server):
    server_p = connected_server
    LOGGER.info(f"Server Connected: {server_p.contents.host.decode()}")
    log_all_fields(server_p.contents)

def test_open_core(open_core):
    core_p = open_core
    LOGGER.info(f"Core Open: {core_p.contents.core_con_info.contents.core.decode()}")
    log_all_fields(core_p.contents)

def test_query_reset_classes(open_core):
    core_p = open_core
    rst_class_vector = c_uint32(0)
    ret = mcd_qry_rst_classes_f(core_p, byref(rst_class_vector))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

def test_query_memory_spaces(queried_memory_spaces):
    memspace_p, num_memspaces = queried_memory_spaces
    for i in range(num_memspaces):
        memspace = memspace_p[i]
        LOGGER.info(f"Memory space found ({i}/{num_memspaces-1}): [{memspace.mem_space_id}] {memspace.mem_space_name.decode()} [type: 0x{memspace.mem_type:x}]")
        log_all_fields(memspace)

def test_query_register_groups(queried_register_groups):
    reggroups_p, num_reggroups = queried_register_groups
    for i in range(num_reggroups):
        reggroup = reggroups_p[i]
        LOGGER.info(f"Register group found ({i}/{num_reggroups-1}): [{reggroup.reg_group_id}] {reggroup.reg_group_name.decode()}")
        log_all_fields(reggroup)

def test_query_registers(queried_registers):
    reg_p, num_regs = queried_registers
    assert(num_regs > 0)
    for i in range(num_regs):
        reg = reg_p[i]
        LOGGER.info(f"Register found ({i}/{num_regs-1}): [{reg.reg_group_id}:{reg.addr.address}] {reg.regname.decode()}")
        log_all_fields(reg)

def test_query_reset_classes(open_core, queried_reset_classes):
    for i in range(32):
        rst_class = c_uint8(i)
        rst_info = mcd_rst_info_st()
        ret = mcd_qry_rst_class_info_f(open_core, rst_class, byref(rst_info))
        if queried_reset_classes.value & (1 << i):
            assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
            LOGGER.info(f"Reset class: {rst_info.info_str.decode()}")
        else:
            assert(ret != mcd_return_et.MCD_RET_ACT_NONE)

# QEMU only supports breakpoints at logical addresses

def test_trig_ip(open_core, queried_reset_classes, queried_trig_info, logical_memspace, read_pc):
    assert(queried_trig_info.type & mcd_trig_type_et.MCD_TRIG_TYPE_IP)

    core = open_core
    ret = mcd_rst_f(core, queried_reset_classes, True)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    trig_id = c_uint32(0)
    trig = mcd_trig_simple_core_st(
        sizeof(mcd_trig_simple_core_st),
        mcd_trig_type_et.MCD_TRIG_TYPE_IP,
        0,
        0,
        0,
        False,
        0,
        mcd_addr_st(0x80000000, logical_memspace.mem_space_id, 0, 0),
        0x4,
    )

    ret = mcd_create_trig_f(core, byref(trig), byref(trig_id))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    trig_queried = mcd_trig_simple_core_st(sizeof(mcd_trig_simple_core_st))
    ret = mcd_qry_trig_f(core, trig_id, sizeof(mcd_trig_simple_core_st), byref(trig_queried))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert_equal(trig, trig_queried)

    trig_state = mcd_trig_state_st()
    ret = mcd_qry_trig_state_f(core, trig_id, byref(trig_state))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(trig_state.captured == False)

    ret = mcd_run_f(core, True)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    state = mcd_core_state_st(mcd_core_state_et.MCD_CORE_STATE_UNKNOWN)
    while state.state != mcd_core_state_et.MCD_CORE_STATE_DEBUG:
        ret = mcd_qry_state_f(core, byref(state))
        assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(state.trig_id == trig_id.value)

    trig_state = mcd_trig_state_st()
    ret = mcd_qry_trig_state_f(core, trig_id, byref(trig_state))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(trig_state.captured)

    assert(read_pc() == 0x80000000)

    ret = mcd_remove_trig_f(core, trig_id)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

def test_trig_read(open_core, queried_reset_classes, queried_trig_info, logical_memspace, set_pc, read_pc, assemble):
    assert(queried_trig_info.type & mcd_trig_type_et.MCD_TRIG_TYPE_READ)
    core = open_core
    ret = mcd_rst_f(core, queried_reset_classes, True)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    set_pc(0x80000000)

    assemble(0x80000020, 0x123)

    # increment value at address 0x80000020
    assemble(0x80000000,
        0x00000097, # auipc x1, 0
        0x02008093, # addi x1, x1, 0x20
        0x0000a103, # lw x2, 0(x1)
        0x00010113, # addi x2, x2, 0
        0x0020a023, # sw x2, 0(x1)
        0x0000006f, # jal x0, 0 (infinite loop)
    )

    trig_id = c_uint32(0)
    trig = mcd_trig_simple_core_st(
        sizeof(mcd_trig_simple_core_st),
        mcd_trig_type_et.MCD_TRIG_TYPE_READ,
        0,
        0,
        0,
        False,
        0,
        mcd_addr_st(0x80000020, logical_memspace.mem_space_id, 0, 0),
        0x4,
    )

    ret = mcd_create_trig_f(core, byref(trig), byref(trig_id))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    trig_queried = mcd_trig_simple_core_st(sizeof(mcd_trig_simple_core_st))
    ret = mcd_qry_trig_f(core, trig_id, sizeof(mcd_trig_simple_core_st), byref(trig_queried))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert_equal(trig, trig_queried)

    trig_state = mcd_trig_state_st()
    ret = mcd_qry_trig_state_f(core, trig_id, byref(trig_state))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(trig_state.captured == False)

    ret = mcd_run_f(core, True)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    state = mcd_core_state_st(mcd_core_state_et.MCD_CORE_STATE_UNKNOWN)
    while state.state != mcd_core_state_et.MCD_CORE_STATE_DEBUG:
        ret = mcd_qry_state_f(core, byref(state))
        assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(state.trig_id == trig_id.value)

    assert(read_pc() == 0x8000000C)

    trig_state = mcd_trig_state_st()
    ret = mcd_qry_trig_state_f(core, trig_id, byref(trig_state))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(trig_state.captured)

    ret = mcd_remove_trig_f(core, trig_id)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

def test_trig_write(open_core, queried_reset_classes, queried_trig_info, logical_memspace, set_pc, read_pc, assemble):
    assert(queried_trig_info.type & mcd_trig_type_et.MCD_TRIG_TYPE_WRITE)
    core = open_core
    ret = mcd_rst_f(core, queried_reset_classes, True)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    set_pc(0x80000000)

    assemble(0x80000020, 0x123)

    # increment value at address 0x80000020
    assemble(0x80000000,
        0x00000097, # auipc x1, 0
        0x02008093, # addi x1, x1, 0x20
        0x0000a103, # lw x2, 0(x1)
        0x00010113, # addi x2, x2, 0
        0x0020a023, # sw x2, 0(x1)
        0x0000006f, # jal x0, 0 (infinite loop)
    )

    trig_id = c_uint32(0)
    trig = mcd_trig_simple_core_st(
        sizeof(mcd_trig_simple_core_st),
        mcd_trig_type_et.MCD_TRIG_TYPE_WRITE,
        0,
        0,
        0,
        False,
        0,
        mcd_addr_st(0x80000020, logical_memspace.mem_space_id, 0, 0),
        0x4,
    )

    ret = mcd_create_trig_f(core, byref(trig), byref(trig_id))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    trig_queried = mcd_trig_simple_core_st(sizeof(mcd_trig_simple_core_st))
    ret = mcd_qry_trig_f(core, trig_id, sizeof(mcd_trig_simple_core_st), byref(trig_queried))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert_equal(trig, trig_queried)

    trig_state = mcd_trig_state_st()
    ret = mcd_qry_trig_state_f(core, trig_id, byref(trig_state))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(trig_state.captured == False)

    ret = mcd_run_f(core, True)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    state = mcd_core_state_st(mcd_core_state_et.MCD_CORE_STATE_UNKNOWN)
    while state.state != mcd_core_state_et.MCD_CORE_STATE_DEBUG:
        ret = mcd_qry_state_f(core, byref(state))
        assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(state.trig_id == trig_id.value)

    assert(read_pc() == 0x80000014)

    trig_state = mcd_trig_state_st()
    ret = mcd_qry_trig_state_f(core, trig_id, byref(trig_state))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(trig_state.captured)

    ret = mcd_remove_trig_f(core, trig_id)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

def test_trig_rw(open_core, queried_reset_classes, queried_trig_info, logical_memspace, set_pc, read_pc, assemble):
    assert(queried_trig_info.type & mcd_trig_type_et.MCD_TRIG_TYPE_RW)
    core = open_core
    ret = mcd_rst_f(core, queried_reset_classes, True)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    set_pc(0x80000000)

    assemble(0x80000020, 0x123)

    # increment value at address 0x80000020
    assemble(0x80000000,
        0x00000097, # auipc x1, 0
        0x02008093, # addi x1, x1, 0x20
        0x0000a103, # lw x2, 0(x1)
        0x00010113, # addi x2, x2, 0
        0x0020a023, # sw x2, 0(x1)
        0x0000006f, # jal x0, 0 (infinite loop)
    )

    trig_id = c_uint32(0)
    trig = mcd_trig_simple_core_st(
        sizeof(mcd_trig_simple_core_st),
        mcd_trig_type_et.MCD_TRIG_TYPE_RW,
        0,
        0,
        0,
        False,
        0,
        mcd_addr_st(0x80000020, logical_memspace.mem_space_id, 0, 0),
        0x4,
    )

    ret = mcd_create_trig_f(core, byref(trig), byref(trig_id))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    trig_state = mcd_trig_state_st()
    ret = mcd_qry_trig_state_f(core, trig_id, byref(trig_state))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(trig_state.captured == False)

    ret = mcd_run_f(core, True)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    state = mcd_core_state_st(mcd_core_state_et.MCD_CORE_STATE_UNKNOWN)
    while state.state != mcd_core_state_et.MCD_CORE_STATE_DEBUG:
        ret = mcd_qry_state_f(core, byref(state))
        assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(state.trig_id == trig_id.value)

    assert(read_pc() == 0x8000000C)

    trig_state = mcd_trig_state_st()
    ret = mcd_qry_trig_state_f(core, trig_id, byref(trig_state))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(trig_state.captured)

    ret = mcd_remove_trig_f(core, trig_id)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

def test_step(open_core, queried_reset_classes, queried_trig_info, logical_memspace, read_pc, assemble):
    test_trig_ip(open_core, queried_reset_classes, queried_trig_info, logical_memspace, read_pc)
    assert(read_pc() == 0x80000000)
    assemble(0x80000000, 0x00000013) # nop
    ret = mcd_step_f(open_core, False, mcd_core_step_type_et.MCD_CORE_STEP_TYPE_INSTR, 1)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(read_pc() == 0x80000004)

def test_step_while_running(open_core):
    ret = mcd_run_f(open_core, True)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    ret = mcd_step_f(open_core, False, mcd_core_step_type_et.MCD_CORE_STEP_TYPE_INSTR, 1)
    assert(ret != mcd_return_et.MCD_RET_ACT_NONE)
    error_info = mcd_error_info_st()
    mcd_qry_error_info_f(open_core, byref(error_info))
    LOGGER.info(f"Step while running failed: {error_info.error_str.decode()}")

def test_remove_trig_set(open_core, logical_memspace):
    core = open_core
    start_index = c_uint32(0)
    num_trigs = c_uint32(0)
    ret = mcd_qry_trig_set_f(core, start_index, byref(num_trigs), None)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(num_trigs.value == 0)

    trig_id = c_uint32(0)
    trig = mcd_trig_simple_core_st(
        sizeof(mcd_trig_simple_core_st),
        mcd_trig_type_et.MCD_TRIG_TYPE_WRITE,
        0,
        0,
        0,
        False,
        0,
        mcd_addr_st(0x80000020, logical_memspace.mem_space_id, 0, 0),
        0x4,
    )

    ret = mcd_create_trig_f(core, byref(trig), byref(trig_id))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    num_trigs = c_uint32(1)
    trig_id_queried = c_uint32()
    ret = mcd_qry_trig_set_f(open_core, start_index, byref(num_trigs), byref(trig_id_queried))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(num_trigs.value == 1)
    assert(trig_id_queried.value == trig_id.value)

    ret = mcd_remove_trig_set_f(core)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    num_trigs = c_uint32(0)
    ret = mcd_qry_trig_set_f(open_core, start_index, byref(num_trigs), None)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    assert(num_trigs.value == 0)

def test_close_server(connected_server):
    server_p = connected_server
    ret = mcd_close_server_f(server_p)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

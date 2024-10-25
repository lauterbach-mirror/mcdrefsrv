from mcd_api import *
import pytest
import os
import logging

LOGGER = logging.getLogger("mcd")

ACTIVE_CORE_ID = 0
GDB_REGS_MEM_SPACE_ID = 4

# Helper function
def log_all_fields(s):
    for field_name, field_type in s._fields_:
        field = getattr(s, field_name)
        LOGGER.debug(f"{field_name}: {field}")

@pytest.fixture(scope="module")
def spawned_target(request, spawn_qemu):
    spawn_qemu(request, "qemu-system-arm", f"-M virt,secure=on -cpu cortex-a15")

@pytest.fixture(scope="module")
def open_core(request, open_core_with_id):
    return open_core_with_id(request, ACTIVE_CORE_ID)

def test_open_server(connected_server):
    server_p = connected_server
    LOGGER.info(f"Server Connected: {server_p.contents.host.decode()}")
    log_all_fields(server_p.contents)

def test_open_core(open_core):
    core_p = open_core
    LOGGER.info(f"Core Open: {core_p.contents.core_con_info.contents.core.decode()}")
    log_all_fields(core_p.contents)

def test_query_memory_spaces(queried_memory_spaces):
    memspace_p, num_memspaces = queried_memory_spaces
    assert(num_memspaces == GDB_REGS_MEM_SPACE_ID + 1)
    for i in range(num_memspaces):
        memspace = memspace_p[i]
        name = memspace.mem_space_name.decode()
        LOGGER.info(f"Memory space found ({i}/{num_memspaces-1}): [{memspace.mem_space_id}] {memspace.mem_space_name.decode()} [type: 0x{memspace.mem_type:x}]")
        if i == GDB_REGS_MEM_SPACE_ID:
            assert("GDB" in name)
        log_all_fields(memspace)

def test_query_register_groups(queried_register_groups):
    reggroup_p, num_reggroups = queried_register_groups
    assert(num_reggroups > 0)
    for i in range(num_reggroups):
        reggroup = reggroup_p[i]
        LOGGER.info(f"Register group found ({i}/{num_reggroups-1}): [{reggroup.reg_group_id}] {reggroup.reg_group_name.decode()}")
        log_all_fields(reggroup)

def test_query_registers(queried_registers):
    reg_p, num_regs = queried_registers
    assert(num_regs > 0)
    for i in range(num_regs):
        reg = reg_p[i]
        LOGGER.info(f"Register found ({i}/{num_regs-1}): [{reg.reg_group_id}:{reg.addr.address}] {reg.regname.decode()}")
        log_all_fields(reg)

def test_read_physical_memory(open_core):
    core_p = open_core
    txlist = mcd_txlist_st()
    txlist.num_tx = 1
    txlist.num_tx_ok = 0
    txlist.tx = (mcd_tx_st*1)()

    valid_tx = mcd_tx_st()
    valid_tx.addr = mcd_addr_st()
    valid_tx.addr.address = 0x40000000
    valid_tx.addr.mem_space_id = 1
    valid_tx.access_type = mcd_tx_access_type_et.MCD_TX_AT_R
    valid_tx.access_width = 112
    valid_tx.data = (c_uint8 * 1000)()
    valid_tx.num_bytes = 112#
    txlist.tx[0] = valid_tx
    ret = mcd_execute_txlist_f(core_p, byref(txlist))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    # Test that secure memory is not accessible via non-secure memory space
    invalid_tx = valid_tx
    invalid_tx.addr.address = 0xe000000
    txlist.num_tx = 1
    txlist.num_tx_ok = 0
    txlist.tx[0] = invalid_tx
    ret = mcd_execute_txlist_f(core_p, byref(txlist))
    assert(ret == mcd_return_et.MCD_RET_ACT_HANDLE_ERROR)
    error_info = mcd_error_info_st()
    mcd_qry_error_info_f(core_p, byref(error_info))
    assert(error_info.error_code == mcd_error_code_et.MCD_ERR_TXLIST_READ)

def test_read_physical_secure_memory(open_core):
    core_p = open_core
    txlist = mcd_txlist_st()
    txlist.num_tx = 1
    txlist.num_tx_ok = 0
    txlist.tx = (mcd_tx_st*1)()

    valid_tx = mcd_tx_st()
    valid_tx.addr = mcd_addr_st()
    valid_tx.addr.address = 0xe000000
    valid_tx.addr.mem_space_id = 2
    valid_tx.access_type = mcd_tx_access_type_et.MCD_TX_AT_R
    valid_tx.access_width = 112
    valid_tx.data = (c_uint8 * 1000)()
    valid_tx.num_bytes = 112#
    txlist.tx[0] = valid_tx
    ret = mcd_execute_txlist_f(core_p, byref(txlist))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    # Non-secure memory is accessible via secure memory space
    valid_ns_tx = valid_tx
    valid_ns_tx.addr.address = 0x40000000
    txlist.num_tx = 1
    txlist.num_tx_ok = 0
    txlist.tx[0] = valid_ns_tx
    ret = mcd_execute_txlist_f(core_p, byref(txlist))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    
    invalid_tx = valid_tx
    invalid_tx.addr.address = 0xdffffc0
    txlist.num_tx = 1
    txlist.num_tx_ok = 0
    txlist.tx[0] = invalid_tx
    ret = mcd_execute_txlist_f(core_p, byref(txlist))
    assert(ret == mcd_return_et.MCD_RET_ACT_HANDLE_ERROR)
    error_info = mcd_error_info_st()
    mcd_qry_error_info_f(core_p, byref(error_info))
    assert(error_info.error_code == mcd_error_code_et.MCD_ERR_TXLIST_READ)

def test_close_server(connected_server):
    server_p = connected_server
    ret = mcd_close_server_f(server_p)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

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

@pytest.fixture(scope="module")
def spawned_target(request, spawn_qemu):
    spawn_qemu(request, "qemu-system-riscv64", f"-M virt -cpu rv64")

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

def test_clock(open_core):
    core_p = open_core
    ret = mcd_stop_f(core_p, True)
    t1 = c_uint64(0)
    t2 = c_uint64(0)

    ret = mcd_qry_current_time_f(core_p, byref(t1))
    assert (ret == mcd_return_et.MCD_RET_ACT_NONE)
    LOGGER.info(f"t1 = {t1.value/1e9}ms")
    ret = mcd_qry_current_time_f(core_p, byref(t2))
    assert (ret == mcd_return_et.MCD_RET_ACT_NONE)
    LOGGER.info(f"t2 = {t2.value/1e9}ms")
    assert(t1.value == t2.value)

    ret = mcd_run_f(core_p, True)
    assert (ret == mcd_return_et.MCD_RET_ACT_NONE)

    time.sleep(0.5)
    ret = mcd_qry_current_time_f(core_p, byref(t1))
    assert (ret == mcd_return_et.MCD_RET_ACT_NONE)
    LOGGER.info(f"t1 = {t1.value/1e9}ms")
    time.sleep(0.5)
    ret = mcd_qry_current_time_f(core_p, byref(t2))
    assert (ret == mcd_return_et.MCD_RET_ACT_NONE)
    LOGGER.info(f"t2 = {t2.value/1e9}ms")
    assert(t1.value != t2.value)

def test_close_server(connected_server):
    server_p = connected_server
    ret = mcd_close_server_f(server_p)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

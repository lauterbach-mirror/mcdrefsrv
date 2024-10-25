from mcd_api import *
import pytest
import os
import logging

LOGGER = logging.getLogger("mcd")

ACTIVE_CORE_ID = 0

# Helper function
def log_all_fields(s):
    for field_name, field_type in s._fields_:
        field = getattr(s, field_name)
        LOGGER.debug(f"{field_name}: {field}")

@pytest.fixture(scope="module")
def spawned_target(request, spawn_qemu):
    spawn_qemu(request, "qemu-system-i386", f"-M q35 -cpu IvyBridge")

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
    for i in range(num_memspaces):
        memspace = memspace_p[i]
        name = memspace.mem_space_name.decode()
        LOGGER.info(f"Memory space found ({i}/{num_memspaces-1}): [{memspace.mem_space_id}] {memspace.mem_space_name.decode()} [type: 0x{memspace.mem_type:x}]")
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

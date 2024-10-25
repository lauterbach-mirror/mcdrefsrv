from mcd_api import *
import pytest
import os
import logging

LOGGER = logging.getLogger("mcd")

NUM_CORES = 2

# Helper function
def log_all_fields(s):
    for field_name, field_type in s._fields_:
        field = getattr(s, field_name)
        LOGGER.debug(f"{field_name}: {field}")

@pytest.fixture(scope="module")
def spawned_target(request, spawn_qemu):
    spawn_qemu(request, "qemu-system-riscv64", f"-M virt -cpu rv64 -smp {NUM_CORES}")

@pytest.fixture(scope="module")
def open_cores(request, open_core_with_id):
    cores = []
    for i in range(NUM_CORES):
        cores.append(open_core_with_id(request, i))
    return cores

def test_open_cores(open_cores):
    for core in open_cores:
        LOGGER.info(f"Core found ({core.contents.core_con_info.contents.core_id}): {core.contents.core_con_info.contents.core.decode()}")
        num_memspaces = c_uint32(0)
        start_index = c_uint32(0)
        ret = mcd_qry_mem_spaces_f(core, start_index, byref(num_memspaces), None)
        assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

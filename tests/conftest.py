from mcd_api import *
from datetime import datetime
import pytest
import os
import logging
import subprocess
import time
LOGGER = logging.getLogger("mcd")
fileHandle = None

# Helper function
def log_all_fields(s):
    for field_name, field_type in s._fields_:
        field = getattr(s, field_name)
        LOGGER.debug(f"{field_name}: {field}")

TESTDIR_NAME = 'tests'
RELATIVE_PATH_TO_DLL = '../build/libmcd_client_stub.so' if os.name == 'posix' else '../build/Debug/mcd_client_stub.dll'

MAX_NUM_SYSTEMS = 1
MAX_NUM_DEVICES = 1
MAX_NUM_CORES = 20
MAX_NUM_MEMSPACES = 20
MAX_NUM_REG_GROUPS = 20
MAX_NUM_REGISTERS = 1000

SPAWN_OWN_QEMU = True

QEMU_DIR = None
WSL_OPTS = None

if "QEMU_DIR" in os.environ:
    QEMU_DIR = os.environ["QEMU_DIR"]

if "WSL_QEMU_DIR" in os.environ:
    QEMU_DIR = os.environ["WSL_QEMU_DIR"]

if "WSL_OPTS" in os.environ:
    WSL_OPTS = os.environ["WSL_OPTS"]

# Need to be overwritten in target modules:

@pytest.fixture(scope="module")
def spawned_target():
    raise NotImplemented

@pytest.fixture(scope="module")
def open_core():
    raise NotImplemented

def get_testdir_path(request):
    path = []
    testdir_found = False
    for p in request.config.inipath.parts:
        path.append(p)
        if p == TESTDIR_NAME:
            testdir_found = True
            break
    assert(testdir_found)
    return path

@pytest.fixture(scope="session", autouse=True)
def setup_logger(request):
    global fileHandle
    if fileHandle is None:
        path = get_testdir_path(request)
        path.append("logs")
        path.append(datetime.now().strftime("test_mcd.%Y%m%d%H%M%S.log"))
        logfile = os.path.join(*path)
        fileHandle = logging.FileHandler(logfile)
        LOGGER.addHandler(fileHandle)
        LOGGER.setLevel(logging.INFO)

@pytest.fixture(scope="session")
def dll_connected(request):
    path = get_testdir_path(request)
    path.append(RELATIVE_PATH_TO_DLL)
    path_to_dll = os.path.join(*path)
    LOGGER.info("Connecting to DLL")
    connect_to_dll(path_to_dll)
    def disconnect():
        LOGGER.info("Disconnecting from DLL")
        disconnect_from_dll()
    request.addfinalizer(disconnect)

@pytest.fixture(scope="module")
def spawn_qemu():
    def spawn_func_wsl(request, qemu, qemu_opts):
        LOGGER.info(f"Spawning {qemu}")
        # The echo command is mandatory here since
        # we need to be able to retrieve the relevant PID later to be able to close QEMU
        cmd_line = (f"wsl {WSL_OPTS} "
                     "echo $$ && "
                    f"{QEMU_DIR}/{qemu} {qemu_opts} -mcd default -S -nographic")
        wsl_process = subprocess.Popen(cmd_line, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(0.5)
        def close_qemu():
            LOGGER.info(f"Closing {qemu}")
            wsl_process.terminate()
            pid, _ = wsl_process.communicate()
            subprocess.run(["wsl", *os.environ["WSL_OPTS"].split(), "kill", pid.decode().strip()])
            time.sleep(0.5)
        request.addfinalizer(close_qemu)

    def spawn_func_posix(request, qemu, qemu_opts):
        LOGGER.info(f"Spawning {qemu}")
        cmd_line = (f"{QEMU_DIR}/{qemu} {qemu_opts} -mcd default -S -nographic")
        qemu_process = subprocess.Popen(cmd_line.split(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(0.5)
        def close_qemu():
            LOGGER.info(f"Closing {qemu}")
            qemu_process.terminate()
            time.sleep(0.5)
        request.addfinalizer(close_qemu)

    def dummy_func(request, qemu, qemu_opts):
        pass

    if SPAWN_OWN_QEMU:
        return spawn_func_posix if os.name =='posix' else spawn_func_wsl

    return dummy_func

@pytest.fixture(scope="module")
def api_compatible(request, dll_connected):
    version_req = mcd_api_version_st(1,1)
    impl_info = mcd_impl_version_info_st()
    ret = mcd_initialize_f(byref(version_req), byref(impl_info))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    LOGGER.info(f"API v{impl_info.v_api.v_api_major}.{impl_info.v_api.v_api_minor} ({impl_info.v_api.author.decode()})")
    LOGGER.info(f"Implementation v{impl_info.v_imp_minor}.{impl_info.v_imp_minor}.{impl_info.v_imp_build} {impl_info.date.decode()} ({impl_info.vendor.decode()})")
    def exit_mcd():
        LOGGER.info("Exiting MCD")
        mcd_exit_f()
    request.addfinalizer(exit_mcd)

@pytest.fixture(scope="module")
def connected_server(request, spawned_target, api_compatible):
    server_p = pointer(mcd_server_st())
    host = c_char()
    config_string = c_char()
    LOGGER.info("Opening server")
    ret = mcd_open_server_f(byref(host), byref(config_string), byref(server_p))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    return server_p

@pytest.fixture(scope="module")
def queried_cores(connected_server):
    cores = []
    start_index = c_uint32(0)

    num_systems = c_uint32(0)
    system_con_info_p = (mcd_core_con_info_st*MAX_NUM_SYSTEMS)()
    ret = mcd_qry_systems_f(start_index, byref(num_systems), system_con_info_p)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    ret = mcd_qry_systems_f(start_index, byref(num_systems), system_con_info_p)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    # Currently, only QEMU is supported
    # always one system with one device
    assert(num_systems.value == 1)
    for i in range(start_index.value, num_systems.value):
        system = system_con_info_p[i]
        LOGGER.info(f"System found ({i+1}/{num_systems.value}): {system_con_info_p[i].system.decode()}")
        log_all_fields(system)

        num_devices = c_uint32(0)
        device_con_info_p = (mcd_core_con_info_st*MAX_NUM_DEVICES)()
        ret = mcd_qry_devices_f(byref(system), start_index, byref(num_devices), device_con_info_p)
        assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
        ret = mcd_qry_devices_f(byref(system), start_index, byref(num_devices), device_con_info_p)
        assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
        assert(num_devices.value == 1)
        for i in range(start_index.value, num_devices.value):
            device_name = device_con_info_p[i].device.decode()
            LOGGER.info(f"Device found ({i+1}/{num_devices.value}): {device_name}")
            device = device_con_info_p[i]
            log_all_fields(device)

            num_cores = c_uint32(0)
            core_con_info_p = (mcd_core_con_info_st*MAX_NUM_CORES)()
            ret = mcd_qry_cores_f(byref(device), start_index, byref(num_cores), core_con_info_p)
            assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
            ret = mcd_qry_cores_f(byref(device), start_index, byref(num_cores), core_con_info_p)
            assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
            for i in range(start_index.value, num_cores.value):
                LOGGER.info(f"Core found ({i+1}/{num_cores.value}): {core_con_info_p[i].core.decode()}")
                core = core_con_info_p[i]
                log_all_fields(core)
                cores.append(core)

    return cores

@pytest.fixture(scope="module")
def open_core_with_id(queried_cores):
    cores = [*queried_cores]
    def _method(request, core_id):
        assert(len(cores) > core_id)
        core = cores[core_id]
        core_p = pointer(mcd_core_st())
        LOGGER.info("Opening core")
        ret = mcd_open_core_f(byref(core), byref(core_p))
        assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
        def close_core():
            LOGGER.info("Closing core")
            mcd_close_core_f(core_p)
        request.addfinalizer(close_core)
        return core_p
    return _method

@pytest.fixture(scope="module")
def queried_register_groups(open_core):
    core = open_core
    num_reggroups = c_uint32(0)
    reggroup_p = (mcd_register_group_st*MAX_NUM_REG_GROUPS)()
    start_index = c_uint32(0)
    ret = mcd_qry_reg_groups_f(core, start_index, byref(num_reggroups), reggroup_p)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    ret = mcd_qry_reg_groups_f(core, start_index, byref(num_reggroups), reggroup_p)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    return (reggroup_p, num_reggroups.value)

@pytest.fixture(scope="module")
def queried_memory_spaces(open_core):
    core = open_core
    num_memspaces = c_uint32(0)
    memspace_p = (mcd_memspace_st*MAX_NUM_MEMSPACES)()
    start_index = c_uint32(0)
    ret = mcd_qry_mem_spaces_f(core, start_index, byref(num_memspaces), memspace_p)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    ret = mcd_qry_mem_spaces_f(core, start_index, byref(num_memspaces), memspace_p)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    return (memspace_p, num_memspaces.value)

@pytest.fixture(scope="module")
def queried_registers(open_core):
    core = open_core
    num_regs = c_uint32(0)
    reg_p = (mcd_register_info_st*MAX_NUM_REGISTERS)()
    start_index = c_uint32(0)
    ret = mcd_qry_reg_map_f(core, 0, start_index, byref(num_regs), reg_p)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    ret = mcd_qry_reg_map_f(core, 0, start_index, byref(num_regs), reg_p)
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)
    return (reg_p, num_regs.value)

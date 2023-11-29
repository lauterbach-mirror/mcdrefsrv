import pathlib
import platform
import time

from .mcd_test_values import*
from .mcdstub_data import*

class RoutinesMcdRefSrv():
    def __init__(self) -> None:
        self.mcdrefsrv: McdRefSrv = None
        self.mcdlib_path = None

    def setup_mcdrefsrv(self) -> int:
        operating_system = platform.system()
        if operating_system == 'Linux':
            # path to .so file with extension
            self.mcdlib_path = pathlib.Path('build/linux-default/src/libmcd_shared_library.so')
        elif operating_system == 'Windows':
            # path to .dll file without extension
            self.mcdlib_path = pathlib.Path('build/windows-default/src/Debug/mcd_shared_library')
        else:
            raise ValueError(f'The operating system {operating_system} is not supported')
        self.mcdrefsrv = McdRefSrv(self.mcdlib_path)
        return 0

    def start_routine(self) -> c_uint32:
        # init function
        version_req = mcd_api_version_st(1,1)
        impl_info = mcd_impl_version_info_st()
        error = self.mcdrefsrv.mcd_initialize_f(version_req, impl_info)
        if (error != MCD_ERR_NONE):
            return error
        # open server function
        system_key = c_char()
        config_string = c_char()
        error = self.mcdrefsrv.mcd_open_server_f(system_key, config_string, self.mcdrefsrv.server.serverptr)
        return error

    def target_query_routine(self) -> c_uint32:
        # get systems
        num_systems = c_uint32(0)
        error = self.mcdrefsrv.mcd_qry_systems_f(c_uint32(0), num_systems, mcd_core_con_info_st())
        system_con_info = [mcd_core_con_info_st() for i in range(num_systems.value)]
        for system_index in range(num_systems.value):
            error = self.mcdrefsrv.mcd_qry_systems_f(c_uint32(system_index), c_uint32(1), system_con_info[system_index])
            # get devices
            num_devices = c_uint32(0)
            error = self.mcdrefsrv.mcd_qry_devices_f(mcd_core_con_info_st(), c_uint32(0), num_devices, mcd_core_con_info_st())
            device_con_info = [mcd_core_con_info_st() for i in range(num_devices.value)]
            for device_index in range(num_devices.value):
                error = self.mcdrefsrv.mcd_qry_devices_f(system_con_info[system_index], c_uint32(device_index), c_uint32(1), device_con_info[device_index])
                # get cores
                num_cores = c_uint32(0)
                error = self.mcdrefsrv.mcd_qry_cores_f(mcd_core_con_info_st(), c_uint32(0), num_cores, mcd_core_con_info_st())
                for core_index in range(num_cores.value):
                    core_con_info = mcd_core_con_info_st()
                    error = self.mcdrefsrv.mcd_qry_cores_f(device_con_info[device_index], c_uint32(core_index), c_uint32(1), core_con_info)
                    # store core
                    core = Core(self.mcdrefsrv.server, core_con_info)
                    self.mcdrefsrv.server.cores.append(core)
        return error

    def open_cores_routine(self) -> c_uint32:
        error = c_uint32(MCD_ERR_GENERAL)
        for core in self.mcdrefsrv.server.cores:
            error = self.mcdrefsrv.mcd_open_core_f(core.core_con_info, core.coreptr)
            if (error != MCD_ERR_NONE):
                return error
        return error

    def core_query_routine(self) -> c_uint32:
        error = c_uint32(MCD_ERR_GENERAL)
        for core in self.mcdrefsrv.server.cores:
            # get reset classes
            error = core.store_resets()
            if (error != MCD_ERR_NONE):
                return error
            # get triggers
            error = core.store_triggers()
            if (error != MCD_ERR_NONE):
                return error
            # get mem spaces
            error = core.store_memspaces()
            if (error != MCD_ERR_NONE):
                return error
            # get reg groups
            error = core.store_reggroups()
            if (error != MCD_ERR_NONE):
                return error
            # get regs
            error = core.store_regs()
            if (error != MCD_ERR_NONE):
                return error
        return error

    def close_cores_routine(self) -> c_uint32:
        error = c_uint32(MCD_ERR_GENERAL)
        for core in self.mcdrefsrv.server.cores:
            error = self.mcdrefsrv.mcd_close_core_f(core.coreptr)
            if (error != MCD_ERR_NONE):
                return error
        return error

    def shutdown_routine(self) -> c_uint32:
         # close server function
        error = self.mcdrefsrv.mcd_close_server_f(self.mcdrefsrv.server.serverptr)
        time.sleep(0.5) # wait for qemu to safely detach
        if (error != MCD_ERR_NONE):
            return error
        # exit function
        self.mcdrefsrv.mcd_exit_f()
        return MCD_ERR_NONE

    def state_routine(self) -> c_uint32:
        error = c_uint32(MCD_ERR_GENERAL)
        for core in self.mcdrefsrv.server.cores:
            error = self.mcdrefsrv.mcd_qry_state_f(core.coreptr, core.last_state)
            if (error != MCD_ERR_NONE):
                return error
            print(core.last_state.info_str)
            print(core.last_state.stop_str)
        return error

    def reset_pc_routine(self) -> c_uint32:
        error = c_uint32(MCD_ERR_GENERAL)
        # create txlist for setting pc register to 0
        transaction = mcd_txlist_st(pointer(arm_write_pc_tx_list), 1, 0)
        for index in range(transaction.num_tx):
            transaction.tx.contents.data[index] = c_uint8(0)
        for core in self.mcdrefsrv.server.cores:
            error = self.mcdrefsrv.mcd_execute_txlist_f(core.coreptr, transaction)
            if (error != MCD_ERR_NONE):
                return error
        return error

    def read_pc_routine(self) -> c_uint32:
        error = c_uint32(MCD_ERR_GENERAL)
        transaction = mcd_txlist_st(pointer(arm_read_pc_tx_list), 1, 0)
        for core in self.mcdrefsrv.server.cores:
            error = self.mcdrefsrv.mcd_execute_txlist_f(core.coreptr, transaction)
            if (error != MCD_ERR_NONE):
                return error
            p = transaction.tx.contents.data
            print(p[3], p[2], p[1], p[0])
        return error

    def break_routine(self) -> c_uint32:
        error = c_uint32(MCD_ERR_GENERAL)
        for core in self.mcdrefsrv.server.cores:
            error = self.mcdrefsrv.mcd_stop_f(core.coreptr, c_bool(False))
            if (error != MCD_ERR_NONE):
                return error
        return error

    def go_routine(self) -> c_uint32:
        error = c_uint32(MCD_ERR_GENERAL)
        for core in self.mcdrefsrv.server.cores:
            error = self.mcdrefsrv.mcd_run_f(core.coreptr, c_bool(False))
            if (error != MCD_ERR_NONE):
                return error
        return error

    def step_routine(self) -> c_uint32:
        error = c_uint32(MCD_ERR_GENERAL)
        for core in self.mcdrefsrv.server.cores:
            error = self.mcdrefsrv.mcd_step_f(core.coreptr, c_bool(False), c_uint32(2), c_uint32(1))
            if (error != MCD_ERR_NONE):
                return error
        time.sleep(0.1) # wait until the QEMU VM has performed the step
        return error

    def break_init_routine(self) -> c_uint32:
        # This routine makes sure that the cpu is stopped after the setUp
        # As a result all unit tests start with a stopped and reset CPU
        error = self.state_routine()
        if (error != MCD_ERR_NONE):
                return error
        if (self.mcdrefsrv.server.cores[0].last_state.info_str.decode("utf-8") == STATE_STR_INIT_HALTED):
            # we want to transition from halted to stopped --> send a go before the break
            error = self.go_routine()
            if (error != MCD_ERR_NONE):
                return error
        # stop cpu
        error = self.break_routine()
        if (error != MCD_ERR_NONE):
                return error
        # reset pc
        error = self.reset_pc_routine()
        return error

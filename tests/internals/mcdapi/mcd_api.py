from ctypes import*
from typing import List

from .mcd_structs import*

class RegGroup():
    def __init__(self) -> None:
        self.reggroup_info = mcd_register_group_st()
        self.regs: List[mcd_register_info_st] = []

class Core():
    def __init__(self, parent, core_con_info: mcd_core_con_info_st) -> None:
        self.parent = parent
        self.core_con_info = core_con_info
        self.coreptr = pointer(mcd_core_st())
        self.rst_class_vector = c_uint32(0)
        self.reset_classes = [mcd_rst_info_st() for i in range(32)]
        self.trig_info = mcd_trig_info_st()
        self.memspaces: List[mcd_memspace_st] = []
        self.reggroups: List[RegGroup] = []
        self.last_state = mcd_core_state_st()
    
    def get_reg_group(self, reg_group_id: int) -> RegGroup:
        for reggroup in self.reggroups:
            if reggroup.reggroup_info.reg_group_id == reg_group_id:
                return reggroup

    def store_resets(self) -> c_uint32:
        error = self.parent.parent.mcd_qry_rst_classes_f(self.coreptr, self.rst_class_vector)
        if (error != MCD_ERR_NONE):
            return error
        for index, bit in enumerate(reversed("{:032b}".format((self.rst_class_vector.value)))):
            if (bit == '0'):
                continue
            rst_class = c_int8(index)
            error = self.parent.parent.mcd_qry_rst_class_info_f(self.coreptr, rst_class, self.reset_classes[index])
            if (error != MCD_ERR_NONE):
                return error
        return MCD_ERR_NONE

    def store_triggers(self) -> c_uint32:
        error = self.parent.parent.mcd_qry_trig_info_f(self.coreptr, self.trig_info)
        return error

    def store_memspaces(self) -> c_uint32:
        num_memspaces = c_uint32(0)
        error = self.parent.parent.mcd_qry_mem_spaces_f(self.coreptr, c_uint32(0), num_memspaces, mcd_memspace_st())
        if (error != MCD_ERR_NONE):
            return error
        self.memspaces = [mcd_memspace_st() for i in range(num_memspaces.value)]
        for memspace_index in range(num_memspaces.value):
            error = self.parent.parent.mcd_qry_mem_spaces_f(self.coreptr, c_uint32(memspace_index), c_uint32(1), self.memspaces[memspace_index])
            if (error != MCD_ERR_NONE):
                return error
        return MCD_ERR_NONE

    def store_reggroups(self) -> c_uint32:
        num_reggroups = c_uint32(0)
        error = self.parent.parent.mcd_qry_reg_groups_f(self.coreptr, c_uint32(0), num_reggroups, mcd_register_group_st())
        if (error != MCD_ERR_NONE):
            return error
        self.reggroups = [RegGroup() for i in range(num_reggroups.value)]
        for reggroup_index in range(num_reggroups.value):
            error = self.parent.parent.mcd_qry_reg_groups_f(self.coreptr, c_uint32(reggroup_index), c_uint32(1), self.reggroups[reggroup_index].reggroup_info)
            if (error != MCD_ERR_NONE):
                return error
        return MCD_ERR_NONE

    def store_regs(self) -> c_uint32:
        num_regs = c_uint32(0)
        error = self.parent.parent.mcd_qry_reg_map_f(self.coreptr, c_uint32(0), c_uint32(0), num_regs, mcd_register_info_st())
        if (error != MCD_ERR_NONE):
            return error
        for reg_index in range(num_regs.value):
            reg_info = mcd_register_info_st()
            error = self.parent.parent.mcd_qry_reg_map_f(self.coreptr, c_uint32(0), c_uint32(reg_index), c_uint32(1), reg_info)
            if (error != MCD_ERR_NONE):
                return error
            self.get_reg_group(reg_info.reg_group_id).regs.append(reg_info)
        return MCD_ERR_NONE

class Server():
    def __init__(self, parent) -> None:
        self.parent = parent
        self.serverptr = pointer(mcd_server_st())
        self.cores: List[Core] = []

class McdRefSrv():
    def __init__(self, mcdlib_path) -> None:
        self.mcdlib = CDLL(mcdlib_path)
        self.server = Server(self)

    # administrative functions

    def mcd_initialize_f(self, version_req: mcd_api_version_st, impl_info: mcd_impl_version_info_st) -> c_uint32:
        return self.mcdlib.mcd_initialize_f(byref(version_req), byref(impl_info))

    def mcd_open_server_f(self, system_key: c_char, config_string: c_char, server: mcd_server_st) -> c_uint32:
        return self.mcdlib.mcd_open_server_f(byref(system_key), byref(config_string), byref(server))

    def mcd_close_server_f(self, server: mcd_server_st) -> c_uint32:
        return self.mcdlib.mcd_close_server_f(server)

    def mcd_open_core_f(self, core_con_info: mcd_core_con_info_st, core: mcd_core_st) -> c_uint32:
        return self.mcdlib.mcd_open_core_f(byref(core_con_info), byref(core))

    def mcd_close_core_f(self, core: mcd_core_st) -> c_uint32:
        return self.mcdlib.mcd_close_core_f(core)

    def mcd_exit_f(self) -> None:
        self.mcdlib.mcd_exit_f()

    # query functions

    def mcd_qry_systems_f(self, start_index: c_uint32, num_systems: c_uint32, system_con_info: mcd_core_con_info_st) -> c_uint32:
        return self.mcdlib.mcd_qry_systems_f(start_index, byref(num_systems), byref(system_con_info))

    def mcd_qry_devices_f(self, system_con_info: mcd_core_con_info_st, start_index: c_uint32, num_devices: c_uint32, device_con_info: mcd_core_con_info_st) -> c_uint32:
        return self.mcdlib.mcd_qry_devices_f(byref(system_con_info), start_index, byref(num_devices), byref(device_con_info))

    def mcd_qry_cores_f(self, connection_info: mcd_core_con_info_st, start_index: c_uint32, num_cores: c_uint32, core_con_info: mcd_core_con_info_st) -> c_uint32:
        return self.mcdlib.mcd_qry_cores_f(byref(connection_info), start_index, byref(num_cores), byref(core_con_info))

    def mcd_qry_rst_classes_f(self, core: mcd_core_st, rst_class_vector: c_uint32) -> c_uint32:
        return self.mcdlib.mcd_qry_rst_classes_f(core, byref(rst_class_vector))

    def mcd_qry_rst_class_info_f(self, core: mcd_core_st, rst_class: c_int8, rst_info: mcd_rst_info_st) -> c_uint32:
        return self.mcdlib.mcd_qry_rst_class_info_f(core, rst_class, byref(rst_info))

    def mcd_qry_trig_info_f(self, core: mcd_core_st, trig_info: mcd_trig_info_st) -> c_uint32:
        return self.mcdlib.mcd_qry_trig_info_f(core, byref(trig_info))

    def mcd_qry_mem_spaces_f(self, core: mcd_core_st, start_index: c_uint32, num_mem_spaces: c_uint32, mem_spaces: mcd_memspace_st) -> c_uint32:
        return self.mcdlib.mcd_qry_mem_spaces_f(core, start_index, byref(num_mem_spaces), byref(mem_spaces))

    def mcd_qry_reg_groups_f(self, core: mcd_core_st, start_index: c_uint32, num_reg_groups: c_uint32, reg_groups: mcd_register_group_st) -> c_uint32:
        return self.mcdlib.mcd_qry_reg_groups_f(core, start_index, byref(num_reg_groups), byref(reg_groups))

    def mcd_qry_reg_map_f(self, core: mcd_core_st, reg_group_id: c_uint32, start_index: c_uint32, num_regs: c_uint32, reg_info: mcd_register_info_st) -> c_uint32:
        return self.mcdlib.mcd_qry_reg_map_f(core, reg_group_id, start_index, byref(num_regs), byref(reg_info))

    def mcd_qry_state_f(self, core: mcd_core_st, state: mcd_core_state_st) -> c_uint32:
        return self.mcdlib.mcd_qry_state_f(core, byref(state))

    def mcd_qry_error_info_f(self, core: mcd_core_st, error_info: mcd_error_info_st) -> c_uint32:
        return self.mcdlib.mcd_qry_error_info_f(core, error_info)

    # debug command functions

    def mcd_rst_f(self, core: mcd_core_st, rst_class_vector: c_uint32, rst_and_halt: c_bool) -> c_uint32:
        return self.mcdlib.mcd_rst_f(core, rst_class_vector, rst_and_halt)

    def mcd_run_f(self, core: mcd_core_st, global_bool: c_bool) -> c_uint32:
        return self.mcdlib.mcd_run_f(core, global_bool)

    def mcd_stop_f(self, core: mcd_core_st, global_bool: c_bool) -> c_uint32:
        return self.mcdlib.mcd_stop_f(core, global_bool)

    def mcd_step_f(self, core: mcd_core_st, global_bool: c_bool, step_type: c_uint32, n_steps: c_uint32) -> c_uint32:
        return self.mcdlib.mcd_step_f(core, global_bool, step_type, n_steps)

    def mcd_execute_txlist_f(self, core: mcd_core_st, txlist: mcd_txlist_st) -> c_uint32:
        return self.mcdlib.mcd_execute_txlist_f(core, byref(txlist))

    def mcd_create_trig_f(self, core: mcd_core_st, trig: c_void_p, trig_id: c_uint32) -> c_uint32:
        return self.mcdlib.mcd_create_trig_f(core, byref(trig), byref(trig_id))

    def mcd_activate_trig_set_f(self, core: mcd_core_st) -> c_uint32:
        return self.mcdlib.mcd_activate_trig_set_f(core)

    def mcd_remove_trig_f(self, core: mcd_core_st, trig_id: c_uint32) -> c_uint32:
        return self.mcdlib.mcd_remove_trig_f(core, trig_id)

from .mcdapi.mcd_api import*

arm_pc_addr = mcd_addr_st(15, 5, 0, 0)
arm_write_pc_tx_list = mcd_tx_st(arm_pc_addr, 2, 0, 0, 0, pointer(c_uint8(0)), 4, 0)
arm_read_pc_tx_list = mcd_tx_st(arm_pc_addr, 1, 0, 0, 0, pointer(c_uint8(0)), 4, 0)

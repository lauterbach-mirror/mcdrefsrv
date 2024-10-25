from mcd_api import *
import pytest
import logging

LOGGER = logging.getLogger("mcd")

def test_initialize(dll_connected):
    version_req_valid = mcd_api_version_st(1,1)
    version_req_invalid = mcd_api_version_st(1,0)
    impl_info = mcd_impl_version_info_st()
    error_info = mcd_error_info_st()

    ret = mcd_initialize_f(byref(version_req_valid), byref(impl_info))
    assert(ret == mcd_return_et.MCD_RET_ACT_NONE)

    ret = mcd_initialize_f(byref(version_req_invalid), byref(impl_info))
    assert(ret == mcd_return_et.MCD_RET_ACT_HANDLE_ERROR)
    mcd_qry_error_info_f(None, byref(error_info))
    assert(error_info.error_code == mcd_error_code_et.MCD_ERR_GENERAL)

    ret = mcd_initialize_f(None, byref(impl_info))
    assert(ret == mcd_return_et.MCD_RET_ACT_HANDLE_ERROR)
    mcd_qry_error_info_f(None, byref(error_info))
    assert(error_info.error_code == mcd_error_code_et.MCD_ERR_PARAM)

    ret = mcd_initialize_f(byref(version_req_valid), None)
    assert(ret == mcd_return_et.MCD_RET_ACT_HANDLE_ERROR)
    mcd_qry_error_info_f(None, byref(error_info))
    assert(error_info.error_code == mcd_error_code_et.MCD_ERR_PARAM)

def test_exit_mcd(dll_connected):
    mcd_exit_f()
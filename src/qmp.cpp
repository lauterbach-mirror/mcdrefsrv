/*
 * MIT License
 *
 * Copyright (c) 2025 Lauterbach GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * QEMU Machine Protocol (QMP) support for MCD
 * https://wiki.qemu.org/Documentation/QMP
 */

#include <string.h>

#include <iostream>
#include <sstream>

#include "json.hpp"
#include "mcd_rpc.h"

template <typename T>
void json_get_to_optional(const nlohmann::json &j, const char *key, T &obj)
{
    if (j.contains(key)) {
        j.at(key).get_to(obj);
    }
}

#define JSON_STRING_GET_TO_ARRAY(j, key, arr) do {                     \
    const std::string &_s = j.at(key);                                 \
    _s.copy(arr, sizeof(arr));                                         \
} while (0)

void to_json(nlohmann::json &j, const mcd_core_con_info_st &info)
{
    j = nlohmann::json{{"host", info.host},
                       {"server-port", info.server_port},
                       {"server-key", info.server_key},
                       {"system-key", info.system_key},
                       {"device-key", info.device_key},
                       {"system", info.system},
                       {"system-instance", info.system_instance},
                       {"acc-hw", info.acc_hw},
                       {"device-type", info.device_type},
                       {"device", info.device},
                       {"device-id", info.device_id},
                       {"core", info.core},
                       {"core-type", info.core_type},
                       {"core-id", info.core_id}};
}

static void from_json(const nlohmann::json &j, mcd_core_con_info_st &info)
{
    JSON_STRING_GET_TO_ARRAY(j, "host", info.host);
    j.at("server-port").get_to(info.server_port);
    JSON_STRING_GET_TO_ARRAY(j, "server-key", info.server_key);
    JSON_STRING_GET_TO_ARRAY(j, "system-key", info.system_key);
    JSON_STRING_GET_TO_ARRAY(j, "device-key", info.device_key);
    JSON_STRING_GET_TO_ARRAY(j, "system", info.system);
    JSON_STRING_GET_TO_ARRAY(j, "system-instance", info.system_instance);
    JSON_STRING_GET_TO_ARRAY(j, "acc-hw", info.acc_hw);
    j.at("device-type").get_to(info.device_type);
    JSON_STRING_GET_TO_ARRAY(j, "device", info.device);
    j.at("device-id").get_to(info.device_id);
    JSON_STRING_GET_TO_ARRAY(j, "core", info.core);
    j.at("core-type").get_to(info.core_type);
    j.at("core-id").get_to(info.core_id);
}

static void from_json(const nlohmann::json &j, mcd_error_info_st &info)
{
    j.at("return-status").get_to(info.return_status);
    j.at("error-code").get_to(info.error_code);
    j.at("error-events").get_to(info.error_events);
    JSON_STRING_GET_TO_ARRAY(j, "error-str", info.error_str);
}

static void from_json(const nlohmann::json &j, mcd_memspace_st &ms)
{
    j.at("mem-space-id").get_to(ms.mem_space_id);
    JSON_STRING_GET_TO_ARRAY(j, "mem-space-name", ms.mem_space_name);
    j.at("mem-type").get_to(ms.mem_type);
    j.at("bits-per-mau").get_to(ms.bits_per_mau);
    j.at("invariance").get_to(ms.invariance);
    j.at("endian").get_to(ms.endian);
    j.at("min-addr").get_to(ms.min_addr);
    j.at("max-addr").get_to(ms.max_addr);
    j.at("num-mem-blocks").get_to(ms.num_mem_blocks);
    j.at("supported-access-options").get_to(ms.supported_access_options);
    j.at("core-mode-mask-read").get_to(ms.core_mode_mask_read);
    j.at("core-mode-mask-write").get_to(ms.core_mode_mask_write);
}

static void from_json(const nlohmann::json &j, mcd_register_group_st &rg)
{
    j.at("reg-group-id").get_to(rg.reg_group_id);
    JSON_STRING_GET_TO_ARRAY(j, "reg-group-name", rg.reg_group_name);
    j.at("n-registers").get_to(rg.n_registers);
}

static void to_json(nlohmann::json &j, const mcd_addr_st &a)
{
    j = nlohmann::json{{"address", a.address},
                       {"mem-space-id", a.mem_space_id},
                       {"addr-space-id", a.addr_space_id},
                       {"addr-space-type", a.addr_space_type}};
}

static void from_json(const nlohmann::json &j, mcd_addr_st &a)
{
    j.at("address").get_to(a.address);
    j.at("mem-space-id").get_to(a.mem_space_id);
    j.at("addr-space-id").get_to(a.addr_space_id);
    j.at("addr-space-type").get_to(a.addr_space_type);
}

static void from_json(const nlohmann::json &j, mcd_register_info_st &r)
{
    j.at("addr").get_to(r.addr);
    j.at("reg-group-id").get_to(r.reg_group_id);
    JSON_STRING_GET_TO_ARRAY(j, "regname", r.regname);
    j.at("regsize").get_to(r.regsize);
    j.at("core-mode-mask-read").get_to(r.core_mode_mask_read);
    j.at("core-mode-mask-write").get_to(r.core_mode_mask_write);
    j.at("side-effects-read").get_to(r.has_side_effects_read);
    j.at("side-effects-write").get_to(r.has_side_effects_write);
    j.at("reg-type").get_to(r.reg_type);
    j.at("hw-thread-id").get_to(r.hw_thread_id);
}

void to_json(nlohmann::json &j, const mcd_tx_st &tx)
{
    std::vector<uint8_t> data{tx.data, tx.data + tx.num_bytes};
    j = nlohmann::json{
        {"addr", tx.addr},           {"access-type", tx.access_type},
        {"options", tx.options},     {"access-width", tx.access_width},
        {"core-mode", tx.core_mode}, {"data", data},
        {"num-bytes", tx.num_bytes}, {"num-bytes-ok", tx.num_bytes_ok}};
}

void from_json(const nlohmann::json &j, mcd_tx_st &tx)
{
    j.at("addr").get_to(tx.addr);
    j.at("access-type").get_to(tx.access_type);
    j.at("options").get_to(tx.options);
    j.at("access-width").get_to(tx.access_width);
    j.at("core-mode").get_to(tx.core_mode);
    j.at("num-bytes").get_to(tx.num_bytes);
    j.at("num-bytes-ok").get_to(tx.num_bytes_ok);
    const nlohmann::json &arr = j.at("data");
    std::copy(arr.begin(), arr.end(), tx.data);
}

void to_json(nlohmann::json &j, const mcd_txlist_st &l)
{
    std::vector<mcd_tx_st> tx{l.tx, l.tx + l.num_tx};
    j = nlohmann::json{
        {"tx", tx}, {"num-tx", l.num_tx}, {"num-tx-ok", l.num_tx_ok}};
}

void from_json(const nlohmann::json &j, mcd_txlist_st &l)
{
    j.at("num-tx").get_to(l.num_tx);
    j.at("num-tx-ok").get_to(l.num_tx_ok);
    const nlohmann::json &arr = j.at("tx");
    mcd_tx_st *tx = l.tx;
    for (const auto &j_arr : arr) {
        j_arr.get_to(*tx);
        tx++;
    }
    // std::copy(arr.begin(), arr.end(), l.tx);
}

void from_json(const nlohmann::json &j, mcd_trig_info_st &i)
{
    j.at("type").get_to(i.type);
    j.at("option").get_to(i.option);
    j.at("action").get_to(i.action);
    j.at("trig-number").get_to(i.trig_number);
    j.at("state-number").get_to(i.state_number);
    j.at("counter-number").get_to(i.counter_number);
    j.at("sw-breakpoints").get_to(i.sw_breakpoints);
}

static void to_json(nlohmann::json &j, const mcd_trig_simple_core_st &t)
{
    j = nlohmann::json{{"type", t.type},
                       {"option", t.option},
                       {"action", t.action},
                       {"action-param", t.action_param},
                       {"modified", !!t.modified},
                       {"state-mask", t.state_mask},
                       {"addr-start", t.addr_start},
                       {"addr-range", t.addr_range}};
}

void from_json(const nlohmann::json &j, mcd_trig_simple_core_st &t)
{
    t.struct_size = sizeof(mcd_trig_simple_core_st);
    j.at("type").get_to(t.type);
    j.at("option").get_to(t.option);
    j.at("action").get_to(t.action);
    j.at("action-param").get_to(t.action_param);
    j.at("modified").get_to((bool &)t.modified);
    j.at("state-mask").get_to(t.state_mask);
    j.at("addr-start").get_to(t.addr_start);
    j.at("addr-range").get_to(t.addr_range);
}

static void to_json(nlohmann::json &j, const mcd_trig_complex_core_st &t)
{
    j = nlohmann::json{{"type", t.type},
                       {"option", t.option},
                       {"action", t.action},
                       {"action-param", t.action_param},
                       {"modified", !!t.modified},
                       {"state-mask", t.state_mask},
                       {"addr-start", t.addr_start},
                       {"addr-range", t.addr_range},
                       {"data-start", t.data_start},
                       {"data-range", t.data_range},
                       {"data-mask", t.data_mask},
                       {"data-size", t.data_size},
                       {"hw-thread-id", t.hw_thread_id},
                       {"sw-thread-id", t.sw_thread_id},
                       {"core-mode-mask", t.core_mode_mask}};
}

void from_json(const nlohmann::json &j, mcd_trig_complex_core_st &t)
{
    t.struct_size = sizeof(mcd_trig_complex_core_st);
    j.at("type").get_to(t.type);
    j.at("option").get_to(t.option);
    j.at("action").get_to(t.action);
    j.at("action-param").get_to(t.action_param);
    j.at("modified").get_to(t.modified);
    j.at("state-mask").get_to(t.state_mask);
    j.at("addr-start").get_to(t.addr_start);
    j.at("addr-range").get_to(t.addr_range);
    j.at("data-start").get_to(t.data_start);
    j.at("data-range").get_to(t.data_range);
    j.at("data-mask").get_to(t.data_mask);
    j.at("data-size").get_to(t.data_size);
    j.at("hw-thread-id").get_to(t.hw_thread_id);
    j.at("sw-thread-id").get_to(t.sw_thread_id);
    j.at("core-mode-mask").get_to(t.core_mode_mask);
}

static void to_json(nlohmann::json &j, const mcd_rpc_trig_st &t)
{
    if (t.is_simple_core) {
        j = nlohmann::json{{"trig-simple-core", *t.simple_core}};
    } else if (t.is_complex_core) {
        j = nlohmann::json{{"trig-complex-core", *t.complex_core}};
    }
}

static void from_json(const nlohmann::json &j, mcd_rpc_trig_st &t)
{
    if (j.contains("trig-simple-core")) {
        if (!t.is_simple_core) {
            throw std::runtime_error(
                "trigger container not big enough for trig_simple_core");
        }
        j.at("trig-simple-core").get_to(*t.simple_core);
        t.is_complex_core = false;
    } else if (j.contains("trig-complex-core")) {
        if (!t.is_complex_core) {
            throw std::runtime_error(
                "trigger container not big enough for trig_complex_core");
        }
        j.at("trig-complex-core").get_to(*t.complex_core);
        t.is_simple_core = false;
    } else {
        t.is_simple_core = false;
        t.is_complex_core = false;
    }
}

void from_json(const nlohmann::json &j, mcd_trig_state_st &s)
{
    j.at("active").get_to(s.active);
    j.at("captured").get_to(s.captured);
    j.at("captured-valid").get_to(s.captured_valid);
    j.at("count-value").get_to(s.count_value);
    j.at("count-valid").get_to(s.count_valid);
}

void from_json(const nlohmann::json &j, mcd_trig_set_state_st &s)
{
    j.at("active").get_to(s.active);
    j.at("state").get_to(s.state);
    j.at("state-valid").get_to(s.state_valid);
    j.at("trig-bus").get_to(s.trig_bus);
    j.at("trig-bus-valid").get_to(s.trig_bus_valid);
    j.at("trace").get_to(s.trace);
    j.at("trace-valid").get_to(s.trace_valid);
    j.at("analysis").get_to(s.analysis);
    j.at("analysis-valid").get_to(s.analysis_valid);
}

void from_json(const nlohmann::json &j, mcd_core_state_st &s)
{
    j.at("state").get_to(s.state);
    j.at("event").get_to(s.event);
    j.at("hw-thread-id").get_to(s.hw_thread_id);
    j.at("trig-id").get_to(s.trig_id);
    JSON_STRING_GET_TO_ARRAY(j, "stop-str", s.stop_str);
    JSON_STRING_GET_TO_ARRAY(j, "info-str", s.info_str);
}

void from_json(const nlohmann::json &j, mcd_rst_info_st &i)
{
    j.at("class-vector").get_to(i.class_vector);
    JSON_STRING_GET_TO_ARRAY(j, "info-str", i.info_str);
}

void from_json(const nlohmann::json &j, mcd_ctrig_info_st &i)
{
    j.at("ctrig-id").get_to(i.ctrig_id);
    JSON_STRING_GET_TO_ARRAY(j, "info-str", i.info_str);
}

static void to_json(nlohmann::json &j, const mcd_open_server_args &args)
{
    j = nlohmann::json{{"system-key", args.system_key},
                       {"config-string", args.config_string}};
}

static void from_json(const nlohmann::json &j, mcd_open_server_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "server-uid", res.server.server_uid);

    if (j.contains("config-string")) {
        const std::string &s{
            j.at("config-string").get_ref<const std::string &>()};
        mcd_char_t *config_string{new mcd_char_t[s.size() + 1]};
        s.copy(config_string, s.size());
        config_string[s.size()] = '\0';
        res.server.config_string = config_string;
    } else {
        res.server.config_string = nullptr;
    }

    if (j.contains("host")) {
        const std::string &s{j.at("host").get_ref<const std::string &>()};
        mcd_char_t *host{new mcd_char_t[s.size() + 1]};
        s.copy(host, s.size());
        host[s.size()] = '\0';
        res.server.host = host;
    } else {
        res.server.host = nullptr;
    }
}

static void to_json(nlohmann::json &j, const mcd_close_server_args &args)
{
    j = nlohmann::json{
        {"server-uid", args.server_uid},
    };
}

static void from_json(const nlohmann::json &j, mcd_close_server_result &res)
{
    j.at("return-status").get_to(res.return_status);
}

static void to_json(nlohmann::json &j, const mcd_qry_systems_args &args)
{
    j = nlohmann::json{
        {"start-index", args.start_index},
        {"num-systems", args.num_systems},
    };
}

static void from_json(const nlohmann::json &j, mcd_qry_systems_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "num-systems", *res.num_systems);
    if (j.contains("system-con-info")) {
        const nlohmann::json &arr = j.at("system-con-info");
        std::copy(arr.begin(), arr.end(), res.system_con_info);
    }
}

static void to_json(nlohmann::json &j, const mcd_qry_devices_args &args)
{
    j = nlohmann::json{
        {"system-con-info", *args.system_con_info},
        {"start-index", args.start_index},
        {"num-devices", args.num_devices},
    };
}

static void from_json(const nlohmann::json &j, mcd_qry_devices_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "num-devices", *res.num_devices);
    if (j.contains("device-con-info")) {
        const nlohmann::json &sj = j.at("device-con-info");
        std::copy(sj.begin(), sj.end(), res.device_con_info);
    }
}

static void to_json(nlohmann::json &j, const mcd_qry_cores_args &args)
{
    j = nlohmann::json{
        {"connection-info", *args.connection_info},
        {"start-index", args.start_index},
        {"num-cores", args.num_cores},
    };
}

static void from_json(const nlohmann::json &j, mcd_qry_cores_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "num-cores", *res.num_cores);
    if (j.contains("core-con-info")) {
        const nlohmann::json &sj = j.at("core-con-info");
        std::copy(sj.begin(), sj.end(), res.core_con_info);
    }
}

static void to_json(nlohmann::json &j, const mcd_open_core_args &args)
{
    j = nlohmann::json{
        {"core-con-info", *args.core_con_info},
    };
}

static void from_json(const nlohmann::json &j, mcd_open_core_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "core-uid", res.core.core_uid);
    if (j.contains("core-con-info")) {
        res.core.core_con_info = new mcd_core_con_info_st{};
        j.at("core-con-info").get_to(*res.core.core_con_info);
    }
}

static void to_json(nlohmann::json &j, const mcd_close_core_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
    };
}

static void from_json(const nlohmann::json &j, mcd_close_core_result &res)
{
    j.at("return-status").get_to(res.return_status);
}

static void to_json(nlohmann::json &j, const mcd_qry_error_info_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
    };
}

static void from_json(const nlohmann::json &j, mcd_qry_error_info_result &res)
{
    *res.error_info = j.template get<mcd_error_info_st>();
}

static void to_json(nlohmann::json &j, const mcd_qry_mem_spaces_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"start-index", args.start_index},
        {"num-mem-spaces", args.num_mem_spaces},
    };
}

static void from_json(const nlohmann::json &j, mcd_qry_mem_spaces_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "num-mem-spaces", *res.num_mem_spaces);
    if (j.contains("mem-spaces")) {
        const nlohmann::json &sj = j.at("mem-spaces");
        std::copy(sj.begin(), sj.end(), res.mem_spaces);
    }
}

static void to_json(nlohmann::json &j, const mcd_qry_reg_groups_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"start-index", args.start_index},
        {"num-reg-groups", args.num_reg_groups},
    };
}

static void from_json(const nlohmann::json &j, mcd_qry_reg_groups_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "num-reg-groups", *res.num_reg_groups);
    if (j.contains("reg-groups")) {
        const nlohmann::json &sj = j.at("reg-groups");
        std::copy(sj.begin(), sj.end(), res.reg_groups);
    }
}

static void to_json(nlohmann::json &j, const mcd_qry_reg_map_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"reg-group-id", args.reg_group_id},
        {"start-index", args.start_index},
        {"num-regs", args.num_regs},
    };
}

static void from_json(const nlohmann::json &j, mcd_qry_reg_map_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "num-regs", *res.num_regs);
    if (j.contains("reg-info")) {
        const nlohmann::json &sj = j.at("reg-info");
        std::copy(sj.begin(), sj.end(), res.reg_info);
    }
}

static void to_json(nlohmann::json &j, const mcd_execute_txlist_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"txlist", *args.txlist},
    };
}

static void from_json(const nlohmann::json &j, mcd_execute_txlist_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "txlist", *res.txlist);
}

static void to_json(nlohmann::json &j, const mcd_qry_trig_info_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
    };
}

static void from_json(const nlohmann::json &j, mcd_qry_trig_info_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "trig-info", *res.trig_info);
}

static void to_json(nlohmann::json &j, const mcd_qry_ctrigs_args &args)
{
    j = nlohmann::json{{"core-uid", args.core_uid},
                       {"start-index", args.start_index},
                       {"num-ctrigs", args.num_ctrigs}};
}

static void from_json(const nlohmann::json &j, mcd_qry_ctrigs_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "num-ctrigs", *res.num_ctrigs);
    if (j.contains("ctrig-info")) {
        const nlohmann::json &sj = j.at("ctrig-info");
        std::copy(sj.begin(), sj.end(), res.ctrig_info);
    }
}

static void to_json(nlohmann::json &j, const mcd_create_trig_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"trig", *args.trig},
    };
}

static void from_json(const nlohmann::json &j, mcd_create_trig_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "trig", *res.trig);
    json_get_to_optional(j, "trig-id", *res.trig_id);
}

static void to_json(nlohmann::json &j, const mcd_qry_trig_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"trig-id", args.trig_id},
    };
}

static void from_json(const nlohmann::json &j, mcd_qry_trig_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "trig", *res.trig);
}

static void to_json(nlohmann::json &j, const mcd_remove_trig_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"trig-id", args.trig_id},
    };
}

static void from_json(const nlohmann::json &j, mcd_remove_trig_result &res)
{
    j.at("return-status").get_to(res.return_status);
}

static void to_json(nlohmann::json &j, const mcd_qry_trig_state_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"trig-id", args.trig_id},
    };
}

static void from_json(const nlohmann::json &j, mcd_qry_trig_state_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "trig-state", *res.trig_state);
}

static void to_json(nlohmann::json &j, const mcd_activate_trig_set_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
    };
}

static void from_json(const nlohmann::json &j,
                      mcd_activate_trig_set_result &res)
{
    j.at("return-status").get_to(res.return_status);
}

static void to_json(nlohmann::json &j, const mcd_remove_trig_set_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
    };
}

static void from_json(const nlohmann::json &j, mcd_remove_trig_set_result &res)
{
    j.at("return-status").get_to(res.return_status);
}

static void to_json(nlohmann::json &j, const mcd_qry_trig_set_args &args)
{
    j = nlohmann::json{{"core-uid", args.core_uid},
                       {"start-index", args.start_index},
                       {"num-trigs", args.num_trigs}};
}

static void from_json(const nlohmann::json &j, mcd_qry_trig_set_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "num-trigs", *res.num_trigs);
    if (j.contains("trig-ids")) {
        const nlohmann::json &sj = j.at("trig-ids");
        std::copy(sj.begin(), sj.end(), res.trig_ids);
    }
}

static void to_json(nlohmann::json &j, const mcd_qry_trig_set_state_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
    };
}

static void from_json(const nlohmann::json &j,
                      mcd_qry_trig_set_state_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "trig-state", *res.trig_state);
}

static void to_json(nlohmann::json &j, const mcd_run_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"global", args.global},
    };
}

static void from_json(const nlohmann::json &j, mcd_run_result &res)
{
    j.at("return-status").get_to(res.return_status);
}

static void to_json(nlohmann::json &j, const mcd_stop_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"global", args.global},
    };
}

static void from_json(const nlohmann::json &j, mcd_stop_result &res)
{
    j.at("return-status").get_to(res.return_status);
}

static void to_json(nlohmann::json &j, const mcd_step_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"global", args.global},
        {"step-type", args.step_type},
        {"n-steps", args.n_steps},
    };
}

static void from_json(const nlohmann::json &j, mcd_step_result &res)
{
    j.at("return-status").get_to(res.return_status);
}

static void to_json(nlohmann::json &j, const mcd_set_global_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"enable", args.enable},
    };
}

static void from_json(const nlohmann::json &j, mcd_set_global_result &res)
{
    j.at("return-status").get_to(res.return_status);
}

static void to_json(nlohmann::json &j, const mcd_qry_state_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
    };
}

static void from_json(const nlohmann::json &j, mcd_qry_state_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "state", *res.state);
}

static void to_json(nlohmann::json &j, const mcd_qry_rst_classes_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
    };
}

static void from_json(const nlohmann::json &j, mcd_qry_rst_classes_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "rst-class-vector", *res.rst_class_vector);
}

static void to_json(nlohmann::json &j, const mcd_qry_rst_class_info_args &args)
{
    j = nlohmann::json{
        {"core-uid", args.core_uid},
        {"rst-class", args.rst_class},
    };
}

static void from_json(const nlohmann::json &j,
                      mcd_qry_rst_class_info_result &res)
{
    j.at("return-status").get_to(res.return_status);
    json_get_to_optional(j, "rst-info", *res.rst_info);
}

static void to_json(nlohmann::json &j, const mcd_rst_args &args)
{
    j = nlohmann::json{{"core-uid", args.core_uid},
                       {"rst-class-vector", args.rst_class_vector},
                       {"rst-and-halt", args.rst_and_halt}};
}

static void from_json(const nlohmann::json &j, mcd_rst_result &res)
{
    j.at("return-status").get_to(res.return_status);
}

struct ostreambuf : public std::streambuf {
private:
    char_type *const buffer;

public:
    ostreambuf(char_type *buffer, std::streamsize bufferLength) : buffer(buffer)
    {
        std::streambuf::setp(buffer, buffer + bufferLength);
    }

    virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                             std::ios_base::openmode which) override
    {
        if (!(which == std::ios_base::out && dir == std::ios_base::cur))
            throw std::runtime_error("seekoff parameters not supported");

        pbump(static_cast<int>(off));
        return pptr() - pbase();
    }
};

uint32_t marshal_mcd_exit(char *buf, size_t buf_size)
{
    ostreambuf b{buf, (std::streamsize)buf_size};
    std::ostream json_output{&b};
    json_output << nlohmann::json{{"execute", "mcd-exit"}};
    return (uint32_t)json_output.tellp();
}

uint32_t marshal_mcd_open_server_args(mcd_open_server_args const *args,
                                      char *buf, size_t buf_size)
{
    ostreambuf b{buf, (std::streamsize)buf_size};
    std::ostream json_output{&b};
    json_output << nlohmann::json{{"execute", "mcd-open-server"},
                                  {"arguments", *args}};
    return (uint32_t)json_output.tellp();
}

mcd_return_et unmarshal_mcd_open_server_result(char const *buf,
                                               mcd_open_server_result *res,
                                               mcd_error_info_st *error_info)
{
    const char *json_line = buf;
    while (*json_line != '\0') {
        /* find next occurence of '\n' or '\0'; */
        size_t len{1};
        while (json_line[len] != '\n' && json_line[len] != '\0') {
            len++;
        }
        std::string_view line{json_line, len};
        try {
            nlohmann::json response = nlohmann::json::parse(line);
            response.at("return").get_to(*res);
            return MCD_RET_ACT_NONE;
        } catch (const std::exception &) {
            /* skip line */
        }
        json_line += len;
    }
    /* not found */
    return MCD_RET_ACT_HANDLE_ERROR;
}

#define DEFINE_QMP(function, qmp)                                              \
    uint32_t marshal_##function##_args(function##_args const *args,            \
                                       char *buf, size_t buf_size)             \
    {                                                                          \
        ostreambuf b{buf, (std::streamsize)buf_size};                          \
        std::ostream json_output{&b};                                          \
        json_output << nlohmann::json{{"execute", qmp}, {"arguments", *args}}; \
        return (uint32_t)json_output.tellp();                                  \
    }                                                                          \
    mcd_return_et unmarshal_##function##_result(char const *buf,               \
                                                function##_result *res,        \
                                                mcd_error_info_st *error_info) \
    {                                                                          \
        const char *json_line = buf;                                           \
        while (*json_line != '\0') {                                           \
            /* find next occurence of '\n' or '\0'; */                         \
            size_t len{1};                                                     \
            while (json_line[len] != '\n' && json_line[len] != '\0') {         \
                len++;                                                         \
            }                                                                  \
            std::string_view line{json_line, len};                             \
            try {                                                              \
                nlohmann::json response = nlohmann::json::parse(line);         \
                response.at("return").get_to(*res);                            \
                return MCD_RET_ACT_NONE;                                       \
            } catch (const std::exception &) {                                 \
                /* skip line */                                                \
            }                                                                  \
            json_line += len;                                                  \
        }                                                                      \
        /* not found */                                                        \
        return MCD_RET_ACT_HANDLE_ERROR;                                       \
    }

DEFINE_QMP(mcd_close_server, "mcd-close-server")
DEFINE_QMP(mcd_qry_systems, "mcd-qry-systems")
DEFINE_QMP(mcd_qry_devices, "mcd-qry-devices")
DEFINE_QMP(mcd_qry_cores, "mcd-qry-cores")
DEFINE_QMP(mcd_open_core, "mcd-open-core")
DEFINE_QMP(mcd_close_core, "mcd-close-core")
DEFINE_QMP(mcd_qry_error_info, "mcd-qry-error-info")
DEFINE_QMP(mcd_qry_mem_spaces, "mcd-qry-mem-spaces")
DEFINE_QMP(mcd_qry_reg_groups, "mcd-qry-reg-groups")
DEFINE_QMP(mcd_qry_reg_map, "mcd-qry-reg-map")
DEFINE_QMP(mcd_execute_txlist, "mcd-execute-txlist")
DEFINE_QMP(mcd_qry_trig_info, "mcd-qry-trig-info")
DEFINE_QMP(mcd_qry_ctrigs, "mcd-qry-ctrigs")
DEFINE_QMP(mcd_create_trig, "mcd-create-trig")
DEFINE_QMP(mcd_qry_trig, "mcd-qry-trig")
DEFINE_QMP(mcd_remove_trig, "mcd-remove-trig")
DEFINE_QMP(mcd_qry_trig_state, "mcd-qry-trig-state")
DEFINE_QMP(mcd_activate_trig_set, "mcd-activate-trig-set")
DEFINE_QMP(mcd_remove_trig_set, "mcd-remove-trig-set")
DEFINE_QMP(mcd_qry_trig_set, "mcd-qry-trig-set")
DEFINE_QMP(mcd_qry_trig_set_state, "mcd-qry-trig-set-state")
DEFINE_QMP(mcd_run, "mcd-run")
DEFINE_QMP(mcd_stop, "mcd-stop")
DEFINE_QMP(mcd_step, "mcd-step")
DEFINE_QMP(mcd_set_global, "mcd-set-global")
DEFINE_QMP(mcd_qry_state, "mcd-qry-state")
DEFINE_QMP(mcd_qry_rst_classes, "mcd-qry-rst-classes")
DEFINE_QMP(mcd_qry_rst_class_info, "mcd-qry-rst-class-info")
DEFINE_QMP(mcd_rst, "mcd-rst")

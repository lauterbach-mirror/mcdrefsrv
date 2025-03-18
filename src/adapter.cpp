/*
MIT License

Copyright (c) 2024 Lauterbach GmbH

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "adapter.hpp"

const mcd_error_info_st MCD_ERROR_INVALID_NULL_PARAM{
    .return_status{MCD_RET_ACT_HANDLE_ERROR},
    .error_code{MCD_ERR_PARAM},
    .error_events{MCD_ERR_EVT_NONE},
    .error_str{"null was invalidly passed as a parameter"},
};

TxAdapter::TxAdapter()
    : server_access{std::nullopt}, requires_server_access{false}
{
}

void TxAdapter::grant_server_access(const mcd_core_st *core)
{
    if (!requires_server_access) {
        /* avoid unnecessary operations */
        return;
    }

    server_access = [c = core](mcd_txlist_st *txlist,
                               mcd_error_info_st &error) {
        mcd_return_et ret{mcd_execute_txlist_f(c, txlist)};
        if (ret != MCD_RET_ACT_NONE) {
            mcd_qry_error_info_f(c, &error);
        }
        return ret;
    };
}

void TxAdapter::free_server_request(mcd_txlist_st &&server_request)
{
    for (uint32_t i = 0; i < server_request.num_tx; i++) {
        delete server_request.tx[i].data;
    }
    delete server_request.tx;
}

mcd_return_et TxAdapter::convert_address_to_server(mcd_addr_st &addr,
                                                   mcd_error_info_st &error)
{
    error = {
        .return_status{MCD_RET_ACT_HANDLE_ERROR},
        .error_code{MCD_ERR_PARAM},
        .error_events{MCD_ERR_EVT_NONE},
        .error_str{"address conversion not implemented for current adapter"},
    };

    return error.return_status;
}

TxAdapter *PassthroughTxAdapter::clone()
{
    return new PassthroughTxAdapter{*this};
}

mcd_return_et PassthroughTxAdapter::convert_address_to_server(
    mcd_addr_st &addr, mcd_error_info_st &error)
{
    return MCD_RET_ACT_NONE;
}

mcd_return_et PassthroughTxAdapter::yield_server_request(
    mcd_tx_st &client_request, mcd_txlist_st &server_request,
    mcd_error_info_st &)
{
    server_request = {
        .tx{&client_request},
        .num_tx{1},
        .num_tx_ok{0},
    };

    return MCD_RET_ACT_NONE;
}

void PassthroughTxAdapter::free_server_request(mcd_txlist_st &&)
{
    /* do nothing since nothing has been allocated */
}

mcd_return_et PassthroughTxAdapter::collect_client_response(
    mcd_tx_st &client_response, const mcd_txlist_st &server_response,
    mcd_error_info_st &error)
{
    if (server_response.num_tx != 1 || server_response.num_tx_ok != 1) {
        error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_TXLIST_TX},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"Server responded with an invalid amount of ok "
                       "transactions"},
        };
    }

    const mcd_tx_st &server_tx{server_response.tx[0]};
    if (server_tx.num_bytes != client_response.num_bytes ||
        server_tx.num_bytes != server_tx.num_bytes_ok) {
        error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_TXLIST_TX},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{"Server responded with an invalid amount of ok bytes"},
        };
    }

    uint8_t *client_data{client_response.data};
    client_response = server_tx;
    client_response.data = client_data;
    for (uint32_t i = 0; i < server_tx.num_bytes; i++) {
        client_response.data[i] = server_tx.data[i];
    }

    return MCD_RET_ACT_NONE;
}

MemorySpace::MemorySpace(const mcd_memspace_st &info, TxAdapter *tx_adapter)
    : info{info}, tx_adapter{tx_adapter}
{
}

MemorySpace::MemorySpace(const MemorySpace &other)
    : info{other.info}, tx_adapter{other.tx_adapter->clone()}
{
}

MemorySpace::MemorySpace(MemorySpace &&other)
    : info{other.info}, tx_adapter{other.tx_adapter}
{
    other.tx_adapter = 0;
}

MemorySpace::~MemorySpace()
{
    if (tx_adapter) delete tx_adapter;
};

MemorySpace &MemorySpace::operator=(const MemorySpace &other)
{
    if (this != &other) {
        if (this->tx_adapter) {
            delete this->tx_adapter;
        }

        this->tx_adapter = other.tx_adapter->clone();
    }

    return *this;
}

MemorySpace &MemorySpace::operator=(MemorySpace &&other)
{
    if (this != &other) {
        if (this->tx_adapter) {
            delete this->tx_adapter;
        }

        this->tx_adapter = other.tx_adapter;
        other.tx_adapter = 0;
    }

    return *this;
}

TxAdapter *MemorySpace::get_tx_adapter() const { return tx_adapter; }

Core::Core(const mcd_core_con_info_st &info, uint32_t core_uid)
    : info{info}, core_uid{core_uid}, updated{false}
{
}

mcd_return_et Core::update_core_database(mcd_error_info_st &mcd_error)
{
    mcd_core_st unbound_core{
        .instance{this},
        .core_con_info{&this->info},
    };

    uint32_t num_mem_spaces{0}, num_reg_groups{0};

    this->server_memory_spaces.clear();
    this->server_register_groups.clear();

    if (mcd_qry_mem_spaces_f(&unbound_core, 0, &num_mem_spaces, nullptr) !=
        MCD_RET_ACT_NONE) {
        return MCD_RET_ACT_HANDLE_ERROR;
    }

    for (uint32_t i = 0; i < num_mem_spaces; i++) {
        mcd_memspace_st ms;
        uint32_t num = 1;
        if (mcd_qry_mem_spaces_f(&unbound_core, i, &num, &ms) !=
            MCD_RET_ACT_NONE) {
            return MCD_RET_ACT_HANDLE_ERROR;
        }

        MemorySpace mem_space{ms, new PassthroughTxAdapter{}};
        this->server_memory_spaces.push_back(std::move(mem_space));
    }

    if (mcd_qry_reg_groups_f(&unbound_core, 0, &num_reg_groups, nullptr) !=
        MCD_RET_ACT_NONE) {
        mcd_qry_error_info_f(&unbound_core, &mcd_error);
        return mcd_error.return_status;
    }

    for (uint32_t i = 0; i < num_reg_groups; i++) {
        mcd_register_group_st rg;
        uint32_t num = 1;
        if (mcd_qry_reg_groups_f(&unbound_core, i, &num, &rg) !=
            MCD_RET_ACT_NONE) {
            mcd_qry_error_info_f(&unbound_core, &mcd_error);
            return mcd_error.return_status;
        }

        mcd_register_info_st *regs{new mcd_register_info_st[rg.n_registers]};

        if (mcd_qry_reg_map_f(&unbound_core, rg.reg_group_id, 0,
                              &rg.n_registers, regs) != MCD_RET_ACT_NONE) {
            mcd_qry_error_info_f(&unbound_core, &mcd_error);
            return mcd_error.return_status;
        }

        RegGroup reg_group{
            .info{rg},
            .registers{regs, regs + rg.n_registers},
        };

        this->server_register_groups.push_back(std::move(reg_group));

        delete[] regs;
    }

    if (this->convert_server_data_to_client(mcd_error) != MCD_RET_ACT_NONE) {
        return mcd_error.return_status;
    }

    this->updated = true;
    return MCD_RET_ACT_NONE;
}

bool Core::core_database_updated() const { return this->updated; }

mcd_return_et Core::query_mem_spaces(uint32_t start_index,
                                     uint32_t *num_mem_spaces,
                                     mcd_memspace_st *mem_spaces,
                                     mcd_error_info_st &error) const
{
    if (*num_mem_spaces == 0) {
        *num_mem_spaces = (uint32_t)this->client_memory_spaces.size();
    } else {
        if (!mem_spaces) {
            error = MCD_ERROR_INVALID_NULL_PARAM;
            return error.return_status;
        }

        for (uint32_t i = 0; i < *num_mem_spaces; i++) {
            uint32_t mem_space_index{i + start_index};
            if (mem_space_index >= this->client_memory_spaces.size()) {
                error = {
                    .return_status{MCD_RET_ACT_HANDLE_ERROR},
                    .error_code{MCD_ERR_PARAM},
                    .error_events{MCD_ERR_EVT_NONE},
                    .error_str{
                        "mem_space_index is equal or larger than the number "
                        "of available memory spaces"},
                };
                return error.return_status;
            }

            mem_spaces[i] = this->client_memory_spaces.at(mem_space_index).info;
        }
    }

    return MCD_RET_ACT_NONE;
}

mcd_return_et Core::query_reg_groups(uint32_t start_index,
                                     uint32_t *num_reg_groups,
                                     mcd_register_group_st *reg_groups,
                                     mcd_error_info_st &error) const
{
    if (*num_reg_groups == 0) {
        *num_reg_groups = (uint32_t)this->client_register_groups.size();
    } else {
        if (!reg_groups) {
            error = MCD_ERROR_INVALID_NULL_PARAM;
            return error.return_status;
        }

        for (uint32_t i = 0; i < *num_reg_groups; i++) {
            uint32_t reg_group_index{i + start_index};
            if (reg_group_index >= this->client_register_groups.size()) {
                error = {
                    .return_status{MCD_RET_ACT_HANDLE_ERROR},
                    .error_code{MCD_ERR_PARAM},
                    .error_events{MCD_ERR_EVT_NONE},
                    .error_str{
                        "reg_group_index is equal or larger than the number "
                        "of available register groups"},
                };
                return error.return_status;
            }

            reg_groups[i] =
                this->client_register_groups.at(reg_group_index).info;
        }
    }

    return MCD_RET_ACT_NONE;
}

mcd_return_et Core::query_reg_map(uint32_t reg_group_id, uint32_t start_index,
                                  uint32_t *num_regs,
                                  mcd_register_info_st *reg_info,
                                  mcd_error_info_st &error) const
{
    std::vector<RegGroup>::const_iterator rg_begin{
        this->client_register_groups.begin()};
    std::vector<RegGroup>::const_iterator rg_end{
        this->client_register_groups.end()};

    if (reg_group_id != 0) {
        while (rg_begin < rg_end) {
            if (rg_begin->info.reg_group_id == reg_group_id) {
                rg_end = rg_begin + 1;
                break;
            } else {
                rg_begin++;
            }
        }
    }

    if (rg_begin == this->client_register_groups.end()) {
        error = {
            .return_status{MCD_RET_ACT_HANDLE_ERROR},
            .error_code{MCD_ERR_REG_GROUP_ID},
            .error_events{MCD_ERR_EVT_NONE},
            .error_str{},
        };
        return error.return_status;
    }

    if (*num_regs == 0) {
        do {
            *num_regs += rg_begin->info.n_registers;
        } while (++rg_begin < rg_end);
        return MCD_RET_ACT_NONE;
    }

    if (!reg_info) {
        error = MCD_ERROR_INVALID_NULL_PARAM;
        return error.return_status;
    }

    int32_t offset{(int32_t)start_index};
    while (offset > 0 && rg_begin < rg_end) {
        offset -= rg_begin->info.n_registers;
        rg_begin++;
    }

    if (offset < 0) {
        rg_begin--;
    }

    offset *= -1;

    for (uint32_t i = 0; i < *num_regs; i++) {
        if (offset >= (int32_t)rg_begin->info.n_registers) {
            offset = 0;
            rg_begin++;
        }

        if (rg_begin >= rg_end) {
            error = {
                .return_status{MCD_RET_ACT_HANDLE_ERROR},
                .error_code{MCD_ERR_PARAM},
                .error_events{MCD_ERR_EVT_NONE},
                .error_str{"reg_index is equal or larger than the number of "
                           "available registers"},
            };
            return error.return_status;
        }

        reg_info[i] = rg_begin->registers.at(offset);
        offset++;
    }

    return MCD_RET_ACT_NONE;
}

mcd_return_et Core::get_tx_adapter(const mcd_addr_st &addr,
                                   TxAdapter **tx_adapter,
                                   mcd_error_info_st &error) const
{
    for (const MemorySpace &ms : this->client_memory_spaces) {
        if (ms.info.mem_space_id == addr.mem_space_id) {
            *tx_adapter = ms.get_tx_adapter();
            return MCD_RET_ACT_NONE;
        }
    }

    for (const MemorySpace &ms : this->server_memory_spaces) {
        if (ms.info.mem_space_id == addr.mem_space_id) {
            *tx_adapter = ms.get_tx_adapter();
            return MCD_RET_ACT_NONE;
        }
    }

    error = {
        .return_status{MCD_RET_ACT_HANDLE_ERROR},
        .error_code{MCD_ERR_PARAM},
        .error_events{MCD_ERR_EVT_NONE},
        .error_str{"unknown memory space ID"},
    };

    return error.return_status;
}

mcd_return_et Core::convert_address_to_server(mcd_addr_st &addr,
                                              mcd_error_info_st &error) const
{
    TxAdapter *tx_adapter;
    if (get_tx_adapter(addr, &tx_adapter, error) != MCD_RET_ACT_NONE) {
        return error.return_status;
    }

    if (tx_adapter) {
        return tx_adapter->convert_address_to_server(addr, error);
    }
}
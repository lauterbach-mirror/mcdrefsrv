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

#pragma once

#include <optional>
#include <functional>

#include "mcd_api.h"

/* There are cases in which the MCD client and server interpret transactions
 * differently.
 * Suppose the following situation:
 * The MCD client wants to access a register with address C.
 * At the MCD server, the register is accessed via the address S != C and
 * requires another register to be set beforehand. Furthermore, the server
 * provides the register's data in a different form.
 * 
 * The issued transaction by the client has to result in the following
 * operations:
 * 1. Set other register
 * 2. Change address from C to S and convert data
 * 3. Access register
 * 4. Reset other register
 * 5. Convert data and change address from S to C
 * 
 * MCD's transaction lists cover this problem nicely:
 * 1. An issued transaction results in a transaction list
 * 2. The transaction list is transmitted to the server
 * 3. The resulting transaction list is converted back into the issued
 * transaction
 * 
 * Note: The main difficulty arises in resetting original values (4. in the
 * above example): When the adapter is instantiated, the register content is
 * unknown! The problem can be solved by a callback function such that the
 * adapter can issue transactions on its own.
 */

class TxAdapter
{
protected:
    std::optional<
        std::function<mcd_return_et(mcd_txlist_st *, mcd_error_info_st &)>>
        server_access;
    bool requires_server_access;

public:
    TxAdapter();
    virtual TxAdapter *clone() = 0;
    virtual ~TxAdapter() = default;

    void grant_server_access(const mcd_core_st *core);

    /* Allocates memory. The memory is owned be the caller */
    /* requires callback function for any additional transactions */
    /* might throw a mcd*/
    virtual mcd_return_et yield_server_request(mcd_tx_st &client_request,
                                               mcd_txlist_st &server_request,
                                               mcd_error_info_st &error) = 0;

    /* Note to implementors: Override in derived class if server_request is
     * allocated differently */
    virtual void free_server_request(mcd_txlist_st &&server_request);

    /* Consumes the transaction list and deallocates any memory. */
    virtual mcd_return_et collect_client_response(
        mcd_tx_st &client_response, const mcd_txlist_st &server_response,
        mcd_error_info_st &error) = 0;

    virtual mcd_return_et convert_address_to_server(mcd_addr_st &addr,
                                                    mcd_error_info_st &error);
};

struct PassthroughTxAdapter : TxAdapter {
    virtual TxAdapter *clone() override;

    virtual mcd_return_et yield_server_request(
        mcd_tx_st &client_request, mcd_txlist_st &server_request,
        mcd_error_info_st &error) override;

    virtual void free_server_request(mcd_txlist_st &&) override;

    virtual mcd_return_et collect_client_response(
        mcd_tx_st &client_response, const mcd_txlist_st &server_response,
        mcd_error_info_st &error) override;

    virtual mcd_return_et convert_address_to_server(
        mcd_addr_st &addr, mcd_error_info_st &error) override;
};

class MemorySpace
{
    TxAdapter *tx_adapter;

public:
    const mcd_memspace_st info;

    MemorySpace() = delete;
    MemorySpace(const mcd_memspace_st &info, TxAdapter *tx_adapter);
    MemorySpace(const MemorySpace &other);
    MemorySpace(MemorySpace &&other);
    ~MemorySpace();

    MemorySpace &operator=(const MemorySpace &other);
    MemorySpace &operator=(MemorySpace &&other);

    TxAdapter *get_tx_adapter() const;
};

struct RegGroup {
    mcd_register_group_st info;
    std::vector<mcd_register_info_st> registers;
};

class Core
{
    bool updated;
    std::vector<MemorySpace> server_memory_spaces;
    std::vector<RegGroup> server_register_groups;
    std::vector<MemorySpace> client_memory_spaces;
    std::vector<RegGroup> client_register_groups;

    /** \brief Converts the server-side core database to a client-side view.
     *
     * When this function is called, the server-side core database is already
     * fetched and the information about memory spaces and registers are in
     * server_memory_spaces and server_register_groups, respectively.
     *
     * After the function returns, client_memory_spaces and
     * client_register_groups are filled. When the core information gets queried
     * by the client, those will be provided.
     *
     * Note to implementors: If your client expects the core information
     * differently than provided by the server, this has to be known at
     * compile-time. You can then implement this function in a different file
     * and adjust CMake accordingly.
     */
    mcd_return_et convert_server_data_to_client(mcd_error_info_st &mcd_error);

public:
    const mcd_core_con_info_st info;

    /** \brief Core UID as provided by the server */
    const uint32_t core_uid;

    /**
     * \brief Initializes a new \c Core instance.
     *
     * @param host Host name of the server socket.
     * @param port TCP port number of the server socket.
     */
    Core(const mcd_core_con_info_st &info, uint32_t core_uid);

    /** \brief Fetches server-side information about registers, register groups
     * and memory spaces and converts them for the client.
     */
    mcd_return_et update_core_database(mcd_error_info_st &mcd_error);

    bool core_database_updated() const;

    /** \brief Provides the register groups to the client.
     */
    mcd_return_et query_reg_groups(uint32_t start_index,
                                   uint32_t *num_reg_groups,
                                   mcd_register_group_st *reg_groups,
                                   mcd_error_info_st &error) const;

    /** \brief Provides the memory spaces to the client.
     */
    mcd_return_et query_mem_spaces(uint32_t start_index,
                                   uint32_t *num_mem_spaces,
                                   mcd_memspace_st *mem_spaces,
                                   mcd_error_info_st &error) const;

    /** \brief Provides the registers to the client.
     */
    mcd_return_et query_reg_map(uint32_t reg_group_id, uint32_t start_index,
                                uint32_t *num_regs,
                                mcd_register_info_st *reg_info,
                                mcd_error_info_st &error) const;

    /** \brief Returns a reference to a \c TxAdapter for a client's transaction.
     */
    mcd_return_et get_tx_adapter(const mcd_addr_st &addr,
                                 TxAdapter **tx_adapter,
                                 mcd_error_info_st &error) const;

    mcd_return_et convert_address_to_server(mcd_addr_st &addr,
                                            mcd_error_info_st &error) const;
};

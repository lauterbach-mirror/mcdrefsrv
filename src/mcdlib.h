/*
MIT License

Copyright (c) 2023 Lauterbach GmbH, Nicolas Eder

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

/* 
 * This header includes all library specific functions classes and defines.
 * The used MCD API structs can be found in mcd_api.h.
 * The shared defines between the mcdstub and this library can be found in mcd_shared_defines.h.
 */

#pragma once

#include "mcd_api.h"
#include "mcd_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <memory>
#include <map>

#if defined(WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())
#define SHUTDOWN_ALL SD_BOTH
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#define SOCKET int
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define GETSOCKETERRNO() (errno)
#define SHUTDOWN_ALL SHUT_RDWR
#endif

/* start defines */

/* TODO: needed because of faulty windows compiler, could be improved with newer compiler */
#define STRING_TO_CHAR(a) const_cast<char*>(a.c_str())
#define STRING_TO_INT(s) atoi(s.c_str())

/* tcp transmission defines */
#if defined(WIN32)
#define MCD_DEFAULT_IPV4 "127.0.0.1"
#else
#define MCD_DEFAULT_IPV4 "0.0.0.0"
#endif
#define MCD_LOCALHOST_STR "localhost"
#define MCD_TCP_DATALEN	1000
#define MCD_TCP_PACKETLEN 1024
#define MCD_MAX_RECEIVED_DATALEN 4096

/* config_string arguments */
#define CONFIG_STR_ARG_HOST "MCDHostName"
#define CONFIG_STR_ARG_PORT "McdServerPort"

/* tcp parsing states */
#define RS_IDLE	1
#define RS_GETLINE 2
#define RS_DATAEND 3
#define RS_CONTINUE 4

/* internal error codes */
#define ERROR_NONE 0
#define ERROR_GENERAL -1
#define ERROR_READ 1
#define ERROR_WRITE 2
#define ERROR_TRANSACTON 3

/* info about this library */
#define MCD_LIB_VENDOR "Lauterbach Engineering"
#define MCD_LIB_BUILD (strtoul(REVISION_STRING + 0,0,0))
#define MCD_LIB_DATE __DATE__
#define MCD_API_IMP_DATE_LEN 16 /* this is hardcoded in mcd_api.h */

/* error messages */
#define ERROR_STRING_STANDARD "No specific error message set"

/* end defines */

/* start classes */

class MCDServer;
class System;
class Device;
class Core;
class RegGroup;
class MCDError;

/**
 * \brief A register group as in the MCD API.
 */

class RegGroup {
	std::string reg_group_name;
	uint32_t reg_group_id;
	std::vector<mcd_register_info_st> registers;
public:
	/**
	 * \brief Class constructor.
	 */
	RegGroup(std::string i_name, uint32_t reg_group_id);
	/**
	 * \brief Provides info for the register group query.
	 * 
	 * This functions copies local data on the register group into i_reg_groups.
	 * @param[out] i_reg_groups Holds query info.
	 */
	void get_qry_info(mcd_register_group_st *i_reg_groups);
	/**
	 * \brief Adds a registers to the registers vector.
	 * 
	 * @param[in] i_system Register to be added.
	 */
	void add_register(mcd_register_info_st i_register);
	/**
	 * \brief Provides info for the register query.
	 * 
	 * This functions copies local data on the registers into i_reg_info.
	 * @param[in] i_start_index First requested register.
	 * @param[in] i_num_regs Number of requested registers.
	 * @param[out] i_reg_info Holds query info.
	 */
	void qry_get_register_data(uint32_t i_start_index, uint32_t i_num_regs, mcd_register_info_st *i_reg_info);
	/**
	 * \brief Returns the number of currently stored registers in this group.
	 */
	uint32_t get_num_registers();
};

/**
 * \brief A core as in the MCD API.
 * 
 * The core class is the lowest element in the target data structure.
 * Before a core can be debugged the open_core function of the MCDServer has to be called.
 * After debugging the close_core function is called. For SMP debugging multiple cores can be opened at the same time.
 * It is also possible to debug an SMP system with one core instance and multiple RegGroups as explained in \ref store_reg_group_info.
 */
class Core {
	std::string core;
	uint32_t core_id;
	std::map<uint8_t, std::string> reset_info;
	uint32_t reset_classes;
	mcd_trig_info_st trigger_info;
	std::map<uint32_t, mcd_memspace_st> mem_spaces;
	std::map<uint32_t, RegGroup> reg_groups;
	std::map<uint32_t, uint32_t> opcode_lookup;
	std::map<uint32_t, mcd_trig_simple_core_st> trigger;
	std::map<uint32_t, uint8_t> active_trigger;
	uint32_t opcode_memspace_id;
	uint32_t id_memspace_id;
public:
	/**
	 * \brief Default class constructor. Only for implicit calls.
	 */
	Core();
	/**
	 * \brief Class constructor.
	 */
	Core(std::string i_core, uint32_t i_core_id);
	/**
	 * \brief Provides info for the cores query.
	 * 
	 * This functions copies local data on the core into i_core_con_info.
	 * @param[out] i_core_con_info Holds query info.
	 */
	void get_qry_info(mcd_core_con_info_st *i_core_con_info);
	/**
	 * \brief Provides info for the state query.
	 * 
	 * The state gets queried multiple times a second.
	 * This function requests the current state of the core from the mcdstub.
	 * Then it translates that data into MCD API language and copies it to i_state.
	 * The data also includes which breakpoint was hit, when in debug state.
	 * @param[out] i_state Holds query info.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	int get_qry_state(MCDServer& i_server, mcd_core_state_st* i_state);
	/**
	 * \brief Provides info for the trigger query.
	 * 
	 * @param[out] i_trig_info Holds query info.
	 */
	void get_trigger_info(mcd_trig_info_st *i_trig_info);
	/**
	 * \brief Stores trigger info from the mcdstub.
	 * 
	 * This function requests all available trigger types and their options from the mcdstub.
	 * Then it stores them into the trigger_info struct.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	void store_trigger_info(MCDServer &i_server);
	/**
	 * \brief Stores reset info from the mcdstub.
	 * 
	 * This function requests all available resets from the mcdstub.
	 * Each reset is requested with a separate packet.
	 * Then it stores them into the reset_classes vector and reset_info map.
	 * Every bit which is set to one the vector indicates a reset class. The map stores info for each of these reset classes.
	 * The indeces in the vector are used as keys for the map.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	void store_reset_data(MCDServer &i_server);
	/**
	 * \brief Stores memory spaces info from the mcdstub.
	 * 
	 * This function requests all memory spaces from the mcdstub.
	 * Each memory space is requested with a separate packet.
	 * Then it stores them into the mem_spaces map. The map uses IDs as keys.
	 * The ID zero is reserved and the memory spaces will be presented to TRACE32 in ascending order.
	 * It is important to give the default memory spaces lower IDs than others because TRACE32 tends to be greedy
	 * and selects the first provided spaces as default spaces.
	 * For example the default secure memory space will be the first memory space with the word "Secure" in its title.
	 * (The title must not include "NS" or "Non")
	 * In case a register memory space is received.
	 * It gets checked weither it has \c MCD_GRP_KEYWORD or \c MCD_CP_KEYWORD in its name.
	 * If the former is true, this registers in this space will later be accessed by their IDs.
	 * If the latter is true, they will be accessed by their opcode.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	void store_mem_space_info(MCDServer &i_server);
	/**
	 * \brief Stores register groups info from the mcdstub.
	 * 
	 * This functions request all register groups from the mcdstub.
	 * Each register group is requested with a separate packet.
	 * Then it stores them into the reg_groups map. The map uses IDs as keys.
	 * The ID zero is reserved and the register groups will be presented to TRACE32 in ascending order.
	 * If wanted the groups can be used for SMP configurations:
	 * In this case only one core instance has to be created and each thread get its own register group with all GPR registers.
	 * The correct thread will be selected with the hw_thread_id of each mcd_register_info_st.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	void store_reg_group_info(MCDServer &i_server);
	/**
	 * \brief Stores register info from the mcdstub.
	 * 
	 * This functions request all registers from the mcdstub.
	 * Each register is requested with a separate packet.
	 * Then it stores them into the correct \ref RegGroup.
	 * While doing this it converts the register name to all uppercase, because thats how TRACE32 expects them.
	 * It also checks whether the register will be accassed by ID or by opcode with the \ref get_reg_access_type function.
	 * If the register gets accessed by opcode, the opcode and original ID of the register are stored in the opcode_lookup map.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	void store_reg_info(MCDServer &i_server);
	/**
	 * \brief Provides the correct info string for a reset class.
	 * 
	 * @param[in] i_reset_class ID of the desired reset class.
	 */
	std::string get_reset_info(uint8_t i_reset_class);
	/**
	 * \brief Returns the local reset_classes vector.
	 */
	uint32_t get_reset_class_vector();
	/**
	 * \brief Returns the number of currently stored memory spaces.
	 */
	uint32_t get_num_mem_spaces();
	/**
	 * \brief Returns the number of currently stored register groups.
	 */
	uint32_t get_num_reg_groups();
	/**
	 * \brief Returns the number of all currently stored registers.
	 */
	uint32_t get_num_registers();
	/**
	 * \brief Returns the number of currently stored trigger.
	 */
	uint32_t get_num_trigger();
	/**
	 * \brief Returns a currently unused, valid trigger id.
	 * 
	 * The trigger ID must not be zero and different from any other trigger ID currently in use.
	 * @param[in,out] i_trig_id Trigger ID to be used.
	 */
	void get_valid_trig_id(uint32_t& i_trig_id);
	/**
	 * \brief Returns the trigger ID corresponding to the parameters.
	 * 
	 * This function translates the custom trigger type to the MCD API trigger type.
	 * Then it searches though all existing trigger for one with the correct type and address.
	 * If no trigger is found, the function returnes zero.
	 * @param[in] i_type Trigger type.
	 * @param[in] i_address Trigger address.
	 * @param[in] i_trigger Map with all trigger.
	 */
	uint32_t find_trigger_id(uint32_t i_type, uint64_t i_address, std::map<uint32_t, mcd_trig_simple_core_st> i_trigger);
	/**
	 * \brief Adds a trigger to the trigger map.
	 * 
	 * @param[in] i_trigger_st Trigger to be added.
	 */
	void add_trigger(uint32_t i_index, mcd_trig_simple_core_st i_trigger_st);
	/**
	 * \brief Activates all trigger.
	 * 
	 * This function activates all trigger, which have not been activated before.
	 * It checks the active_trigger map for already activated trigger and adds newly activated ones to the map.
	 * Finally, it sends a TCP packet to the mcdstub for each newly activated trigger.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	int activate_trigger(MCDServer& i_server);
	/**
	 * \brief Removes a specific trigger.
	 * 
	 * This functions send a packet to the mcdstub to deactivate the trigger with the i_trig_id.
	 * Then it deletes the trigger from the trigger and active_trigger map.
	 * @param[in] i_server MCDServer for TCP connection.
	 * @param[in] i_trig_id ID of the trigger to be removed.
	 */
	int remove_trigger(MCDServer& i_server, uint32_t i_trig_id);
	/**
	 * \brief Provides info for the register query.
	 * 
	 * This function copies data on all the registers into i_reg_info.
	 * @param[in] i_start_index First requested register.
	 * @param[in] i_num_regs Number of requested registers.
	 * @param[out] i_reg_info Holds query info.
	 */
	void qry_get_register_data(uint32_t i_start_index, uint32_t i_num_regs, mcd_register_info_st *i_reg_info);
	/**
	 * \brief Provides info for the memory spaces query.
	 * 
	 * This function copies local data on the requested memory space into i_mem_space_info.
	 * @param[in] i_mem_space_id ID of the requested memory space.
	 * @param[out] i_mem_space_info Holds query info.
	 */
	void qry_get_mem_space_data(uint32_t i_mem_space_id, mcd_memspace_st *i_mem_space_info);
	/**
	 * \brief Returns the memory space with the ID i_mem_space_id.
	 * @param[in] i_mem_space_id ID of the requested memory space.
	 */
	mcd_memspace_st& get_mem_space(uint32_t i_mem_space_id);
	/**
	 * \brief Returns the register group with the ID i_group_id.
	 * 
	 * @param[in] i_group_id Index of the requested register group.
	 */
	RegGroup& get_reg_group(uint32_t i_group_id);
	/**
	 * \brief Calls \ref register_transaction for register access and \ref mem_transaction for memory access.
	 * 
	 * @param[in] i_server MCDServer for TCP connection.
	 * @param[in] i_transaction mcd_tx_st as provided by TRACE32.
	 */
	int execute_transaction(mcd_tx_st* i_transaction, MCDServer &i_server);
	/**
	 * \brief Handles register read and write transactions.
	 * 
	 * Before the access this function determines whether the requested register is accessed by ID or by opcode.
	 * This is done using the opcode_memspace_id member of the core.
	 * If indeed an opcode is provided, it gets translated to an ID with the opcode_lookup map.
	 * Then the actual reading and writing takes place.
	 * The register data gets transmitted as a hex string, while TRACE32 uses byte arrays.
	 * To convert between the two formats the \ref hex_to_byte_array and \ref byte_array_to_hex functions are used.
	 * @param[in] i_server MCDServer for TCP connection.
	 * @param[in] i_transaction mcd_tx_st as provided by TRACE32.
	 */
	int register_transaction(mcd_tx_st* i_transaction, MCDServer &i_server);
	/**
	 * \brief Handles memory read and write transactions.
	 * 
	 * The memory data gets transmitted as a hex string, while TRACE32 uses byte arrays.
	 * To convert between the two formats the \ref hex_to_byte_array and \ref byte_array_to_hex functions are used.
	 * @param[in] i_server MCDServer for TCP connection.
	 * @param[in] i_transaction mcd_tx_st as provided by TRACE32.
	 */
	int mem_transaction(mcd_tx_st* i_transaction, MCDServer &i_server);
	/**
	 * \brief Executes a reset command.
	 * 
	 * Tells the mcdstub to execute the selected reset. And halts the target if desired.
	 * @param[in] i_server MCDServer for TCP connection.
	 * @param[in] i_reset_id ID of the reset.
	 * @param[in] i_rst_and_halt True if the target is halted after a reset was performed.
	 */
	int execute_reset(MCDServer &i_server, uint8_t i_reset_id, mcd_bool_t i_rst_and_halt);
	/**
	 * \brief Executes a core specific go command.
	 * 
	 * Tells the mcdstub to execute a go for this core.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	int execute_go(MCDServer &i_server);
	/**
	 * \brief Executes a core specific break command.
	 * 
	 * Tells the mcdstub to execute a break for this core.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	int execute_break(MCDServer &i_server);
	/**
	 * \brief Executes a core specific step command.
	 * 
	 * Tells the mcdstub to execute a step for this core.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	int execute_step(MCDServer &i_server);
};

/**
 * \brief A device as in the MCD API. It can have multiple cores.
 * 
 * The device class is the second highest element in the target data structure.
 * It contains all device specific functions and owns one or multiple cores.
 */
class Device {
	std::string host;
	uint32_t server_port;
	std::string device_key;
	std::string acc_hw;
	uint32_t device_type;
	std::string device;
	uint32_t device_id;
	std::vector<std::unique_ptr<Core>> cores;
public:
	/**
	 * \brief Default Class constructor. Only for implicit calls.
	 */
	Device();
	/**
	 * \brief Class constructor.
	 */
	Device(std::string i_host, uint32_t i_server_port, std::string i_device, uint32_t i_device_id);
	/**
	 * \brief Provides info for the devices query.
	 * 
	 * This function copies local data on the device into i_device_con_info.
	 * @param[out] i_device_con_info Holds query info.
	 */
	void get_qry_info(mcd_core_con_info_st *i_device_con_info);
	/**
	 * \brief Returns a pointer to the core with index i_index.
	 * 
	 * @param[in] i_index Index of the requested core.
	 */
	Core* get_core(uint32_t i_index);
	/**
	 * \brief Adds a core to the cores vector.
	 * 
	 * @param[in] i_system Core to be added.
	 */
	void add_core(std::unique_ptr<Core> i_core);
	/**
	 * \brief Returns the number of currently stored systems.
	 */
	uint32_t get_num_cores();
};

/**
 * \brief A system as in the MCD API. It can have multiple devices.
 * 
 * The system class is the highest element in the target data structure.
 * It contains all system specific functions and own ones or multiple devices.
 */

class System {
	std::string system_key;
	std::string system;
	std::string system_instance;
	std::vector<std::unique_ptr<Device>> devices;
public:
	/**
	 * \brief Default Class constructor. Only for implicit calls.
	 */
	System();
	/**
	 * \brief Class constructor.
	 */
	System(std::string i_system, std::string i_system_instance);
	/**
	 * \brief Provides info for the systems query.
	 * 
	 * This function copies local data on the system into i_system_con_info.
	 * @param[out] i_system_con_info Holds query info.
	 */
	void get_qry_info(mcd_core_con_info_st *i_system_con_info);
	/**
	 * \brief Executes a global go command.
	 * 
	 * Tells the mcdstub to execute a go for all cores in the system.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	int execute_go(MCDServer &i_server);
	/**
	 * \brief Executes a global break command.
	 * 
	 * Tells the mcdstub to execute a break for all cores in the system.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	int execute_break(MCDServer &i_server);
	/**
	 * \brief Executes a global step command.
	 * 
	 * Tells the mcdstub to execute a step for all cores in the system.
	 * @param[in] i_server MCDServer for TCP connection.
	 */
	int execute_step(MCDServer &i_server);
	/**
	 * \brief Returns a pointer to the device with index i_index.
	 * 
	 * @param[in] i_index Index of the requested device.
	 */
	Device* get_device(uint32_t i_index);
	/**
	 * \brief Adds a device to the devices vector.
	 * 
	 * @param[in] i_system Device to be added.
	 */
	void add_device(std::unique_ptr<Device> i_device);
	/**
	 * \brief Returns the number of currently stored devices.
	 */
	uint32_t get_num_devices();
};

/**
 * \brief Stores the error code and info string of an error.
 */

class MCDError {
public:
	mcd_return_et err_code;
	std::string err_info;
	/**
	 * \brief Class constructor.
	 */
	MCDError();
};

/**
 * \brief Head element of the library data strucutre and all TCP handling.
 * 
 * The MCDServer stores all connected MCD systems and with that also devices and cores.
 * It also includes functions to connect to the mcdstub and send and receive packages.
 * In case an error occurs, the exact data about the error is stored in the \ref MCDError member.
 */

class MCDServer {
	SOCKET socket_fd; /**< The TCP socket used to communicate with the mcdstub */
	std::string port; /**< The port for TCP communication */
	std::string mcd_ipv4; /**< The IP address of the TCP server (mcdstub) */
	char last_packet[MCD_TCP_PACKETLEN]; /**< The last sent TCP packet */
	int state; /**< The current TCP packet parsing state */
	int line_buf_index; /**< The current line_buf index while parsing a TCP packet */
	int line_sum; /**< The sum of all parsed TCP packet characters */
	char line_buf[MCD_TCP_DATALEN]; /**< The data of the last received TCP packet */
	MCDError last_error; /**< The last occured error */
	std::vector<std::unique_ptr<System>> systems; /**< Vector with all stored systems */
public:
	/**
	 * \brief Class constructor.
	 */
	MCDServer(std::string i_port, std::string i_mcd_ipv4);
	/**
	 * \brief Returns the current socket_fd.
	 */
	SOCKET get_socket_fd();
	/**
	 * \brief Creates a socket and connects to the QEMU mcdstub.
	 */
	int tcp_connect();
	/**
	 * \brief Closes the before openend TCP socket.
	 */
	void tcp_close_socket();
	/**
	 * \brief Detaches the debugger from QEMU.
	 * 
	 * This function tells the mcdstub, that the debugging session is over. Then the socket_fd is reset.
	 * After calling this function a new debugging session can be started.
	 */
	void detach();
	/**
	 * \brief Adds meta data to TCP data.
	 * 
	 * In this function a header and footer is added to the TCP data.
	 * The mcdstub will only accept TCP packets in this format.
	 * Then the packet gets send with \ref tcp_send_packet.
	 * @param[in] i_buffer TCP packet data.
	 */
	void put_packet(char *i_buffer);
	/**
	 * \brief Sends a TCP packet.
	 * 
	 * This function sends a TCP packet with the send method of the socket_fd.
	 * @param[in] i_buffer Complete TCP packet.
	 * @param[in] i_len length of the TCP packet.
	 */
	void tcp_send_packet(char *i_buffer, int i_len);
	/**
	 * \brief Handles the TCP packet receiving.
	 * 
	 * This function calls \ref tcp_receive_packet until the last package was acknowledged by the mcdstub and all data has been received.
	 * In case no data is expected, i_only_acknowledge can be set to true.
	 * @param[in] i_only_acknowledge If set to true the function only waits for an acknowledgement from the mcdstub.
	 */
	void handle_receiving(bool i_only_acknowledge);
	/**
	 * \brief Receives a TCP packet and triggers the data extraction process.
	 * 
	 * This function first receives a TCP packet with the recv method of the socket_fd.
	 * Then it calls \ref read_byte for each character of the received packet.
	 */
	void tcp_receive_packet();
	/**
	 * \brief Extracts the data from a received TCP packet and resends the last packet if not acknowledged.
	 * 
	 * In case the last sent packet was not acknowledged from the mcdstub, this function resends it.
	 * If it was acknowledged this function parses the response from the mcdstub byte by byte.
	 * It extracts the data in the packet and sends an acknowledging response when finished.
	 * @param[in] i_ch Character of the received TCP packet, which should be parsed.
	 */
	void read_byte(char i_ch);
	/**
	 * \brief First TCP packet exchange between the library and the mcdstub.
	 * 
	 * This function send a TCP packet to make sure the mcdstub is connected correctly.
	 * This packet also triggers debuggin preparations in the mcdstub.
	 */
	int initialize_handshake();
	/**
	 * \brief Collects data about the system, device and core configuration.
	 * 
	 * This function requests the current system, device and core configuration from the mcdstub.
	 * Then it calls \ref store_device_cores to store the data in the library.
	 */
	int fetch_system_data();
	/**
	 * \brief Stores a device and cores for a system.
	 * 
	 * This function evaluates the data requested in \ref fetch_system_data and stores it for a given system.
	 * @param[in] i_system The system to which the device and cores belong to.
	 */
	void store_device_cores(System *i_system);
	/**
	 * \brief Prepares a core for debugging.
	 * 
	 * This function tells the mcdstub which core wants to be debugged TRACE32.
	 * Then the mcdstub prepares all data for this core.
	 * @param[in] i_core_index Index of the opened core.
	 */
	int open_core(uint32_t i_core_index);
	/**
	 * \brief Ends debugging for a core.
	 * 
	 * This function tells the mcdstub which core will no longer be debugged by TRACE32.
	 * Then the mcdstub frees all memory for that core.
	 * @param[in] i_core_index Index of the closed core.
	 */
	int close_core(uint32_t i_core_index);
	/**
	 * \brief Returns the last set error.
	 * 
	 * This function returnes a reference to the last \ref MCDError object.
	 */
	MCDError& get_last_error();
	/**
	 * \brief Sets the last error.
	 * 
	 * This function sets the MCD error code and an info string for the last occured error.
	 * @param[in] i_err_code The MCD error code of the last error.
	 * @param[in] i_err_info The info string about the last error.
	 */
	void set_last_error(mcd_return_et i_err_code, std::string i_err_info);
	/**
	 * \brief Returns the line buffer.
	 */
	char* get_line_buffer();
	/**
	 * \brief Returns a pointer to the system with index i_index.
	 * 
	 * @param[in] i_index Index of the requested system.
	 */
	System* get_system(uint32_t i_index);
	/**
	 * \brief Adds a system to the systems vector.
	 * 
	 * @param[in] i_system System to be added.
	 */
	void add_system(std::unique_ptr<System> i_system);
	/**
	 * \brief Returns the number of currently stored systems.
	 */
	uint32_t get_num_systems();
};

/* end classes */

/* start global variables */
/** \defgroup globalvariables Global variables

All global variables used in the library.

*/

/** \addtogroup globalvariables
 * @{ */

extern std::unique_ptr<MCDServer> g_mcd_server; /**< Unique pointer to the complete data structure used in the library */

/** @} */
/* end global variables */

/* start helper functions */
/** \defgroup helperfunctions Helper functions

All helper functions, which are used thoughout the library.
*/

/** \addtogroup helperfunctions 
 * @{ */

/**
 * \brief Extracts the arguments from the config_string.
 * 
 * The config_string can be set by the user with the "SYStem.MCDconfig config_string" command in TRACE32.
 * It can include an IP address called MCDHostName and a port called McdServerPort.
 * This function extracts one argument value per call.
 * If the desired arument is not part of the config_string a default value gets returned.
 * 
 * @param[in] i_config_string The config_string from TRACE32.
 * @param[out] i_output The extracted value.
 * @param[in] i_lookup The arumgents name.
 * @param[in] i_default_value The default value, which gets returned in case the argument was not in the config_string.
 */

void extract_argument_from_string(const std::string& i_config_string, std::string *i_output, std::string i_lookup, std::string i_default_value);

/**
 * \brief Extracts all arguments and values from a TCP string received from the mcdstub.
 * 
 * This function parses the data of the TCP packet as a string and stores the arguments and values into a map.
 * @param[in] i_origin The data of a received TCP packet.
 * @param[out] i_argument_map The Map in which the extracted arguments get stored.
 * @param[in] i_level The level selects the separating characters used for parsin.
 */

void deconstruct_input_string(const std::string i_origin, std::map<std::string, std::string> & i_argument_map, const int i_level = 0);

/**
 * \brief Converts a hex string into an integer byte array.
 * 
 * @param[in] i_hex_data_str The hex data as a string.
 * @param[out] i_transaction The struct in which the integer byte array gets stored.
 */

int hex_to_byte_array(std::string i_hex_data_str, mcd_tx_st* i_transaction);

/**
 * \brief Converts an integer byte array into a hex string.
 * 
 * @param[out] i_hex_data_str The string in which the hex data gets stored.
 * @param[in] i_transaction The struct which provides the integer byte array.
 */

int byte_array_to_hex(std::string& i_hex_data_str, mcd_tx_st* i_transaction);

/**
 * \brief Converts an integer into a hex char.
 * 
 * @param[in] i_int The input integer.
 */

char int_to_hex_char(uint8_t i_int);

/**
 * \brief Converts a hex character into an integer.
 * 
 * @param[in] i_hex_char The input hex character.
 */

uint8_t hex_char_to_int(char i_hex_char);

/**
 * \brief Converts a string into an unsigned 32-bit integer.
 * 
 * @param[in] i_string The input string.
 */

uint32_t atouint32_t(std::string const & i_string);

/**
 * \brief Converts a string into an unsigned 64-bit integer.
 * 
 * @param[in] i_string The input string.
 */

uint64_t atouint64_t(std::string const & i_string);

/**
 * \brief Converts a string into an unsigned 8-bit integer.
 * 
 * @param[in] i_string The input string.
 */

uint8_t atouint8_t(std::string const & i_string);

/**
 * \brief Converts an MCD trigger type into a custom breakpoint define.
 * 
 * @param[in] i_type The MCD trigger type.
 */

int trigger_to_qemu_breakpoint(uint32_t i_type);

/**
 * \brief Returns the correct register access type.
 * 
 * A register can either be accessed with its ID or with an opcode (in case it is an ARM CP15 register).
 * The function returns 0 if the access is dony by ID and 1 of it is done by opcode.
 * @param[in] i_regname The register to check.
 */

int get_reg_access_type(std::string i_regname);

/**
 * \brief Converts an integer to a string.
 * 
 * This is necessary because the MSVC compiler in Visual Studio 2019 has problems with the std::to_string function.
 * @param[in] i_number The integer to be converted.
 */

std::string my_to_string(int i_number);

/** @} */
/* end helper functions */

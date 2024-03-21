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
 * This file includes all class functions.
 * Helper functions can be found in internals.cpp
 */

#include "mcdlib.h"
#include "mcd_shared_defines.h"
#include <cassert>
#include <limits>

std::unique_ptr<MCDServer> g_mcd_server;

MCDServer::MCDServer(std::string i_port, std::string i_mcd_ipv4)
: port(i_port), mcd_ipv4(i_mcd_ipv4) {}

SOCKET MCDServer::get_socket_fd() {
    return this->socket_fd;
}

uint32_t MCDServer::get_num_systems() {
    return this->systems.size();
}

System* MCDServer::get_system(uint32_t i_index) {
    return this->systems.at(i_index).get();
}

void MCDServer::add_system(std::unique_ptr<System> i_system) {
    this->systems.push_back(std::move(i_system));
}

int MCDServer::tcp_connect() {
    struct sockaddr_in server_address;
#if defined(WIN32)
    WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d)) {
    	return -1;
	}
#endif
    /* 1. creating socket */
    this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (!ISVALIDSOCKET(socket_fd)) {
        this->tcp_close_socket();
        return -1;
    }
    memset(&server_address, 0, sizeof(server_address));
    int tcp_port = STRING_TO_INT(this->port);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(this->mcd_ipv4.c_str());
    server_address.sin_port = htons(tcp_port);

    /* 2. connecting to the qemu tcp server */
    if (connect(this->socket_fd, (struct sockaddr*)&server_address, sizeof(server_address))!= 0) {
        this->tcp_close_socket();
        return -1;
    }
    else {
        this->state = RS_IDLE;
        return 0;
    }

}

void MCDServer::tcp_close_socket() {
    shutdown(this->socket_fd, SHUTDOWN_ALL);
    CLOSESOCKET(this->socket_fd);
    this->socket_fd = 0;
#if defined(WIN32)
    WSACleanup();
#endif
}

void MCDServer::detach() {
    /* 1. detaching from qemu */
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c", TCP_CHAR_CLOSE_SERVER);
    this->put_packet(buffer);
    this->handle_receiving(true);
}

void MCDServer::put_packet(char *i_buffer) {
    /* 1. add acknowledge buffer and suffix to last_packet */
    memset(this->last_packet, 0, sizeof(this->last_packet));
    snprintf(this->last_packet, sizeof(this->last_packet), "%c%s%c%c", TCP_COMMAND_START, i_buffer, TCP_COMMAND_END, TCP_WAS_LAST);
    /* 2. send the packet */
    this->tcp_send_packet(this->last_packet, sizeof(this->last_packet));
}

void MCDServer::tcp_send_packet(char *i_buffer, int i_len) {
    send(this->socket_fd, i_buffer, i_len, 0);
}

void MCDServer::tcp_receive_packet() {
    /* 1. receive the buffer */
    char buffer[MCD_TCP_PACKETLEN] = {0};
    recv(this->socket_fd, buffer, sizeof(buffer), 0);
    /* 2. iterate over the buffer */
    int i;
    for (i = 0; i < MCD_TCP_PACKETLEN; i++) {
        this->read_byte(buffer[i]);
        if (buffer[i] == 0) {
            break;
        }
    }
}

void MCDServer::read_byte(char i_ch) {
    /* 1. check acknowledgement if there was previous communnication */
    if (this->last_packet[0] != 0) {
        if (i_ch == TCP_NOT_ACKNOWLEDGED) {
            /* The previous packet was not acknowledged! -> resend the packet */
            this->tcp_send_packet(this->last_packet, sizeof(this->last_packet));
        }
        if (i_ch == TCP_ACKNOWLEDGED || i_ch == TCP_COMMAND_START) {
            /* either acknowledged or a new communication starts -> we discard previous communication */
            memset(this->last_packet, 0, sizeof(this->last_packet));
        }
        if (i_ch != TCP_COMMAND_START) {
            /* we only continue if we are processing a new commant. otherwise we skip to ne next character in the packet which sould be a $ */
            return;
        }
    }
    /* 2. parse packet char by char */
    char reply;
    switch(this->state) {
    case RS_IDLE:
            if (i_ch == TCP_COMMAND_START) {
                /* start of command */
                this->line_buf_index = 0;
                this->line_sum = 0;
                this->state = RS_GETLINE;
            }
            break;
    case RS_GETLINE:
            if (i_ch == TCP_COMMAND_END) {
                /* end of command, start of checksum */
                this->line_buf[this->line_buf_index++] = 0;
                this->line_sum += i_ch;
                this->state = RS_DATAEND;
            }
            else if (this->line_buf_index >= sizeof(this->line_buf) - 1) {
                /* the input string is too long for the linebuffer! */
                printf("input string is too long!!\n");
                this->state = RS_IDLE;
            }
            else {
                /* this means the character is part of the real content fo the packet and we copy it to the line_buf */
                this->line_buf[this->line_buf_index++] = i_ch;
                this->line_sum += i_ch;
            }
            break;
    case RS_DATAEND:
            /* we are now done with copying the data and in the suffix of the packet */
            if (i_ch == TCP_WAS_NOT_LAST) {
                /*
                 * ~ indicates that there is an additional package coming
                 * acknowledged -> send +
                 */
                reply = TCP_ACKNOWLEDGED;
                this->state = RS_IDLE;
            }
            else if (i_ch == TCP_WAS_LAST) {
                /*
                 * acknowledged -> send +
                 * | indicates that there is no additional package coming
                 */
                reply = TCP_ACKNOWLEDGED;
                this->state = RS_CONTINUE;
            }
            else {
                /* not acknowledged -> send - */
                reply = TCP_NOT_ACKNOWLEDGED;
                this->tcp_send_packet(&reply, 1);
                /* waiting for package to get resent */
                this->state = RS_IDLE;
            }
            break;
    case RS_CONTINUE:
        /* we are done here so just skip through */
        return;
    default:
        abort();
    }
}

void MCDServer::handle_receiving(bool i_only_acknowledge) {
    /* loop until acknowledge is received */
    while (this->last_packet[0] != 0) {
        this->tcp_receive_packet();
    }
    /* if i_only_acknowledge is set, we return here */
    if (i_only_acknowledge) {
        return;
    }
    /* loop until all data is received */
    while (this->state != RS_CONTINUE) {
        this->tcp_receive_packet();
    }
    this->state = RS_IDLE;
}

int MCDServer::initialize_handshake() {
    /* 1. request handshake */
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c", TCP_CHAR_OPEN_SERVER);
    this->put_packet(buffer);
    /* 2. get response */
    this->handle_receiving(false);
    /* 3. act on the response */
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, MCD_TCP_DATALEN, "%s", TCP_HANDSHAKE_SUCCESS);
    if (strcmp(this->line_buf, buffer)==0) {
        memset(this->line_buf, 0, sizeof(this->line_buf));
        return 0;
    }
    else {
        memset(this->line_buf, 0, sizeof(this->line_buf));
        return -1;
    }    
}

void MCDServer::store_device_cores(System *i_system) {
    /* 1. get info from qemu */
    std::map<std::string, std::string> device_core_data;
    deconstruct_input_string(this->line_buf, device_core_data, 0);
    std::string device_name = device_core_data.at(TCP_ARGUMENT_DEVICE);
    /* 2. create device */
    std::unique_ptr<Device> device = std::make_unique<Device>(STRING_TO_CHAR(mcd_ipv4), STRING_TO_INT(port), STRING_TO_CHAR(device_name), 0);
    /* 3. get core info */
    std::string gen_core_name = (device_core_data.at(TCP_ARGUMENT_CORE));
    std::string nr_cores = device_core_data.at(TCP_ARGUMENT_AMOUNT_CORE);
    int nr_cores_int = STRING_TO_INT(nr_cores);
    /* 4. store cores */
    for (int i = 0; i < nr_cores_int; i++) {
        std::string core_name = gen_core_name + "-" + my_to_string(i);
        std::unique_ptr<Core> core = std::make_unique<Core>(STRING_TO_CHAR(core_name), i);
        device->add_core(std::move(core));
    }
    /* 5. store device */
    i_system->add_device(std::move(device));
}

int MCDServer::fetch_system_data() {
    /* 1. request system data */
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%s", TCP_CHAR_QUERY, QUERY_ARG_SYSTEM);
    this->put_packet(buffer);
    this->handle_receiving(false);
    std::unique_ptr<System> system = std::make_unique<System>(this->line_buf, "0");
    
    /* 2. request core and device data and store it */
    snprintf(buffer, MCD_TCP_DATALEN, "%c%s", TCP_CHAR_QUERY, QUERY_ARG_CORES);
    this->put_packet(buffer);
    this->handle_receiving(false);
    this->store_device_cores(system.get());

    /* 3. store the new system */
    this->add_system(std::move(system));
    
    return 0;
}

int MCDServer::open_core(uint32_t i_core_index) {
    /* 1. tell qemu which core to setup */
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%d", TCP_CHAR_OPEN_CORE, i_core_index);
    this->put_packet(buffer);
    /* 2. check if acknowledged */
    this->handle_receiving(true);
    return 0;
}

int MCDServer::close_core(uint32_t i_core_index) {
    /* 1. tell qemu which core to close */
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%d", TCP_CHAR_CLOSE_CORE, i_core_index);
    this->put_packet(buffer);
    /* 2. check if acknowledged */
    this->handle_receiving(true);
    return 0;
}

MCDError& MCDServer::get_last_error() {
    return this->last_error;
}

void MCDServer::set_last_error(mcd_return_et i_err_code, std::string i_err_info) {
    this->last_error.err_code = i_err_code;
    this->last_error.err_info = i_err_info;
}

char* MCDServer::get_line_buffer() {
    return this->line_buf;
}

System::System() {}

System::System(std::string i_system, std::string i_system_instance)
/* we dont't use the system key */
: system_key(""), system(i_system), system_instance(i_system_instance) {}



void System::get_qry_info(mcd_core_con_info_st *i_system_con_info) {
    strncpy(i_system_con_info->system_key, this->system_key.c_str(), MCD_KEY_LEN);
    strncpy(i_system_con_info->system, this->system.c_str(), MCD_UNIQUE_NAME_LEN);
    strncpy(i_system_con_info->system_instance, this->system_instance.c_str(), MCD_UNIQUE_NAME_LEN);
}

Device* System::get_device(uint32_t i_index) {
    return this->devices.at(i_index).get();
}

void System::add_device(std::unique_ptr<Device> i_device) {
    this->devices.push_back(std::move(i_device));
}

uint32_t System::get_num_devices() {
    return this->devices.size();
}

int System::execute_go(MCDServer &i_server) {
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%d%c%d", TCP_CHAR_GO, POSITIVE_FLAG, ARGUMENT_SEPARATOR, 0);
    g_mcd_server->put_packet(buffer);
    g_mcd_server->handle_receiving(true);
    return ERROR_NONE;
}

int System::execute_break(MCDServer &i_server) {
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%d%c%d", TCP_CHAR_BREAK, POSITIVE_FLAG, ARGUMENT_SEPARATOR, 0);
    g_mcd_server->put_packet(buffer);
    g_mcd_server->handle_receiving(true);
    return ERROR_NONE;
}

int System::execute_step(MCDServer &i_server) {
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%d%c%d", TCP_CHAR_STEP, POSITIVE_FLAG, ARGUMENT_SEPARATOR, 0);
    i_server.put_packet(buffer);
    i_server.handle_receiving(true);
    return ERROR_NONE;
}

Device::Device() {}

Device::Device(std::string i_host, uint32_t i_server_port, std::string i_device, uint32_t i_device_id)
/* we dont use the device_key nor the hardware accelorator, not sure what the device type is */
: host(i_host), server_port(i_server_port), device_key(""), acc_hw(""), device_type(0), device(i_device), device_id(i_device_id) {}

void Device::get_qry_info(mcd_core_con_info_st *i_device_con_info) {
    strncpy(i_device_con_info->host, this->host.c_str(), MCD_HOSTNAME_LEN);
    i_device_con_info->server_port = this->server_port;
    strncpy(i_device_con_info->device_key, this->device_key.c_str(), MCD_KEY_LEN);
    strncpy(i_device_con_info->acc_hw, this->acc_hw.c_str(), MCD_UNIQUE_NAME_LEN);
    i_device_con_info->device_type = this->device_type;
    strncpy(i_device_con_info->device, this->device.c_str(), MCD_UNIQUE_NAME_LEN);
    i_device_con_info->device_id = this->device_id;
}

Core* Device::get_core(uint32_t i_index) {
    return this->cores.at(i_index).get();
}

void Device::add_core(std::unique_ptr<Core> i_core) {
    this->cores.push_back(std::move(i_core));
}

uint32_t Device::get_num_cores() {
    return this->cores.size();
}

Core::Core() {}

Core::Core(std::string i_core, uint32_t i_core_id)
: core(i_core), core_id(i_core_id), opcode_memspace_id(0), id_memspace_id(0) {}

void Core::get_qry_info(mcd_core_con_info_st *i_core_con_info) {
    strncpy(i_core_con_info->core, this->core.c_str(), MCD_UNIQUE_NAME_LEN);
    i_core_con_info->core_id = this->core_id;
}

uint32_t Core::find_trigger_id(uint32_t i_type, uint64_t i_address, std::map<uint32_t, mcd_trig_simple_core_st> i_trigger) {
    /* 1. get mcd trigger type */
    mcd_trig_type_et mcd_trigger_type;
    switch (i_type) {
    case MCD_BREAKPOINT_READ:
        mcd_trigger_type = MCD_TRIG_TYPE_READ;
        break;
    case MCD_BREAKPOINT_WRITE:
        mcd_trigger_type = MCD_TRIG_TYPE_WRITE;
        break;
    case MCD_BREAKPOINT_RW:
        mcd_trigger_type = MCD_TRIG_TYPE_RW;
        break;
    default:
        /* TRACE32 knows itself which hw breakpoint was hit. */
        return 0;
    }
    /* 2. search all trigger */
    for (auto const& x : i_trigger) {
        mcd_trig_simple_core_st current_trigger = x.second;
        if (current_trigger.addr_start.address == i_address && current_trigger.type == mcd_trigger_type) {
            return x.first;
        }
    }
    /* couldn't find trigger */
    return 0;
}

int Core::get_qry_state(MCDServer& i_server, mcd_core_state_st* i_state) {
    /* 1. request state data from qemu */
    const char *line_buffer = i_server.get_line_buffer();
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%s%d", TCP_CHAR_QUERY, QUERY_ARG_STATE, this->core_id);
    i_server.put_packet(buffer);
    i_server.handle_receiving(false);

    /* 2. process response */
    std::map<std::string, std::string> state_data;
    deconstruct_input_string(line_buffer, state_data, 0);
    std::string state_str = state_data.at(TCP_ARGUMENT_STATE);
    if (state_str == CORE_STATE_RUNNING) {
        i_state->state = MCD_CORE_STATE_RUNNING;
    }
    else if (state_str == CORE_STATE_HALTED) {
        i_state->state = MCD_CORE_STATE_HALTED;
    }
    else if (state_str == CORE_STATE_DEBUG) {
        i_state->state = MCD_CORE_STATE_DEBUG;
    }
    else {
        i_state->state = MCD_CORE_STATE_UNKNOWN;
    }
    i_state->event = atouint32_t(state_data.at(TCP_ARGUMENT_EVENT));
    i_state->hw_thread_id= atouint32_t(state_data.at(TCP_ARGUMENT_THREAD));
    /* 3. checkout trigger */
    uint32_t bp_type = atouint32_t(state_data.at(TCP_ARGUMENT_TYPE));
    uint64_t bp_address = atouint64_t(state_data.at(TCP_ARGUMENT_ADDRESS));
    i_state->trig_id = this->find_trigger_id(bp_type, bp_address, this->trigger);
    strncpy(i_state->stop_str, state_data.at(TCP_ARGUMENT_STOP_STRING).c_str(), MCD_INFO_STR_LEN);
    strncpy(i_state->info_str, state_data.at(TCP_ARGUMENT_INFO_STRING).c_str(), MCD_INFO_STR_LEN);
    return 0;
}

void Core::store_reset_data(MCDServer &i_server) {
    /* 1. request reset info from qemu */
    const char *line_buffer = i_server.get_line_buffer();
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%s%s%d", TCP_CHAR_QUERY, QUERY_ARG_RESET, QUERY_FIRST, this->core_id);
    i_server.put_packet(buffer);
    i_server.handle_receiving(false);
    /* 2. process the response until the last packet was sent */
    size_t pos1 = 0;
    std::string i_buffer_str = line_buffer;
    int current_handle = 0;
    const uint32_t reset_class_bit = 1;
    this->reset_classes = 0;
    while(true) {
        pos1 = i_buffer_str.find(QUERY_END_INDEX, 0);
        /* store the current data packet */
        std::map<std::string, std::string> reset_data;
        deconstruct_input_string(&line_buffer[pos1+1], reset_data, 0);
        /* store data */
        uint32_t reset_id = atouint32_t(reset_data.at(TCP_ARGUMENT_ID));
        std::string reset_name = reset_data.at(TCP_ARGUMENT_NAME);
        this->reset_info.insert({reset_id, reset_name});
        /* set the correct bit to one in the reset_classes vector */
        this->reset_classes = (this->reset_classes) | (reset_class_bit << reset_id);
        /* check if this was the last packet */
        current_handle = STRING_TO_INT(i_buffer_str.substr(0, pos1));
        if (current_handle==0) {
            break;
        }
        /*request the next packet */
        snprintf(buffer, MCD_TCP_DATALEN, "%c%s%s%d", TCP_CHAR_QUERY, QUERY_ARG_RESET, QUERY_CONSEQUTIVE, current_handle);
        i_server.put_packet(buffer);
        i_server.handle_receiving(false);
        i_buffer_str = line_buffer;
    }
}

void Core::store_trigger_info(MCDServer &i_server) {
    /* 1. request trigger info from qemu */
    const char *line_buffer = i_server.get_line_buffer();
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%s", TCP_CHAR_QUERY, QUERY_ARG_TRIGGER);
    i_server.put_packet(buffer);
    i_server.handle_receiving(false);
    /* 2. pocess data */
    std::map<std::string, std::string> trigger_data;
    deconstruct_input_string(line_buffer, trigger_data, 0);
    std::string type_data = trigger_data.at(TCP_ARGUMENT_TYPE);
    mcd_trig_type_et type = 0;
    if (type_data.find(my_to_string(MCD_BREAKPOINT_HW)) != std::string::npos) {
        type |= MCD_TRIG_TYPE_IP;
    }
    if (type_data.find(my_to_string(MCD_BREAKPOINT_READ)) != std::string::npos) {
        type |= MCD_TRIG_TYPE_READ;
    }
    if (type_data.find(my_to_string(MCD_BREAKPOINT_WRITE)) != std::string::npos) {
        type |= MCD_TRIG_TYPE_WRITE;
    }
    if (type_data.find(my_to_string(MCD_BREAKPOINT_RW)) != std::string::npos) {
        type |= MCD_TRIG_TYPE_RW;
    }
    std::string option_data = trigger_data.at(TCP_ARGUMENT_OPTION);
    mcd_trig_opt_et option = 0;
    if (option_data.find(MCD_TRIG_OPT_VALUE) != std::string::npos) {
        option |= MCD_TRIG_OPT_DATA_IS_CONDITION;
    }
    std::string action_data = trigger_data.at(TCP_ARGUMENT_ACTION);
    mcd_trig_action_et action = 0;
    if (action_data.find(MCD_TRIG_ACT_BREAK) != std::string::npos) {
        action |= MCD_TRIG_ACTION_DBG_DEBUG;
    }
    /* 3. store data */
    mcd_trig_info_st trigger_info = {
        .type = type,
        .option = option,
        .action = action,
        .trig_number = atouint32_t(trigger_data.at(TCP_ARGUMENT_AMOUNT_TRIGGER)),
    };
    this->trigger_info = trigger_info;
}

void Core::store_mem_space_info(MCDServer &i_server) {
    /* 1. request memory spaces info from qemu */
    const char *line_buffer = i_server.get_line_buffer();
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%s%s%d", TCP_CHAR_QUERY, QUERY_ARG_MEMORY, QUERY_FIRST, this->core_id);
    i_server.put_packet(buffer);
    i_server.handle_receiving(false);
    /* 2. process the response until the last packet was sent */
    size_t pos1 = 0;
    std::string i_buffer_str = line_buffer;
    int current_handle = 0;
    while(true) {
        pos1 = i_buffer_str.find(QUERY_END_INDEX, 0);
        /* store the current data packet */
        std::map<std::string, std::string> mem_space_data;
        deconstruct_input_string(&line_buffer[pos1+1], mem_space_data, 0);
        /* store data */
        uint32_t mem_space_id = atouint32_t(mem_space_data.at(TCP_ARGUMENT_ID));
        std::string mem_space_name = mem_space_data.at(TCP_ARGUMENT_NAME);
        uint32_t mem_space_type= atouint32_t(mem_space_data.at(TCP_ARGUMENT_TYPE));
        /* check if this a register memspace */
        if (mem_space_type == MCD_MEM_SPACE_IS_REGISTERS) {
            if (mem_space_name.find(MCD_GRP_KEYWORD) != std::string::npos) {
                this->id_memspace_id = mem_space_id;
            }
            else if (mem_space_name.find(MCD_CP_KEYWORD) != std::string::npos) {
                this->opcode_memspace_id = mem_space_id;
            }
        }
        mcd_memspace_st new_mem_space_st = {
            .mem_space_id = mem_space_id,
            .mem_type = mem_space_type,
            .bits_per_mau = atouint32_t(mem_space_data.at(TCP_ARGUMENT_BITS_PER_MAU)),
            .invariance = atouint8_t(mem_space_data.at(TCP_ARGUMENT_INVARIANCE)),
            .endian = atouint32_t(mem_space_data.at(TCP_ARGUMENT_ENDIAN)),
            .min_addr = atouint64_t(mem_space_data.at(TCP_ARGUMENT_MIN)),
            .max_addr = atouint64_t(mem_space_data.at(TCP_ARGUMENT_MAX)),
            .supported_access_options = atouint32_t(mem_space_data.at(TCP_ARGUMENT_SUPPORTED_ACCESS_OPTIONS)),
        };
        strncpy(new_mem_space_st.mem_space_name, STRING_TO_CHAR(mem_space_name), MCD_MEM_SPACE_NAME_LEN);
        this->mem_spaces.insert({mem_space_id, new_mem_space_st});
        /* check if this was the last packet */
        current_handle = STRING_TO_INT(i_buffer_str.substr(0, pos1));
        if (current_handle==0) {
            break;
        }
        /* request the next packet */
        snprintf(buffer, MCD_TCP_DATALEN, "%c%s%s%d", TCP_CHAR_QUERY, QUERY_ARG_MEMORY, QUERY_CONSEQUTIVE, current_handle);
        i_server.put_packet(buffer);
        i_server.handle_receiving(false);
        i_buffer_str = line_buffer;
    }
}

void Core::store_reg_group_info(MCDServer &i_server) {
    /* 1. request reg group info from qemu */
    const char *line_buffer = i_server.get_line_buffer();
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%s%s%d", TCP_CHAR_QUERY, QUERY_ARG_REGGROUP, QUERY_FIRST, this->core_id);
    i_server.put_packet(buffer);
    i_server.handle_receiving(false);

    /* 2. process the response until the last packet was sent */
    size_t pos1 = 0;
    std::string i_buffer_str = line_buffer;
    int current_handle = 0;
    while(true) {
        pos1 = i_buffer_str.find(QUERY_END_INDEX, 0);
        /* store the current data packet */
        std::map<std::string, std::string> reg_group_data;
        deconstruct_input_string(&line_buffer[pos1+1], reg_group_data, 0);
        uint32_t group_id = atouint32_t(reg_group_data.at(TCP_ARGUMENT_ID));
        std::string group_name = reg_group_data.at(TCP_ARGUMENT_NAME);
        RegGroup reg_group = RegGroup(group_name, group_id);
        this->reg_groups.insert({group_id, reg_group});
        /* check if this was the last packet */
        current_handle = STRING_TO_INT(i_buffer_str.substr(0, pos1));
        if (current_handle==0) {
            break;
        }
        /* request the next packet */
        snprintf(buffer, MCD_TCP_DATALEN, "%c%s%s%d", TCP_CHAR_QUERY, QUERY_ARG_REGGROUP, QUERY_CONSEQUTIVE, current_handle);
        i_server.put_packet(buffer);
        i_server.handle_receiving(false);
        i_buffer_str = line_buffer;
    }
}

void Core::store_reg_info(MCDServer &i_server) {
    /* 1. request register info from qemu */
    const char *line_buffer = i_server.get_line_buffer();
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%s%s%d", TCP_CHAR_QUERY, QUERY_ARG_REG, QUERY_FIRST, this->core_id);
    i_server.put_packet(buffer);
    i_server.handle_receiving(false);
    /* 2. process the response until the last packet was sent */
    size_t pos1 = 0;
    std::string i_buffer_str = line_buffer;
    int current_handle = 0;
    while(true) {
        pos1 = i_buffer_str.find(QUERY_END_INDEX, 0);
        /* store the current data packet */
        std::map<std::string, std::string> reg_data;
        deconstruct_input_string(&line_buffer[pos1+1], reg_data, 0);
        /* check whether we want to access the register by ID or by OPCODE */
        std::string regname = reg_data.at(TCP_ARGUMENT_NAME);
        /* qemu uses lowercase names for registers, TRACE32 uses upper case */
        for (int i = 0; i < regname.length(); i++) {
            regname[i] = toupper(regname[i]);
        }
        uint32_t reg_id = atouint64_t(reg_data.at(TCP_ARGUMENT_ID));
        uint32_t mem_space_id = 0;
        uint32_t opcode = atouint32_t(reg_data.at(TCP_ARGUMENT_OPCODE));
        if (get_reg_access_type(regname) && opcode) {
            mem_space_id = this->opcode_memspace_id;
            /* store opcode */
            this->opcode_lookup.insert({opcode, reg_id});
        }
        else {
            mem_space_id = this->id_memspace_id;
        }
        /* store register */
        mcd_addr_st new_address = {
            .address = reg_id,
            .mem_space_id = mem_space_id,
        };
        uint32_t reg_group_id = atouint32_t(reg_data.at(TCP_ARGUMENT_REGGROUPID));
        mcd_register_info_st new_register = {
            .addr = new_address,
            .reg_group_id = reg_group_id,
            .regsize = atouint32_t(reg_data.at(TCP_ARGUMENT_SIZE)),
            .reg_type = atouint32_t(reg_data.at(TCP_ARGUMENT_TYPE)),
            .hw_thread_id = atouint32_t(reg_data.at(TCP_ARGUMENT_THREAD)),
        };
        strncpy(new_register.regname, STRING_TO_CHAR(regname), MCD_REG_NAME_LEN);
        this->reg_groups.at(reg_group_id).add_register(new_register);
        /* check if this was the last packet */
        current_handle = STRING_TO_INT(i_buffer_str.substr(0, pos1));
        if (current_handle==0) {
            break;
        }
        /* request the next packet */
        snprintf(buffer, MCD_TCP_DATALEN, "%c%s%s%d", TCP_CHAR_QUERY, QUERY_ARG_REG, QUERY_CONSEQUTIVE, current_handle);
        i_server.put_packet(buffer);
        i_server.handle_receiving(false);
        i_buffer_str = line_buffer;
    }
}

std::string Core::get_reset_info(uint8_t i_reset_class) {
    return this->reset_info.at(i_reset_class);
}

uint32_t Core::get_reset_class_vector() {
    return this->reset_classes;
}

void Core::get_trigger_info(mcd_trig_info_st *i_trig_info) {
    *i_trig_info = this->trigger_info;
}

uint32_t Core::get_num_mem_spaces() {
    return this->mem_spaces.size();
}

uint32_t Core::get_num_reg_groups() {
    return this->reg_groups.size();
}

uint32_t Core::get_num_registers() {
    uint32_t num_registers = 0;
    for (auto const& x : this->reg_groups) {
        RegGroup reg_group = x.second;
        num_registers = num_registers + reg_group.get_num_registers();
    }
    return num_registers;
}

uint32_t Core::get_num_trigger() {
    return this->trigger.size();
}

void Core::qry_get_register_data(uint32_t i_start_index, uint32_t i_num_regs, mcd_register_info_st *i_reg_info) {
    uint32_t current_parse_index = i_start_index;
    uint32_t current_store_index = 0;
    uint32_t regs_still_to_parse = i_num_regs;
    for (auto const& x : this->reg_groups) {
        RegGroup reg_group = x.second;
        uint32_t num_registers = reg_group.get_num_registers();
        if (current_parse_index>=num_registers) {
            current_parse_index = current_parse_index - num_registers;
        }
        else {
            uint32_t max_parse_index = current_parse_index + (regs_still_to_parse - 1); /* regs_still_to_parse includes the current index */
            if (max_parse_index < num_registers) {
                reg_group.qry_get_register_data(current_parse_index, regs_still_to_parse, &i_reg_info[current_store_index]);
                return;
            }
            else {
                reg_group.qry_get_register_data(current_parse_index, num_registers - current_parse_index, &i_reg_info[current_store_index]);
                regs_still_to_parse = regs_still_to_parse - (num_registers - current_parse_index);
                current_store_index = current_store_index + (num_registers - current_parse_index);
                current_parse_index = 0;
            }
        }
    }
}

mcd_memspace_st& Core::get_mem_space(uint32_t i_mem_space_id) {
    return this->mem_spaces.at(i_mem_space_id);
}

RegGroup& Core::get_reg_group(uint32_t i_group_id) {
    return this->reg_groups.at(i_group_id);
}

int Core::register_transaction(mcd_tx_st* i_transaction, MCDServer &i_server) {
    const char *line_buffer = i_server.get_line_buffer();
    char buffer[MCD_TCP_DATALEN] = {0};
    /* check if we got an opcode instad of an id */
    uint64_t reg_id = 0;
    if (i_transaction->addr.mem_space_id == this->opcode_memspace_id) {
        try {
        reg_id = this->opcode_lookup.at(i_transaction->addr.address);
        }
        catch (const std::out_of_range& oor){
            /* indicates that no bits could be read */
            return ERROR_NONE;
        }
    }
    else {
        reg_id = i_transaction->addr.address;
    }
    if (i_transaction->access_type==MCD_TX_AT_R) {
        /* 1. send read request */
        snprintf(buffer, MCD_TCP_DATALEN, "%c%d%c%lu", TCP_CHAR_READ_REGISTER, this->core_id, ARGUMENT_SEPARATOR, reg_id);
        i_server.put_packet(buffer);
        i_server.handle_receiving(false);
        /* 2. process data */
        std::string data_str = line_buffer;
        if (hex_to_byte_array(data_str, i_transaction)!=0) {
            return ERROR_READ; 
        }
        return ERROR_NONE;
    }
    else if (i_transaction->access_type==MCD_TX_AT_W) {
        /* 1. pepare data */
        std::string hex_data = "";
        if (byte_array_to_hex(hex_data, i_transaction)!=0) {
            return ERROR_TRANSACTON;
        }
        /* 2. send data */
        snprintf(buffer, MCD_TCP_DATALEN, "%c%d%c%lu%c%d%c%s", TCP_CHAR_WRITE_REGISTER, this->core_id, ARGUMENT_SEPARATOR, reg_id,
            ARGUMENT_SEPARATOR, i_transaction->num_bytes, ARGUMENT_SEPARATOR, hex_data.c_str());
        i_server.put_packet(buffer);
        i_server.handle_receiving(false);
        /* 3. check if it was successful */
        if (strcmp(line_buffer, TCP_EXECUTION_SUCCESS)==0) {
            return ERROR_NONE;
        }
        else return ERROR_WRITE;
    }
    else {
        /* TODO: read/write */
        return ERROR_TRANSACTON;
    }
}

int Core::mem_transaction(mcd_tx_st* i_transaction, MCDServer &i_server) {
    const char *line_buffer = i_server.get_line_buffer();
    char buffer[MCD_TCP_DATALEN] = {0};
    if (i_transaction->access_type==MCD_TX_AT_R) {
        /* 1. send read request */
        snprintf(buffer, MCD_TCP_DATALEN, "%c%u%c%u%c%lu%c%u",
            TCP_CHAR_READ_MEMORY, this->core_id,
            ARGUMENT_SEPARATOR, i_transaction->addr.mem_space_id,
            ARGUMENT_SEPARATOR, i_transaction->addr.address,
            ARGUMENT_SEPARATOR, i_transaction->num_bytes);
        i_server.put_packet(buffer);
        i_server.handle_receiving(false);
        /* 2. process data */
        if (strcmp(line_buffer, TCP_EXECUTION_ERROR)==0) {
            return ERROR_READ;
        }
        std::string hex_data = line_buffer;
        if (hex_to_byte_array(hex_data, i_transaction)!=0) {
            return ERROR_READ;
        }
        return ERROR_NONE;
    }
    else if (i_transaction->access_type==MCD_TX_AT_W) {
        /* 1. pepare data */
        std::string hex_data = "";
        if (byte_array_to_hex(hex_data, i_transaction)!=0) {
            return ERROR_TRANSACTON;
        }
        /* 2. send data */
        snprintf(buffer, MCD_TCP_DATALEN, "%c%u%c%u%c%lu%c%u%c%s",
            TCP_CHAR_WRITE_MEMORY, this->core_id,
            ARGUMENT_SEPARATOR, i_transaction->addr.mem_space_id,
            ARGUMENT_SEPARATOR, i_transaction->addr.address,
            ARGUMENT_SEPARATOR, i_transaction->num_bytes,
            ARGUMENT_SEPARATOR, hex_data.c_str());
        i_server.put_packet(buffer);
        i_server.handle_receiving(false);
        /* 3. check if it was successful */
        if (strcmp(line_buffer, TCP_EXECUTION_SUCCESS)==0) {
            return ERROR_NONE;
        }
        else return ERROR_WRITE;
    }
    else {
        /* TODO: read/write */
        return ERROR_TRANSACTON;
    }
}

int Core::execute_transaction(mcd_tx_st* i_transaction, MCDServer &i_server) {
    /* differentiate for gpr, cp regs and memory */
    mcd_memspace_st desired_memspace = get_mem_space(i_transaction->addr.mem_space_id);
    if (desired_memspace.mem_type == MCD_MEM_SPACE_IS_REGISTERS) {
        /* register access */
        return this->register_transaction(i_transaction, i_server);
    }
    else {
        /* memory access */
        return this->mem_transaction(i_transaction, i_server);
    }
}

int Core::execute_go(MCDServer &i_server) {
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%d%c%d", TCP_CHAR_GO, NEGATIVE_FLAG, ARGUMENT_SEPARATOR, this->core_id);
    g_mcd_server->put_packet(buffer);
    g_mcd_server->handle_receiving(true);
    return ERROR_NONE;
}

int Core::execute_break(MCDServer &i_server) {
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%d%c%d", TCP_CHAR_BREAK, NEGATIVE_FLAG, ARGUMENT_SEPARATOR, this->core_id);
    g_mcd_server->put_packet(buffer);
    g_mcd_server->handle_receiving(true);
    return ERROR_NONE;
}

int Core::execute_step(MCDServer &i_server) {
    char buffer[MCD_TCP_DATALEN] = {0};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%d%c%d", TCP_CHAR_STEP, NEGATIVE_FLAG, ARGUMENT_SEPARATOR, this->core_id);
    i_server.put_packet(buffer);
    i_server.handle_receiving(true);
    return ERROR_NONE;
}

int Core::execute_reset(MCDServer &i_server, uint8_t i_reset_id, mcd_bool_t i_rst_and_halt) {
    char buffer[MCD_TCP_DATALEN] = {};
    snprintf(buffer, MCD_TCP_DATALEN, "%c%d", TCP_CHAR_RESET, i_reset_id);
    g_mcd_server->put_packet(buffer);
    g_mcd_server->handle_receiving(true);
    /* stop the CPU */
    if (i_rst_and_halt) {
        if (this->execute_break(i_server)!=0){
            return ERROR_GENERAL;
        }
    }
    return ERROR_NONE;
}

void Core::add_trigger(uint32_t i_index, mcd_trig_simple_core_st i_trigger_st) {
    this->trigger.insert({i_index, i_trigger_st});
}

int Core::activate_trigger(MCDServer& i_server) {
    /* activates all trigger for this core */
    const char *line_buffer = i_server.get_line_buffer();
    char buffer[MCD_TCP_DATALEN] = {0};
    
    for (auto const& x : this->trigger) {
        /* scip already activated trigger */
        if (this->active_trigger.find(x.first) != this->active_trigger.end()) {
            continue;
        }
        /* insert trigger */
        mcd_trig_simple_core_st current_trigger = x.second;
        int bp_type = trigger_to_qemu_breakpoint(current_trigger.type);
        if (bp_type == 0) {
            return ERROR_GENERAL;
            /* not supported type */
        }
        snprintf(buffer, MCD_TCP_DATALEN, "%c%d%c%d%c%lu%c%d", TCP_CHAR_BREAKPOINT_INSERT, this->core_id, ARGUMENT_SEPARATOR, bp_type,
            ARGUMENT_SEPARATOR, current_trigger.addr_start.address, ARGUMENT_SEPARATOR, x.first);
        i_server.put_packet(buffer);
        i_server.handle_receiving(false);
        if (strcmp(line_buffer, TCP_EXECUTION_SUCCESS) != 0) {
            return ERROR_GENERAL;
        }
        /* add to active trigger */
        this->active_trigger.insert({x.first, x.first});
    }
    
    return ERROR_NONE;
}

int Core::remove_trigger(MCDServer& i_server, uint32_t i_trig_id) {
    /* 1. delete trigger within qemu */
    const char *line_buffer = i_server.get_line_buffer();
    char buffer[MCD_TCP_DATALEN] = {0};

    mcd_trig_simple_core_st current_trigger = this->trigger.at(i_trig_id);
    int bp_type = trigger_to_qemu_breakpoint(current_trigger.type);
    if (bp_type == 0) {
        return ERROR_GENERAL;
        /* not supported type */
    }
    snprintf(buffer, MCD_TCP_DATALEN, "%c%d%c%d%c%lu%c%d", TCP_CHAR_BREAKPOINT_REMOVE, this->core_id, ARGUMENT_SEPARATOR, bp_type,
            ARGUMENT_SEPARATOR, current_trigger.addr_start.address, ARGUMENT_SEPARATOR, i_trig_id);
    i_server.put_packet(buffer);
    i_server.handle_receiving(false);
    if (strcmp(line_buffer, TCP_EXECUTION_SUCCESS) != 0) {
        return ERROR_GENERAL;
    }
    /* 2. delete trigger locally */
    this->trigger.erase(i_trig_id);
    this->active_trigger.erase(i_trig_id);

    return ERROR_NONE;
}

void Core::get_valid_trig_id(uint32_t& i_trig_id) {
    if (i_trig_id == 0) {
        /* id mustn't be 0 */
        i_trig_id++;
    }
    while (this->trigger.find(i_trig_id) != this->trigger.end()) {
        i_trig_id++;
    }
}

void Core::qry_get_mem_space_data(uint32_t i_mem_space_id, mcd_memspace_st *i_mem_space_info) {
    mcd_memspace_st mem_space_st = this->get_mem_space(i_mem_space_id);
    *i_mem_space_info = mem_space_st;
}

MCDError::MCDError() {}

RegGroup::RegGroup(std::string i_name, uint32_t i_reg_group_id)
: reg_group_name(i_name), reg_group_id(i_reg_group_id){}

void RegGroup::get_qry_info(mcd_register_group_st *i_reg_groups) {
    strncpy(i_reg_groups->reg_group_name, this->reg_group_name.c_str(), MCD_REG_NAME_LEN);
    i_reg_groups->reg_group_id = this->reg_group_id;
    i_reg_groups->n_registers = this->registers.size();
}

void RegGroup::add_register(mcd_register_info_st i_register) {
    this->registers.push_back(i_register);
}

void RegGroup::qry_get_register_data(uint32_t i_start_index, uint32_t i_num_regs, mcd_register_info_st *i_reg_info) {
    /* returns all regs from the reg group */
    for (int i = i_start_index; i < (i_num_regs + i_start_index); i++) {
        i_reg_info[i - i_start_index] = this->registers.at(i);
    }
}

uint32_t RegGroup::get_num_registers() {
    return this->registers.size();
}

mcd_return_et mcd_open_server_f(const mcd_char_t* i_system_key, const mcd_char_t* i_config_string, mcd_server_st** i_server) {
    /* working with the config_string */
    std::string mcd_ipv4;
    std::string mcd_server_port;
    std::string mcd_ipv4_arg = CONFIG_STR_ARG_HOST;
    std::string mcd_server_port_arg = CONFIG_STR_ARG_PORT;
    extract_argument_from_string(i_config_string, &mcd_ipv4, mcd_ipv4_arg, MCD_DEFAULT_IPV4);
    extract_argument_from_string(i_config_string, &mcd_server_port, mcd_server_port_arg, MCD_DEFAULT_TCP_PORT);
    /* check if localhost was passed as argument */
    if (mcd_ipv4.compare(MCD_LOCALHOST_STR)==0) {
        mcd_ipv4 = MCD_DEFAULT_IPV4;
    }
    /* allocate memory local data structure */
    g_mcd_server = std::make_unique<MCDServer>(mcd_server_port, mcd_ipv4);
    /* connect to tcp server */
    if (g_mcd_server->tcp_connect()!=0) {
        return MCD_ERR_CONNECTION;
    }
    /* initial handshake */
    if (g_mcd_server->initialize_handshake()!=0) {
        g_mcd_server->tcp_close_socket();
        return MCD_ERR_CONNECTION;
    }
    /* create objects for cores and devices (get info from qemu) */
    if (g_mcd_server->fetch_system_data()!=0) {
        return MCD_ERR_CONNECTION;
    }
    /* set standard error */
    g_mcd_server->set_last_error(MCD_ERR_GENERAL, ERROR_STRING_STANDARD);
    /* allocate memory */
    mcd_server_st *server_ptr = new mcd_server_st;
    server_ptr->host = mcd_ipv4.c_str();
    server_ptr->config_string = i_config_string;
    server_ptr->instance = g_mcd_server.get();
    *i_server = server_ptr;

    return MCD_ERR_NONE;
}

mcd_return_et mcd_initialize_f(const mcd_api_version_st* i_version_req, mcd_impl_version_info_st* i_impl_info) {
    /*set general information on this API*/
    mcd_api_version_st api_version;
    mcd_impl_version_info_st impl_api_version;
    memset(&api_version, 0, sizeof(api_version));
    memset(&impl_api_version, 0, sizeof(impl_api_version));
    api_version.v_api_major = MCD_API_VER_MAJOR;
    api_version.v_api_minor = MCD_API_VER_MINOR;
    strncpy((char*)api_version.author, MCD_API_VER_AUTHOR, MCD_API_IMP_VENDOR_LEN);

    impl_api_version.v_api = api_version;
    impl_api_version.v_imp_major = MCD_API_VER_MAJOR;
    impl_api_version.v_imp_minor = MCD_API_VER_MINOR;
    impl_api_version.v_imp_build = MCD_LIB_BUILD;
    strncpy((char*)impl_api_version.vendor, MCD_LIB_VENDOR, MCD_API_IMP_VENDOR_LEN);
    strncpy((char*)impl_api_version.date, MCD_LIB_DATE, MCD_API_IMP_DATE_LEN);

    /*set general information on this API*/
    *i_impl_info = impl_api_version;

    /*compare version number*/
    if (i_version_req->v_api_major != api_version.v_api_major) {
        return MCD_ERR_GENERAL;
    }
    else {
        if (i_version_req->v_api_minor != api_version.v_api_minor) {
            return MCD_ERR_GENERAL;
        }
        else {
            return MCD_ERR_NONE;
        }
    }
}

mcd_return_et mcd_qry_systems_f(uint32_t i_start_index, uint32_t* i_num_systems, mcd_core_con_info_st *i_system_con_info) {
    if (*i_num_systems == 0) {
        /* provide the number of systems */
        *i_num_systems = g_mcd_server->get_num_systems();
        return MCD_ERR_NONE;
    }

    for (uint32_t current_index = 0; current_index < *i_num_systems; current_index++) {
        /* get correct struct and system */
        System *system = g_mcd_server->get_system(i_start_index + current_index);
        mcd_core_con_info_st *ptr_to_current_st = &i_system_con_info[current_index];

        /* get new data from the system */
        system->get_qry_info(ptr_to_current_st);
    }
    return MCD_ERR_NONE;
}

mcd_return_et mcd_qry_devices_f(const mcd_core_con_info_st *i_system_con_info, uint32_t i_start_index, uint32_t *i_num_devices, mcd_core_con_info_st *i_device_con_info) {
    System *main_system = g_mcd_server->get_system(atoi(i_system_con_info->system_instance));

    if (*i_num_devices==0) {
        /* provide the number of devices */
        *i_num_devices = main_system->get_num_devices();
        return MCD_ERR_NONE;
    }

    /* output data */
    for (uint32_t current_index = 0; current_index < *i_num_devices;current_index++) {
        /* get correct struct and device */
        mcd_core_con_info_st *ptr_to_current_st = &i_device_con_info[current_index];
        Device *device = main_system->get_device(i_start_index + current_index);

        /* copy provided data (system) */
        strncpy(ptr_to_current_st->system_key, i_system_con_info->system_key, MCD_KEY_LEN);
        strncpy(ptr_to_current_st->system, i_system_con_info->system, MCD_UNIQUE_NAME_LEN);
        strncpy(ptr_to_current_st->system_instance, i_system_con_info->system_instance, MCD_UNIQUE_NAME_LEN);

        /* get new data from the device */
        device->get_qry_info(ptr_to_current_st);
    }
    return MCD_ERR_NONE;
}

mcd_return_et mcd_qry_cores_f(const mcd_core_con_info_st *i_connection_info, uint32_t i_start_index, uint32_t *i_num_cores, mcd_core_con_info_st *i_core_con_info) {
    System *main_system = g_mcd_server->get_system(atoi(i_connection_info->system_instance));
    Device *device = main_system->get_device(i_connection_info->device_id);
    
    if (*i_num_cores==0) {
        /* provide the number of cores */
        *i_num_cores = device->get_num_cores();
        return MCD_ERR_NONE;
    }

    for (uint32_t current_index = 0; current_index < *i_num_cores;current_index++) {
        /* get correct struct and core */
        mcd_core_con_info_st *ptr_to_current_st = &i_core_con_info[current_index];
        Core *core = device->get_core(i_start_index + current_index);

        /* copy provided data (system) */
        strncpy(ptr_to_current_st->system_key, i_connection_info->system_key, MCD_KEY_LEN);
        strncpy(ptr_to_current_st->system, i_connection_info->system, MCD_UNIQUE_NAME_LEN);
        strncpy(ptr_to_current_st->system_instance, i_connection_info->system_instance, MCD_UNIQUE_NAME_LEN);
        /* copy provided data (device) */
        strncpy(ptr_to_current_st->host, i_connection_info->host, MCD_HOSTNAME_LEN);
        ptr_to_current_st->server_port = i_connection_info->server_port;
        strncpy(ptr_to_current_st->device_key, i_connection_info->device_key, MCD_KEY_LEN);
        strncpy(ptr_to_current_st->acc_hw, i_connection_info->acc_hw, MCD_UNIQUE_NAME_LEN);
        ptr_to_current_st->device_type = i_connection_info->device_type;
        strncpy(ptr_to_current_st->device, i_connection_info->device, MCD_UNIQUE_NAME_LEN);
        ptr_to_current_st->device_id = i_connection_info->device_id;

        /* get new data from the core */
        core->get_qry_info(ptr_to_current_st);
    }
    return MCD_ERR_NONE;
}

mcd_return_et mcd_open_core_f(const mcd_core_con_info_st *i_core_con_info, mcd_core_st **i_core) {
    /* get the correct core */
    System *main_system = g_mcd_server->get_system(atoi(i_core_con_info->system_instance));
    Device *device = main_system->get_device(i_core_con_info->device_id);
    Core *core = device->get_core(i_core_con_info->core_id);

    /* sendo open core to qemu */
    int return_vaue = g_mcd_server->open_core(i_core_con_info->core_id);
    if (return_vaue!=0) return MCD_ERR_CONNECTION;

    /* allocate memory and return the core and all info on it */
    mcd_core_st *core_ptr = new mcd_core_st;
    core_ptr->core_con_info = i_core_con_info;
    core_ptr->instance = core;
    *i_core = core_ptr;

    return MCD_ERR_NONE;
}

mcd_return_et mcd_close_core_f(const mcd_core_st *i_core) {
    /* 1. tell qemu which core to close */
    int return_vaue = g_mcd_server->close_core(i_core->core_con_info->core_id);
    if (return_vaue!=0) return MCD_ERR_CONNECTION;
    /* 2. free memory */
    try {
        delete i_core;
    }
    catch (...) {
        g_mcd_server->set_last_error(MCD_ERR_CONNECTION, "failed deleting the core struct");
        return MCD_ERR_CONNECTION;
    }
    return MCD_ERR_NONE;
}

mcd_return_et mcd_qry_rst_classes_f(const mcd_core_st *i_core, uint32_t *i_rst_class_vector) {
    /* get info from correct core */
    Core *core = (Core*)i_core->instance;
    core->store_reset_data(*g_mcd_server);
    *i_rst_class_vector = core->get_reset_class_vector();

    return MCD_ERR_NONE;
}

mcd_return_et mcd_qry_rst_class_info_f(const mcd_core_st *i_core, uint8_t i_rst_class, mcd_rst_info_st *i_rst_info) {
    /* get info from correct core */
    Core *core = (Core*)i_core->instance;
    strncpy(i_rst_info->info_str, core->get_reset_info(i_rst_class).c_str(), MCD_INFO_STR_LEN);

    /* set the correct vector */
    uint32_t class_vector = 1;
    class_vector = class_vector << i_rst_class;
    i_rst_info->class_vector = class_vector;

    return MCD_ERR_NONE;
}

mcd_return_et mcd_rst_f(const mcd_core_st *i_core, uint32_t i_rst_class_vector, mcd_bool_t i_rst_and_halt) {
    /* get correct core */
    Core *core = (Core*)i_core->instance;
    uint8_t reset_id = 0;
    /* get correct reset */
    if (i_rst_class_vector == core->get_reset_class_vector()) {
        /* initial reset call in sys.up command -> just perform full system reset, because it includes all others */
        uint8_t reset_id = 0;
        core->execute_reset(*g_mcd_server, reset_id, i_rst_and_halt);
    }
    else if ((i_rst_class_vector | core->get_reset_class_vector()) == core->get_reset_class_vector()) {
        uint32_t current_vector = 1;
        while (reset_id<32) {
            if (current_vector!=i_rst_class_vector) {
                current_vector = current_vector << 1;
                reset_id++;
            }
            else {
                core->execute_reset(*g_mcd_server, reset_id, i_rst_and_halt);
            }
        }
    }
    else {
        return MCD_ERR_PARAM;
    }
    return MCD_ERR_NONE;
}

mcd_return_et mcd_qry_trig_info_f(const mcd_core_st *i_core, mcd_trig_info_st *i_trig_info) {
    /* get info from correct core */
    Core *core = (Core*)i_core->instance;
    core->store_trigger_info(*g_mcd_server);
    core->get_trigger_info(i_trig_info);

    return MCD_ERR_NONE;
}

mcd_return_et mcd_qry_ctrigs_f(const mcd_core_st *i_core, uint32_t i_start_index, uint32_t *i_num_ctrigs, mcd_ctrig_info_st *i_ctrig_info) {
    /* TODO: add custom trigger */
    return MCD_ERR_NONE;
}

mcd_return_et mcd_qry_traces_f(const mcd_core_st *i_core, uint32_t i_start_index, uint32_t *i_num_traces, mcd_trace_info_st *i_trace_info) {
    /* TODO: add trace support */
    return MCD_ERR_NONE;
}

mcd_return_et mcd_qry_mem_spaces_f(const mcd_core_st *i_core, uint32_t i_start_index, uint32_t *i_num_mem_spaces, mcd_memspace_st *i_mem_spaces) {
    /* get correct core */
    Core *core = (Core*)i_core->instance;


    if (*i_num_mem_spaces==0) {
        /* request info from qemu and provide the number of memory spaces */
        core->store_mem_space_info(*g_mcd_server);
        *i_num_mem_spaces = core->get_num_mem_spaces();
        return MCD_ERR_NONE;
    }

    for (uint32_t current_index = 0; current_index < *i_num_mem_spaces;current_index++) {
        /* get correct struct and memspace (mem_space ID has to start at 1 not at zero!) */
        mcd_memspace_st *ptr_to_current_st = &i_mem_spaces[current_index];
        /* copy data */
        core->qry_get_mem_space_data(i_start_index + current_index + 1, ptr_to_current_st);
    }
    return MCD_ERR_NONE;
}

mcd_return_et mcd_qry_reg_groups_f(const mcd_core_st *i_core, uint32_t i_start_index, uint32_t *i_num_reg_groups, mcd_register_group_st *i_reg_groups) {
    /* get correct core */
    Core *core = (Core*)i_core->instance;

    if (*i_num_reg_groups==0) {
        /* request info from qemu and provide the number of reg goups */
        core->store_reg_group_info(*g_mcd_server);
        *i_num_reg_groups = core->get_num_reg_groups();
        return MCD_ERR_NONE;
    }

    for (uint32_t current_index = 0; current_index < *i_num_reg_groups;current_index++) {
        /* get correct struct and reggroup (reg_group ID has to start at 1 not at zero!) */
        RegGroup reg_group = core->get_reg_group(i_start_index + current_index + 1);
        mcd_register_group_st *ptr_to_current_st = &i_reg_groups[current_index];

        /* copy data */
        reg_group.get_qry_info(ptr_to_current_st);
    }

    return MCD_ERR_NONE;
}

mcd_return_et mcd_qry_reg_map_f(const mcd_core_st *i_core, uint32_t i_reg_group_id, uint32_t i_start_index, uint32_t *i_num_regs, mcd_register_info_st *i_reg_info) {
    /* get correct core */
    Core *core = (Core*)i_core->instance;
    if (i_reg_group_id==0) {
        /* return all registers from all groups! */
        if (*i_num_regs == 0) {
            /* request info from qemu */
            if (core->get_num_registers() == 0) {
                core->store_reg_info(*g_mcd_server);
            }
            *i_num_regs = core->get_num_registers();
            return MCD_ERR_NONE;
        }
        core->qry_get_register_data(i_start_index, *i_num_regs, i_reg_info);
    }
    else {
        /* return the regs from the provided group (only used in DIAG mode) */
        RegGroup reg_group = core->get_reg_group(i_reg_group_id);
        if (*i_num_regs == 0) {
            /* request info from qemu */
            if (core->get_num_registers() == 0) {
                core->store_reg_info(*g_mcd_server);
            }
            *i_num_regs = reg_group.get_num_registers();
        }
        reg_group.qry_get_register_data(i_start_index, *i_num_regs, i_reg_info);
    }
    return MCD_ERR_NONE;
}

mcd_return_et mcd_close_server_f(const mcd_server_st *i_server) {
    /* close connection and free memory */
    MCDServer *myserver = (MCDServer*)i_server->instance;
    myserver->detach();
    myserver->tcp_close_socket();
    try {
        delete i_server;
    }
    catch (...) {
        g_mcd_server->set_last_error(MCD_ERR_CONNECTION, "failed deleting the server struct");
        return MCD_ERR_CONNECTION;
    }

    return MCD_ERR_NONE;
}

mcd_return_et mcd_qry_state_f(const mcd_core_st *i_core, mcd_core_state_st *i_state) {
    /* get right core and request current state */
    Core *core = (Core*)i_core->instance;
    int return_value = core->get_qry_state(*g_mcd_server, i_state);
    if (return_value != 0) return MCD_ERR_GENERAL;

    return MCD_ERR_NONE;
}

mcd_return_et mcd_run_f(const mcd_core_st *i_core, mcd_bool_t i_global) {
    if (i_global) {
        /*TODO: currently only working for one system -> add parent pointers in core and device */
        if (g_mcd_server->get_system(0)->execute_go(*g_mcd_server)!=0) {
            return MCD_ERR_GENERAL;
        }
    }
    else {
        Core *core = (Core*)i_core->instance;
        if (core->execute_go(*g_mcd_server)!=0) {
            return MCD_ERR_GENERAL;
        }
    }
    return MCD_ERR_NONE;
}

mcd_return_et mcd_stop_f(const mcd_core_st *i_core, mcd_bool_t i_global) {
    if (i_global) {
        /*TODO: currently only working for one system -> add parent pointers in core and device */
        if (g_mcd_server->get_system(0)->execute_break(*g_mcd_server)!=0) {
            return MCD_ERR_GENERAL;
        }
    }
    else {
        Core *core = (Core*)i_core->instance;
        if (core->execute_break(*g_mcd_server)!=0) {
            return MCD_ERR_GENERAL;
        }
    }
    return MCD_ERR_NONE;
}

mcd_return_et mcd_step_f(const mcd_core_st *i_core, mcd_bool_t i_global, mcd_core_step_type_et i_step_type, uint32_t i_n_steps) {
    if (i_global) {
        /*TODO: currently only working for one system -> add parent pointers in core and device */
        for (uint32_t index = 0; index< i_n_steps; index++) {
            if (g_mcd_server->get_system(0)->execute_step(*g_mcd_server)!=0) {
                return MCD_ERR_GENERAL;
            }
        }
    }
    else {
        Core *core = (Core*)i_core->instance;
        for (uint32_t index = 0; index< i_n_steps; index++) {
            if (core->execute_step(*g_mcd_server)!=0) {
                return MCD_ERR_GENERAL;
            }
        }
    }
    return MCD_ERR_NONE;
}

mcd_return_et mcd_execute_txlist_f(const mcd_core_st *i_core, mcd_txlist_st *i_txlist) {
    Core *core = (Core*)i_core->instance;
    int transaction_index, return_value;
    /* execute transaction by transaction */
    for (transaction_index = 0; transaction_index< i_txlist->num_tx; transaction_index++) {
        /* 1. get correct transaction */
        mcd_tx_st* current_transaction = &i_txlist->tx[transaction_index];
        /* 2. execute transaction */
        return_value = core->execute_transaction(current_transaction, *g_mcd_server);
        /* 3. increase counter if successfull */
        switch (return_value) {
            case ERROR_NONE:
                i_txlist->num_tx_ok++;
                break;
            case ERROR_READ:
                return MCD_ERR_TXLIST_READ;
                break;
            case ERROR_WRITE:
                return MCD_ERR_TXLIST_WRITE;
                break;
            case ERROR_TRANSACTON:
                return MCD_ERR_TXLIST_TX;
                break;
            default:
                return MCD_ERR_TXLIST_TX;
                break;
        } 
    }
    return MCD_ERR_NONE;
}

mcd_return_et mcd_create_trig_f(const mcd_core_st *i_core, void *i_trig, uint32_t *i_trig_id) {
    Core *core = (Core*)i_core->instance;
    /* get an available trigger id and store the trigger */
    uint32_t trig_id = core->get_num_trigger();
    core->get_valid_trig_id(trig_id);
    core->add_trigger(trig_id, *(mcd_trig_simple_core_st*)i_trig);
    *i_trig_id = trig_id;

    return MCD_ERR_NONE;
}

mcd_return_et mcd_activate_trig_set_f(const mcd_core_st *i_core) {
    /* activates all breakpoints (even the ones already activated) */
    Core *core = (Core*)i_core->instance;
    if (core->activate_trigger(*g_mcd_server) != ERROR_NONE) {
        return MCD_ERR_TRIG_ACCESS;
    }
    return MCD_ERR_NONE;
}

mcd_return_et mcd_remove_trig_f(const mcd_core_st *i_core, uint32_t i_trig_id) {
    Core *core = (Core*)i_core->instance;
    if (core->remove_trigger(*g_mcd_server, i_trig_id) != ERROR_NONE) {
        return MCD_ERR_TRIG_ACCESS;
    }
    return MCD_ERR_NONE;
}

void mcd_qry_error_info_f(const mcd_core_st *i_core, mcd_error_info_st *i_error_info) {
    /* we land here if an error occurs during the any mcd function */
    /* for now return a general error, this makes sure the app doens't crash during error search */
    i_error_info->error_code = -1;
    i_error_info->return_status = g_mcd_server->get_last_error().err_code;
    strncpy(i_error_info->error_str, g_mcd_server->get_last_error().err_info.c_str(), MCD_INFO_STR_LEN);
    i_error_info->error_events = MCD_ERR_EVT_NONE;
}

void mcd_exit_f(void) {
    /* do some final closing operations (free memory etc) */
}


/* unsused mcd api function that result in warnings if not declared */
mcd_return_et mcd_qry_servers_f(const mcd_char_t *i_host, mcd_bool_t i_running, uint32_t i_start_index, uint32_t *i_num_servers, mcd_server_info_st *i_server_info) {
    return MCD_ERR_FN_UNIMPLEMENTED;
}

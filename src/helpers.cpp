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
 * This file includes all helper functions.
 */

#include "mcdlib.h"
#include "mcd_shared_defines.h"
#include <stdexcept>
#include <set>
#include <sstream>

void extract_argument_from_config_string(const std::string& i_config_string, std::string *i_output, std::string i_lookup, std::string i_default_value) {

    i_lookup = i_lookup + "=";

    size_t pos1 = i_config_string.find(i_lookup);
    if (pos1 != std::string::npos) {
        size_t pos2 = i_config_string.find("\"", pos1 + i_lookup.length());
        if ((pos2 != std::string::npos) && (pos2 == pos1 + i_lookup.length())) {
            size_t pos3 = i_config_string.find("\"", pos2 + 1);
            if (pos3 != std::string::npos) {
                *i_output = i_config_string.substr(pos2 + 1, pos3 - pos2 - 1);
                return;
            }
        }
    }
    *i_output = i_default_value;
}

void deconstruct_tcp_data(const std::string i_origin, std::map<std::string, std::string> & i_argument_map, const int i_level) {
    std::string allocator;
    std::string arg_end;
    /* select separation characters */
    if (i_level==0) {
        allocator = "=";
        arg_end = ".";
    }
    else if (i_level==1) {
        allocator = ":";
        arg_end = ";";
    }
    /* start parsing */
    size_t pos1 = 0;
    size_t pos2, pos3;
    std::string key, value;
    while (true) {
        pos2 = i_origin.find(allocator, pos1);
        if (pos2 == std::string::npos) {
            return;
        }
        key = i_origin.substr(pos1, pos2 - pos1);
        pos2 += 1;
        pos3 = i_origin.find(arg_end, pos2);
        if (pos3 == std::string::npos) {
            return;
        }
        value = i_origin.substr(pos2, pos3 - pos2);
        /* add data to map */
        i_argument_map.insert({key, value});
        pos1 = pos3 + 1;
    }
}

uint8_t hex_char_to_int(char i_hex_char) {
    if(i_hex_char >= '0' && i_hex_char <= '9')
        return i_hex_char - '0';
    if(i_hex_char >= 'A' && i_hex_char <= 'F')
        return i_hex_char - 'A' + 10;
    if(i_hex_char >= 'a' && i_hex_char <= 'f')
        return i_hex_char - 'a' + 10;
    throw std::invalid_argument("Invalid input string");
}

char int_to_hex_char(uint8_t i_int) {
    if (i_int < 10) {
        return i_int + '0';
    }
    else if (i_int < 16) {
        return i_int - 10 + 'a';
    }
    throw std::invalid_argument("Invalid input int");
}

int hex_to_byte_array(std::string i_hex_data_str, mcd_tx_st* i_transaction) {
    uint8_t byte_num = 0;
    for (int index = 0; index < i_hex_data_str.size(); index+=2) {
        /* every 2 ascii hex characters form one byte! */
        uint8_t byte_value = hex_char_to_int(i_hex_data_str[index])*16 + hex_char_to_int(i_hex_data_str[index+1]);

        i_transaction->data[byte_num] = byte_value;
        byte_num++;
        i_transaction->num_bytes_ok++;
    }
    return 0;
}

int byte_array_to_hex(std::string& i_hex_data_str, mcd_tx_st* i_transaction) {
    for (int index = 0; index < i_transaction->num_bytes; index++) {
        uint8_t byte_value = i_transaction->data[index];
        i_hex_data_str.push_back(int_to_hex_char(byte_value >> 4));
        i_hex_data_str.push_back(int_to_hex_char(byte_value & 0xf));
        i_transaction->num_bytes_ok++;
    }
    return 0;
}

uint32_t atouint32_t(std::string const & i_string) {
    uint32_t res = 0;
    size_t i = 0;

    for (; i < i_string.size(); ++i)
    {
        const char c = i_string[i];
        if (not std::isdigit(c)) 
            throw std::runtime_error(std::string("Non-numeric character: ") + c);
        res *= 10;
        res += c - '0';
    }
    return res;
}

uint64_t atouint64_t(std::string const & i_string) {
    uint64_t res = 0;
    size_t i = 0;

    bool sign = false;

    if (i_string[i] == '-')
    {
        /* we use this to get the maximum number! anything oher than -1 is not excepted */
        res = -1;
        return res;
    }

    for (; i < i_string.size(); ++i)
    {
        const char c = i_string[i];
        if (not std::isdigit(c)) 
            throw std::runtime_error(std::string("Non-numeric character: ") + c);
        res *= 10;
        res += c - '0';
    }
    return res;
}

uint8_t atouint8_t(std::string const & i_string) {
    uint8_t res = 0;
    size_t i = 0;

    for (; i < i_string.size(); ++i)
    {
        const char c = i_string[i];
        if (not std::isdigit(c)) 
            throw std::runtime_error(std::string("Non-numeric character: ") + c);
        res *= 10;
        res += c - '0';
    }
    return res;
}

int trigger_to_qemu_breakpoint(uint32_t i_type) {
    switch (i_type) {
        case MCD_TRIG_TYPE_IP:
            return MCD_BREAKPOINT_HW;
        case MCD_TRIG_TYPE_READ:
            return MCD_BREAKPOINT_READ;
        case MCD_TRIG_TYPE_WRITE:
            return MCD_BREAKPOINT_WRITE;
        case MCD_TRIG_TYPE_RW:
            return MCD_BREAKPOINT_RW;
        default:
            return 0;
    }
}

std::set<std::string> ca15_id_regs {
    "PC",
	"CPSR",
	"SPSR_SVC",
	"R0",
	"R1",
	"R2",
	"R3",
	"R4",
	"R5",
	"R6",
	"R7",
	"R8_USR",
	"R9_USR",
	"R10_USR",
	"R11_USR",
	"R12_USR",
	"R13_SVC",
	"R14_SVC",
	"SPSR_FIQ", 
	"R8_FIQ",
	"R9_FIQ",
	"R10_FIQ",
	"R11_FIQ",
	"R12_FIQ",
	"R13_FIQ",
	"R14_FIQ",
	"R13_USR",
	"R14_USR",
	"SPSR_ABT",
	"R13_ABT",
	"R14_ABT",
	"SPSR_IRQ",
	"R13_IRQ",
	"R14_IRQ",
	"SPSR_UND",
	"R13_UND",
	"R14_UND",
	"DACR",
	"SCTLR",
	"TTBR0",
	"FCSEIDR",
	"TTBR1",
	"TTBCR",
	"CONTEXTIDR",
	"SPSR_MON",
	"R13_MON",
	"R14_MON",
	"SCR",
	"VBAR",
	"MVBAR",
	"SPSR_HYP",
	"R13_HYP",
	"R14_HYP",
	"HVBAR",
	"HTTBR",
	"HTCR",
    "VTTBR",
	"VTCR",
	"TTBR0_64",
	"TTBR1_64",
	"HCR",
	"S_VBAR",
	"N_VBAR",
	"S_FCSEIDR",
	"N_FCSEIDR",
	"S_CONTEXTIDR",
	"N_CONTEXTIDR",
    "S_DACR", 
	"N_DACR",
	"S_SCTLR",
	"N_SCTLR",
	"HSCTLR",
};

int arm_get_reg_access_type(std::string i_regname) {
    if (ca15_id_regs.find(i_regname) != ca15_id_regs.end()) {
        return 0;
    }
    return 1;
}

std::string my_to_string(int i_number) {
    std::ostringstream ss;
    ss << i_number;
    return ss.str();
}

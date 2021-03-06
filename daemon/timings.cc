/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2015 Couchbase, Inc
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include "timings.h"
#include <memcached/protocol_binary.h>
#include <platform/platform.h>
#include "timing_histogram.h"

Timings::Timings() {
    reset();
}

void Timings::reset(void) {
    for (int ii = 0; ii < MAX_NUM_OPCODES; ++ii) {
        timings[ii].reset();
    }
}

void Timings::collect(const uint8_t opcode, const hrtime_t nsec) {
    timings[opcode].add(nsec);
}

std::string Timings::generate(const uint8_t opcode) {
    return timings[opcode].to_string();
}

uint64_t Timings::get_aggregated_mutation_stats() {
    static uint8_t mutations[] = {
        PROTOCOL_BINARY_CMD_ADD,
        PROTOCOL_BINARY_CMD_ADDQ,
        PROTOCOL_BINARY_CMD_APPEND,
        PROTOCOL_BINARY_CMD_APPENDQ,
        PROTOCOL_BINARY_CMD_DECREMENT,
        PROTOCOL_BINARY_CMD_DECREMENTQ,
        PROTOCOL_BINARY_CMD_DELETE,
        PROTOCOL_BINARY_CMD_DELETEQ,
        PROTOCOL_BINARY_CMD_GAT,
        PROTOCOL_BINARY_CMD_GATQ,
        PROTOCOL_BINARY_CMD_INCREMENT,
        PROTOCOL_BINARY_CMD_INCREMENTQ,
        PROTOCOL_BINARY_CMD_PREPEND,
        PROTOCOL_BINARY_CMD_PREPENDQ,
        PROTOCOL_BINARY_CMD_REPLACE,
        PROTOCOL_BINARY_CMD_REPLACEQ,
        PROTOCOL_BINARY_CMD_SET,
        PROTOCOL_BINARY_CMD_SETQ,
        PROTOCOL_BINARY_CMD_TOUCH,
        PROTOCOL_BINARY_CMD_INVALID};

    uint64_t ret = 0;
    for (auto cmd : mutations) {
        ret += timings[cmd].get_total();
    }
    return ret;
}

uint64_t Timings::get_aggregated_retrival_stats() {
    static uint8_t retrival[] = {
        PROTOCOL_BINARY_CMD_GAT,
        PROTOCOL_BINARY_CMD_GATQ,
        PROTOCOL_BINARY_CMD_GET,
        PROTOCOL_BINARY_CMD_GETK,
        PROTOCOL_BINARY_CMD_GETKQ,
        PROTOCOL_BINARY_CMD_GETQ,
        PROTOCOL_BINARY_CMD_GET_LOCKED,
        PROTOCOL_BINARY_CMD_GET_RANDOM_KEY,
        PROTOCOL_BINARY_CMD_GET_REPLICA,
        PROTOCOL_BINARY_CMD_INVALID };

    uint64_t ret = 0;
    for (auto cmd : retrival) {
        ret += timings[cmd].get_total();
    }
    return ret;
}

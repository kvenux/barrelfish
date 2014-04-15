/*
 * Copyright (c) 2014, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Haldeneggsteig 4, CH-8092 Zurich. Attn: Systems Group.
 */

#ifndef COREBOOT_H
#define COREBOOT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <barrelfish/barrelfish.h>
#include <barrelfish/spawn_client.h>
#include <barrelfish/invocations_arch.h>
#include <barrelfish_kpi/capabilities.h>

#include <elf/elf.h>
#include <bench/bench.h>
#include <vfs/vfs.h>
#include <octopus/octopus.h>
#include <octopus/capability_storage.h>
#include <acpi_client/acpi_client.h>
#include <spawndomain/spawndomain.h>

#include <if/monitor_defs.h>
#include <if/monitor_blocking_rpcclient_defs.h>
#include <if/intermon_defs.h>

#define DEBUG

#ifdef DEBUG
#undef DEBUG
#define DEBUG(x...) debug_printf(x)
#else
#define DEBUG(x...) ((void)0)
#endif

// TODO(gz): Remove!
#pragma GCC diagnostic ignored "-Wunused-function"


// common.c
void boot_core_reply(struct monitor_binding *st, errval_t msgerr);
void power_down_response(struct monitor_binding *st, coreid_t target);
errval_t create_or_get_kcb_cap(coreid_t coreid);
errval_t give_kcb_to_new_core(coreid_t destination_id, struct capref new_kcb);
errval_t invoke_monitor_cap_remote(capaddr_t cap, int bits, bool is_remote,
                                   bool *has_descendents);
errval_t frame_alloc_identify(struct capref *dest, size_t bytes,
                              size_t *retbytes, struct frame_identity *id);
errval_t lookup_module(const char *module_name, lvaddr_t *binary_virt,
                       genpaddr_t *binary_phys, size_t *binary_size);
errval_t cap_mark_remote(struct capref cap);

// x86boot.c
errval_t spawn_xcore_monitor(coreid_t coreid, int hwid,
                             enum cpu_type cpu_type,
                             const char *cpu_ext,
                             const char *cmdline,
                             struct frame_identity urpc_frame_id);
errval_t invoke_start_core(void);
errval_t get_architecture_config(enum cpu_type type,
                                 genpaddr_t *arch_page_size,
                                 const char **monitor_binary,
                                 const char **cpu_binary);

#endif // COREBOOT_H
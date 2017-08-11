/** \file
 *  \brief Example message application
 */

/*
 * Copyright (c) 2010, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Haldeneggsteig 4, CH-8092 Zurich. Attn: Systems Group.
 */

#include <stdio.h>
#include <string.h>

#include <barrelfish/barrelfish.h>
#include <barrelfish/nameservice_client.h>
#include <barrelfish/dispatch.h>
#include <barrelfish_kpi/init.h>
#include <barrelfish/debug.h>
//#include <barrelfish/sys_debug.h>
//#include <barrelfish/syscall_arch.h>
//#include <barrelfish_kpi/sys_debug.h>
//#include <bench/bench.h>
//#include <if/bench_defs.h>
//#include <barrelfish/sys_debug.h>
//#include <arch/x86/barrelfish/perfmon.h>
//#include <arch/x86/barrelfish_kpi/perfmon_amd.h>

#include <if/xmplmsg_defs.h>

const char *service_name = "xmpl-msg_service";

/* --------------------- Client ------------------------------ */


static void send_string_cb(void *a)
{
    errval_t err;
    // send succesful, nothing to do
    struct xmplmsg_binding *b = (struct xmplmsg_binding*)a;
    debug_printf("send 3#\n");
    err = xmplmsg_msg_string__tx(b, NOP_CONT, "Fuck World Again!");
}


static void send_string_ready(void *a)
{
    errval_t err;

    struct xmplmsg_binding *b = (struct xmplmsg_binding*)a;
    struct event_closure txcont = MKCONT(send_string_cb, b);

    err = xmplmsg_msg_string__tx(b, txcont, "Fuck World");

    if (err_is_fail(err)) {
      DEBUG_ERR(err, "error sending msg_string message\n");
    }
}


static void send_ints_cb(void *a)
{
    errval_t err;

    struct xmplmsg_binding *b = (struct xmplmsg_binding*)a;
    struct event_closure txcont = MKCONT(send_string_cb, b);

    debug_printf("send 2#\n");
    err = xmplmsg_msg_string__tx(b, txcont, "Hello World");

    if (err_is_fail(err)) {
      DEBUG_ERR(err, "error sending msg_string message\n");
      if (err_no(err) == FLOUNDER_ERR_TX_BUSY) {
            struct waitset *ws = get_default_waitset();
	    txcont = MKCONT(send_string_ready, b);
            err = b->register_send(b, ws, txcont);
            if (err_is_fail(err)) {
                // note that only one continuation may be registered at a time
                DEBUG_ERR(err, "register_send on binding failed!");
            }
        }
    }
}

static void send_ints_ready(void *a)
{
    errval_t err;

    struct xmplmsg_binding *b = (struct xmplmsg_binding*)a;

    struct event_closure txcont = MKCONT(send_ints_cb, b);

    err = xmplmsg_msg_ints__tx(b, txcont, 0x1, 0x10);

    if (err_is_fail(err)) {
      DEBUG_ERR(err, "error sending msg_ints message\n");
    }
}

static void bind_cb(void *st, errval_t err, struct xmplmsg_binding *b)
{
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "bind failed failed");
    }

    struct event_closure txcont = MKCONT(send_ints_cb, b);

    debug_printf("send 1#\n");
    cycles_t cur_cycle = 0;
    //sys_debug_hardware_timer_read((uintptr_t *)&cur_cycle);
    //cur_cycle = bench_tsc();
    cur_cycle = rdtsc();
    //struct sysret sr = syscall2(SYSCALL_DEBUG, DEBUG_HARDWARE_TIMER_READ);
    //cur_cycle = sr.value;
    debug_printf("sys time %ld", cur_cycle);

    //err = xmplmsg_msg_ints__tx(b, txcont, 0x1, 0x10);
    err = xmplmsg_msg_ints__tx(b, txcont, (int)(cur_cycle>>32), (int)cur_cycle);

    if (err_is_fail(err)) {
      DEBUG_ERR(err, "error sending msg_ints message\n");

      if (err_no(err) == FLOUNDER_ERR_TX_BUSY) {
            struct waitset *ws = get_default_waitset();
	    txcont = MKCONT(send_ints_ready, b);
            err = b->register_send(b, ws, txcont);
            if (err_is_fail(err)) {
                // note that only one continuation may be registered at a time
                DEBUG_ERR(err, "register_send on binding failed!");
            }
        }
    }
}

static void start_client(void)
{
    errval_t err;
    iref_t iref;

    err = nameservice_blocking_lookup(service_name, &iref);
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "nameservice_blocking_lookup failed");
    }

    err = xmplmsg_bind(iref, 
                     bind_cb, 
                     NULL /* state for bind_cb */,
                     get_default_waitset(),
                     IDC_BIND_FLAGS_DEFAULT);
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "bind failed");
    }
}

/* --------------------- Main ------------------------------ */

int main(int argc, char *argv[]) 
{
    errval_t err;

    // debug_my_cspace();
    start_client();

    struct waitset *ws = get_default_waitset();
    while (1) {
        err = event_dispatch(ws);
        if (err_is_fail(err)) {
            DEBUG_ERR(err, "in event_dispatch");
            break;
        }
    }

    return EXIT_FAILURE;
}

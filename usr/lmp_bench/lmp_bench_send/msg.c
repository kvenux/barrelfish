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

#include <if/lmpbench_defs.h>

const char *service_name = "lmpbench_service";
#define NULL_CAP (struct capref){ /*cnode*/ NULL_CNODE, /*slot*/ 0 }

#define ITERATIONS      1000
static int currentiter;
static cycles_t cur_cycle = 0;

/* --------------------- Client ------------------------------ */


// static void send_ints_cb(void *a)
// {
//     errval_t err;

//     struct lmpbench_binding *b = (struct lmpbench_binding*)a;
//     struct event_closure txcont = MKCONT(send_string_cb, b);

//     debug_printf("send 2#\n");
//     err = lmpbench_msg_string__tx(b, txcont, "Hello World");

//     if (err_is_fail(err)) {
//       DEBUG_ERR(err, "error sending msg_string message\n");
//     }
// }

// static void send_ints_ready(void *a)
// {
//     errval_t err;

//     struct lmpbench_binding *b = (struct lmpbench_binding*)a;

//     struct event_closure txcont = MKCONT(send_ints_cb, b);

//     err = lmpbench_msg_ints__tx(b, txcont, 0x1, 0x10);

//     if (err_is_fail(err)) {
//       DEBUG_ERR(err, "error sending msg_ints message\n");
//     }
// }

static void send_ints_txcon(void *a)
{
    debug_printf("sending finished\n");
    return;
}

static void bind_cb(void *st, errval_t err, struct lmpbench_binding *b)
{
    char s[1000];
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "bind failed failed");
    }

    struct event_closure txcont = MKCONT(send_ints_txcon, b);

    // dispatcher_handle_t handle = curdispatcher();
    // struct capref dcb = get_dispatcher_generic(handle)->dcb_cap;
    // cap_invoke1(dcb, DispatcherCmd_GetEndpointsNum);

    // cap_invoke1(dcb, DispatcherCmd_GetEndpointsNum);

    debug_printf("send 1#\n");
    //sys_debug_hardware_timer_read((uintptr_t *)&cur_cycle);
    //cur_cycle = bench_tsc();
    cur_cycle = rdtsc();
    //struct sysret sr = syscall2(SYSCALL_DEBUG, DEBUG_HARDWARE_TIMER_READ);
    //cur_cycle = sr.value;
    debug_printf("sys time %ld\n", cur_cycle);
    err = lmpbench_msg_ints__tx(b, txcont, cur_cycle);

    //err = lmpbench_msg_ints__tx(b, txcont, 0x1, 0x10);
    // err = lmpbench_msg_ints__tx(b, txcont, (int)(cur_cycle>>32), (int)cur_cycle);

    for(currentiter = 0; currentiter < ITERATIONS; currentiter++) {
        thread_yield();
        cur_cycle = rdtsc();
        // err = lmpbench_msg_ints__tx(b, NOP_CONT, cur_cycle);
        sprintf(s, "%ld", cur_cycle);
        err = lmpbench_msg_string__tx(b, NOP_CONT, s);
        // err = lmpbench_msg_string__tx(b, NOP_CONT, "Fuck World Again!");
        thread_yield();
    
        if (err_is_fail(err)) {
          DEBUG_ERR(err, "error sending msg_string message\n");
          if(err_no(err) == FLOUNDER_ERR_TX_BUSY){
            thread_yield_dispatcher(NULL_CAP);
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

    err = lmpbench_bind(iref, 
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
    // dispatcher_handle_t handle = curdispatcher();
    // struct capref dcb = get_dispatcher_generic(handle)->dcb_cap;
    // int ep_num = cap_invoke1(dcb, DispatcherCmd_GetEndpointsNum).value;
    // debug_printf("Endpoint num: %d\n", ep_num);
    // // struct capability *cap = 0;
    // // int ep_cap = cap_invoke3(dcb, DispatcherCmd_GetAllEndpoints, 3, (uintptr_t)cap).value;
    // // debug_printf("Endpoint cap: %d\n", ep_cap);
    // // debug_printf("Endpoint cap: %x\n", cap);
    // debug_endpoints();
    // struct capref dst_capref = debug_get_endpoint(2);
    // struct capref ep_capref = debug_get_endpoint(7);
    // // debug_get_endpoint(2);
    // capaddr_t dst_capaddr = get_cap_addr(dst_capref);
    // debug_printf("Dest ep cap addr: %d\n", dst_capaddr);
    // capaddr_t ep_capaddr = get_cap_addr(ep_capref);
    // debug_printf("Grant ep cap addr: %d\n", ep_capaddr);
    // err = cap_invoke3(dcb, DispatcherCmd_GrantEndpointCap, ep_capaddr, dst_capaddr).error;

    // err = cap_invoke3(dcb, DispatcherCmd_GetAllEndpoints, 3, (uintptr_t)cap).value;


    // err = cap_invoke2(ep_capref, 9853, dst_capaddr).error;
    // err = invoke_dispatcher_dump_capabilities(dcb);

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

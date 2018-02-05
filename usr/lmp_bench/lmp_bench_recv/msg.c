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

#include <if/lmpbench_defs.h>

#define ITERATIONS      1000

const char *service_name = "lmpbench_service";
static long lmp_recv_cnt = 0;
static long lmp_recv_avg = 0;
static long recv_all_cnt = 0;
static long last_consumption = 0;

/* --------------------- Server ------------------------------ */

static void rx_msg_ints(struct lmpbench_binding *b, uint64_t ia)
{
    // debug_printf("server: received msg_ints:\n\tia:%d, ib:%d\n", ia);
    debug_printf("server: received msg_long: %ld\n", ia);
    cycles_t cur_cycle = 0;
    cur_cycle = rdtsc();
    debug_printf("server: time comsuming: %ld\n", cur_cycle-ia);
}

static void rx_msg_string(struct lmpbench_binding *b, const char *str)
{
    long cycle_recv = atol(str);
    cycles_t cur_cycle = 0;
    cur_cycle = rdtsc();
    recv_all_cnt++;
    // debug_printf("server: %ld# time comsuming: %ld\n", recv_all_cnt%100, cur_cycle - cycle_recv);
    if(cur_cycle - cycle_recv > last_consumption*10){
        last_consumption = cur_cycle - cycle_recv;
        return;
    }
    last_consumption = cur_cycle - cycle_recv;
    lmp_recv_avg = (lmp_recv_avg*lmp_recv_cnt + (cur_cycle - cycle_recv))/(lmp_recv_cnt+1);
    lmp_recv_cnt++;
    if(recv_all_cnt == ITERATIONS){
        printf("lmp_bench recv avg time comsuming: %ld\n", lmp_recv_avg);
    }
    if(recv_all_cnt == ITERATIONS){
        errval_t err;
        dispatcher_handle_t handle = curdispatcher();
        struct capref dcb = get_dispatcher_generic(handle)->dcb_cap;
        // int ep_num = cap_invoke1(dcb, DispatcherCmd_GetEndpointsNum).value;
        // debug_printf("Endpoint num: %d\n", ep_num);
        struct capref dst_capref = debug_get_endpoint(4);
        struct capref ep_capref = debug_get_endpoint(3);
        // // debug_get_endpoint(2);
        capaddr_t dst_capaddr = get_cap_addr(dst_capref);
        // debug_printf("Dest ep cap addr: %d\n", dst_capaddr);
        capaddr_t ep_capaddr = get_cap_addr(ep_capref);
        // debug_printf("Grant ep cap addr: %d\n", ep_capaddr);
        // err = cap_invoke3(dcb, DispatcherCmd_GrantEndpointCap, ep_capaddr, dst_capaddr).error;
    
        // err = cap_invoke3(dcb, DispatcherCmd_GetAllEndpoints, 3, (uintptr_t)cap).value;
        struct capref remove_capref = debug_get_endpoint(2);
        capaddr_t remove_capaddr = get_cap_addr(remove_capref);
        int remove_caplevel = get_cap_level(remove_capref);
        // capaddr_t remove_cnode_capaddr get_croot_addr(cap);
        // struct capref croot = get_croot_capref(remove_capref);
        // err = invoke_cnode_delete(croot, remove_capaddr, remove_caplevel);
        // cap_invoke1(dcb, DispatcherCmd_GetEndpointsNum);
    
        long remove_time_avg = 0;
        long remove_num = 0;
        for(int i=0;i< 1000;i++){
            long start = rdtsc();
            err = cap_invoke3(dcb, DispatcherCmd_RemoveEndpointCap, remove_capaddr, remove_caplevel).error;
            long end = rdtsc();
            remove_time_avg = (remove_time_avg*remove_num + (end-start))/(remove_num + 1);
            remove_num++;
        }
        printf("remove benchmark consuming: %ld\n", remove_time_avg);

        long grant_time_avg = 0;
        long grant_num = 0;
        for(int i=0;i< 1000;i++){
            long start = rdtsc();
            err = cap_invoke3(dcb, DispatcherCmd_GrantEndpointCap, ep_capaddr, dst_capaddr).error;
            long end = rdtsc();
            grant_time_avg = (grant_time_avg*grant_num + (end-start))/(grant_num + 1);
            grant_num++;
        }
        printf("grant benchmark consuming: %ld\n", grant_time_avg);
    }
    // debug_printf("server: received msg_string:\n\t%s\n", str);
    // debug_endpoints();
    // dispatcher_handle_t handle = curdispatcher();
    // struct capref dcb = get_dispatcher_generic(handle)->dcb_cap;
    // cap_invoke1(dcb, DispatcherCmd_GetEndpointsNum);
    // free(str);
}

static struct lmpbench_rx_vtbl rx_vtbl = {
    .msg_ints = rx_msg_ints,
    .msg_string = rx_msg_string,
};


static errval_t connect_cb(void *st, struct lmpbench_binding *b)
{
    b->rx_vtbl = rx_vtbl;

    return SYS_ERR_OK;
}


static void export_cb(void *st, errval_t err, iref_t iref)
{
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "export failed");
    }

    err = nameservice_register(service_name, iref);
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "nameservice_register failed");
    }
}

static void start_server(void)
{
    errval_t err;

    debug_printf("server before exporting\n");
    err = lmpbench_export(NULL /* state pointer for connect/export callbacks */,
			  export_cb, connect_cb,
			  get_default_waitset(),
			  IDC_EXPORT_FLAGS_DEFAULT);
    if (err_is_fail(err)) {
        USER_PANIC_ERR(err, "export failed");
    }
}

/* --------------------- Main ------------------------------ */

int main(int argc, char *argv[])
{
    errval_t err;

    // debug_my_cspace();
    debug_endpoints();
    // dispatcher_handle_t handle = curdispatcher();
    // struct capref dcb = get_dispatcher_generic(handle)->dcb_cap;
    // cap_invoke1(dcb, DispatcherCmd_GetEndpointsNum);
    // err = cap_retype(cap_selfep, cap_dispatcher, 0, ObjType_EndPoint, 0, 1);
    // if (err_is_fail(err)) {
    //     DEBUG_ERR(err, "Failed to create our endpoint to self");
    //     abort();
    // }
    // dispatcher_handle_t handle = curdispatcher();
    // struct capref dcb = get_dispatcher_generic(handle)->dcb_cap;
    // int ep_num = cap_invoke1(dcb, DispatcherCmd_GetEndpointsNum).value;
    // debug_printf("Endpoint num: %d\n", ep_num);
    start_server();
    // ep_num = cap_invoke1(dcb, DispatcherCmd_GetEndpointsNum).value;
    // debug_printf("Endpoint num: %d\n", ep_num);

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

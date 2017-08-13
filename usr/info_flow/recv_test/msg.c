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

#include <if/xmplmsg_defs.h>

const char *service_name = "xmpl-msg_service";

/* --------------------- Server ------------------------------ */

static void rx_msg_ints(struct xmplmsg_binding *b, int ia, int ib)
{
    debug_printf("server: received msg_ints:\n\tia:%d, ib:%d\n", ia, ib);
    long c = (long)ia << 32 | ib;
    debug_printf("server: received msg_long: %ld\n", c);
    cycles_t cur_cycle = 0;
    cur_cycle = rdtsc();
    debug_printf("server: time comsuming: %ld\n", cur_cycle-c);
}


static void rx_msg_string(struct xmplmsg_binding *b, const char *str)
{
    debug_printf("server: received msg_string:\n\t%s\n", str);
    // free(str);
}

static struct xmplmsg_rx_vtbl rx_vtbl = {
    .msg_ints = rx_msg_ints,
    .msg_string = rx_msg_string,
};

static errval_t connect_cb(void *st, struct xmplmsg_binding *b)
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
    err = xmplmsg_export(NULL /* state pointer for connect/export callbacks */,
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

    // err = cap_retype(cap_selfep, cap_dispatcher, 0, ObjType_EndPoint, 0, 1);
    // if (err_is_fail(err)) {
    //     DEBUG_ERR(err, "Failed to create our endpoint to self");
    //     abort();
    // }
    start_server();

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

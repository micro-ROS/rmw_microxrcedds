// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "../types.h"

#include <rmw_microxrcedds_c/config.h>
#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rmw/ret_types.h>
#include <rmw/error_handling.h>

bool __attribute__((weak)) custom_transport_open(struct uxrCustomTransport * transport) 
{
    (void) transport;
    return false;
}

bool __attribute__((weak)) custom_transport_close(struct uxrCustomTransport * transport)
{
    (void) transport;
    return false;
}

size_t __attribute__((weak)) custom_transport_write(struct uxrCustomTransport* transport, const uint8_t * buf, size_t len, uint8_t * err)
{
    (void) transport;
    (void) buf;
    (void) len;
    (void) err;
    return 0;
}

size_t __attribute__((weak)) custom_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err)
{
    (void) transport;
    (void) buf;
    (void) len;
    (void) timeout;
    (void) err;
    return 0;
}

rmw_uxrce_transport_params_t rmw_uxrce_transport_default_params =
{ 
    .framing = false,
    .args = NULL,
    .open_cb = custom_transport_open,
    .close_cb = custom_transport_close,
    .write_cb = custom_transport_write,
    .read_cb = custom_transport_read
};

rmw_ret_t rmw_uros_set_custom_transport(
        bool framing,
        void* args,
        open_custom_func open_cb,
        close_custom_func close_cb,
        write_custom_func write_cb,
        read_custom_func read_cb)
{
    if (NULL != open_cb &&
            NULL != close_cb &&
            NULL != write_cb &&
            NULL != read_cb)
    {
        rmw_uxrce_transport_default_params.framing  = framing;
        rmw_uxrce_transport_default_params.args     = args;
        rmw_uxrce_transport_default_params.open_cb  = open_cb;
        rmw_uxrce_transport_default_params.close_cb = close_cb;
        rmw_uxrce_transport_default_params.write_cb = write_cb;
        rmw_uxrce_transport_default_params.read_cb  = read_cb;
    }
    else
    {
        RMW_SET_ERROR_MSG("Uninitialised arguments.");
        return(RMW_RET_INVALID_ARGUMENT);
    }
    return(RMW_RET_OK);
}
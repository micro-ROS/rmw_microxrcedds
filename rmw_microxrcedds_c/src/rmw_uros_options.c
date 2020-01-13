#include "rmw_uros/options.h"

#include "types.h"

#include <rmw_microxrcedds_c/config.h>
#include <rmw/allocators.h>
#include <rmw/ret_types.h>
#include <rmw/error_handling.h>

rmw_ret_t rmw_uros_init_options(int argc, const char* const argv[], rmw_init_options_t* rmw_options)
{
    if (NULL == rmw_options)
    {
        RMW_SET_ERROR_MSG("Uninitialised rmw_init_options.");
        return RMW_RET_INVALID_ARGUMENT;
    }
    rmw_ret_t ret = RMW_RET_OK;
    // rmw_options->impl = rmw_options->allocator.allocate(sizeof(rmw_init_options_impl_t), rmw_options->allocator.state);
#ifdef MICRO_XRCEDDS_SERIAL
    if (argc >= 2)
    {
        strcpy(rmw_options->impl->serial_device, argv[1]);
    }
    else
    {
        RMW_SET_ERROR_MSG("Wrong number of arguments in rmw options. Needs one argument with the serial device.");
        ret = RMW_RET_INVALID_ARGUMENT;
    }
    
#elif defined(MICRO_XRCEDDS_UDP)
    if (argc >= 3)
    {
        strcpy(rmw_options->impl->agent_address, argv[1]);
        strcpy(rmw_options->impl->agent_port, argv[2]);
    }
    else
    {
        RMW_SET_ERROR_MSG("Wrong number of arguments in rmw options. Needs an Agent IP and port.");
        ret = RMW_RET_INVALID_ARGUMENT;
    }
#endif
    return ret;
}

rmw_ret_t rmw_uros_options_set_serial_device(const char* dev, rmw_init_options_t* rmw_options)
{   
#ifdef MICRO_XRCEDDS_SERIAL
    if (NULL == rmw_options)
    {
        RMW_SET_ERROR_MSG("Uninitialised rmw_init_options.");
        return RMW_RET_INVALID_ARGUMENT;
    }

    if(strlen(dev) <= MAX_SERIAL_DEVICE){
        strcpy(rmw_options->impl->serial_device, dev);
    }else{
        RMW_SET_ERROR_MSG("serial port configuration overflow");
        return RMW_RET_INVALID_ARGUMENT;
    }   
    return RMW_RET_OK;
#else
    (void) dev;
    (void) rmw_options;

    RMW_SET_ERROR_MSG("MICRO_XRCEDDS_SERIAL not set.");
    return RMW_RET_INVALID_ARGUMENT;
#endif
}

rmw_ret_t rmw_uros_options_set_udp_address(const char* ip, const char* port, rmw_init_options_t* rmw_options)
{
#ifdef MICRO_XRCEDDS_UDP
    if (NULL == rmw_options)
    {
        RMW_SET_ERROR_MSG("Uninitialised rmw_init_options.");
        return RMW_RET_INVALID_ARGUMENT;
    }

    if(strlen(ip) <= MAX_IP_LEN){
        strcpy(rmw_options->impl->agent_address, ip);
    }else{
        RMW_SET_ERROR_MSG("default ip configuration overflow");
        return RMW_RET_INVALID_ARGUMENT;
    }
    
    if(strlen(port) <= MAX_PORT_LEN){
        strcpy(rmw_options->impl->agent_port, port);
    }else{
        RMW_SET_ERROR_MSG("default port configuration overflow");
        return RMW_RET_INVALID_ARGUMENT;
    }

    return RMW_RET_OK;
#else
    (void) ip;
    (void) port;
    (void) rmw_options;

    RMW_SET_ERROR_MSG("MICRO_XRCEDDS_UDP not set.");
    return RMW_RET_INVALID_ARGUMENT;
#endif
}

rmw_ret_t rmw_uros_options_set_client_key(uint32_t client_key, rmw_init_options_t* rmw_options)
{
    if (NULL == rmw_options)
    {
        RMW_SET_ERROR_MSG("Uninitialised rmw_init_options.");
        return RMW_RET_INVALID_ARGUMENT;
    }

    rmw_options->impl->client_key = client_key;

    return RMW_RET_OK;
}

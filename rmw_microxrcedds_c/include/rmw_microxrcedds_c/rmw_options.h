#ifndef RMW_MICROXRCEDDS_C__RMW_OPTIONS_H
#define RMW_MICROXRCEDDS_C__RMW_OPTIONS_H

typedef struct  rmw_init_options_impl_t
{
  // #ifdef MICRO_XRCEDDS_SERIAL
    char serial_device[50];
  // #elif defined(MICRO_XRCEDDS_UDP)
    char agent_address[16];
    uint16_t agent_port;
  // #endif

} rmw_init_options_impl_t;

#endif // !RMW_MICROXRCEDDS_C__RMW_OPTIONS_H
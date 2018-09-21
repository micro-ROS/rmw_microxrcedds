#ifndef RMW_PUBLISHER_H_
#define RMW_PUBLISHER_H_

#include <rmw/types.h>
#include <rosidl_generator_c/message_type_support_struct.h>

rmw_publisher_t* create_publisher(const rmw_node_t* node, const rosidl_message_type_support_t* type_support,
                                  const char* topic_name, const rmw_qos_profile_t* qos_policies);

#endif // !RMW_PUBLISHER_H_
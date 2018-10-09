#ifndef RMW_SUBSCRIBER_H_
#define RMW_SUBSCRIBER_H_

#include <rmw/types.h>
#include <rosidl_generator_c/message_type_support_struct.h>

rmw_subscription_t* create_subscriber(const rmw_node_t* node, const rosidl_message_type_support_t* type_support,
                                      const char* topic_name, const rmw_qos_profile_t* qos_policies,
                                      bool ignore_local_publications);

#endif // !RMW_SUBSCRIBER_H_
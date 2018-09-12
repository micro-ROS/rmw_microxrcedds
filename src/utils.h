#ifndef RMW_MICRORTPS_UTILS_H_
#define RMW_MICRORTPS_UTILS_H_

#include "types.h"

#include <rmw/rmw.h>

#define EPROS_PRINT_TRACE() ; // printf("func %s, in file %s:%d\n", __func__, __FILE__, __LINE__);

void rmw_delete(void* rmw_allocated_ptr);
void rmw_node_delete(rmw_node_t* node);
void rmw_publisher_delete(rmw_publisher_t* publisher);
void rmw_subscription_delete(rmw_subscription_t* subscriber);

void customnode_clear(CustomNode* node);

int generate_name(const mrObjectId* id, char name[], size_t buffer_size);
int generate_type_name(const message_type_support_callbacks_t* members, const char* sep, char type_name[],
                       size_t buffer_size);

int build_participant_xml(size_t domain_id, const char* participant_name, char xml[], size_t buffer_size);
int build_publisher_xml(const char* publisher_name, char xml[], size_t buffer_size);
int build_subscriber_xml(const char* subscriber_name, char xml[], size_t buffer_size);
int build_topic_xml(const char* topic_name, const message_type_support_callbacks_t* members,
                    const rmw_qos_profile_t* qos_policies, char xml[], size_t buffer_size);
int build_datawriter_xml(const char* topic_name, const message_type_support_callbacks_t* members,
                         const rmw_qos_profile_t* qos_policies, char xml[], size_t buffer_size);
int build_datareader_xml(const char* topic_name, const message_type_support_callbacks_t* members,
                         const rmw_qos_profile_t* qos_policies, char xml[], size_t buffer_size);

#endif // !UTILS_H
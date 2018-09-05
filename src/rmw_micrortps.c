#include "rmw_micrortps.h"

#include "identifier.h"
#include "rmw_node.h"
#include "rmw_publisher.h"
#include "rmw_subscriber.h"
#include "utils.h"

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rosidl_typesupport_micrortps_c/identifier.h"

#include <micrortps/client/client.h>

// Default array mem size
#ifndef rmw_micrortps_default_Mem_size
#define rmw_micrortps_default_Mem_size 5
#endif


// Wait set
#ifndef rmw_micrortps_Internal_wait_set_size
#define rmw_micrortps_Internal_wait_set_size rmw_micrortps_default_Mem_size
#endif

struct private_wait_set_t
{
    rmw_wait_set_t wait_set;
    struct private_wait_set_t *Next;
};

struct private_wait_set_t Internal_wait_set_Mem[rmw_micrortps_Internal_wait_set_size];
struct private_wait_set_t *Unused_Internal_wait_set = NULL;



const char* rmw_get_implementation_identifier()
{
    EPROS_PRINT_TRACE()
    return eprosima_micrortps_identifier;
}

rmw_ret_t rmw_init()
{
    EPROS_PRINT_TRACE()


    // initialize wait set memory 
    Unused_Internal_wait_set = Internal_wait_set_Mem;
    for (unsigned i=0; i < rmw_micrortps_Internal_wait_set_size - 1; i++)
    {
        Internal_wait_set_Mem[i].Next = &Internal_wait_set_Mem[i++];
    }
    Internal_wait_set_Mem[rmw_micrortps_Internal_wait_set_size - 1].Next = NULL;


    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_node_t* rmw_create_node(const char* name, const char* namespace, size_t domain_id,
                            const rmw_node_security_options_t* security_options)
{
    EPROS_PRINT_TRACE()
    rmw_node_t* rmw_node = NULL;
    if (!name || strlen(name) == 0)
    {
        RMW_SET_ERROR_MSG("name is null");
    }
    else if (!namespace || strlen(namespace) == 0)
    {
        RMW_SET_ERROR_MSG("node handle not from this implementation");
    }
    else if (!security_options)
    {
        RMW_SET_ERROR_MSG("security_options is null");
    }
    else
    {
        rmw_node = create_node(name, namespace, domain_id);
    }
    return rmw_node;
}

const rmw_guard_condition_t* rmw_node_get_graph_guard_condition(const rmw_node_t* node)
{
    // TODO
    (void)node;
    EPROS_PRINT_TRACE()
    rmw_guard_condition_t* ret     = (rmw_guard_condition_t*)rmw_allocate(sizeof(rmw_guard_condition_t));
    ret->data                      = NULL;
    ret->implementation_identifier = eprosima_micrortps_identifier;
    return ret;
}

rmw_publisher_t* rmw_create_publisher(const rmw_node_t* node, const rosidl_message_type_support_t* type_support,
                                      const char* topic_name, const rmw_qos_profile_t* qos_policies)
{
    EPROS_PRINT_TRACE()
    rmw_publisher_t* rmw_publisher = NULL;
    if (!node)
    {
        RMW_SET_ERROR_MSG("node handle is null");
    }
    else if (!type_support)
    {
        RMW_SET_ERROR_MSG("type support is null");
    }
    else if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) != 0)
    {
        RMW_SET_ERROR_MSG("node handle not from this implementation");
    }
    else if (!topic_name || strlen(topic_name) == 0)
    {
        RMW_SET_ERROR_MSG("publisher topic is null or empty string");
        return NULL;
    }
    else if (!qos_policies)
    {
        RMW_SET_ERROR_MSG("qos_profile is null");
        return NULL;
    }
    else
    {
        rmw_publisher = create_publisher(node, type_support, topic_name, qos_policies);
    }
    return rmw_publisher;
}

rmw_ret_t rmw_publish_serialized_message(const rmw_publisher_t* publisher,
                                         const rmw_serialized_message_t* serialized_message)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_serialize(const void* ros_message, const rosidl_message_type_support_t* type_support,
                        rmw_serialized_message_t* serialized_message)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_deserialize(const rmw_serialized_message_t* serialized_message,
                          const rosidl_message_type_support_t* type_support, void* ros_message)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_subscription_t* rmw_create_subscription(const rmw_node_t* node, const rosidl_message_type_support_t* type_support,
                                            const char* topic_name, const rmw_qos_profile_t* qos_policies,
                                            bool ignore_local_publications)
{
    EPROS_PRINT_TRACE()
    rmw_subscription_t* rmw_subscription = NULL;
    if (!node)
    {
        RMW_SET_ERROR_MSG("node handle is null");
    }
    else if (!type_support)
    {
        RMW_SET_ERROR_MSG("type support is null");
    }
    else if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) != 0)
    {
        RMW_SET_ERROR_MSG("node handle not from this implementation");
    }
    else if (!topic_name || strlen(topic_name) == 0)
    {
        RMW_SET_ERROR_MSG("subscription topic is null or empty string");
        return NULL;
    }
    else if (!qos_policies)
    {
        RMW_SET_ERROR_MSG("qos_profile is null");
        return NULL;
    }
    else
    {
        rmw_subscription = create_subscriber(node, type_support, topic_name, qos_policies, ignore_local_publications);
    }
    return rmw_subscription;
}

rmw_ret_t rmw_take(const rmw_subscription_t* subscription, void* ros_message, bool* taken)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_take_with_info(const rmw_subscription_t* subscription, void* ros_message, bool* taken,
                             rmw_message_info_t* message_info)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_take_serialized_message(const rmw_subscription_t* subscription,
                                      rmw_serialized_message_t* serialized_message, bool* taken)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_take_serialized_message_with_info(const rmw_subscription_t* subscription,
                                                rmw_serialized_message_t* serialized_message, bool* taken,
                                                rmw_message_info_t* message_info)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_client_t* rmw_create_client(const rmw_node_t* node, const rosidl_service_type_support_t* type_support,
                                const char* service_name, const rmw_qos_profile_t* qos_policies)
{
    EPROS_PRINT_TRACE()
    return NULL;
}

rmw_ret_t rmw_destroy_client(rmw_node_t* node, rmw_client_t* client)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_send_request(const rmw_client_t* client, const void* ros_request, int64_t* sequence_id)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_take_response(const rmw_client_t* client, rmw_request_id_t* request_header, void* ros_response,
                            bool* taken)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_service_t* rmw_create_service(const rmw_node_t* node, const rosidl_service_type_support_t* type_support,
                                  const char* service_name, const rmw_qos_profile_t* qos_policies)
{
    EPROS_PRINT_TRACE()
    return NULL;
}

rmw_ret_t rmw_destroy_service(rmw_node_t* node, rmw_service_t* service)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_take_request(const rmw_service_t* service, rmw_request_id_t* request_header, void* ros_request,
                           bool* taken)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_send_response(const rmw_service_t* service, rmw_request_id_t* request_header, void* ros_response)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_guard_condition_t* rmw_create_guard_condition(void)
{
    EPROS_PRINT_TRACE()
    return NULL;
}

rmw_ret_t rmw_destroy_guard_condition(rmw_guard_condition_t* guard_condition)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_trigger_guard_condition(const rmw_guard_condition_t* guard_condition)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_wait_set_t* rmw_create_wait_set(size_t max_conditions)
{
    EPROS_PRINT_TRACE()

    (void)max_conditions;

    struct private_wait_set_t* Retured_wait_set;
  
    // Check if there are any available
    if (Unused_Internal_wait_set == NULL)
    {
        RMW_SET_ERROR_MSG("Not available wait set");
        return NULL;
    }

    // Get first
    Retured_wait_set = Unused_Internal_wait_set;

    // Remove given from the stack
    Unused_Internal_wait_set = Unused_Internal_wait_set->Next;
    Retured_wait_set->Next = NULL;

    // Configure 
    Retured_wait_set->wait_set.data = (void*)Retured_wait_set;
    Retured_wait_set->wait_set.guard_conditions = NULL;
    Retured_wait_set->wait_set.implementation_identifier = rmw_get_implementation_identifier();
    
    // Return vaule
    EPROS_PRINT_TRACE()
    return &Retured_wait_set->wait_set;
}

rmw_ret_t rmw_destroy_wait_set(rmw_wait_set_t* wait_set)
{
    EPROS_PRINT_TRACE()

    // Check if null
    if (wait_set == NULL)
    {
        RMW_SET_ERROR_MSG("wait_set pointer is null");
        return RMW_RET_ERROR;
    }
    if (wait_set->data == NULL)
    {
        RMW_SET_ERROR_MSG("wait_set->data is null");
        return RMW_RET_ERROR;
    }

    // Extract
    struct private_wait_set_t* Internal_wait_set = wait_set->data;
    
    
    //> Add to unused stack
    Internal_wait_set->Next = Unused_Internal_wait_set;
    Unused_Internal_wait_set = Internal_wait_set;
    
    
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_wait(rmw_subscriptions_t* subscriptions, rmw_guard_conditions_t* guard_conditions,
                   rmw_services_t* services, rmw_clients_t* clients, rmw_wait_set_t* wait_set,
                   const rmw_time_t* wait_timeout)
{
    EPROS_PRINT_TRACE()
    

    
    
    
    return RMW_RET_OK;
}

rmw_ret_t rmw_get_node_names(const rmw_node_t* node, rcutils_string_array_t* node_names)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_count_publishers(const rmw_node_t* node, const char* topic_name, size_t* count)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_count_subscribers(const rmw_node_t* node, const char* topic_name, size_t* count)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_get_gid_for_publisher(const rmw_publisher_t* publisher, rmw_gid_t* gid)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_compare_gids_equal(const rmw_gid_t* gid1, const rmw_gid_t* gid2, bool* result)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_service_server_is_available(const rmw_node_t* node, const rmw_client_t* client, bool* is_available)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_set_log_6severity(rmw_log_severity_t severity)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_get_topic_names_and_types(const rmw_node_t* node, rcutils_allocator_t* allocator, bool no_demangle,
                                        rmw_names_and_types_t* topic_names_and_types)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_get_service_names_and_types(const rmw_node_t* node, rcutils_allocator_t* allocator,
                                          rmw_names_and_types_t* service_names_and_types)
{
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}
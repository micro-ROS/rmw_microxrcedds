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

#include <limits.h>

// Declare internal memory structs
//RMW_MICRORTPS_DECLARE_INTENAL_MEMORY(Internal_wait_set_t, INTERNAL_WAIT_SET)

const char* rmw_get_implementation_identifier()
{
    EPROS_PRINT_TRACE()
    return eprosima_micrortps_identifier;
}

rmw_ret_t rmw_init()
{
    EPROS_PRINT_TRACE()

    // initialize internal memories
    //RMW_MICRORTPS_INIT_INTERNAL_MEM(INTERNAL_WAIT_SET);

    init_rmw_node();

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
    else if (strcmp(type_support->typesupport_identifier, rmw_get_implementation_identifier()) != 0)
    {
        RMW_SET_ERROR_MSG("TypeSupport handle not from this implementation");
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

    // Extract subscriber info



    // Extract serialiced message



    // Desserialice using typesupport



    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_take_with_info(const rmw_subscription_t* subscription, void* ros_message, bool* taken,
                             rmw_message_info_t* message_info)
{
    EPROS_PRINT_TRACE()

    // Not used variables
    (void) message_info; 


    // Preconfigure taken
    if (taken != NULL)
    {
        *taken = false;
    }


    // Check id
    if (strcmp(subscription->implementation_identifier, rmw_get_implementation_identifier()) != 0)
    {
        RMW_SET_ERROR_MSG("Wrong implementation");
        return RMW_RET_ERROR;
    }


    // Extract subscriber info
    CustomSubscription *custom_subscription = (CustomSubscription *)subscription->data;


    // Check reading zone
    Endianness endianness;
    if (custom_subscription->TmpRawBuffer.Read == custom_subscription->TmpRawBuffer.Write)
    {
        RMW_SET_ERROR_MSG("Nothing to be read from temporal raw buffer");
        return RMW_RET_ERROR;
    }
    if ((custom_subscription->TmpRawBuffer.Read + sizeof(endianness) + sizeof(custom_subscription->TmpRawBuffer.RawDataSize)) >= custom_subscription->TmpRawBuffer.Write)
    {
        RMW_SET_ERROR_MSG("Error in raw buffer. Temporal raw buffer will be restarted");

        custom_subscription->TmpRawBuffer.Read = custom_subscription->TmpRawBuffer.MemHead;
        custom_subscription->TmpRawBuffer.Write = custom_subscription->TmpRawBuffer.MemHead;
        custom_subscription->TmpRawBuffer.MemTail = &custom_subscription->TmpRawBuffer.MemHead[sizeof(custom_subscription->TmpRawBuffer.MemHead)];

        return RMW_RET_ERROR;
    }


    // Extract raw message from temporal buffer
    memcpy(&endianness, custom_subscription->TmpRawBuffer.Read, sizeof(endianness));
    custom_subscription->TmpRawBuffer.Read += sizeof(endianness);

    memcpy(&custom_subscription->TmpRawBuffer.RawDataSize, custom_subscription->TmpRawBuffer.Read, sizeof(custom_subscription->TmpRawBuffer.RawDataSize));
    custom_subscription->TmpRawBuffer.Read += sizeof(custom_subscription->TmpRawBuffer.RawDataSize);

    if ((custom_subscription->TmpRawBuffer.Read + custom_subscription->TmpRawBuffer.RawDataSize) > custom_subscription->TmpRawBuffer.Write)
    {
        RMW_SET_ERROR_MSG("Error in raw buffer. Temporal raw buffer will be restarted");

        custom_subscription->TmpRawBuffer.Read = custom_subscription->TmpRawBuffer.MemHead;
        custom_subscription->TmpRawBuffer.Write = custom_subscription->TmpRawBuffer.MemHead;
        custom_subscription->TmpRawBuffer.MemTail = &custom_subscription->TmpRawBuffer.MemHead[sizeof(custom_subscription->TmpRawBuffer.MemHead)];

        return RMW_RET_ERROR;
    }

    struct MicroBuffer serialization;    
    init_micro_buffer(&serialization, custom_subscription->TmpRawBuffer.Read , custom_subscription->TmpRawBuffer.RawDataSize);
    serialization.endianness = endianness;

    custom_subscription->TmpRawBuffer.Read += sizeof(custom_subscription->TmpRawBuffer.RawDataSize);


    // Extract serialiced message using typesupport
    bool OK = custom_subscription->type_support->cdr_deserialize(&serialization, ros_message);
    if (taken != NULL)
    {
        *taken = OK;
    }
    if (!OK)
    {
        return RMW_RET_ERROR;
    }

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
    /*
    (void)max_conditions;

    // Check if available
    if (RMW_MICRORTPS_CHECK_AVAILABLE(INTERNAL_WAIT_SET))
    {
        RMW_SET_ERROR_MSG("Not available memory for Retured_wait_set");
        return NULL;
    }

    // Extract from internal memory
    Internal_wait_set_t* Retured_wait_set;
    RMW_MICRORTPS_EXTRACT_FROM_INTERNAL_MEM(INTERNAL_WAIT_SET, Retured_wait_set);

    // Configure
    Retured_wait_set->wait_set.data                      = (void*)Retured_wait_set;
    Retured_wait_set->wait_set.guard_conditions          = NULL;
    Retured_wait_set->wait_set.implementation_identifier = rmw_get_implementation_identifier();
    */    

    // Return vaule
    EPROS_PRINT_TRACE()
    return NULL;
    //return &Retured_wait_set->wait_set;
}

rmw_ret_t rmw_destroy_wait_set(rmw_wait_set_t* wait_set)
{
    EPROS_PRINT_TRACE()

    /*
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

    // Check implementation
    if (wait_set->implementation_identifier != rmw_get_implementation_identifier())
    {
        RMW_SET_ERROR_MSG("Wrong rmw implementation");
        return RMW_RET_ERROR;
    }

    // Extract
    Internal_wait_set_t* Internal_wait_set = wait_set->data;

    // Release
    RMW_MICRORTPS_RETURN_TO_INTERNAL_MEM(INTERNAL_WAIT_SET, Internal_wait_set)
    */
    
    EPROS_PRINT_TRACE()
    return RMW_RET_OK;
}

rmw_ret_t rmw_wait(rmw_subscriptions_t* subscriptions, rmw_guard_conditions_t* guard_conditions,
                   rmw_services_t* services, rmw_clients_t* clients, rmw_wait_set_t* wait_set,
                   const rmw_time_t* wait_timeout)
{
    EPROS_PRINT_TRACE()

    // Wait set is not used
    (void)wait_set;
  

    // Search sesion 
    mrSession* session = NULL;
    if ((subscriptions != NULL) && (subscriptions->subscriber_count > 0))
    {
        // Extract first session pointer
        for (size_t i = 0; i < subscriptions->subscriber_count; ++i) 
        {
            if (subscriptions->subscribers[0] != NULL)
            {
                session = ((CustomSubscription *)subscriptions->subscribers[0])->session;
                break;
            }
        }
    }
    /*
    else if ((services != NULL) && (services->service_count > 0))
    {
        // Extract first session pointer
        //services->services[0];
    }
    */
    /*
    else if ((clients != NULL) && (clients->client_count > 0))
    {
        // Extract first session pointer
        //clients->clients[0];
    }
    */


    // Check if sesion is null 
    if (session == NULL)
    {
        return RMW_RET_ERROR;
    }



    // Check if timeout
    uint64_t Timeout;
    if (wait_timeout != NULL)
    {
        // Convert to int (checking overflow)
        if (wait_timeout->sec >= (UINT64_MAX / 1000))
        {
            // Overflow
            Timeout = INT_MAX;
            RMW_SET_ERROR_MSG("Wait timeout overflow");
        }
        else
        {
            Timeout = wait_timeout->sec * 1000;

            uint64_t timeout_ms = wait_timeout->nsec / 1000;
            if ((UINT64_MAX - Timeout) <= timeout_ms)
            {
                // Overflow
                Timeout = INT_MAX;
                RMW_SET_ERROR_MSG("Wait timeout overflow");
            }
            else
            {
                Timeout += timeout_ms;
                if (Timeout > INT_MAX)
                {
                    // Overflow
                    Timeout = INT_MAX;
                    RMW_SET_ERROR_MSG("Wait timeout overflow");
                }
            }
        }
    }
    else
    {
        Timeout = 0;
    }
  
  
    // read until no more data are available
    if (mr_run_session_until_timeout(session, (int)Timeout))
    {
        // TODO Use timeout on while.
        // Read until no more data is available
        while (mr_run_session_until_timeout(session, 0)){}
    }
    else
    {
        return RMW_RET_TIMEOUT;
    }


    // Clean non-received
    if (subscriptions != NULL) 
    {
        for (size_t i = 0; i < subscriptions->subscriber_count; ++i)
        {
            // Check if there are any data
            CustomSubscription * custom_subscription = (CustomSubscription *)(subscriptions->subscribers[i]);
            if (custom_subscription->TmpRawBuffer.Write == custom_subscription->TmpRawBuffer.Read)
            {
                subscriptions->subscribers[i] = NULL;
            }
        }
    }
    if (services != NULL) 
    {
        for (size_t i = 0; i < services->service_count; ++i) 
        {
            RMW_SET_ERROR_MSG("Services are not supported yet");
            services->services[i] = NULL;
        }
    }
    if (clients != NULL) 
    {
        for (size_t i = 0; i < clients->client_count; ++i) 
        {
            RMW_SET_ERROR_MSG("Clients are not supported yet");
            clients->clients[i] = NULL;
        }
    }

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
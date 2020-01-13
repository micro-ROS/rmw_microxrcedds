// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "utils.h"

#ifdef HAVE_C_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_c/identifier.h>
#endif
#ifdef HAVE_CPP_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_cpp/identifier.h>
#endif

#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>

#include "./utils.h"

rmw_service_t *
rmw_create_service(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rmw_qos_profile_t * qos_policies)
{
  EPROS_PRINT_TRACE()
  rmw_service_t * rmw_service = NULL;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
  } else if (!type_support) {
    RMW_SET_ERROR_MSG("type support is null");
  } else if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
  } else if (!service_name || strlen(service_name) == 0) {
    RMW_SET_ERROR_MSG("service name is null or empty string");
  } else if (!qos_policies) {
    RMW_SET_ERROR_MSG("qos_profile is null");
  } else {

    rmw_service = (rmw_service_t *)rmw_allocate(
      sizeof(rmw_service_t));
    rmw_service->data = NULL;
    rmw_service->implementation_identifier = rmw_get_implementation_identifier();
    rmw_service->service_name = (const char *)(rmw_allocate(sizeof(char) * (strlen(service_name) + 1)));
    if (!rmw_service->service_name) {
      RMW_SET_ERROR_MSG("failed to allocate memory");
      goto fail;
    }
    memcpy(rmw_service->service_name, service_name, strlen(service_name) + 1);

    CustomNode * custom_node = (CustomNode *)node->data;
    struct Item * memory_node = get_memory(&custom_node->service_mem);
    if (!memory_node) {
      RMW_SET_ERROR_MSG("Not available memory node");
      goto fail;
    }

    CustomService * custom_service = (CustomService *)memory_node->data;
    custom_service->owner_node = custom_node;
    custom_service->service_gid.implementation_identifier =
      rmw_get_implementation_identifier();
    custom_service->session = &custom_node->session;
    custom_service->history_write_index = 0;
    custom_service->history_read_index = 0;

    const rosidl_service_type_support_t * type_support_xrce = NULL;
#ifdef ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE
    type_support_xrce = get_service_typesupport_handle(
      type_support, ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE);
#endif
#ifdef ROSIDL_TYPESUPPORT_MICROXRCEDDS_CPP__IDENTIFIER_VALUE
    if (NULL == type_support_xrce) {
      type_support_xrce = get_service_typesupport_handle(
      type_support, ROSIDL_TYPESUPPORT_MICROXRCEDDS_CPP__IDENTIFIER_VALUE);
    }
#endif
    if (NULL == type_support_xrce) {
      RMW_SET_ERROR_MSG("Undefined type support");
      goto fail;
    }

    custom_service->type_support_callbacks =
      (const service_type_support_callbacks_t *)type_support_xrce->data;

    if (custom_service->type_support_callbacks == NULL) {
      RMW_SET_ERROR_MSG("type support data is NULL");
      goto fail;
    } else if (sizeof(uxrObjectId) > RMW_GID_STORAGE_SIZE) {
      RMW_SET_ERROR_MSG("Not enough memory for impl ids");
      goto fail;
    }

#ifdef MICRO_XRCEDDS_USE_XML
    char xml_buffer[RMW_XML_BUFFER_LENGTH];
#elif defined(MICRO_XRCEDDS_USE_REFS)
    char profile_name[RMW_REF_BUFFER_LENGTH];
#endif

    custom_service->service_id = uxr_object_id(custom_node->id_gen++, UXR_REPLIER_ID);

    memset(custom_service->service_gid.data, 0, RMW_GID_STORAGE_SIZE);
    memcpy(custom_service->service_gid.data, &custom_service->service_id,
      sizeof(uxrObjectId));

    uint16_t service_req;
#ifdef MICRO_XRCEDDS_USE_XML
    char service_name_id[20];
    generate_name(&custom_service->service_id, service_name_id, sizeof(service_name_id));
    if (!build_service_xml(service_name_id, service_name, false, custom_service->type_support_callbacks, qos_policies, xml_buffer, sizeof(xml_buffer))) {
      RMW_SET_ERROR_MSG("failed to generate xml request for service creation");
      goto fail;
    }
    service_req = uxr_buffer_create_replier_xml(&custom_node->session,
        custom_node->reliable_output, custom_service->service_id,
        custom_node->participant_id, xml_buffer, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
    // CHECK IF THIS IS NECESSARY
    // service_req = uxr_buffer_create_replier_ref(&custom_node->session,
    //     custom_node->reliable_output, custom_service->subscriber_id,
    //     custom_node->participant_id, "", UXR_REPLACE);
#endif

    rmw_service->data = custom_service;

    uint16_t requests[] = {service_req};
    uint8_t status[1];
    if (!uxr_run_session_until_all_status(&custom_node->session, 1000, requests,
      status, 1))
    {
      RMW_SET_ERROR_MSG("Issues creating Micro XRCE-DDS entities");
      put_memory(&custom_node->service_mem, &custom_service->mem);
      goto fail;
    }

    uxrDeliveryControl delivery_control;
    delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
    delivery_control.min_pace_period = 0;
    delivery_control.max_elapsed_time = UXR_MAX_ELAPSED_TIME_UNLIMITED;
    delivery_control.max_bytes_per_second = UXR_MAX_BYTES_PER_SECOND_UNLIMITED;
    custom_service->request_id = uxr_buffer_request_data(&custom_node->session,
      custom_node->reliable_output, custom_service->service_id,
      custom_node->reliable_input, &delivery_control);
  }
  return rmw_service;

fail:
  rmw_service_delete(rmw_service);
  rmw_service = NULL;
  return rmw_service;
}

rmw_ret_t
rmw_destroy_service(
  rmw_node_t * node,
  rmw_service_t * service)
{
  (void) node;

  EPROS_PRINT_TRACE()

  rmw_free(service);

  return RMW_RET_OK;
}

// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "rmw_node.h"  // NOLINT
#include "types.h"
#include "utils.h"
#include "identifiers.h"

#include <rmw_microxrcedds_c/config.h>

#ifdef MICRO_XRCEDDS_SERIAL
#include <fcntl.h>  // O_RDWR, O_NOCTTY, O_NONBLOCK
#include <termios.h>
#endif

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>

#include "./types.h"
#include "./utils.h"

#ifdef MICRO_XRCEDDS_SERIAL
#define CLOSE_TRANSPORT(transport) uxr_close_serial_transport(transport)
#elif defined(MICRO_XRCEDDS_UDP)
#define CLOSE_TRANSPORT(transport) uxr_close_udp_transport(transport)
#else
#define CLOSE_TRANSPORT(transport)
#endif

static struct MemPool node_memory;
static CustomNode custom_nodes[RMW_UXRCE_MAX_NODES];

void init_rmw_node()
{
  init_nodes_memory(&node_memory, custom_nodes, RMW_UXRCE_MAX_NODES);
}

void on_status(
  uxrSession * session, uxrObjectId object_id, uint16_t request_id, uint8_t status,
  void * args)
{
  (void)session;
  (void)object_id;
  (void)request_id;
  (void)status;
  (void)args;
}

void on_topic(
  uxrSession * session, uxrObjectId object_id, uint16_t request_id, uxrStreamId stream_id,
  struct ucdrBuffer * serialization, void * args)
{
  (void)session;
  (void)request_id;
  (void)stream_id;

  CustomNode * node = (CustomNode *)args;

  struct Item * subscription_item = node->subscription_mem.allocateditems;
  CustomSubscription * custom_subscription = NULL;
  while (subscription_item != NULL) {
    custom_subscription = (CustomSubscription *)subscription_item->data;
    if ((custom_subscription->datareader_id.id == object_id.id) &&
      (custom_subscription->datareader_id.type == object_id.type))
    { 
      ptrdiff_t lenght = serialization->final - serialization->iterator;
      
      custom_subscription->micro_buffer_lenght[custom_subscription->history_write_index] = lenght;
      memcpy(custom_subscription->micro_buffer[custom_subscription->history_write_index],
          serialization->iterator, lenght);
      

      // TODO (Pablo): Circular overlapping buffer implemented: use qos
      if (custom_subscription->micro_buffer_in_use && custom_subscription->history_write_index == custom_subscription->history_read_index){
        custom_subscription->history_read_index = (custom_subscription->history_read_index + 1) % RMW_UXRCE_MAX_HISTORY;
      }

      custom_subscription->history_write_index = (custom_subscription->history_write_index + 1) % RMW_UXRCE_MAX_HISTORY;
      custom_subscription->micro_buffer_in_use = true;

      break;
    }
    subscription_item = subscription_item->next;
  }
}

void on_request(uxrSession* session, uxrObjectId object_id, uint16_t request_id, SampleIdentity* sample_id, uint8_t* request_buffer, size_t request_len, void* args)
{
  (void)session;
  (void)object_id;

  CustomNode * node = (CustomNode *)args;

  struct Item * service_item = node->service_mem.allocateditems;
  CustomService * custom_service = NULL;

  while (service_item != NULL) {
    custom_service = (CustomService *)service_item->data;
    if (custom_service->request_id == request_id){
      custom_service->micro_buffer_lenght[custom_service->history_write_index] = request_len;
      memcpy(custom_service->micro_buffer[custom_service->history_write_index], 
          request_buffer, request_len);
      memcpy(&custom_service->sample_id[custom_service->history_write_index], 
          sample_id, sizeof(SampleIdentity));

      // TODO (Pablo): Circular overlapping buffer implemented: use qos
      if (custom_service->micro_buffer_in_use && custom_service->history_write_index == custom_service->history_read_index){
        custom_service->history_read_index = (custom_service->history_read_index + 1) % RMW_UXRCE_MAX_HISTORY;
      }

      custom_service->history_write_index = (custom_service->history_write_index + 1) % RMW_UXRCE_MAX_HISTORY;
      custom_service->micro_buffer_in_use = true;

      break;
    }
    service_item = service_item->next;
  }
}

void on_reply(uxrSession* session, uxrObjectId object_id, uint16_t request_id, uint16_t reply_id, uint8_t* buffer, size_t len, void* args)
{ 
  (void)session;
  (void)object_id;

  CustomNode * node = (CustomNode *)args;

  struct Item * client_item = node->client_mem.allocateditems;
  CustomClient * custom_client = NULL;

  while (client_item != NULL) {
    custom_client = (CustomClient *)client_item->data;
    if (custom_client->request_id == request_id)
    { 

      custom_client->micro_buffer_lenght[custom_client->history_write_index] = len;
      memcpy(custom_client->micro_buffer[custom_client->history_write_index], buffer,len);
      custom_client->reply_id[custom_client->history_write_index] = reply_id;

      // TODO (Pablo): Circular overlapping buffer implemented: use qos
      if (custom_client->micro_buffer_in_use && custom_client->history_write_index == custom_client->history_read_index){
        custom_client->history_read_index = (custom_client->history_read_index + 1) % RMW_UXRCE_MAX_HISTORY;
      }

      custom_client->history_write_index = (custom_client->history_write_index + 1) % RMW_UXRCE_MAX_HISTORY;
      custom_client->micro_buffer_in_use = true;

      break;
  }
  client_item = client_item->next;
}

}

void clear_node(rmw_node_t * node)
{
  CustomNode * micro_node = (CustomNode *)node->data;
  // TODO(Borja) make sure that session deletion deletes participant and related entities.
  uxr_delete_session(&micro_node->session);
  CLOSE_TRANSPORT(&micro_node->transport);
  rmw_node_delete(node);

  put_memory(&node_memory, &micro_node->mem);
}

rmw_node_t * create_node(const char * name, const char * namespace_, size_t domain_id, const rmw_context_t * context)
{
  if (!context) {
    RMW_SET_ERROR_MSG("context is null");
    return NULL;
  }

  struct Item * memory_node = get_memory(&node_memory);
  if (!memory_node) {
    RMW_SET_ERROR_MSG("Not available memory node");
    return NULL;
  }

  CustomNode * node_info = (CustomNode *)memory_node->data;

#ifdef MICRO_XRCEDDS_SERIAL
  int fd = open(context->impl->connection_params.serial_device, O_RDWR | O_NOCTTY);
  if (0 < fd) {
    struct termios tty_config;
    memset(&tty_config, 0, sizeof(tty_config));
    if (0 == tcgetattr(fd, &tty_config)) {
      /* Setting CONTROL OPTIONS. */
      tty_config.c_cflag |= CREAD;          // Enable read.
      tty_config.c_cflag |= CLOCAL;         // Set local mode.
      tty_config.c_cflag &= ~PARENB;        // Disable parity.
      tty_config.c_cflag &= ~CSTOPB;        // Set one stop bit.
      tty_config.c_cflag &= ~CSIZE;         // Mask the character size bits.
      tty_config.c_cflag |= CS8;            // Set 8 data bits.
      tty_config.c_cflag &= ~CRTSCTS;       // Disable hardware flow control.

      /* Setting LOCAL OPTIONS. */
      tty_config.c_lflag &= ~ICANON;        // Set non-canonical input.
      tty_config.c_lflag &= ~ECHO;          // Disable echoing of input characters.
      tty_config.c_lflag &= ~ECHOE;         // Disable echoing the erase character.
      tty_config.c_lflag &= ~ISIG;          // Disable SIGINTR, SIGSUSP, SIGDSUSP
                                            // and SIGQUIT signals.

      /* Setting INPUT OPTIONS. */
      tty_config.c_iflag &= ~IXON;          // Disable output software flow control.
      tty_config.c_iflag &= ~IXOFF;         // Disable input software flow control.
      tty_config.c_iflag &= ~INPCK;         // Disable parity check.
      tty_config.c_iflag &= ~ISTRIP;        // Disable strip parity bits.
      tty_config.c_iflag &= ~IGNBRK;        // No ignore break condition.
      tty_config.c_iflag &= ~IGNCR;         // No ignore carrier return.
      tty_config.c_iflag &= ~INLCR;         // No map NL to CR.
      tty_config.c_iflag &= ~ICRNL;         // No map CR to NL.

      /* Setting OUTPUT OPTIONS. */
      tty_config.c_oflag &= ~OPOST;         // Set raw output.

      /* Setting OUTPUT CHARACTERS. */
      tty_config.c_cc[VMIN] = 34;
      tty_config.c_cc[VTIME] = 10;

      /* Setting BAUD RATE. */
      cfsetispeed(&tty_config, B115200);
      cfsetospeed(&tty_config, B115200);

      if (0 == tcsetattr(fd, TCSANOW, &tty_config)) {
        if (!uxr_init_serial_transport(&node_info->transport,
          &node_info->serial_platform, fd, 0, 1))
        {
          RMW_SET_ERROR_MSG("Can not create an serial connection");
          return NULL;
        }
      }
    }
  }
  printf("Serial mode => dev: %s\n", context->impl->connection_params.serial_device);

#elif defined(MICRO_XRCEDDS_UDP)
  // TODO(Borja) Think how we are going to select transport to use
  #ifdef MICRO_XRCEDDS_IPV4
    uxrIpProtocol ip_protocol = UXR_IPv4;
  #elif defined(MICRO_XRCEDDS_IPV6)
    uxrIpProtocol ip_protocol = UXR_IPv6;
  #endif

    if (!uxr_init_udp_transport(&node_info->transport, &node_info->udp_platform, ip_protocol, context->impl->connection_params.agent_address, context->impl->connection_params.agent_port)) {
      RMW_SET_ERROR_MSG("Can not create an udp connection");
      return NULL;
    }
  printf("UDP mode => ip: %s - port: %s\n", context->impl->connection_params.agent_address, context->impl->connection_params.agent_port);
#elif defined(MICRO_XRCEDDS_CUSTOM)
  if (!uxr_init_serial_transport(&node_info->transport, &node_info->serial_platform, 0, 0, 1))
  {
    RMW_SET_ERROR_MSG("Can not create an custom serial connection");
    return NULL;
  }
#endif

  uxr_init_session(&node_info->session, &node_info->transport.comm, context->impl->connection_params.client_key);
  uxr_set_topic_callback(&node_info->session, on_topic, node_info);
  uxr_set_status_callback(&node_info->session, on_status, NULL);
  uxr_set_request_callback(&node_info->session, on_request, node_info);
  uxr_set_reply_callback(&node_info->session, on_reply, node_info);


  node_info->reliable_input = uxr_create_input_reliable_stream(
    &node_info->session, node_info->input_reliable_stream_buffer,
    node_info->transport.comm.mtu * RMW_UXRCE_STREAM_HISTORY, RMW_UXRCE_STREAM_HISTORY);
  node_info->reliable_output =
    uxr_create_output_reliable_stream(&node_info->session, node_info->output_reliable_stream_buffer,
      node_info->transport.comm.mtu * RMW_UXRCE_STREAM_HISTORY, RMW_UXRCE_STREAM_HISTORY);

  node_info->best_effort_input = uxr_create_input_best_effort_stream(&node_info->session);
  node_info->best_effort_output = uxr_create_output_best_effort_stream(&node_info->session,
      node_info->output_best_effort_stream_buffer,node_info->transport.comm.mtu);

  rmw_node_t * node_handle = NULL;
  node_handle = rmw_node_allocate();
  if (!node_handle) {
    RMW_SET_ERROR_MSG("failed to allocate rmw_node_t");
    return NULL;
  }
  node_handle->implementation_identifier = rmw_get_implementation_identifier();
  node_handle->data = node_info;
  node_handle->name = (const char *)(rmw_allocate(sizeof(char) * (strlen(name) + 1)));
  if (!node_handle->name) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    CLOSE_TRANSPORT(&node_info->transport);
    rmw_node_delete(node_handle);
    return NULL;
  }
  memcpy((char *)node_handle->name, name, strlen(name) + 1);

  node_handle->namespace_ = rmw_allocate(sizeof(char) * (strlen(namespace_) + 1));
  if (!node_handle->namespace_) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    CLOSE_TRANSPORT(&node_info->transport);
    rmw_node_delete(node_handle);
    return NULL;
  }
  memcpy((char *)node_handle->namespace_, namespace_, strlen(namespace_) + 1);

  if (!uxr_create_session(&node_info->session)) {
    CLOSE_TRANSPORT(&node_info->transport);
    rmw_node_delete(node_handle);
    RMW_SET_ERROR_MSG("failed to create node session on Micro ROS Agent.");
    return NULL;
  }

  // Create the Node participant. At this point a Node correspond with
  // a Session with one participant.
  node_info->participant_id = uxr_object_id(node_info->id_gen++, UXR_PARTICIPANT_ID);
  uint16_t participant_req;
#ifdef MICRO_XRCEDDS_USE_XML
  char participant_xml[RMW_UXRCE_XML_BUFFER_LENGTH];
  if (!build_participant_xml(domain_id, name, participant_xml, sizeof(participant_xml))) {
    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
    return NULL;
  }
  participant_req =
    uxr_buffer_create_participant_xml(&node_info->session, node_info->reliable_output,
      node_info->participant_id, (uint16_t)domain_id, participant_xml, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
  char profile_name[RMW_UXRCE_REF_BUFFER_LENGTH];
  if (!build_participant_profile(profile_name, sizeof(profile_name))) {
    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
    return NULL;
  }
  participant_req = uxr_buffer_create_participant_ref(&node_info->session,
      node_info->reliable_output,
      node_info->participant_id, domain_id, profile_name, UXR_REPLACE);
#endif
  uint8_t status[1];
  uint16_t requests[] = {participant_req};

  if (!uxr_run_session_until_all_status(&node_info->session, 1000, requests, status, 1)) {
    uxr_delete_session(&node_info->session);
    CLOSE_TRANSPORT(&node_info->transport);
    rmw_node_delete(node_handle);
    RMW_SET_ERROR_MSG("Issues creating micro XRCE-DDS entities");
    return NULL;
  }

  // TODO(Borja) create utils methods to handle publishers array.
  customnode_clear(node_info);

  return node_handle;
}

rmw_node_t *
rmw_create_node(
  rmw_context_t * context,
  const char * name,
  const char * namespace,
  size_t domain_id,
  const rmw_node_security_options_t * security_options)
{
  (void) context;
  EPROS_PRINT_TRACE()
  rmw_node_t * rmw_node = NULL;
  if (!name || strlen(name) == 0) {
    RMW_SET_ERROR_MSG("name is null");
  } else if (!namespace || strlen(namespace) == 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
  } else if (!security_options) {
    RMW_SET_ERROR_MSG("security_options is null");
  } else {
    rmw_node = create_node(name, namespace, domain_id, context);
  }
  return rmw_node;
}

rmw_ret_t rmw_destroy_node(rmw_node_t * node)
{
  EPROS_PRINT_TRACE()
  rmw_ret_t result_ret = RMW_RET_OK;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }

  if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
    return RMW_RET_ERROR;
  }

  if (!node->data) {
    RMW_SET_ERROR_MSG("node impl is null");
    return RMW_RET_ERROR;
  }

  clear_node(node);
  return result_ret;
}

rmw_ret_t
rmw_node_assert_liveliness(const rmw_node_t * node)
{
  (void) node;
  RMW_SET_ERROR_MSG("function not implemeted");
  return RMW_RET_ERROR;
}

const rmw_guard_condition_t *
rmw_node_get_graph_guard_condition(const rmw_node_t * node)
{
  (void)node;
  EPROS_PRINT_TRACE()
  rmw_guard_condition_t *
  ret = (rmw_guard_condition_t *)rmw_allocate(sizeof(rmw_guard_condition_t));
  ret->data = NULL;
  ret->implementation_identifier = eprosima_microxrcedds_identifier;
  return ret;
}

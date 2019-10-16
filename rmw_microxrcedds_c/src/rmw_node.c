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

#include "./rmw_node.h"  // NOLINT

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
#endif

static struct MemPool node_memory;
static CustomNode custom_nodes[MAX_NODES];

void init_rmw_node()
{
  init_nodes_memory(&node_memory, custom_nodes, MAX_NODES);
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

  // Get node pointer
  CustomNode * node = (CustomNode *)args;

  // Search subscription
  struct Item * subscription_item = node->subscription_mem.allocateditems;
  CustomSubscription * custom_subscription = NULL;
  while (true) {
    // Check if end of stack
    if (subscription_item == NULL) {
      return;
    }

    // Compare id
    custom_subscription = (CustomSubscription *)subscription_item->data;
    if ((custom_subscription->datareader_id.id == object_id.id) &&
      (custom_subscription->datareader_id.type == object_id.type))
    {
      custom_subscription->waiting_for_response = false;
      break;
    }

    // Next subscription of the stack
    subscription_item = subscription_item->next;
  }

  // Check if temporal micro buffer is on use
  if (custom_subscription->micro_buffer_in_use) {
    RMW_SET_ERROR_MSG("Internal memory error");
    return;
  }

  // not waiting for response any more
  node->on_subscription = true;


  // Copy microbuffer data
  memcpy(&custom_subscription->micro_buffer, serialization,
    sizeof(custom_subscription->micro_buffer));
  custom_subscription->micro_buffer_in_use = true;
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

rmw_node_t * create_node(const char * name, const char * namespace_, size_t domain_id)
{
  // TODO(Javier) Need to be changed into a to thread-save code.
  //  The suggested option rand_r() is not valid for this purpose.
  //  This change is pending to new feature in Micro XRCE-DDS that will provide an unused ID.
  //  When removed, the random initalization code in rmw_inint() must be removed.
  uint32_t key = rand();  // NOLINT

  struct Item * memory_node = get_memory(&node_memory);
  if (!memory_node) {
    RMW_SET_ERROR_MSG("Not available memory node");
    return NULL;
  }

  CustomNode * node_info = (CustomNode *)memory_node->data;

#ifdef MICRO_XRCEDDS_SERIAL
  int fd = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY);
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
  printf("Serial mode => dev: %s\n", SERIAL_DEVICE);

#elif defined(MICRO_XRCEDDS_UDP)
  // TODO(Borja) Think how we are going to select transport to use
  if (!uxr_init_udp_transport(&node_info->transport, &node_info->udp_platform, UDP_IP, UDP_PORT)) {
    RMW_SET_ERROR_MSG("Can not create an udp connection");
    return NULL;
  }
  printf("UDP mode => ip: %s - port: %hu\n", UDP_IP, UDP_PORT);
#endif

  uxr_init_session(&node_info->session, &node_info->transport.comm, key);
  uxr_set_topic_callback(&node_info->session, on_topic, node_info);
  uxr_set_status_callback(&node_info->session, on_status, NULL);

  node_info->reliable_input = uxr_create_input_reliable_stream(
    &node_info->session, node_info->input_reliable_stream_buffer,
    node_info->transport.comm.mtu * MAX_HISTORY, MAX_HISTORY);
  node_info->reliable_output =
    uxr_create_output_reliable_stream(&node_info->session, node_info->output_reliable_stream_buffer,
      node_info->transport.comm.mtu * MAX_HISTORY, MAX_HISTORY);

  rmw_node_t * node_handle = NULL;
  node_handle = rmw_node_allocate();
  if (!node_handle) {
    RMW_SET_ERROR_MSG("failed to allocate rmw_node_t");
    return NULL;
  }
  node_handle->implementation_identifier = rmw_get_implementation_identifier();
  node_handle->data = node_info;
  node_handle->name = (const char *)(rmw_allocate(sizeof(char) * strlen(name) + 1));
  if (!node_handle->name) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    CLOSE_TRANSPORT(&node_info->transport);
    rmw_node_delete(node_handle);
    return NULL;
  }
  memcpy((char *)node_handle->name, name, strlen(name) + 1);

  node_handle->namespace_ = rmw_allocate(sizeof(char) * strlen(namespace_) + 1);
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
  char participant_xml[300];
  if (!build_participant_xml(domain_id, name, participant_xml, sizeof(participant_xml))) {
    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
    return NULL;
  }
  participant_req =
    uxr_buffer_create_participant_xml(&node_info->session, node_info->reliable_output,
      node_info->participant_id,
      domain_id, participant_xml, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
  char profile_name[20];
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

# RMW Micro XRCE-DDS implementation

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

## Overview

All packages contained in this repository are a part of the Micro-ROS project stack.
For more information about Micro-ROS project click [here](https://microros.github.io/micro-ROS/).

## Packages

The repository contains the following packages:

### rmw_microxrcedds_c

This layer is the ROS 2 Middleware Abstraction Interface written in C.
This package provides a middleware implementation for XRCE-DDS (rmw layer).
The implementation wraps the latest code from eProsima's Micro XRCE-DDS client to communicate with the DDS world.
This library defines the interface used by upper layers in the ROS 2 stack, and that is implemented using XRCE-DDS middleware in the lower layers.
For further information about `rmw_microxrcedds` click [here](https://github.com/micro-ROS/micro-ROS-doc/blob/dashing/rmw_microxrcedds/README.md).

#### Library build Configurations

The middleware implementation uses static memory assignations.
Because of this, assignations of the memory are upper bounded so must be configured by the user before the build process.
By default, the package sets the values for all memory bounded.
The upper bound is configurable by a file that sets the values during the build process.
The configuration file is placed in `rmw_microxrcedds_c/rmw_microxrcedds.config` and has the following configurable parameters.

- *RMW_UXRCE_TRANSPORT* (udp/serial): This parameter sets the type of communication that the Micro XRCE-DDS client uses.

- *RMW_UXRCE_IP*: In case you are using the UDP communication mode, this value indicates the IP of the Micro XRCE-Agent.

<<<<<<< HEAD
- *RMW_UXRCE_PORT*: In case you are using the UDP communication mode, this value indicates the port used by the Micro XRCE-Agent.
=======
| Name                                      | Description                                                                                                                                                                                    | Default |
| ----------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------- |
| RMW_UXRCE_TRANSPORT                       | Sets Micro XRCE-DDS transport to use. (udp, serial, custom)                                                                                                                                    | udp     |
| RMW_UXRCE_IPV                             | Sets Micro XRCE-DDS IP version to use. (ipv4, ipv6)                                                                                                                                            | ipv4    |
| RMW_UXRCE_CREATION_MODE                   | Sets creation mode in Micro XRCE-DDS. (bin, refs)                                                                                                                                              | bin     |
| RMW_UXRCE_MAX_HISTORY                     | This value sets the number of history slots available for RMW subscriptions, </br> requests and replies                                                                                        | 8       |
| RMW_UXRCE_MAX_SESSIONS                    | This value sets the maximum number of Micro XRCE-DDS sessions.                                                                                                                                 | 1       |
| RMW_UXRCE_MAX_NODES                       | This value sets the maximum number of nodes.                                                                                                                                                   | 4       |
| RMW_UXRCE_MAX_PUBLISHERS                  | This value sets the maximum number of topic publishers for an application.                                                                                                                           | 4       |
| RMW_UXRCE_MAX_SUBSCRIPTIONS               | This value sets the maximum number of topic subscriptions for an application.                                                                                                                        | 4       |
| RMW_UXRCE_MAX_SERVICES                    | This value sets the maximum number of service servers for an application.                                                                                                                             | 4       |
| RMW_UXRCE_MAX_CLIENTS                     | This value sets the maximum number of service clients for an application.                                                                                                                              | 4       |
| RMW_UXRCE_MAX_TOPICS                      | This value sets the maximum number of topics for an application. </br> If set to -1 RMW_UXRCE_MAX_TOPICS = RMW_UXRCE_MAX_PUBLISHERS + </br> RMW_UXRCE_MAX_SUBSCRIPTIONS + RMW_UXRCE_MAX_NODES. | -1      |
| RMW_UXRCE_MAX_WAIT_SETS                   | This value sets the maximum number of wait sets for an application.                                                                                                                            | 4       |
| RMW_UXRCE_MAX_GUARD_CONDITION             | This value sets the maximum number of guard conditions for an application.                                                                                                                     | 4       |
| RMW_UXRCE_NODE_NAME_MAX_LENGTH            | This value sets the maximum number of characters for a node name.                                                                                                                              | 60      |
| RMW_UXRCE_TOPIC_NAME_MAX_LENGTH           | This value sets the maximum number of characters for a topic name.                                                                                                                             | 60      |
| RMW_UXRCE_TYPE_NAME_MAX_LENGTH            | This value sets the maximum number of characters for a type name.                                                                                                                              | 100     |
| RMW_UXRCE_REF_BUFFER_LENGTH               | This value sets the maximum number of characters for a reference buffer.                                                                                                                       | 100     |
| RMW_UXRCE_ENTITY_CREATION_DESTROY_TIMEOUT | This value sets the default maximum time to wait for an XRCE entity creation </br> and destroy in milliseconds. If set to 0 best effort is used.                                               | 1000    |
| RMW_UXRCE_ENTITY_CREATION_TIMEOUT         | This value sets the maximum time to wait for an XRCE entity creation </br> in milliseconds. If set to 0 best effort is used.                                                                   | 1000    |
| RMW_UXRCE_ENTITY_DESTROY_TIMEOUT          | This value sets the maximum time to wait for an XRCE entity destroy </br> in milliseconds. If set to 0 best effort is used.                                                                    | 1000    |
| RMW_UXRCE_PUBLISH_RELIABLE_TIMEOUT        | This value sets the default time to wait for a publication in a </br> reliable mode in milliseconds.                                                                                           | 1000    |
| RMW_UXRCE_STREAM_HISTORY                  | This value sets the number of MTUs to buffer, both input and output.                                                                                                                           | 4       |
| RMW_UXRCE_STREAM_HISTORY_INPUT            | This value sets the number of MTUs to input buffer. </br> It will be ignored if RMW_UXRCE_STREAM_HISTORY_OUTPUT is blank.                                                                      | -       |
| RMW_UXRCE_STREAM_HISTORY_OUTPUT           | This value sets the number of MTUs to output buffer. </br> It will be ignored if RMW_UXRCE_STREAM_HISTORY_INPUT is blank.                                                                      | -       |
| RMW_UXRCE_GRAPH                           | Allows to perform graph-related operations to the user                                                                                                                                         | OFF     |
| RMW_UXRCE_ALLOW_DYNAMIC_ALLOCATIONS       | Enables increasing static pools with dynamic allocation when needed.                                                                                                                           | OFF     |
>>>>>>> fa98772 (Clarify some descriptions a bit better (#242))

- *RMW_UXRCE_DEVICE*: In case you are using the serial communication mode, this value indicates the file descriptor of the serial port (Linux).
- *RMW_UXRCE_CREATION_MODE*: chooses the preferred XRCE-DDS entities creation method. It could be XML or references.

    Both create entities on the associated Micro XRCE-DDS Agent; the difference is that the client dynamically creates XML, and references are preconfigured entities on the Micro XRCE-DDS Agent side.

- *RMW_UXRCE_MAX_HISTORY*: This value sets the number of MTUs to buffer. Micro XRCE-DDS client configuration provides their size.
- *RMW_UXRCE_MAX_NODES*: This value sets the maximum number of nodes.
- *RMW_UXRCE_MAX_PUBLISHERS*: This value sets the maximum number of publishers.
- *RMW_UXRCE_MAX_SUBSCRIPTIONS*: This value sets the maximum number of subscriptions.
- *RMW_UXRCE_MAX_CLIENTS*: This value sets the maximum number of clients.
- *RMW_UXRCE_MAX_SERVICES*: This value sets the maximum number of services.
- *RMW_UXRCE_NODE_NAME_MAX_LENGTH*: This value sets the maximum number of characters for a node name.
- *RMW_UXRCE_TOPIC_NAME_MAX_LENGTH*: This value sets the maximum number of characters for a topic name.
- *RMW_UXRCE_TYPE_NAME_MAX_LENGTH*: This value sets the maximum number of characters for a type name.

### microxrcedds_cmake_module

This package contains a CMake Module for finding and exposing required dependencies for the Micro XRCE-DDS client.

## Purpose of the Project

This software is not ready for production use. It has neither been developed nor
tested for a specific use case. However, the license conditions of the
applicable Open Source licenses allow you to adapt the software to your needs.
Before using it in a safety relevant setting, make sure that the software
fulfills your requirements and adjust it according to any applicable safety
standards, e.g., ISO 26262.

## License

This repository is open-sourced under the Apache-2.0 license. See the [LICENSE](LICENSE) file for details.

For a list of other open-source components included in this repository,
see the file [3rd-party-licenses.txt](3rd-party-licenses.txt).

## Known Issues/Limitations

There are no known limitations.

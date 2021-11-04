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

- *RMW_UXRCE_PORT*: In case you are using the UDP communication mode, this value indicates the port used by the Micro XRCE-Agent.

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

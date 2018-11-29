# RMW Micro XRCE-DDS implementation

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

## Overview

All packages contained in this repository are a part of the Micro-ROS project stack.
For more information about Micro-ROS project click [here](https://microros.github.io/micro-ROS/).

## Packages

The repository contains the following packages:

### rmw_microxrcedds_c

This layer is the ROS 2 Middleware Abstraction Interface written in C language.
This package provides a middleware implementation for XRCE-DDS (rmw layer).
The implementation wraps the latest code from eProsima's Micro XRCE-DDS client to communicate to DDS world.
This library defines the interface used by upper layers in the ROS 2 stack, and that is implemented using XRCE-DDS middleware in the lower layers.
For further information about the rmw_microxrcedds click [here](TODO).

#### Library build Configurations

The middleware implementation uses static memory assignations.
Because of this, assignations of the memory are upper bounded so must be configured by the user before the build process.
By default, the package sets the values for all memory bounded.
The upper bound is configurable by a file that sets the values during the build process.
The configuration file is placed in 'rmw_microxrcedds_c/rmw_microxrcedds.config' and has the following configurable parameters.

- *CONFIG_MICRO_XRCEDDS_TRANSPORT* (udp/serial): This parameter sets the type of communication that the Micro XRCE-DDS client uses.

- *CONFIG_IP*: In case you are using the UDP communication mode, this value indicates the IP of the Micro XRCE-Agent.

- *CONFIG_PORT*: In case you are using the UDP communication mode, this value indicates the port used by the Micro XRCE-Agent.

- *CONFIG_DEVICE*: In case you are using the serial communication mode, this value indicates the file descriptor of the seal port (Linux).
- *CONFIG_MICRO_XRCEDDS_CREATION_MODE*: chooses the prefered XRCE-DDS entities creation method. It could be XML or references.

    Both create entities on the associated the Micro XRCE-DDS Agent; the difference is that the client dynamically creates XML, and references are preconfigured entities on the Micro XRCE-DDS Agent side.

- *CONFIG_MAX_HISTORY*: This value sets the number of MTUs buffers to. Micro XRCE-DDS client configuration provides their size.
- *CONFIG_MAX_NODES*: This value sets the maximum number of nodes.
- *CONFIG_MAX_PUBLISHERS_X_NODE*: This value sets the maximum number of publishers for a node.
- *CONFIG_MAX_SUBSCRIPTIONS_X_NODE*: This value sets the maximum number of subscriptions for a node.
- *CONFIG_RMW_NODE_NAME_MAX_NAME_LENGTH*: This value sets the maximum number of characters for a node name.
- *CONFIG_RMW_TOPIC_NAME_MAX_NAME_LENGTH*: This value sets the maximum number of characters for a topic name.
- *CONFIG_RMW_TYPE_NAME_MAX_NAME_LENGTH*: This value sets the maximum number of characters for a type name.

### microxrcedds_cmake_module

This package contains CMake Module for finding and exposing required dependencies for Micro XRCE-DDS client.

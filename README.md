<a href="http://www.eprosima.com"><img src="http://www.eprosima.com/images/logos/eprosima/logo.png" align="top" hspace="8" vspace="2" width="650" height="200" ></a>

# Overview

All packages contained in this repository are a part of the Micro-ROS poject stack. 
For more information about Micro-ROS project click [here]().


# Packages

The repository contains the belows packages:


## rmw_microxrcedds_c

This layer is the ROS Middleware Abstraction Interface written in C language.
The package provides a middleware implementation for XRCE-DDS (rmw layer).
This implementation will wrap the latest code from eProsima's Micro XRCE-DDS client to comunicate to DDS world.
This library defines the interface used by upper layers in ROS stack and implemented using some middleware in the lower layers.
For farther information about the rmw_microxrcedds click [here]().


### Library build Configurations

In order to not use dynamic memory allocation, the middleware implementation uses static memory assignations.
Because of this, assignationas of the memory are upper bounded so must be configured by the user before the build process.
By default, the package sets the values for all memory bounded.
The upper bound is configurable by file that will set the values during the build process.
The configuration file is placed in 'rmw_microxrcedds_c/rmw_microxrcedds.config' and has the following configurable parameters.


- CONFIG_MICRO_XRCEDDS_TRANSPORT (udp/seral): This parameters sets the type of comunication that the Micro XRCE-DDS client will use. 

- CONFIG_IP: In case you are using the udp communication mode, this value will indicate the ip of the Micro XRCE-Agent.

- CONFIG_PORT: In case you are using the udp communication mode, this value will indicate the port used by the Micro XRCE-Agent.
- CONFIG_DEVICE: In case you are using the serial communication mode, this value will indicate the file descriptor of the seal port (linux).

- CONFIG_MICRO_XRCEDDS_CREATION_MODE: 

- CONFIG_MAX_HISTORY: This value will the number of MTUs buffers.
- CONFIG_MAX_NODES: This value will set the maximum number of nodes.
- CONFIG_MAX_PUBLISHERS_X_NODE: This value will set the maximum number of publisher for a node.
- CONFIG_MAX_SUBSCRIPTIONS_X_NODE: This value will set the maximum number of subcriptions for a node.
- CONFIG_RMW_NODE_NAME_MAX_NAME_LENGTH: This value will set the maximum number of caracters for a node name.
- CONFIG_RMW_TOPIC_NAME_MAX_NAME_LENGTH: This value will set the maximum number of caracters for a topic name.
- CONFIG_RMW_TYPE_NAME_MAX_NAME_LENGTH: This value will set the maximum number of caracters for a type name.


## microxrcedds_cmake_module

This package contains CMake Module for discovering and exposing required dependencies for Micro XRCE-DDS client.
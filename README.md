# Overview

This layer is the ROS Middleware Abstraction Interface.
The package provides an RMW implementation using the latest code from eProsima's middleware for constrained devices.
This library defines the interface used by upper layers in ROS stack and implemented using some middleware in the lower layers.
For farther information about the rmw_microxrcedds click [here]().



# Configuration

In order to no use dynamic memory allocation, the package uses static memory asignations. 
Because of this, assignationas of the memory are upperbounded.
The upperbounder is configurable by file tha will set the values during the build proccess.


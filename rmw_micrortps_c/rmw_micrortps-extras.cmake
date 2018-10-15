# Copyright 2017 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# copied from rmw_micrortps/rmw_micrortps-extras.cmake

find_package(microxrcedds_cmake_module REQUIRED)
find_package(MicroXRCEDDS REQUIRED MODULE)

list(APPEND rmw_micrortps_INCLUDE_DIRS ${MicroXRCEDDS_INCLUDE_DIR})
# specific order: dependents before dependencies
list(APPEND rmw_micrortps_LIBRARIES microxrcedds_client microcdr)

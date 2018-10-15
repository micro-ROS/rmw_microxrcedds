# Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

###############################################################################
#
# CMake module for finding eProsima MICROXRCEDDS.
#
# Output variables:
#
# - MicroXRCEDDS_FOUND: flag indicating if the package was found
# - MicroXRCEDDS_INCLUDE_DIR: Paths to the header files
# 
# Example usage:
#
#   find_package(microxrcedds_cmake_module REQUIRED)
#   find_package(MicroXRCEDDS MODULE)
#   # use MicroXRCEDDS_* variables
#
###############################################################################

# lint_cmake: -convention/filename, -package/stdargs

set(MicroXRCEDDS_FOUND FALSE)

find_package(microcdr REQUIRED CONFIG)
find_package(microxrcedds_client REQUIRED CONFIG)

string(REGEX MATCH "^[0-9]+\\.[0-9]+" microcdr_MAJOR_MINOR_VERSION "${microcdr_VERSION}")
string(REGEX MATCH "^[0-9]+\\.[0-9]+" microxrcedds_client_MAJOR_MINOR_VERSION "${microxrcedds_client_VERSION}")

find_library(MicroCDR_LIBRARY_RELEASE
  NAMES microcdr-${microcdr_MAJOR_MINOR_VERSION})

find_library(MicroCDR_LIBRARY_DEBUG
  NAMES microcdrd-${microcdr_MAJOR_MINOR_VERSION})

if(MicroCDR_LIBRARY_RELEASE AND MicroCDR_LIBRARY_DEBUG)
  set(MicroCDR_LIBRARIES
    optimized ${MicroCDR_LIBRARY_RELEASE}
    debug ${MicroCDR_LIBRARY_DEBUG}
  )
elseif(MicroCDR_LIBRARY_RELEASE)
  set(MicroCDR_LIBRARIES
    ${MicroCDR_LIBRARY_RELEASE}
  )
elseif(MicroCDR_LIBRARY_DEBUG)
  set(MicroCDR_LIBRARIES
    ${MicroCDR_LIBRARY_DEBUG}
  )
else()
  set(MicroCDR_LIBRARIES "")
endif()

find_library(MicroXRCEDDSClient_LIBRARY_RELEASE
  NAMES microxrcedds_client-${microxrcedds_client_MAJOR_MINOR_VERSION})

find_library(MicroXRCEDDSClient_LIBRARY_DEBUG
  NAMES microxrcedds_clientd-${microxrcedds_client_MAJOR_MINOR_VERSION})

if(MicroXRCEDDSClient_LIBRARY_RELEASE AND MicroXRCEDDSClient_LIBRARY_DEBUG)
    set(MicroXRCEDDSClient_LIBRARIES
        optimized ${MicroXRCEDDSClient_LIBRARY_RELEASE}
        debug ${MicroXRCEDDSClient_LIBRARY_DEBUG}
        ${MicroCDR_LIBRARIES}
      )
elseif(MicroXRCEDDSClient_LIBRARY_RELEASE)
    set(MicroXRCEDDSClient_LIBRARIES
        ${MicroXRCEDDSClient_LIBRARY_RELEASE}
        ${MicroCDR_LIBRARIES}
      )
elseif(MicroXRCEDDSClient_LIBRARY_DEBUG)
    set(MicroXRCEDDSClient_LIBRARIES
        ${MicroXRCEDDSClient_LIBRARY_DEBUG}
        ${MicroCDR_LIBRARIES}
      )
else()
    set(MicroXRCEDDSClient_LIBRARIES "")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MicroXRCEDDSClient
        FOUND_VAR MicroXRCEDDSClient_FOUND
  REQUIRED_VARS
        MicroXRCEDDSClient_INCLUDE_DIR
        MicroXRCEDDSClient_LIBRARIES
)

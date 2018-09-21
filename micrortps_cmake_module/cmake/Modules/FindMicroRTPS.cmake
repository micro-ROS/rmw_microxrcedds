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
# CMake module for finding eProsima MicroRTPS.
#
# Output variables:
#
# - MicroRTPS_FOUND: flag indicating if the package was found
# - MicroRTPS_INCLUDE_DIR: Paths to the header files
#
# Example usage:
#
#   find_package(micrortps_cmake_module REQUIRED)
#   find_package(MicroRTPS MODULE)
#   # use MicroRTPS_* variables
#
###############################################################################

# lint_cmake: -convention/filename, -package/stdargs

set(MicroRTPS_FOUND FALSE)

find_path(MicroRTPS_INCLUDE_DIR
  NAMES micrortps/)

find_package(microcdr REQUIRED CONFIG)
find_package(micrortps_client REQUIRED CONFIG)

string(REGEX MATCH "^[0-9]+\\.[0-9]+" microcdr_MAJOR_MINOR_VERSION "${microcdr_VERSION}")
string(REGEX MATCH "^[0-9]+\\.[0-9]+" micrortps_client_MAJOR_MINOR_VERSION "${micrortps_client_VERSION}")

find_library(MicroCDR_LIBRARY_RELEASE
  NAMES microcdr-${microcdr_MAJOR_MINOR_VERSION} microcdr)

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

find_library(MicroRTPSClient_LIBRARY_RELEASE
  NAMES micrortps_client-${micrortps_MAJOR_MINOR_VERSION} micrortps_client)

find_library(MicroRTPSClient_LIBRARY_DEBUG
  NAMES micrortps_clientd-${micrortps_client_MAJOR_MINOR_VERSION})

if(MicroRTPSClient_LIBRARY_RELEASE AND MicroRTPSClient_LIBRARY_DEBUG)
    set(MicroRTPSClient_LIBRARIES
        optimized ${MicroRTPSClient_LIBRARY_RELEASE}
        debug ${MicroRTPSClient_LIBRARY_DEBUG}
        ${MicroCDR_LIBRARIES}
      )
elseif(MicroRTPSClient_LIBRARY_RELEASE)
    set(MicroRTPSClient_LIBRARIES
        ${MicroRTPSClient_LIBRARY_RELEASE}
        ${MicroCDR_LIBRARIES}
      )
elseif(MicroRTPSClient_LIBRARY_DEBUG)
    set(MicroRTPSClient_LIBRARIES
        ${MicroRTPSClient_LIBRARY_DEBUG}
        ${MicroCDR_LIBRARIES}
      )
else()
    set(MicroRTPSClient_LIBRARIES "")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MicroRTPSClient
        FOUND_VAR MicroRTPSClient_FOUND
  REQUIRED_VARS
        MicroRTPSClient_INCLUDE_DIR
        MicroRTPSClient_LIBRARIES
)

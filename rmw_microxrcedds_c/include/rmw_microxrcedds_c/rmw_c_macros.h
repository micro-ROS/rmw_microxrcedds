// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef RMW_MICROXRCEDDS_C__RMW_C_MACROS_H_
#define RMW_MICROXRCEDDS_C__RMW_C_MACROS_H_

#if RMW_AVOID_MEMORY_ALLOCATION
#define RMW_CHECK_TYPE_IDENTIFIERS_MATCH(ElementName, ElementTypeID, ExpectedTypeID, OnFailure) \
  { \
    if (strcmp(ElementTypeID, ExpectedTypeID) != 0) { \
      char __msg[1024]; \
      int ret = rcutils_snprintf( \
        __msg, sizeof(__msg), \
        #ElementName " implementation '%s'(%p) does not match rmw implementation '%s'(%p)", \
        ElementTypeID, (const void *)(ElementTypeID), \
        ExpectedTypeID, (const void *)(ExpectedTypeID)); \
      if (ret < 0) { \
        static const char error_msg[] = \
          "RMW_CHECK_TYPE_IDENTIFIERS_MATCH(): rcutils_snprintf() failed"; \
        memmove(__msg, error_msg, sizeof(error_msg)); \
      } \
      RMW_SET_ERROR_MSG(__msg); \
      OnFailure; \
    } \
  }
#else  // RMW_AVOID_MEMORY_ALLOCATION
#define RMW_CHECK_TYPE_IDENTIFIERS_MATCH(ElementName, ElementTypeID, ExpectedTypeID, OnFailure) \
  { \
    if (strcmp(ElementTypeID, ExpectedTypeID) != 0) { \
      int __bytes_that_would_have_been_written = rcutils_snprintf( \
        NULL, 0, \
        #ElementName " implementation '%s'(%p) does not match rmw implementation '%s'(%p)", \
        ElementTypeID, (const void *)(ElementTypeID), \
        ExpectedTypeID, (const void *)(ExpectedTypeID)); \
      if (__bytes_that_would_have_been_written < 0) { \
        RMW_SET_ERROR_MSG( \
          "RMW_CHECK_TYPE_IDENTIFIERS_MATCH(): rcutils_snprintf(NULL, 0, ...) failed"); \
        OnFailure; \
      } else { \
        char * __msg = \
          (char *)(rmw_allocate(__bytes_that_would_have_been_written + 1)); \
        if (NULL == __msg) { \
          RMW_SET_ERROR_MSG( \
            "RMW_CHECK_TYPE_IDENTIFIERS_MATCH(): rmw_allocate() failed"); \
        } else { \
          int ret = rcutils_snprintf( \
            __msg, __bytes_that_would_have_been_written + 1, \
            #ElementName " implementation '%s'(%p) does not match rmw implementation '%s'(%p)", \
            ElementTypeID, (const void *)(ElementTypeID), \
            ExpectedTypeID, (const void *)(ExpectedTypeID)); \
          if (ret < 0) { \
            RMW_SET_ERROR_MSG( \
              "RMW_CHECK_TYPE_IDENTIFIERS_MATCH(): rcutils_snprintf() failed"); \
          } else { \
            RMW_SET_ERROR_MSG(__msg); \
          } \
        } \
        rmw_free(__msg); \
        OnFailure; \
      } \
    } \
  }
#endif

#endif  // RMW_MICROXRCEDDS_C__RMW_C_MACROS_H_

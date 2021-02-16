// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include "./rmw_uxrce_transports.h"

#include <rmw/error_handling.h>

#include "./rmw_uros/options.h"

rmw_ret_t rmw_uxrce_transport_init(
    rmw_context_impl_t* context_impl,
    rmw_init_options_impl_t* init_options_impl,
    void* transport)
{
#ifdef RMW_UXRCE_TRANSPORT_SERIAL
    const char* serial_device = (NULL == context_impl)
                                ? RMW_UXRCE_DEFAULT_SERIAL_DEVICE
                                : init_options_impl->transport_params.serial_device;

    int fd;
    if (0 < (fd = open(serial_device, O_RDWR | O_NOCTTY)))
    {
        struct termios tty_config;
        memset(&tty_config, 0, sizeof(tty_config));

        if (0 != tcgetattr(fd, &tty_config))
        {
            RMW_SET_ERROR_MSG("rmw_transport_init SERIAL: tgetattr error");
            return(RMW_RET_ERROR);
        }

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
        tty_config.c_cc[VMIN]  = 34;
        tty_config.c_cc[VTIME] = 10;

        /* Setting BAUD RATE. */
        cfsetispeed(&tty_config, B115200);
        cfsetospeed(&tty_config, B115200);

        if (0 != tcsetattr(fd, TCSANOW, &tty_config))
        {
            RMW_SET_ERROR_MSG("rmw_transport_init SERIAL: tcsetattr error");
            return(RMW_RET_ERROR);
        }

        uxrSerialTransport* serial_transport = (NULL == context_impl)
                                               ? (uxrSerialTransport*)transport
                                               : &context_impl->transport;

        if (!uxr_init_serial_transport(serial_transport, fd, 0, 1))
        {
            RMW_SET_ERROR_MSG("rmw_transport_init SERIAL: cannot init XRCE transport");
            return(RMW_RET_ERROR);
        }
    }
    else
    {
        RMW_SET_ERROR_MSG("rmw_transport_init SERIAL: invalid serial device file descriptor");
        return(RMW_RET_ERROR);
    }
    printf("micro-ROS transport: connected using serial mode, dev: '%s'\n",
           serial_device);
#elif defined(RMW_UXRCE_TRANSPORT_UDP)
#ifdef RMW_UXRCE_TRANSPORT_IPV4
    uxrIpProtocol ip_protocol = UXR_IPv4;
#elif defined(RMW_UXRCE_TRANSPORT_IPV6)
    uxrIpProtocol ip_protocol = UXR_IPv6;
#endif

    uxrUDPTransport* udp_transport = (NULL == context_impl)
                                     ? (uxrUDPTransport*)transport
                                     : &context_impl->transport;
    const char* agent_ip = (NULL == context_impl)
                           ? RMW_UXRCE_DEFAULT_UDP_IP
                           : init_options_impl->transport_params.agent_address;
    const char* agent_port = (NULL == context_impl)
                             ? RMW_UXRCE_DEFAULT_UDP_PORT
                             : init_options_impl->transport_params.agent_port;

    if (!uxr_init_udp_transport(udp_transport, ip_protocol, agent_ip, agent_port))
    {
        RMW_SET_ERROR_MSG("rmw_transport_init UDP: cannot init XRCE transport");
        return(RMW_RET_ERROR);
    }
    printf("micro-ROS transport: connected using UDP mode, ip: '%s', port: '%s'\n",
           agent_ip, agent_port);
#elif defined(RMW_UXRCE_TRANSPORT_CUSTOM)
    uxrCustomTransport* custom_transport = (NULL == context_impl)
                                           ? (uxrCustomTransport*)transport
                                           : &context_impl->transport;
    void* args = (NULL == context_impl)
                 ? rmw_uxrce_transport_default_params.args
                 : init_options_impl->transport_params.args;

    if (!uxr_init_custom_transport(custom_transport, args))
    {
        RMW_SET_ERROR_MSG("rmw_transport_init CUSTOM: cannot init XRCE transport");
        return(RMW_RET_ERROR);
    }
#endif
    return(RMW_RET_OK);
}

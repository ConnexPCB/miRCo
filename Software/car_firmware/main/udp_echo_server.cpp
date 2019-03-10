//
// async_udp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "asio.hpp"

#include "Motor.hpp"

static const uint16_t BROADCAST_PORT = 1235;

float filter(float oldValue, float input, float alpha);

extern "C" void asio_main()
{
  /*
    asio::io_context io_context;

    server s(io_context, std::atoi(CONFIG_EXAMPLE_PORT));

    io_context.run();
    */

    std::cout << "[Info] Initializing motor controller" << std::endl;
    Motor left{
      {MCPWM_UNIT_0, MCPWM0A, MCPWM_TIMER_0, MCPWM_OPR_A},
      {50, 1000, 2000}, 25
    }, right{
      {MCPWM_UNIT_1, MCPWM1A, MCPWM_TIMER_1, MCPWM_OPR_A},
      {50, 1000, 2000}, 26
    };

  asio::io_service ioService;

  asio::ip::udp::socket socket(ioService);
  asio::ip::udp::endpoint recvEndpoint(asio::ip::udp::v4(), 1234);

  socket.open(asio::ip::udp::v4());
  socket.set_option(asio::socket_base::broadcast(true));
  socket.set_option(asio::socket_base::reuse_address(true));
  socket.bind(recvEndpoint);

  std::cout << "[Info] Starting UDP speed server" << std::endl;

  std::thread broadcastThread([&socket](){ 
    std::array<uint8_t, 6> id{0, 0, 0, 0, 0, 0};
    asio::ip::udp::endpoint broadcastEndpoint{asio::ip::address_v4::broadcast(),
      BROADCAST_PORT};;//{asio::ip::address::from_string("192.168.4.255"), BROADCAST_PORT}; 

    for(;;) {
      std::cout << "[Info] Sending broadcast discovery datagram" << std::endl;
      socket.send_to(asio::buffer(id), broadcastEndpoint);

      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  });

  float leftSpeed = 0.f, rightSpeed = 0.f;

  while(true) {
    std::array<int8_t, 2048> recvData;
    auto len = socket.receive_from(asio::buffer(recvData), recvEndpoint);

    if(len != 2) {
      std::cerr << "[Error] Received invalid data (length = " << len << ")" << std::endl;
    }
    else {
      leftSpeed = filter(leftSpeed, recvData[0], 0.07f);
      rightSpeed = filter(rightSpeed, recvData[1], 0.07f); //Big ESC

      std::cout << "[Info] Received speed update: " << leftSpeed << ", " << rightSpeed << std::endl;

      left.setSpeed(std::max(0, static_cast<int>(leftSpeed+45)) / 55.f / 4.f);
      right.setSpeed(std::max(0, static_cast<int>(rightSpeed+10)) / 90.f / 4.f); //Big ESC
    }
  }
}

float filter(float oldValue, float newValue, float alpha) {
  return newValue*alpha + oldValue*(1.f - alpha);
}

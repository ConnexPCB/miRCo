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

using asio::ip::udp;

class server
{
public:
  server(asio::io_context& io_context, short port)
    : socket_(io_context, udp::endpoint(udp::v4(), port))
  {
    do_receive();
  }

  void do_receive()
  {
    socket_.async_receive_from(
        asio::buffer(data_, max_length), sender_endpoint_,
        [this](std::error_code ec, std::size_t bytes_recvd)
        {
          if (!ec && bytes_recvd > 0)
          {
            std::cout << data_ << std::endl;
            do_send(bytes_recvd);
          }
          else
          {
            do_receive();
          }
        });
  }

  void do_send(std::size_t length)
  {
    socket_.async_send_to(
        asio::buffer(data_, length), sender_endpoint_,
        [this](std::error_code /*ec*/, std::size_t  bytes /*bytes_sent*/)
        {
          do_receive();
        });
  }

private:
  udp::socket socket_;
  udp::endpoint sender_endpoint_;
  enum { max_length = 1024 };
  char data_[max_length];
};

extern "C" void asio_main()
{
  /*
    asio::io_context io_context;

    server s(io_context, std::atoi(CONFIG_EXAMPLE_PORT));

    io_context.run();
    */

    std::cout << "[Info] Initializing motor controller" << std::endl;
    Motor m1{
      {MCPWM_UNIT_0, MCPWM0A, MCPWM_TIMER_0, MCPWM_OPR_A},
      {50, 1000, 2000}, 25
    }, m2{
      {MCPWM_UNIT_1, MCPWM1A, MCPWM_TIMER_1, MCPWM_OPR_A},
      {50, 1000, 2000}, 26
    };

    std::cout << "[Info] Initializing start button" << std::endl;

    gpio_config_t gpioConfig;
    gpioConfig.pin_bit_mask = 0x01;
    gpioConfig.mode = GPIO_MODE_INPUT;
    gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&gpioConfig);

    std::cout << "[Info] Waiting for start button" << std::endl;
    while(gpio_get_level(GPIO_NUM_0) == 1);

    std::cout << "[Info] Starting motor control loop" << std::endl;

    float speed = 0.;
    int dir = 1;
    while(true) {
      speed += dir*0.01f;
      if(speed >= 1.f) {
        speed = 1.f;
        dir = -1;
      }
      else if(speed <= 0.f) {
        speed = 0.f;
        dir = 1;
      }

      m1.setSpeed(speed);
      m2.setSpeed(speed);
      std::cout << "[Info] Setting speed to " << speed << std::endl;

      vTaskDelay(5);
    }
}

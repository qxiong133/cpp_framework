// =====================================================================================
// 
//       Filename:  testboost.cc
// 
//    Description:  temp test
// 
//        Version:  1.0
//        Created:  2009-12-10 19:26:17
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  liaoxinwei (Comet), cometliao@gmail.com
//        Company:  eddy
// 
// =====================================================================================
//
// timer.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost::posix_time;

void print(int a, const boost::system::error_code& e)
{
  if (!e)
    std::cout << "Hello, world!\n";

  ptime time = microsec_clock::local_time();
  cout << time << endl;
}

void print2(int a, const boost::system::error_code& e)
{
  if (!e)
    std::cout << "Hello, world2!\n";
}
int main()
{
  boost::asio::io_service io;

  ptime time = microsec_clock::local_time();
  cout << time << endl;
    boost::asio::deadline_timer t(io, boost::posix_time::millisec(1));
    t.async_wait(boost::bind(print, 1, boost::asio::placeholders::error));
    t.cancel();
    t.async_wait(boost::bind(print2, 1, boost::asio::placeholders::error));
  io.run();

  return 0;
}

/****************************************************************************
 *
 *   Copyright (c) 2017 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/*
 *  optical_flow.cpp
 *
 *  Created on: Jan 12, 2017
 *      Author: Christoph
 */

#include "optical_flow.hpp"

void OpticalFlow::initLimitRate()
{
  sum_flow_x = 0.0;
  sum_flow_y = 0.0;
  valid_frame_count = 0;
  sum_flow_quality = 0;
}

int OpticalFlow::limitRate(int flow_quality, const uint32_t frame_time_us, int* dt_us, float* flow_x, float* flow_y)
{
  if (time_last_frame_us_ == 0)	 // first frame
  {
	time_last_frame_us_ = frame_time_us;
	return flow_quality;
  }

  if (output_rate <= 0)
  {	 // don't limit the rate
	*dt_us = frame_time_us - time_last_frame_us_;
	time_last_frame_us_ = frame_time_us;
	return flow_quality;
  }

  if (flow_quality > 0)
  {
	sum_flow_x += *flow_x;
	sum_flow_y += *flow_y;
	sum_flow_quality += flow_quality;
	valid_frame_count++;
  }

  // limit rate according to parameter ouput_rate
  if ((frame_time_us - time_last_frame_us_) > (1.0e6f / output_rate))
  {
	int average_flow_quality = 0;

	// average the flow over the period since last update
	if (valid_frame_count > 0)
	{
	  average_flow_quality = std::floor(sum_flow_quality / valid_frame_count);
	}

	*flow_x = sum_flow_x;
	*flow_y = sum_flow_y;

	// reset variables
	initLimitRate();
	*dt_us = frame_time_us - time_last_frame_us_;
	time_last_frame_us_ = frame_time_us;

	return average_flow_quality;
  }
  else
  {
	return -1;	// signaling that it should not yet publish the values
  }
}

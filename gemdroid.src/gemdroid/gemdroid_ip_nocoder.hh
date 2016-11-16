/**
 * Copyright (c) 2016 The Pennsylvania State University
 * All rights reserved.
 *     
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contact: Shulin Zhao (suz53@cse.psu.edu)
*/

#ifndef GEMDROID_IP_NOC_HH_
#define GEMDROID_IP_NOC_HH_

#include "gemdroid/gemdroid_ip.hh"

class GemDroidIPNocoder : public GemDroidIP
{
private:
	int computeLatency; // cycles needed for processing input size data
	int inBufferSize;
	int outBufferSize;
	int currInBuffer;
	int currOutBuffer;
	int coding_ratio;

	bool updatePower();
	void sendDataOut();
	bool process();
	void requestData();

public:
	 void init (int ip_type, int id, bool isDevice, int computeLatency, int inBufferSize, int outBufferSize, int ratio, int chunkSize, bool isDecoder, int ip_freq, int opt_freq, GemDroid *gemDroid);
	 void tick();
	 void regStats();
	 void resetStats();
	 void printPeriodicStats();

	 void memResponse(uint64_t addr, bool isRead);
	 bool enqueueIPReq(int core_id, uint64_t addr, int size, bool isRead, int frameNum, int flowType, int flowId);
};


#endif /* GEMDROID_IP_NOC_HH_ */

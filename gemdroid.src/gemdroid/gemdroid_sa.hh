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

#ifndef __GEMDROID_SA_HH__
#define __GEMDROID_SA_HH__

#include "base/statistics.hh"
#include "gemdroid_request.hh"

using namespace std;

class GemDroidSA
{
private:
	string desc;
	int sa_id;
	GemDroid *gemDroid;
	long totalQueueSize;
	long m_cyclesToSkip;

	long dynamicActivity;

    list<GemDroidMemMsg> coreMemReq;
    list<GemDroidMemMsg> ipMemReq;
    list<GemDroidMemMsg> memCoreResp;
	list<GemDroidMemMsg> memIpResp;
	list<GemDroidIPResponse> ipCoreResp;
    list<GemDroidIPRequest> ipReq[IP_TYPE_END];

	void sendMemoryResponses();
	void sendMemoryRequests();
	void sendIPRequests();
	void sendIPResponses();
	bool enqueueIPIPRequest(int sendertype, int senderid, int core_id, int iptype, uint64_t addr, int size, bool isRead, int frameNum, int flowType, int flowId);
	void updateActivityCountIn1Ms(int activity_count = 1);
	void process();
	

public:
	
    GemDroidSA(int id, GemDroid *gemDroid);
	void tick();
	void regStats();
	void resetStats();
	void printPeriodicStats();
	double powerIn1ms();
	double getActivity();

	bool enqueueCoreMemRequest(int id, uint64_t addr, bool isRead);
	bool enqueueCoreIPRequest(int iptype, int id, uint64_t addr, int size, bool isRead, int frameNum, int flowType, int flowId);
	bool enqueueIPMemRequest(int type, int id, int core_id, uint64_t addr, bool isRead);
	bool enqueueIPResponse(int senderIPType, int senderIPId, int receiverCoreId, int frameNum, int flowType, int flowId);
	void memResponse(uint64_t addr, bool isRead, int sender_type, int sender_id);
	bool isIPReqLimitReached(int ip_type);

    Stats::Scalar ticks;
    Stats::Scalar numCoreMemReqs;
    Stats::Scalar numIPReqs;
    Stats::Scalar numIPMemReqs;
    Stats::Scalar numMemCoreResponse;
    Stats::Scalar numIPCoreResponse;
    Stats::Scalar numMemIPResponse;
    Stats::Scalar numRejected;

	long stats_ticks;
	long stats_numCoreMemReqs;
	long stats_numIPReqs;
	long stats_numIPMemReqs;
	long stats_numMemCoreResponse;
	long stats_numIPCoreResponse;
	long stats_numMemIPResponse;
	long stats_numRejected;

	int readMemCounter[IP_TYPE_END];
	int writeMemCounter[IP_TYPE_END];

};

#endif //__GEMDROID_SA_HH__

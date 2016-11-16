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


#include "gemdroid/gemdroid_ip_dma.hh"
#include "gemdroid/gemdroid.hh"

void GemDroidIPDMA::init(int ip_type, int id, GemDroid *gemDroid)
{
	this->gemDroid = gemDroid;
	this->ip_type = ip_type;
//For DC, it will be 1*100 + 0 or 1*100 +1
	//this->ip_id = (ip_type*IP_ID_START) + id;
	this->ip_id = id;
	isDevice = false;

	ostringstream oss;
	oss<<ip_id;

	desc="GemDroid.IP_";
	desc += ipTypeToString(ip_type);
	desc += '_';
	desc += oss.str();

	m_reqCount = -1;
	ticks = 0;
}

void GemDroidIPDMA::regStats()
{
	GemDroidIP::regStats();
}

void GemDroidIPDMA::resetStats()
{
	GemDroidIP::resetStats();
}

void GemDroidIPDMA::printPeriodicStats()
{
	GemDroidIP::printPeriodicStats();
}

bool GemDroidIPDMA::enqueueIPReq(int senderId, uint64_t srcAddr, uint64_t destAddr, int size, int frameNum, int flowId)
{
	if(!is_busy) {
		// Process the incoming request
		is_busy = true;
		m_frameNum = frameNum;
		m_reqCount = ceil((double) size / CACHE_LINE_SIZE);
		this->srcAddr = srcAddr;
		this->destAddr = destAddr;
		m_isRead = true;	//First time, it's a read. Then it should be a write.

		m_reqAddr = srcAddr; //current address that is to be transferred.
		m_CPUReqs++;

		return true;
	}
	else {
		 m_IPBusyStalls++;
		 return false;
	 }

	return true;
}

void GemDroidIPDMA::tick()
{
	ticks++;	
	
	if(m_reqCount > 0) {
		sendMemoryReq();

		if(m_reqCount == 0) {
			is_busy = false;
		}


	}
}

void GemDroidIPDMA::memResponse(uint64_t addr, bool isRead)
{
	if (isRead) {
		if(!gemDroid->gemdroid_sa.enqueueIPMemRequest(ip_type, ip_id, 0, destAddr, false)) {
			// Unable to inject. Retry next tick.
			m_memRejected++;
		}
		else {
			//Successfully enqueued
			//cout<<" IPReqs:"<<gemDroid->ipMemRequests<<endl;
			destAddr += CACHE_LINE_SIZE;
			m_MemReqs++;
		}
	}
}

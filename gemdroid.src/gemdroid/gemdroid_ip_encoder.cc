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

#include "gemdroid/gemdroid_ip_encoder.hh"
#include "gemdroid/gemdroid.hh"

void GemDroidIPEncoder::init(int ip_type, int id, bool isDevice, int computeLatency, int inBufferSize, int outBufferSize, int coding_ratio, int processing_chunk_size,  bool isDecoder, int ip_freq, int opt_freq, GemDroid *gemDroid)
{
	GemDroidIP::init(ip_type, id, isDevice, 0, ip_freq, opt_freq, gemDroid);

	this->computeLatency = computeLatency;
	this->inBufferSize = inBufferSize;
	this->outBufferSize = outBufferSize;
	this->coding_ratio = coding_ratio;
	this->targetOutputChunkSizeInCL = processing_chunk_size;

	cout<<desc<<"inited with:" <<"targetOutputChunkSizeInCL = "<<targetOutputChunkSizeInCL<<" coding_ratio="<<coding_ratio\
			<<"inBufferSize="<<inBufferSize<<" outBufferSize="<<outBufferSize<<endl;

	currInBuffer = 0;
	currOutBuffer = 0;
}

bool GemDroidIPEncoder::updatePower()
{
	return (GemDroidIP::updatePower());
}

void GemDroidIPEncoder::regStats()
{
	GemDroidIP::regStats();
}

void GemDroidIPEncoder::resetStats()
{
	GemDroidIP::resetStats();
}

void GemDroidIPEncoder::printPeriodicStats()
{
	GemDroidIP::printPeriodicStats();
}

void GemDroidIPEncoder::memResponse(uint64_t addr, bool isRead)
{
	if (isRead) {
		currInBuffer++;
	}
	else {
	}
	GemDroidIP::memResponse(addr, isRead);
}

bool GemDroidIPEncoder::enqueueIPReq(int core_id, uint64_t addr, int size, bool isRead, int frameNum, int flowType, int flowId)
{
	if (GemDroidIP::enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId)) {
		if(ip_type == IP_TYPE_VE)
			m_reqOutAddr = VE_ADDR_START + FRAME_SIZE;
		else if(ip_type == IP_TYPE_AE)
			m_reqOutAddr = AE_ADDR_START + AUD_FRAME_SIZE;

		m_IPMemStallsFrame = 0;

		assert(currInBuffer == 0);
		assert(currOutBuffer == 0);

		inputProcessedInCL = 0;
		inputToBeProcessedInCL = size / CACHE_LINE_SIZE;
		reqSent = 0;

		// DBGPRINT<<" got request for:"<< size<<" "<<frameNum<< " " << flowType<<" "<<flowId;
		// cout<<" inputToBeProcessedInCL to be:"<<inputToBeProcessedInCL<<endl;
		return true;
	}
	return false;
}

void GemDroidIPEncoder::sendDataOut()
{
	if (currOutBuffer > 0) {
		if(!gemDroid->gemdroid_sa.enqueueIPMemRequest(ip_type, ip_id, core_id, m_reqOutAddr, false)) {
			// Unable to inject. Retry next tick.
			m_memRejected++;
		}
		else {
			//Successfully enqueued
			m_reqOutAddr = m_reqOutAddr + CACHE_LINE_SIZE;
			m_MemReqs++;
			currOutBuffer--;
		}
	}
}

void GemDroidIPEncoder::requestData()
{
	if(m_reqCount > 0 && reqSent < inBufferSize) {
		if(sendMemoryReq())
			reqSent++; //cout<<"Requested data"<<endl;
		else
			; //cout<<"Need to retry"<<endl;
	}
}

// returns true when processed something
bool GemDroidIPEncoder::process()
{
	if (currOutBuffer + targetOutputChunkSizeInCL > outBufferSize)
		return false;

	if (m_cyclesToSkip > 0) {
		m_cyclesToSkip--;

		if (m_cyclesToSkip == 0) {
			inputProcessedInCL++;
			// DBGPRINT<<" completed processing one input CL. Total inputProcessedInCL = "<<inputProcessedInCL<<endl;
			if (inputProcessedInCL % (targetOutputChunkSizeInCL * coding_ratio) == 0) {
				currOutBuffer += targetOutputChunkSizeInCL;
				// DBGPRINT<<" Completed one chunk. currentOutBuffer = " << currOutBuffer<<endl;
			}
		}

		return true;
	}

	if (currInBuffer > 0) {
		m_cyclesToSkip = computeLatency/coding_ratio;
		currInBuffer--;
		reqSent--;
	}
	else
		return false;

	return true;
}

void GemDroidIPEncoder::tick()
{
	ticks++;

	if(!updatePower()) {
		return;
	}

	assert(isPStateActive());

	if (is_busy == false) {
		idleCycles++;
		return;
	}
	else
		m_IPWorkingCycles++;

	sendDataOut();

	requestData();

	bool is_success = false;
	if (inputProcessedInCL < inputToBeProcessedInCL)
		is_success = process();

	if (inputProcessedInCL == inputToBeProcessedInCL && currOutBuffer == 0) {
		is_busy = false;
		gemDroid->gemdroid_sa.enqueueIPResponse(ip_type, ip_id, core_id, m_frameNum, m_flowType, m_flowId);
		// DBGPRINT << " completed processing frame " << m_frameNum << endl;
	}

	if (!is_success) {
		m_IPMemStalls++;
	}
}

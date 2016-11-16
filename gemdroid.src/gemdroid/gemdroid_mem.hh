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

#ifndef __GEMDROID_MEM_HH__
#define __GEMDROID_MEM_HH__

#include "base/statistics.hh"
#include "mem/dramsim2_wrapper.hh"

using namespace std;

class GemDroidMemory
{
private:
	/**
	* The actual DRAMSim2 wrapper
	*/
	DRAMSim2Wrapper dramWrapper;
	int mem_id;
	string desc;
	GemDroid *gemDroid;
	long ticks;
	bool perfectMemory;
	int perfectMemLatency;
	int m_cyclesToStall;

	double m_freq;
    double m_optMemFreq;

	double totalPowerConsumed;
	double powerConsumed; //in the last 1 milli-seconds
    double m_power;

	uint64_t m_addr;
	bool m_isRead;
	int m_type;
	int m_id;
	/**
	* Read completion callback.
	*
	* @param id Channel id of the responder
	* @param addr Address of the request
	* @param cycle Internal cycle count of DRAMSim2
	*/
	void readComplete(unsigned id, uint64_t addr, uint64_t cycle, int sender_type, int sender_id);

	/**
	* Write completion callback.
	*
	* @param id Channel id of the responder
	* @param addr Address of the request
	* @param cycle Internal cycle count of DRAMSim2
	*/
	void writeComplete(unsigned id, uint64_t addr, uint64_t cycle, int sender_type, int sender_id);

public:
	GemDroidMemory(int id, string deviceConfigFile, string systemConfigFile, string filePath,
		string traceFile, long size, bool perfectMemory, bool enableDebug, GemDroid *gemDroid);
	void regStats();
	void resetStats();
	void printPeriodicStats();
	double powerIn1ms();
	
	double getMaxBandwidth(double freq); //in Ghz; GBPS.
	double getMaxBandwidth(); //in Ghz; GBPS.

	void setMemFreq(double freq); //freq in Ghz
	double getMemFreq();  //returns freq in Ghz
    inline double getMaxMemFreq() { return MAX_MEM_FREQ; }
    inline double getMinMemFreq() { return MIN_MEM_FREQ; }
    inline double getOptMemFreq() { return m_optMemFreq; }
    void setMaxMemFreq(); //m_freq in Ghz
    void setOptMemFreq(); //m_freq in Ghz
    void setMinMemFreq(); //m_freq in Ghz
	void incMemFreq(int steps=1); //freq in Ghz
	void decMemFreq(int steps=1); //freq in Ghz
    double getFreqForBandwidth(double bw); //m_freq in Ghz; function return val in GBPS;
	double getBandwidth(); //in GBPS
    double getLastLatency();
    int getNumChannels();
    double getEnergyEst(double currFreq, double currEnergy, double newFreq);
	
	void tick();

	bool enqueueMemReq(int type, int id, int core_id, uint64_t addr, bool isRead);

	Stats::Scalar m_memCPUReqs;
	Stats::Scalar m_memIPReqs;
	Stats::Scalar m_memRejected;
	long stats_m_memCPUReqs;
	long stats_m_memIPReqs;
	long stats_m_memRejected;

};

#endif //__GEMDROID_MEM_HH__

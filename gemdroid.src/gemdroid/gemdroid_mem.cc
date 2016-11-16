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

#include "sim/system.hh"
#include "DRAMSim2/Callback.h"
#include "base/callback.hh"
#include "gemdroid/gemdroid.hh"

using namespace std;

GemDroidMemory::GemDroidMemory(int id, string deviceConfigFile, string systemConfigFile, string filePath,
		string traceFile, long sizeMB, bool perfectMemory, bool enableDebug, GemDroid *gemDroid) :
		dramWrapper(deviceConfigFile, systemConfigFile, filePath, traceFile, sizeMB, enableDebug)
{
	mem_id=id;
	this->gemDroid = gemDroid;
	desc="GemDroid.Memory_";
	desc += (char)(id+'0');
	ticks = 0;
	this->perfectMemory = perfectMemory;
	perfectMemLatency = PEFECT_MEM_LATENCY;
	m_cyclesToStall = 0;
    m_optMemFreq = 0.8; // 800 MHz

	// overall_stats
	m_memCPUReqs = 0;
	m_memIPReqs = 0;
	m_memRejected = 0;
	// maintained for per phase stat used in print function.
	stats_m_memCPUReqs = 0;
	stats_m_memIPReqs = 0;
	stats_m_memRejected = 0;

	DRAMSim::TransactionCompleteCB* read_cb =
		new DRAMSim::Callback<GemDroidMemory, void, unsigned, uint64_t, uint64_t, int , int>(
			this, &GemDroidMemory::readComplete);
	DRAMSim::TransactionCompleteCB* write_cb =
		new DRAMSim::Callback<GemDroidMemory, void, unsigned, uint64_t, uint64_t, int, int>(
			this, &GemDroidMemory::writeComplete);
	dramWrapper.setCallbacks(read_cb, write_cb);

	// Register a callback to compensate for the destructor not
	// being called. The callback prints the DRAMSim2 stats.
	Callback* cb = new MakeCallback<DRAMSim2Wrapper,&DRAMSim2Wrapper::printStats>(dramWrapper);
	if(!perfectMemory)
		registerExitCallback(cb);

    cout<<"Instantiated GemDroid::DRAMSim2 with clock "<<dramWrapper.clockPeriod()<< "ns and queue size "<<dramWrapper.queueSize()<<endl;
}

void GemDroidMemory::regStats()
{
	m_memCPUReqs.name(desc + ".memCPUReqs").desc("GemDroid: Number of memory requests from cores").flags(Stats::display);
	m_memIPReqs.name(desc + ".memIPReqs").desc("GemDroid: Number of memory requests from IPs").flags(Stats::display);
	m_memRejected.name(desc + ".memRejected").desc("GemDroid: Number of memory requests rejected by Memory").flags(Stats::display);
}

void GemDroidMemory::resetStats()
{
	m_memCPUReqs = 0;
	m_memIPReqs = 0;
	m_memRejected = 0;
}

void GemDroidMemory::printPeriodicStats()
{
	//cout<<"-=-=-=-=-=-=-=-=-=-=-=-"<<endl;
	//Print per-phase stats
	if(0) {
		cout<<desc<<".m_memCPUReqs: "<<m_memCPUReqs.value()<<endl;
		cout<<desc<<".m_memIPReqs: "<<m_memIPReqs.value()<<endl;
		cout<<desc<<".m_memReqs: "<<m_memIPReqs.value() + m_memCPUReqs.value()<<endl;
		cout<<desc<<".m_memRejected: "<<m_memRejected.value()<<endl;
	}
	else {
		cout<<desc<<" "\
			<<m_memCPUReqs.value() - stats_m_memCPUReqs<< " "\
			<<m_memIPReqs.value() - stats_m_memIPReqs<< " "\
			<<m_memIPReqs.value()+m_memCPUReqs.value()-stats_m_memIPReqs-stats_m_memCPUReqs<< " "\
			<<m_memRejected.value() - stats_m_memRejected<< " "
			<<endl;
/*		cout<<desc<<".m_memCPUReqs: "	
		cout<<desc<<".m_memIPReqs: "	
		cout<<desc<<".m_memReqs: "		
		cout<<desc<<".m_memRejected: "	
*/	}
	//Set stats with latest numbers
	stats_m_memCPUReqs = m_memCPUReqs.value();
	stats_m_memIPReqs = m_memIPReqs.value();
	stats_m_memRejected = m_memRejected.value();

	dramWrapper.printStats(false);
}

void GemDroidMemory::tick()
{
	// To print periodic stats of memory along with other components add 4 everytime
	ticks++;

	dramWrapper.tick();

/*	if(perfectMemory) {
		 //Perfect Memory
		if(m_cyclesToStall > 0) {
			m_cyclesToStall--;
			if(m_cyclesToStall == 0) {
				// gemDroid->gemdroid_sa.memResponse(m_addr, m_isRead, m_type, m_id);
			}
		}
	}*/
}

void GemDroidMemory::readComplete(unsigned id, uint64_t addr, uint64_t cycle, int sender_type, int sender_id)
{
	// cout << "READ" << addr << "  " << sender_type << "  " << sender_id << endl;
	gemDroid->gemdroid_sa.memResponse(addr, true, sender_type, sender_id);
//	cout<< ticks <<" Response Read "<<addr<<endl;
}

void GemDroidMemory::writeComplete(unsigned id, uint64_t addr, uint64_t cycle, int sender_type, int sender_id)
{
	// cout << "WRITE" << addr << "  " << sender_type << "  " << sender_id << endl;
	gemDroid->gemdroid_sa.memResponse(addr, false, sender_type, sender_id);
//    cout<< ticks <<" Response Write "<<addr<<endl;
}

// enqueue the memory request to either dramsim2 or ruby
// return true when successfully enqueued
bool GemDroidMemory::enqueueMemReq(int type, int id, int core_id, uint64_t addr, bool isRead)
{
	if(addr == 0) {
		cout<<"\n Component "<<type << "_" <<id<<" trying to inject address 0 into DRAM"<<endl;
		assert(0);
	}

	if(!perfectMemory) {
		if(dramWrapper.canAccept()) {

			if (type == IP_TYPE_CPU)
				m_memCPUReqs++;
			else
				m_memIPReqs++;

			gemDroid->appMemReqs[core_id]++;
			gemDroid->ipMemReqs[type]++;

			//DramWrapper expects "isWrite". So, we do !isRead.
			//cout << "JOOMLA: " << ticks << " : " << type << " : " << isRead << " : " << addr << endl;
			dramWrapper.enqueue(!isRead,addr, type, id);
			// cout << "Memory: " << ticks << "  " << id << " enqueued " << isRead << "  " << addr <<endl;

		 	return true;
		}
		else {
			 m_memRejected++;
			 return false;
		}
	}
	else { // Perfect Memory for IPs.
		gemDroid->gemdroid_sa.memResponse(addr, isRead, type, id);
		if (type == IP_TYPE_CPU)
			m_memCPUReqs++;
		else
			m_memIPReqs++;
		return true;
	}
}

double GemDroidMemory::powerIn1ms()
{
   double power = dramWrapper.getPower();

   if (std::isnan(power))
       return m_power;
   else {
       m_power = power;
       return power;
   }
}

void GemDroidMemory::setMemFreq(double freq) //m_freq in Ghz
{
	assert (freq >= (MIN_MEM_FREQ-EPSILON) && freq <= (MAX_MEM_FREQ+EPSILON));

	m_freq = freq;
	dramWrapper.updateFreq(freq);	
}

double GemDroidMemory::getMemFreq() //m_freq in Ghz
{
	return m_freq;	
}

void GemDroidMemory::setMinMemFreq() //m_freq in Ghz
{
	m_freq = MIN_MEM_FREQ;
	dramWrapper.updateFreq(m_freq);		
}

void GemDroidMemory::setMaxMemFreq() //m_freq in Ghz
{
	m_freq = MAX_MEM_FREQ;
	dramWrapper.updateFreq(m_freq);		
}

void GemDroidMemory::setOptMemFreq() //m_freq in Ghz
{
	m_freq = m_optMemFreq;
	dramWrapper.updateFreq(m_freq);		
}

void GemDroidMemory::incMemFreq(int steps) //m_freq in Ghz
{
	if(m_freq+0.1*steps <= MAX_MEM_FREQ) {
		m_freq = m_freq+0.1*steps;
	}
	else
		m_freq = MAX_MEM_FREQ;

	dramWrapper.updateFreq(m_freq);		
}

void GemDroidMemory::decMemFreq(int steps) //m_freq in Ghz
{
	if(m_freq-(0.1*steps) >= MIN_MEM_FREQ) {
		m_freq = m_freq-0.1*steps;
	}
	else
		m_freq = MIN_MEM_FREQ;

	dramWrapper.updateFreq(m_freq);	
}

double GemDroidMemory::getBandwidth() //in GBPS
{
	return dramWrapper.getBandwidth();
}

double GemDroidMemory::getMaxBandwidth(double freq) //m_freq in Ghz; function return val in GBPS;
{
	return (64*2*freq/8);//64 bytes cache line, DDR=2data transfers per clock edge, m_frequency, 8=bits to bytes.
}

double GemDroidMemory::getFreqForBandwidth(double bw) //m_freq in Ghz; function return val in GBPS;
{
	return (bw/16);
}

double GemDroidMemory::getMaxBandwidth() //m_freq in Ghz; function return val in GBPS;
{
	return (64*2*m_freq/8);//64 bytes cache line, DDR=2data transfers per clock, m_frequency, 8=bits to bytes.
}

double GemDroidMemory::getLastLatency()
{
	return dramWrapper.getLatency();
}

int GemDroidMemory::getNumChannels()
{
	return dramWrapper.getNumChannels();
}

double GemDroidMemory::getEnergyEst(double currFreq, double currEnergy, double newFreq)
{
    // double levels = (newFreq - currFreq) * 10;
    // double newEnergy = currEnergy - levels * currEnergy * 0.1;
    double newEnergy = currEnergy * (newFreq / currFreq);

    return newEnergy;
}

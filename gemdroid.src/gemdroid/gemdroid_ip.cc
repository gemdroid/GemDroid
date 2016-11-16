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

#include "gemdroid/gemdroid.hh"
#include "gemdroid/gemdroid_ip.hh"

using namespace std;
using namespace Stats;

GemDroidIP::GemDroidIP()
{

}

void GemDroidIP::init(int ip_type, int id, bool isDevice, int ioLatency, int ip_freq, int opt_freq, GemDroid *gemDroid)
{
	this->gemDroid = gemDroid;
	this->ip_type = ip_type;
	this->isDevice = isDevice;
	//For DC, it will be 1*100 + 0 or 1*100 +1
	//this->ip_id = (ip_type*IP_ID_START) + id;
	this->ip_id = id;
	this->ioLatency = ioLatency;

	ostringstream oss;
	oss<<ip_id;

	desc="GemDroid.IP_";
	desc += ipTypeToString(ip_type);
	desc += '_';
	desc += oss.str();

	setPStateActive();
	m_reqCount = -1;
	ticks = 0;

	idleCycles = 0;
	cyclesToWake = 0;
	power_state = ip_idle;
	idleStreak = 0;
	m_cyclesToSkip = 0;

	m_CPUReqs = 0;
	m_MemReqs = 0;
	m_IPActiveCycles = 0;
	m_IPIdleCycles = 0;
	m_IPLowPowerCycles = 0;
	m_IPBusyStalls = 0;
	m_memRejected = 0;
	m_IPMemStalls = 0;
	m_IPWorkingCycles = 0;

	stats_ticks = 0;
	stats_m_CPUReqs = 0;
	stats_m_MemReqs = 0;
	stats_m_IPBusyStalls = 0;
	stats_m_IPActiveCycles = 0;
	stats_m_IPLowPowerCycles = 0;
	stats_m_IPIdleCycles = 0;
	stats_m_IPWorkingCycles = 0;
	stats_m_memRejected = 0;
	stats_m_IPMemStalls = 0;

	if(ip_type == IP_TYPE_DC) {
		ip_static_power = DC_STATIC_PWR; 	//0.40 WATT Samsung S4 (average brightness); 1.3 for full brightness;
		capacitance = DC_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.400;
	}
	else if(ip_type == IP_TYPE_SND) {
		ip_static_power = SND_STATIC_PWR;
		capacitance = SND_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.400;
	}
	else if(ip_type == IP_TYPE_MIC) {
		ip_static_power = MIC_STATIC_PWR;
		capacitance = MIC_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.400;
	}
	else if(ip_type == IP_TYPE_NW) {
		ip_static_power = NW_STATIC_PWR;
		capacitance = NW_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.400;
	}
	else if(ip_type == IP_TYPE_CAM) {
		ip_static_power = CAM_STATIC_PWR;
		capacitance = CAM_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.400;
	}
	else if(ip_type == IP_TYPE_MMC_IN) {
		ip_static_power = MMC_STATIC_PWR;
		capacitance = MMC_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.400;
	}
	else if(ip_type == IP_TYPE_MMC_OUT) {
		ip_static_power = MMC_STATIC_PWR;
		capacitance = MMC_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.400;
	}
	else if(ip_type == IP_TYPE_VD) {
		ip_static_power = VD_STATIC_PWR;
		capacitance = VD_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.300;
	}
	else if(ip_type == IP_TYPE_VE) {
		ip_static_power = VE_STATIC_PWR;
		capacitance = VE_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.300;
	}
	else if(ip_type == IP_TYPE_AD) {
		ip_static_power = AD_STATIC_PWR;
		capacitance = AD_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.400;
	}
	else if(ip_type == IP_TYPE_AE) {
		ip_static_power = AE_STATIC_PWR; 	
		capacitance = AE_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.400;
	}
	else if(ip_type == IP_TYPE_IMG) {
		ip_static_power = IMG_STATIC_PWR; 	
		capacitance = IMG_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.300;
	}
	else if(ip_type == IP_TYPE_GPU) {
		ip_static_power = GPU_STATIC_PWR;
		capacitance = GPU_DYNAMIC_PWR_PER_CL;
        optimal_freq = 0.200;
	}
	else {
		assert (1);
	}

    // voltage & freq
    // voltage_freq_table[0][0] 	= 0.798 ; voltage_freq_table[0][1] 	= 0.100; 
    voltage_freq_table[0][0] 	= 0.835	; voltage_freq_table[0][1] 	= 0.200; 
    voltage_freq_table[1][0] 	= 0.872	; voltage_freq_table[1][1] 	= 0.300; 
    voltage_freq_table[2][0] 	= 0.909	; voltage_freq_table[2][1] 	= 0.400; 
    voltage_freq_table[3][0] 	= 0.946	; voltage_freq_table[3][1] 	= 0.500; 
    voltage_freq_table[4][0] 	= 0.983	; voltage_freq_table[4][1] 	= 0.600; 
    voltage_freq_table[5][0] 	= 1.02	; voltage_freq_table[5][1] 	= 0.700; 
    voltage_freq_table[6][0] 	= 1.057	; voltage_freq_table[6][1] 	= 0.800; 
    /* voltage_freq_table[7][0] 	= 1.094	; voltage_freq_table[7][1] 	= 0.900; 
    voltage_freq_table[8][0] 	= 1.131	; voltage_freq_table[8][1] 	= 1.000; 
    voltage_freq_table[9][0] 	= 1.168	; voltage_freq_table[9][1]	= 1.100; 
    voltage_freq_table[10][0] 	= 1.205	; voltage_freq_table[10][1]	= 1.200;  */

    // calculate max dynamic values
    for(int i=0; i<IP_DVFS_STATES; i++) {
        voltage_freq_table[i][2] = capacitance * voltage_freq_table[i][0] * voltage_freq_table[i][0] * voltage_freq_table[i][1];
    }

    setIPFreq(ip_freq/1000.0);
    m_optDVFSState = opt_freq;

	m_IPActiveInLast1ms 	= 0;
	m_IPLowInLast1ms		= 0;
	m_IPActivityIn1ms   	= 0;
    m_IPActivityInDVFSEpoch = 0;

	m_IPActiveInLast1us 	= 0;
	m_IPLowInLast1us		= 0;
	m_IPActivityIn1us   	= 0;

	cout<<"Instantiated GemDroidIP type: "<<ipTypeToString(ip_type)<<" Id: "<<id<<endl;

    cout << ipTypeToString(ip_type) << ": Volt-Freq Table: " << endl;
    printDVFSTable();
}

void GemDroidIP::wakeUp()  //Woken up by SA
{
	if (isPStateLowPower()) {
		if(isDevice)
			cyclesToWake = IPDEV_PSTATE_LOWPOWER_EXIT_TIME;
		else
			cyclesToWake = IPACC_PSTATE_LOWPOWER_EXIT_TIME;
	}
	else if (isPStateIdle()) {
		if(isDevice)
			cyclesToWake = IPDEV_PSTATE_IDLE_EXIT_TIME;
		else
			cyclesToWake = IPACC_PSTATE_IDLE_EXIT_TIME;
	}
}

void GemDroidIP::regStats()
{
	ticks.name(desc + ".ticks").desc("GemDroid: Number of ticks for this IP").flags(Stats::display);
	m_CPUReqs.name(desc + ".CPUCalls").desc("GemDroid: Number of IP calls from CPUs").flags(Stats::display);
	m_MemReqs.name(desc + ".MemReqs").desc("GemDroid: Number of memory requests from this IP").flags(Stats::display);
	m_IPBusyStalls.name(desc + ".IPBusyRejects").desc("GemDroid: Number of times calls were rejected because the IP was busy").flags(Stats::display);
	m_IPActiveCycles.name(desc + ".IPActiveCycles").desc("GemDroid: Number of active cycles of this IP").flags(Stats::display);
	m_IPLowPowerCycles.name(desc + ".IPLowPowerCycles").desc("GemDroid: Number of low-power state cycles of this IP").flags(Stats::display);
	m_IPIdleCycles.name(desc + ".IPIdleCycles").desc("GemDroid: Number of idle cycles of this IP").flags(Stats::display);
	m_IPWorkingCycles.name(desc + ".IPWorkingCycles").desc("GemDroid: Number of working cycles of this IP").flags(Stats::display);
	m_memRejected.name(desc + ".memRejected").desc("GemDroid: Number of Ip memory requests rejected by memory").flags(Stats::display);
	m_IPMemStalls.name(desc + ".m_IPMemStalls").desc("GemDroid: Number of Ip cycles that were stalled by memory").flags(Stats::display);
}

void GemDroidIP::resetStats()
{
	ticks = 0;
	m_CPUReqs = 0;
	m_MemReqs = 0;
	m_IPActiveCycles = 0;
	m_IPIdleCycles = 0;
	m_IPWorkingCycles = 0;
	m_IPLowPowerCycles = 0;
	m_IPBusyStalls = 0;
	idleStreak=0;
	m_IPMemStalls = 0;
}

void GemDroidIP::printPeriodicStats()
{
	//cout<<"*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#"<<endl;
	//Print per-phase stats
	//Overall stats
	if(0) {
		cout<<desc<<".m_cycles: "			<< ticks.value()<<endl;
		cout<<desc<<".m_Calls: "			<< m_CPUReqs.value()<<endl;
		cout<<desc<<".m_memReqs: "			<< m_MemReqs.value()<<endl;
		cout<<desc<<".m_IPBusyRejects: "	<< m_IPBusyStalls.value()<<endl;
		cout<<desc<<".m_IPActiveCycles: "	<< m_IPActiveCycles.value()<<endl;
		cout<<desc<<".m_IPLowPowerCycles: "	<< m_IPLowPowerCycles.value()<<endl;
		cout<<desc<<".m_IPIdleCycles: "		<< m_IPIdleCycles.value()<<endl;
		cout<<desc<<".m_IPWorkingCycles: "	<< m_IPWorkingCycles.value() <<endl;
		cout<<desc<<".m_memRejected: "		<< m_memRejected.value()<<endl;	
		cout<<desc<<".m_IPMemStalls: "		<< m_IPMemStalls.value()<<endl;	
	}
	else {		
		/*cout<<desc<<".State: "			    << pStateToString(power_state) << " CyclesToWake: " << cyclesToWake << " Busy: " << is_busy << " m_reqCount: " << m_reqCount<<" m_respCount: " << m_respCount<< endl;
		cout<<desc<<".m_cyclesToSkip: "		<< m_cyclesToSkip<<endl;
		cout<<desc<<".m_frameNum: "			<< m_frameNum<<endl;
		cout<<desc<<".m_cycles: "			<< ticks.value() - stats_ticks<<endl;
		//cout<<desc<<".m_Calls: "			<< m_CPUReqs.value() - stats_m_CPUReqs <<endl;
		cout<<desc<<".m_memReqs: "			<< m_MemReqs.value() - stats_m_MemReqs <<endl;
		cout<<desc<<".m_memBWRequested: "	<< (m_MemReqs.value() - stats_m_MemReqs)*CACHE_LINE_SIZE/250000.0<<" GBPS"<<endl; 		//800Mhz memory. So, 1 sec equal to 800M cycles. 5,000 cycles is equal to 6.25 uSec.
		cout<<desc<<".m_IPBusyRejects: "	<< m_IPBusyStalls.value() - stats_m_IPBusyStalls <<endl;
		cout<<desc<<".m_IPActiveCycles: "	<< m_IPActiveCycles.value() - stats_m_IPActiveCycles <<endl;
		cout<<desc<<".m_IPLowPowerCycles: "	<< m_IPLowPowerCycles.value() - stats_m_IPLowPowerCycles <<endl;
		cout<<desc<<".m_IPIdleCycles: "		<< m_IPIdleCycles.value() - stats_m_IPIdleCycles <<endl;
		cout<<desc<<".m_IPWorkingCycles: "	<< m_IPWorkingCycles.value() - stats_m_IPWorkingCycles <<endl;
		cout<<desc<<".m_memRejected: "		<< m_memRejected.value() - stats_m_memRejected <<endl;
		cout<<desc<<".m_IPMemStalls: "		<< m_IPMemStalls.value() - stats_m_IPMemStalls<<endl;*/
		std::cout.precision (1);

		cout<<desc<<" ";
		cout<<m_cyclesToSkip<<" "<<\
				m_frameNum<<" "<<\
				ticks.value() - stats_ticks<<" "<<\
				m_CPUReqs.value() - stats_m_CPUReqs <<" "<<\
				m_MemReqs.value() - stats_m_MemReqs <<" "<<\
				m_IPBusyStalls.value() - stats_m_IPBusyStalls <<" "<<\
				m_IPActiveCycles.value() - stats_m_IPActiveCycles <<" "<<\
				m_IPLowPowerCycles.value() - stats_m_IPLowPowerCycles <<" "<<\
				m_IPIdleCycles.value() - stats_m_IPIdleCycles <<" "<<\
				m_IPWorkingCycles.value() - stats_m_IPWorkingCycles <<" "<<\
				m_memRejected.value() - stats_m_memRejected <<" "<<\
				m_IPMemStalls.value() - stats_m_IPMemStalls<<" "<<endl;
	}

	//Set stats to latest value
	stats_ticks = ticks.value();
	stats_m_CPUReqs = m_CPUReqs.value();
	stats_m_MemReqs = m_MemReqs.value();
	stats_m_IPBusyStalls = m_IPBusyStalls.value();
	stats_m_IPActiveCycles = m_IPActiveCycles.value();
	stats_m_IPLowPowerCycles = m_IPLowPowerCycles.value();
	stats_m_IPIdleCycles = m_IPIdleCycles.value();
	stats_m_IPWorkingCycles = m_IPWorkingCycles.value();
	stats_m_memRejected = m_memRejected.value();
	stats_m_IPMemStalls = m_IPMemStalls.value();

}

void GemDroidIP::printDVFSTable()
{
    for(int i=0; i<IP_DVFS_STATES; i++)
        cout << i << ": " << voltage_freq_table[i][0] << " " << voltage_freq_table[i][1] << " " << voltage_freq_table[i][2] << endl;
}


void GemDroidIP::streakCalculator()
{
    //ignoring idleStreaks for now.

	/*if(idleStreak > 0) {
		gemDroid->idleStreaksinActivePState[ip_type][ip_id].sample(idleStreak);
		idleStreak=0; //Reset streak of rejects
	}*/
}

bool GemDroidIP::sendMemoryReq()
{
	// Enqueue memory request through gemdroid class
	if(!gemDroid->gemdroid_sa.enqueueIPMemRequest(ip_type, ip_id, core_id, m_reqAddr, m_isRead)) {
		// Unable to inject. Retry next tick.
		m_memRejected++;
		return false;
	}
	else {
		//Successfully enqueued
		//cout<<" IPReqs:"<<gemDroid->ipMemRequests<<endl;
		m_reqCount--;
		m_reqAddr = m_reqAddr + CACHE_LINE_SIZE;
		m_MemReqs++;
		
		IPActivityUpdate();

		return true;
	}
}

void GemDroidIP::memResponse(uint64_t addr, bool isRead)
{
	m_respCount++;
}

void GemDroidIP::setMaxAllowedFreq()
{
    double powerAllowed = gemDroid->getCoordinatedPower() - powerInLastEpoch;
    double freq = getFreqForPower(powerAllowed);

    cout <<"IP "<<ipTypeToString(ip_type)<<": Power Left = " << powerAllowed << " Frequency selected " << freq << endl;
    setIPFreq(freq);
}

void GemDroidIP::setPStateActive()
{
    if(gemDroid->getGovernor() == GOVERNOR_TYPE_INTERACTIVE && ip_type > IP_TYPE_VD)
        setMaxIPFreq();
    else if(gemDroid->getGovernor() == GOVERNOR_TYPE_POWERCAP && ip_type > IP_TYPE_VD) {
        setOptIPFreq();
    }

    gemDroid->markIPRequestStarted(0, ip_type, ip_id, 0);
    power_state = ip_active;
}

double GemDroidIP::getFreqForPower(double power)
{
    for(int i=0; i<IP_DVFS_STATES; i++)
        if(voltage_freq_table[i][2] < power)
            return voltage_freq_table[i][1];

    return voltage_freq_table[IP_DVFS_STATES-1][1];
}

//  returns true if this tick can proceed
bool GemDroidIP::updatePower()
{
	// Update statistics
	if (isPStateActive()) {
		m_IPActiveCycles++;
		m_IPActiveInLast1ms++;
		m_IPActiveInLast1us++;
	}
	else if(isPStateLowPower()){
		m_IPLowPowerCycles++;
		m_IPLowInLast1ms++;	
		m_IPLowInLast1us++;	
		idleCycles++;
	}
	else
		m_IPIdleCycles++;

	//IP is waking up from idle state to Active state
	//Cannot do work while waking up!
	if (cyclesToWake > 0) {
		cyclesToWake--;
		if (cyclesToWake <= 0) {
			assert(!isPStateActive());
			setPStateActive();
		    gemDroid->markIPRequestStarted(core_id, ip_type, ip_id, m_frameNum);
			idleCycles = 0;
		}
		return false;
	}

	// Idle cycles are being incremented during cycles when IP does not do anything. (in tick function)
	// If that number is larger than PSTATE_ENTER_TIME, then move the IP to a lower power state..
	//And, reset idleCycles. Because we are moving down to a lower power state, we cannot proceed with work.
    int enterTime;
    if (isDevice)
        enterTime = IPDEV_PSTATE_ENTER_TIME;
    else
        enterTime = IPACC_PSTATE_ENTER_TIME;

	if (idleCycles >= enterTime) {
		if (isPStateActive()) {
			setPStateLowPower();
		}
		else if (isPStateLowPower()) {
			setPStateIdle();
		}
		idleCycles = 0;
		return false;
	}

	//IP is not waking up; or, it is not going to lower power state.
	//So, if it is in active state now, do work. If in idle or low-power, cannot do work.
	if (isPStateActive())
		return true;
	else
		return false;
}

// enqueue the memory request to either dramsim2 or ruby
// return true when successfully enqueued
// Size in KBs
bool GemDroidIP::enqueueIPReq(int core_id, uint64_t addr, int size, bool isRead, int frameNum, int flowType, int flowId)
{
	if(!is_busy) {
/*		DBGPRINT<<" got a call for size: "<<size<<" framenum: "<<frameNum<<endl;
*/
		// Process the incoming request
		is_busy = true;
		m_reqCount = ceil((double) size / CACHE_LINE_SIZE);
		m_reqAddr = addr;
		m_isRead = isRead;
		m_frameNum = frameNum;
		m_CPUReqs++;
		m_respCount = 0;
		m_dataSize = ceil((double) size);
		idleCycles=0;
		startCycle = ticks.value();
		this->core_id = core_id;
		m_cyclesToSkip = ioLatency * m_reqCount;
        m_flowType = flowType;
        m_flowId = flowId;
		//assert(m_flowId != -1);

		if (isPStateActive()) {
		    gemDroid->markIPRequestStarted(core_id, ip_type, ip_id, m_frameNum);
        }
        else if (isPStateLowPower()) {
/*			DBGPRINT<<" in lowPower mode"<<endl;*/

			if(isDevice)
				cyclesToWake = IPDEV_PSTATE_LOWPOWER_EXIT_TIME;
			else
				cyclesToWake = IPACC_PSTATE_LOWPOWER_EXIT_TIME;
		}
		else if (isPStateIdle()) {
/*			DBGPRINT<<" in idle mode"<<endl;*/
			if(isDevice)
				cyclesToWake = IPDEV_PSTATE_IDLE_EXIT_TIME;
			else
				cyclesToWake = IPACC_PSTATE_IDLE_EXIT_TIME;
		}

		return true;
	 }
	 else {
		 m_IPBusyStalls++;
		 return false;
	 }
}

// returns true when something is processed
bool GemDroidIP::process()
{
	// Process already read data
	if (m_cyclesToSkip > 0)
		m_cyclesToSkip--;
		// Completed processing one cache line
	if(m_cyclesToSkip == 0) {
		return false;
	}

	return true;
}

void GemDroidIP::tick()
{
	//Only for devices this will tick will be called. Acc and GPUs will have their own.

	ticks++;

	if(!updatePower())
		return;

	assert (isPStateActive());

	if(process())
		return;

	if (m_respCount == ceil(m_dataSize / CACHE_LINE_SIZE)) {
		is_busy = false;  // Wait for responses to make it not busy
		m_respCount = 0;
        m_reqCount = 0;
		assert(m_reqCount <= 0);
		//assert(m_flowId != -1);
		gemDroid->gemdroid_sa.enqueueIPResponse(ip_type, ip_id, core_id, m_frameNum, m_flowType, m_flowId);
	}

	if(m_reqCount <= 0 && !is_busy) {
		// is_busy = false; // Don't wait for responses to make it not busy. As soon as requests are sent assume not busy
		idleCycles++;
		return;
	}

	if(is_busy) {
		m_IPWorkingCycles++;
	}

	//If there are more requests to be sent with the previous IP call.. (An IP call has 100's of requests to be injected in consecutive cycles
	if(m_reqCount > 0) {
		bool is_success = sendMemoryReq();

		//Streak of Idle cycles in ActivePState : Calculator Code
		if(is_success)
			streakCalculator();
		else {
			idleStreak++;
			m_IPMemStalls++;
		}

		if(m_reqCount == 0) {
			//DBGPRINT<<" all requests completed."<<endl;
			m_reqCount--;
			assert(isPStateActive());
		}
	}
}

double GemDroidIP::getDynamicPower(double activity)
{
	double dynamicPowerConsumedInThisMS = 0.0;

    dynamicPowerConsumedInThisMS = activity * voltage_freq_table[dvfsState][2];

    return dynamicPowerConsumedInThisMS;
}

double GemDroidIP::getStaticPower(double activePortion, double lowPortion, double idlePortion)
{
	double staticPower = activePortion*ip_static_power + lowPortion*ip_static_power/3;

    return staticPower;
}

int GemDroidIP::getMaxProcessible()
{
	if(ip_type == IP_TYPE_VD)
	 	return VD_MAX;
	else if(ip_type == IP_TYPE_VE)
	 	return VE_MAX;
	else if(ip_type == IP_TYPE_AD)
	 	return AD_MAX;
	else if(ip_type == IP_TYPE_AE)
	 	return AE_MAX;
	else if(ip_type == IP_TYPE_IMG)
	 	return IMG_MAX;
	else if(ip_type == IP_TYPE_CAM)
	 	return CAM_MAX;
 	else if(ip_type == IP_TYPE_SND)
	 	return SND_MAX;
  	else if(ip_type == IP_TYPE_DC)
	 	return DC_MAX;
  	else if(ip_type == IP_TYPE_NW)
	 	return NW_MAX;
  	else if(ip_type == IP_TYPE_MIC)
	 	return MIC_MAX;
  	else if(ip_type == IP_TYPE_MMC_IN || ip_type == IP_TYPE_MMC_OUT)
	 	return MMC_MAX;
  	else if(ip_type == IP_TYPE_GPU)
	 	return GPU_MAX;

    return 1;
}

double GemDroidIP::powerIn1us()
{
	long one_microsec = getIPFreq()*1000; //if 2 ghz core, we have 2K ticks in 1 microsecs

	double staticPowerConsumedInThisuS = getStaticPower(m_IPActiveInLast1us/(double) one_microsec, m_IPLowInLast1us/(double) one_microsec, 0);

	double dynamicPowerConsumedInThisuS = getDynamicPower((m_IPActivityIn1us*1000.0)/getMaxProcessible());

    double powerInLastus = staticPowerConsumedInThisuS + dynamicPowerConsumedInThisuS;

	// cout<<"IP "<<ipTypeToString(ip_type)<<" us power: Static: "<<  staticPowerConsumedInThisuS <<" Dynamic: "<< dynamicPowerConsumedInThisuS << " Total = "<< powerInLastus <<endl;

	m_IPActiveInLast1us 	= 0;
	m_IPLowInLast1us		= 0;
	m_IPActivityIn1us   	= 0;

    return powerInLastus;
}

double GemDroidIP::powerIn1ms()
{
	long one_millisec = getIPFreq()*1000000; //if 2 ghz core, we have 2M ticks in 1 mili secs

	double staticPowerConsumedInThisMS = getStaticPower(m_IPActiveInLast1ms/(double) one_millisec, m_IPLowInLast1ms/(double) one_millisec, 0);

	double dynamicPowerConsumedInThisMS = getDynamicPower((double)m_IPActivityIn1ms/getMaxProcessible());

	 /* if(ip_type == IP_TYPE_GPU ) {
        cout<<"IP "<<ipTypeToString(ip_type)<<endl\
			<< "m_IPActivityIn1ms: 	"<<m_IPActivityIn1ms<<endl\
			<< "m_IPActiveInLast1ms	 :	" << m_IPActiveInLast1ms<<endl\
			<< "m_IPLowInLast1ms	 :	" << m_IPLowInLast1ms\
			<< endl;
//			<< "ip_static_power		 :	" << ip_static_power<<endl
//			capacitance = micPowerIn1ms_DYNAMIC_PWR_PER_CL;
	} */
	//SDT=Static - Dynamic - Total
	cout<<ipTypeToString(ip_type)<<" powerSDT:"<<  staticPowerConsumedInThisMS <<" "<< dynamicPowerConsumedInThisMS << " "<< (staticPowerConsumedInThisMS + dynamicPowerConsumedInThisMS) <<endl;

	m_IPActiveInLast1ms 	= 0;
	m_IPLowInLast1ms		= 0;
	m_IPActivityIn1ms   	= 0;

    powerInLastEpoch = staticPowerConsumedInThisMS + dynamicPowerConsumedInThisMS;

	return powerInLastEpoch;
}

void GemDroidIP::IPActivityUpdate()
{
	m_IPActivityIn1ms += 1;
	m_IPActivityIn1us += 1;
	m_IPActivityInDVFSEpoch += 1;
}

int GemDroidIP::getIndexInTable(double freq)
{
    for(int i = 0; i<IP_DVFS_STATES; i++)
        if(freq == voltage_freq_table[i][1])
            return i;

    return -1;
}

bool GemDroidIP::incFreq(int steps)
{
    assert(!isDevice);

    if (dvfsState < IP_DVFS_STATES-steps-1) {
        dvfsState += steps;
        return true;
    }
    else {
        setMaxIPFreq();
    }

    return false;
}

bool GemDroidIP::decFreq(int steps)
{
    assert(!isDevice);

    if (gemDroid->getGovernor() == GOVERNOR_TYPE_INTERACTIVE && dvfsCounter > 0)
        return false;

    if (dvfsState > steps-1) {
        dvfsState -= steps;
        return true;
    }

    return false;
}

void GemDroidIP::setMaxIPFreq()
{
    assert(!isDevice);

    dvfsState = IP_DVFS_STATES - 1;

    dvfsCounter = INTERACTIVE_GOVERNOR_LAT;
}

void GemDroidIP::setMinIPFreq()
{
    assert(!isDevice);

    dvfsState = 0;
}

void GemDroidIP::setOptIPFreq()
{
    assert(!isDevice);

    dvfsState = m_optDVFSState;
}

void GemDroidIP::setIPFreqInd(int ind)
{
    assert(ind > -1 && ind < IP_DVFS_STATES);

    dvfsState = ind;
}

void GemDroidIP::setIPFreq(double freq)
{
    int ind = getIndexInTable(freq);

    assert(ind > -1 && ind < IP_DVFS_STATES);

    dvfsState = ind;
}

void GemDroidIP::updateDVFS()
{
    dvfsCounter--;
}

double GemDroidIP::getLoadInLastDVFSEpoch()
{
	// double ticks_in_epoch = (DVFS_PERIOD / GEMDROID_FREQ) * ipFreq;

	// cout<<"ticks_in_epoch " << ticks_in_epoch<<endl;

	double ip_util=0;// = (double) m_reqCount / getMaxProcessible();

	/* if (ip_type == IP_TYPE_VD)
        ip_util = (double) m_reqCount / VD_MAX;
    else if (ip_type == IP_TYPE_VE)
        ip_util = (double) m_reqCount / VE_MAX;
    else if (ip_type == IP_TYPE_AD)
        ip_util = (double) m_reqCount / AD_MAX;
    else if (ip_type == IP_TYPE_AE)
        ip_util = (double) m_reqCount / AE_MAX;
    else if (ip_type == IP_TYPE_IMG)
        ip_util = (double) m_reqCount / IMG_MAX; */

    if (ip_type == IP_TYPE_VD)
        ip_util = (double) m_IPActivityInDVFSEpoch / (VD_MAX*gemDroid->lastTimeTook[ip_type]);
    else if (ip_type == IP_TYPE_VE)
        ip_util = (double) m_IPActivityInDVFSEpoch / (VE_MAX*gemDroid->lastTimeTook[ip_type]);
    else if (ip_type == IP_TYPE_AD)
        ip_util = (double) m_IPActivityInDVFSEpoch / (AD_MAX*gemDroid->lastTimeTook[ip_type]);
    else if (ip_type == IP_TYPE_AE)
        ip_util = (double) m_IPActivityInDVFSEpoch / (AE_MAX*gemDroid->lastTimeTook[ip_type]);
    else if (ip_type == IP_TYPE_IMG)
        ip_util = (double) m_IPActivityInDVFSEpoch / (IMG_MAX*gemDroid->lastTimeTook[ip_type]);
	
    m_IPActivityInDVFSEpoch = 0;

    if (ip_util > 1)
    	return 0.99;
    else
    	return ip_util;
}

double GemDroidIP::getPowerEst()
{	
	if(isPStateActive())
		return getDynamicPower(0.5) + getStaticPower(0.5, 0.5, 0); //assume fully active

	return 0;
}

double GemDroidIP::getEnergyEst(double time, double power, double newFreq)
{
    // double energy = time * power;
    double dynPower = power - ip_static_power;
    double activity = dynPower / voltage_freq_table[dvfsState][2];

    int ind = getIndexInTable(newFreq);
    double newTime = getTimeEst(time, getIPFreq(), newFreq);
    double newDynPower = activity * voltage_freq_table[ind][2];
    double newStatEnergy = newTime * ip_static_power;
    double newDynEnergy = newTime * newDynPower;
    double newEnergy = newStatEnergy + newDynEnergy;

    //cout << "EnergyData for IP_"<<ipTypeToString(ip_type) << " " << activity << " " << time << " " << power << " " << energy << endl;
    //cout << "NewEnergyEst for IP_"<<ipTypeToString(ip_type) << " " << activity << " " << newTime << " " << newDynPower+ip_static_power << " " << newEnergy << endl;

    return newEnergy;
}

double GemDroidIP::leewayAvailable(double maxEnergyAllowed, double minTimeNeeded, double lastTimeTook, double lastPowerTook, double &slack)
{
	double currTime = gemDroid->memScaledTime(ip_type, gemDroid->dvfs_core, getIPFreq());
	double currEnergy =  lastTimeTook * lastPowerTook;
	
	for(int k=dvfsState; k<IP_DVFS_STATES; k++) {
		double nextEnergy = getEnergyEst(lastTimeTook, lastPowerTook, getIPFreq(k));
		double nextTime = gemDroid->memScaledTime(ip_type, gemDroid->dvfs_core, getIPFreq(k));

		double diffEnergy = nextEnergy - currEnergy;
		double diffTime = currTime - nextTime;

		if(diffEnergy < 0) {
			cout<<"BUGS1"<<ipTypeToString(ip_type)<<" diffEnergy < 0:"<<currEnergy<<" "<<nextEnergy<<" "<<currTime<<" "<<nextTime<<" "<<getIPFreq()<<endl;
		}
		if(diffTime < 0) {
			cout<<"BUGS2"<<ipTypeToString(ip_type)<<" diffEnergy < 0:"<<currEnergy<<" "<<nextEnergy<<" "<<currTime<<" "<<nextTime<<" "<<getIPFreq()<<endl;
		}

		if(diffEnergy < maxEnergyAllowed && diffTime >= minTimeNeeded){
			slack = slack + diffTime;
			return getIPFreq(k);
		}

	}
	return -1;
}

double GemDroidIP::getFreqForSlackOptimal(double prevTime, double &slack)
{
    double currFreq = getIPFreq();
    double newFreq = currFreq;
    int k=dvfsState;
    int core_id = 0;
    int optFreqInd = getOptIPFreqInd();
    double scaledTime = gemDroid->memScaledTime(ip_type, core_id, getIPFreqInd());

    cout << "DVFS. IP " << ipTypeToString(ip_type) << " Slack left: " << slack << " currFreq: " << currFreq << " currTime: " << prevTime;

    if (slack < 0) {
        newFreq = getIPFreq(dvfsState+2);
        cout << ". Increasing frequency by 2 steps to freq " << newFreq << " for time " << getTimeEst(scaledTime, getIPFreq(), newFreq);
        k = dvfsState + 1;
    }
    else if (dvfsState > optFreqInd) {
        double minFreq = optFreqInd;
        if (ip_type == IP_TYPE_IMG)
            minFreq = dvfsState-2;
        // for(k=dvfsState-1; k>=optFreqInd; k--) {
        for(k=dvfsState-1; k>=minFreq; k--) {
            //if (gemDroid->ip_time_table[ip_type][k] > prevTime + slack)
            if (getTimeEst(scaledTime, getIPFreq(), getIPFreq(k)) > prevTime + slack)
                break;
        }

        // if (k == IP_DVFS_STATES-1)
            // k = IP_DVFS_STATES-2;

        newFreq = getIPFreq(k+1);
        assert(newFreq > 0);

        if (k+1 < dvfsState)
            cout << ". Reducing frequency to " << getIPFreq(k+1) << " for time " << getTimeEst(scaledTime, getIPFreq(), getIPFreq(k+1));
        else
            cout << endl;
    }
    else {
        k=dvfsState-1;
    }

    if(ip_type == IP_TYPE_AD || ip_type == IP_TYPE_AE) {
    	// if((gemDroid->ip_time_table[ip_type][k+1] - prevTime) > 0 )
        	slack -= (AD_AE_RATIO_TO_VD * (getTimeEst(scaledTime, getIPFreq(), getIPFreq(k+1)) - prevTime));
    }
    else {
    	// if((gemDroid->ip_time_table[ip_type][k+1] - prevTime) > 0)
        	slack -= (getTimeEst(scaledTime, getIPFreq(), getIPFreq(k+1)) - prevTime);
    }

    cout << ". Slack left = " << slack << endl;

    return newFreq;
}

double GemDroidIP::getTimeEst(double oldTime, double oldFreq, double newFreq)
{	
	return (oldTime * oldFreq / newFreq);

}

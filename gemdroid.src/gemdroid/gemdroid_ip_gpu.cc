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

#include "gemdroid/gemdroid_ip_gpu.hh"
#include "gemdroid/gemdroid.hh"

void GemDroidIPGPU::init(int ip_type, int id, bool isDevice, std::string em_gputrace_file_name, int ip_freq, int opt_freq, GemDroid *gemDroid)
{

	GemDroidIP::init(ip_type, id, isDevice, 0, ip_freq, opt_freq, gemDroid);

	if(em_gputrace_file_name == "none.txt")
		return;

	//GPU Trace file - should be enabled or not?
	em_gputrace_file.open(em_gputrace_file_name, std::iostream::in);
	if (!em_gputrace_file.is_open()) {
		inform("Cannot open GPU trace file");
		assert(0);
	}
	else {
        m_enabled = true;
		inform("Opened GPUtrace file!");
    }

	std::cout << em_gputrace_file_name << std::endl;

	lines_read_gpu = 0;
	lastDCTick = 0;
	fpsStalls = 0;
	power_state = ip_active;
	writeToDCFlag = false;
	m_addrToDC = 0;

	m_thisMilliSecInstructionsCommitted = 0;
	m_thisMicroSecInstructionsCommitted = 0;

    m_framesDisplayed = 0;
    m_framesDropped = 0;
	m_fpsStallsCount = 0;
	m_FPS = 0;
	
	stat_lines_read_gpu = 0;
	stat_m_fpsStallsCount = 0;
	stat_m_framesDisplayed = 0;
	stat_m_framesDropped = 0;

	//m_flowId = gemDroid->getGPUFlowId();
}

void GemDroidIPGPU::regStats()
{
	GemDroidIP::regStats();
	lines_read_gpu.name(desc + ".GPUTRACE.lines_read_gpu").desc("GemDroid: Number of GPU lines read from trace file").flags(Stats::display);
	m_fpsStallsCount.name(desc + ".gpu_fpsStalls").desc("GemDroid: Number of core stalls for FPS").flags(Stats::display);
	m_framesDisplayed.name(desc + ".gpu_framesDisplayed").desc("GemDroid: Number of frames displayed").flags(Stats::display);
	m_framesDropped.name(desc + ".gpu_framesDropped").desc("GemDroid: Number of frames dropped").flags(Stats::display);
	m_FPS.name(desc + ".GPU_FPS").desc("GemDroid: Frames Per Second (FPS)").flags(Stats::display);
}

void GemDroidIPGPU::resetStats()
{
	GemDroidIP::resetStats();
	lines_read_gpu = 0;
    m_framesDisplayed = 0;
    m_framesDropped = 0;
	m_fpsStallsCount = 0;
	m_FPS = 0;
}

void GemDroidIPGPU::printPeriodicStats()
{
	cout<<"-=-=-=-=-=-=-=-=-=-=-=-"<<endl;
	GemDroidIP::printPeriodicStats();
	
	if(0) {
		cout<<desc<<".GPUTRACE.lines_read_gpu: "<<lines_read_gpu.value()<<endl;
		cout<<desc<<".gpu_fpsStalls: "<<m_fpsStallsCount.value()<<endl;
		cout<<desc<<".gpu_framesDisplayed: "<<m_framesDisplayed.value()<<endl;
		cout<<desc<<".gpu_framesDropped: "<<m_framesDropped.value()<<endl;
		double seconds = (1 / voltage_freq_table[dvfsState][1]) * ticks.value() /(1000 * 1000 * 1000);
			// cout << "Seconds = " << seconds << endl;
		m_FPS = m_framesDisplayed.value() / seconds;
		cout<<desc<<".GPU_FPS: "<<m_FPS.value()<<endl;
	}
	else {
		cout<<desc<<".GPUTRACE.lines_read_gpu: "<<lines_read_gpu.value() - stat_lines_read_gpu<<endl;
		cout<<desc<<".gpu_fpsStalls: "<<m_fpsStallsCount.value() - stat_m_fpsStallsCount<<endl;
		cout<<desc<<".gpu_framesDisplayed: "<<m_framesDisplayed.value() - stat_m_framesDisplayed<<endl;
		cout<<desc<<".gpu_framesDropped: "<<m_framesDropped.value() - stat_m_framesDropped<<endl;
		double seconds = (1 / voltage_freq_table[dvfsState][1]) * ticks.value() /(1000 * 1000 * 1000);
			// cout << "Seconds = " << seconds << endl;
		m_FPS = m_framesDisplayed.value() / seconds;
		cout<<desc<<".GPU_FPS(global): "<<m_FPS.value()<<endl;
	}

	stat_lines_read_gpu = lines_read_gpu.value();
	stat_m_fpsStallsCount = m_fpsStallsCount.value();
	stat_m_framesDisplayed = m_framesDisplayed.value();
	stat_m_framesDropped= m_framesDropped.value();

}

void GemDroidIPGPU::readLine()
{
	string op;
	string dummy;
	long insns;
	uint64_t addr;

	em_gputrace_file>>op;
	lines_read_gpu++;

	if (op == "GPU") { // CPU line
		em_gputrace_file>>insns;
		cyclesToSkip = insns;  //Assuming we can commit 2 instructions per cycle (alike PowerVR architecture)

		assert (insns < 10000000000);
	}
	else if (op == "GMU_st") {
		em_gputrace_file>>addr;
		addr += GPU_ADDR_START;
		// processMMURequest(addr, false);
	}
	else if (op == "GMU_ld") {
		em_gputrace_file>>addr;
		addr += GPU_ADDR_START;
		processMMURequest(addr, true);
	}
	else if (op == "Rendered") {
		// Frame rendered
		em_gputrace_file>>dummy>>dummy;
		int ticksFromLastDC = gemDroid->getTicks() - lastDCTick;

		gemDroid->markIPRequestCompleted(0, ip_type, ip_id, m_frameNum, 20);
		writeToDCFlag = true;
		m_addrToDC = DC1_ADDR_START;
		m_frameNum++;

		if (ticksFromLastDC < GPU_FPS_TICKS) {
			fpsStalls = GPU_FPS_TICKS - ticksFromLastDC;
		}
		else {
			lastDCTick = gemDroid->getTicks();
			m_framesDropped++;
            cout << "Dropper! GPU didn't complete work in Deadline" << endl;
			//if(!gemDroid->gemdroid_sa.enqueueIPIPRequest(ip_type, ip_id, 0, IP_TYPE_DC, DC1_ADDR_START, FRAME_SIZE, true, 0, IP_TYPE_DC))
	    		//cout << "Dropper! GPU cannot enqueue DC IP request into SA" << endl;

	    }
	}
	else if (op == "END") {
		cout << endl <<"*** Read END from GPU trace file. *** " << endl << endl;
		true_fetch = false;
	}
	else {
		cout << "FATAL: Read unknown line from GPU trace file" << op << endl;
		assert(0);
	}
}
void GemDroidIPGPU::tick()
{
	//cout << "GPU tick()" << ticks << endl;
	assert(isEnabled());

	ticks++;

	if(m_reqCount > 0) {
		bool is_success = sendMemoryReq();
		//Streak of Idle cycles in ActivePState : Calculator Code
		if(is_success) {
			streakCalculator();
		}
		else {
			idleStreak++;
			return;  //Not really needed?
		}
	}

	if(fpsStalls > 0) {
		m_fpsStallsCount++;
		fpsStalls -= GEMDROID_FREQ / voltage_freq_table[dvfsState][1];
		idleCycles++;

		// Last stall for FPS. Display frame.
		if (fpsStalls <= 0) {
			lastDCTick = gemDroid->getTicks();

			if (isPStateLowPower()) {
				cyclesToWake = IPACC_PSTATE_LOWPOWER_EXIT_TIME;
			}
			else if (isPStateIdle()) {
				cyclesToWake = IPACC_PSTATE_IDLE_EXIT_TIME;
			}
            else {
                gemDroid->markIPRequestStarted(0, ip_type, ip_id, 0);
            }
		}
		//Hack to update power when FPS stalls is happening.
		if(!updatePower())
	    	return;

		return;
	}

	if (writeToDCFlag) {
		processMMURequest(m_addrToDC, false);
		m_addrToDC += CACHE_LINE_SIZE;
		if(m_addrToDC >= DC1_ADDR_START + FRAME_SIZE) {
			writeToDCFlag = false;
			if(!gemDroid->gemdroid_sa.enqueueIPResponse(ip_type, ip_id, 0, m_framesDisplayed.value(), IP_TYPE_DC, 0))
				cout << "Dropper! GPU cannot enqueue DC IP request into SA" << endl;
			m_framesDisplayed++;
		}
		return;
	}

	if(!updatePower())
	    return;

    assert(isPStateActive());

	if (cyclesToSkip > 0) {
		cyclesToSkip--;		//committing 1 instruction here.
		m_thisMilliSecInstructionsCommitted++;
		m_thisMicroSecInstructionsCommitted++;
        m_IPActivityIn1ms++;
        m_IPActivityIn1us++;
		return;
	}

	readLine();
}


bool GemDroidIPGPU::updatePower()
{
	if(isPStateActive()) {
		m_IPActiveCycles++; 
		m_IPActiveInLast1ms++;
		m_IPActiveInLast1us++;
	}
	else if (isPStateLowPower()) {
		m_IPLowPowerCycles++;
		m_IPLowInLast1ms++;
		m_IPLowInLast1us++;
		idleCycles++;
	}
	else {
		m_IPIdleCycles++;
	}

	if (cyclesToWake > 0) {
		cyclesToWake--;
		if (cyclesToWake == 0) {
			setPStateActive();
			idleCycles = 0;
		}
		return false;
	}

	if (idleCycles >= IPACC_PSTATE_ENTER_TIME) {
		if (isPStateActive()) {
			setPStateLowPower();
		}
		else if (isPStateLowPower()) {
			setPStateIdle();
		}
		idleCycles = 0;
	}
	//GPU is not waking up; or, it is not going to lower power state.
	//So, if it is in active state now, do work. If in idle or low-power, cannot do work.
	if (isPStateActive())
		return true;
	else
		return false;
}

void GemDroidIPGPU::processMMURequest(uint64_t addr, bool isRead)
{
	m_reqCount = 1;
	m_reqAddr = addr;
	m_isRead = isRead;

	sendMemoryReq();
}

double GemDroidIPGPU::getStaticPower(double activePortion, double lowPortion, double idlePortion)
{
	double staticPower = ip_static_power * activePortion + ip_static_power/3 * lowPortion;

    return staticPower;
}

double GemDroidIPGPU::getDynamicPower(double activity)
{
	//DynPower = CV^2F
    if (activity > 1)
        activity = 1;
    double dynamicPowerConsumedInThisMS = activity * voltage_freq_table[dvfsState][2];

    return dynamicPowerConsumedInThisMS;
}

double GemDroidIPGPU::powerIn1ms()
{
	// double general_ip_power = GemDroidIP::powerIn1ms();
	// cout<<"GPU 1ms power = "<<general_ip_power<<endl;

	double one_millisec = getIPFreq()*1000000;

	double staticPowerConsumedInThisMS = getStaticPower(m_IPActiveInLast1ms/(double) one_millisec, m_IPLowInLast1ms/(double) one_millisec, 0);

	double dynamicPowerConsumedInThisMS = getDynamicPower((double) m_thisMilliSecInstructionsCommitted / (one_millisec));

	cout<<"IP "<<ipTypeToString(ip_type)<<" ms power: Static: "<<  staticPowerConsumedInThisMS <<" Dynamic: "<< dynamicPowerConsumedInThisMS << " Total = "<< (staticPowerConsumedInThisMS + dynamicPowerConsumedInThisMS) <<endl;

	// cout<<"GPU static power = "<<gpu_static_pwr<<endl;
    // cout<<"GPU dynamic power = "<<gpu_dynamic_pwr<<endl;

	m_thisMilliSecInstructionsCommitted = 0;
	m_IPActiveInLast1ms = 0;
	m_IPLowInLast1ms = 0;

    return (staticPowerConsumedInThisMS + dynamicPowerConsumedInThisMS);
}

double GemDroidIPGPU::powerIn1us()
{
	double one_microsec = getIPFreq()*1000;

	double staticPowerConsumedInThisuS = getStaticPower(m_IPActiveInLast1us/(double) one_microsec, m_IPLowInLast1us/(double) one_microsec, 0);

	double dynamicPowerConsumedInThisuS = getDynamicPower((double) m_thisMicroSecInstructionsCommitted / (one_microsec));

	m_thisMicroSecInstructionsCommitted = 0;
	m_IPActiveInLast1us = 0;
	m_IPLowInLast1us = 0;

    return (staticPowerConsumedInThisuS + dynamicPowerConsumedInThisuS);
}

double GemDroidIPGPU::getPowerEst()
{
    if(isEnabled() && isPStateActive())
		return getDynamicPower(1) + getStaticPower(1, 0, 0); //assume fully active

    return 0;
}

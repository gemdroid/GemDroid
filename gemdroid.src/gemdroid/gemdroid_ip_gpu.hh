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

#ifndef GEMDROID_IP_GPU_HH_
#define GEMDROID_IP_GPU_HH_

#include "gemdroid/gemdroid_ip.hh"
#include <fstream>

// #define GPU_FREQ (GEMDROID_FREQ / CORE_TO_ACC_FREQ)
// #define GPU_FREQ (GEMDROID_FREQ / gemDroid->getIPFreq())
#define GPU_FPS_TICKS ((GEMDROID_FREQ * 1000 * 1000 * 1000) / TARGET_FPS)

class GemDroidIPGPU : public GemDroidIP
{
private:

    ifstream em_gputrace_file;
    int cyclesToSkip;
    long lastDCTick;
    int fpsStalls;
    bool m_enabled;

    bool writeToDCFlag;
    uint64_t m_addrToDC;
    int m_flowId;

	long m_thisMilliSecInstructionsCommitted;
	long m_thisMicroSecInstructionsCommitted;

	void readLine();
	void processMMURequest(uint64_t addr, bool isRead);
	bool updatePower();

public:
	 void tick();
	 void init (int ip_type, int id, bool isDevice, std::string em_gputrace_file_name, int ip_freq, int opt_freq, GemDroid *gemDroid);
	 void regStats();
	 void resetStats();
	 inline bool isEnabled() { return m_enabled; }
	 void printPeriodicStats();
    double getPowerEst();
    double powerIn1ms();
    double powerIn1us();
    double getDynamicPower(double coreActivity);
    double getStaticPower(double activePortion, double lowPortion, double idlePortion);

	 Stats::Scalar lines_read_gpu;
	 Stats::Scalar m_fpsStallsCount;
	 Stats::Scalar m_framesDisplayed;
	 Stats::Scalar m_framesDropped;
	 Stats::Scalar m_FPS;

	 long stat_lines_read_gpu;
	 long stat_m_fpsStallsCount;
 	 long stat_m_framesDisplayed;
 	 long stat_m_framesDropped;
};

#endif /* GEMDROID_IP_GPU_HH_ */

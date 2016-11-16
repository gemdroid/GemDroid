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

#ifndef __GEMDROID_CORE_HH__
#define __GEMDROID_CORE_HH__

#include "base/statistics.hh"
#include "gemdroid_core_util.hh"

#include <fstream>
#include <cmath>

#define MAX_TRANSACTIONS 100
#define ROB_SIZE 32
#define ROB_DEBUG 0
#define TARGET_AUD_FPS 60
#define FPS_TICKS ((GEMDROID_FREQ * 1000 * 1000 * 1000) / TARGET_FPS)
#define AUD_FPS_TICKS ((GEMDROID_FREQ * 1000 * 1000 * 1000) / TARGET_AUD_FPS)

#define DEADLOCK_PERIOD 100000

// enter time 450 microsecs
// #define CORE_PSTATE_ENTER_TIME 720000
// exit time 1 millisecs
// #define CORE_PSTATE_LOWPOWER_EXIT_TIME 360000
// exit time 3 millisecs: from idle to active
// #define CORE_PSTATE_IDLE_EXIT_TIME 720000

// lowpower enter time 163 microsecs
#define CORE_PSTATE_LOWPOWER_ENTER_TIME 200000 // at lowest freq: 300 MHz
// idle enter time 3600 microsecs from active to idle
#define CORE_PSTATE_IDLE_ENTER_TIME 1128900
// exit time 182 microsecs
#define CORE_PSTATE_LOWPOWER_EXIT_TIME 54600
// exit time 8782 microsecs: from idle to active
#define CORE_PSTATE_IDLE_EXIT_TIME 2634600
#define ENABLE_CORE_IDLE 0

#define CORE_CAPACITANCE (1.0) //   1.0 => for 3 watts
#define CORE_STATIC_PWR (0.35)

#define IN_ORDER true

using namespace std;

enum CPU_PSTATE {
	CPU_PSTATE_ACTIVE,
	CPU_PSTATE_LOWPOWER,
	CPU_PSTATE_IDLE
};

enum APP_TYPE {
	CORE_BOUND,
	DISPLAY_BOUND,
	VIDEO_PLAYBACK,
	AUDIO_PLAYBACK
};


class GemDroidCore
{
private:
	string desc;
	int core_id;
	int app_id;
	int issue_width;
	long ticks;
	int idleCycles;
	int cyclesToWake;
	long idleStreak;
	int cpu_pstate;
	bool needToLookAhead;
	
    double qemu_to_60FPS_speedratio;  // ratio of idle time for CPU

	ifstream em_trace_file;
    ifstream lookahead_trace_file;
	GemDroid *gemDroid;
	int type_of_application;

	long idleStalls;
	long fpsStalls;
	long lastDCTick;
	int audFpsStalls;
	long lastSNDTick;
    bool m_readFBLine;
    double optimal_freq;

	long m_thisMilliSecActivePStateCycles;
	long m_thisMilliSecLowpowerPStateCycles;
	long m_thisMilliSecIdlePStateCycles;
	long m_thisMilliSecInstructionsCommitted;
	long m_thisMilliSecRobFullStalls;
	long m_thisMilliSecIdleStalls;
	long m_thisDVFSEpochInstructionsCommitted;

	long m_thisMicroSecActivePStateCycles;
	long m_thisMicroSecLowpowerPStateCycles;
	long m_thisMicroSecIdlePStateCycles;
	long m_thisMicroSecInstructionsCommitted;
	long m_thisMicroSecRobFullStalls;
	
	//OoO capability
	std::vector<GemDroidOoOTransaction> outstanding_transactions;
	int outstanding_transactions_size;
	int number_of_instr_executed_OoO;
	int deadlocks_faced;

    double voltage_freq_table[CORE_DVFS_STATES][4]; // voltage, freq, static, dynamic

    // double coreFreq;
    // double coreVoltage;
    int dvfsCounter;
    int dvfsState;
    int optDVFSState;
    double powerInLastEpoch;
    int pstateTimer;
    bool flagProfile;    
    long m_profileRobFullStalls;
    long m_profileMemFullStalls;
    long m_profileActiveCycles;
    long m_profileStartCPUCycle;
    long profileStartGemDroidCycle;


	void readLine();
    void setIdleRatio();
	void processIPCall(string op, uint64_t addr, int size);
	void processMMURequest(string op, uint64_t addr);
	/*
	* Commit the head (blocking) transaction. Remove it from the ROB.
	*/
    void addInstructions(long insns);
	void commitHeadTransaction();
	void updateStatInstructionCommit();
	void checkDeadlock();
	void process();
	void inOrderProcess();
	bool isFrameDrop();
	bool isAudioFrameDrop();

	void setPStateActive();
	void setPStateLowPower() { assert(isPStateActive()); pstateTimer=CORE_PSTATE_LOWPOWER_ENTER_TIME; cpu_pstate=CPU_PSTATE_LOWPOWER; }
	void setPStateIdle() { /* assert(isPStateLowPower()); */ pstateTimer=CORE_PSTATE_IDLE_ENTER_TIME; cpu_pstate=CPU_PSTATE_IDLE; }

	void streakCalculator();
    int getIndexInTable(double freq);
    double getStaticPower(double activePortion, double lowPortion, double idlePortion);
    double getDynamicPower(double coreActivity, double coreStallActivity);
    void printDVFSTable();

public:
	GemDroidCore();
	void init(int id, std::string trace_file, int app_id, int core_freq, int opt_freq, int issue_width, GemDroid *gemDroid);
	void tick();
	void regStats();
	void resetStats();
	void printPeriodicStats();
	
	double getLoadInLastDVFSEpoch(); //Return power consumed in the last 1 ms.
	double powerIn1ms(); //Return power consumed in the last 1 ms.
    double powerIn1us();
	double calcPowerConsumed();
    void setCoreFreqInd(int ind);
	void setCoreFreq(double freq); //Set core frequency
	void setMaxAllowedFreq(); //Set max core frequency based on remaining power budget
    inline double getCoreFreq() { return voltage_freq_table[dvfsState][1]; }
    inline double getCoreFreq(int ind) { if (ind >= CORE_DVFS_STATES) ind = CORE_DVFS_STATES-1; else if (ind < 0) ind = 0; return voltage_freq_table[ind][1]; }
    inline int getCoreFreqInd() { return dvfsState; }
    bool incFreq(int steps=1);
    bool decFreq(int steps=1);
    int getMaxCoreFreqInd() { return CORE_DVFS_STATES-1; }
    int getMinCoreFreqInd() { return 0; }
    int getOptCoreFreqInd() { return optDVFSState; }
    double getMaxCoreFreq() { return voltage_freq_table[CORE_DVFS_STATES-1][1]; }
    double getMinCoreFreq() { return voltage_freq_table[0][1]; }
    double getOptCoreFreq() { return voltage_freq_table[optDVFSState][1]; }
    void setMaxCoreFreq();
    void setMinCoreFreq();
    void setOptCoreFreq();
    void updateDVFS();
    double getTimeEst(double time, double oldFreq, double newFreq);
    double getEnergyEst(double time, double power, double newFreq);
    double getPowerEst();
    double getFreqForSlackOptimal(double prevTime, double &slack);
    double getFreqForPower(double power);
    double getProfileFractionOfMemInst(){return ((m_profileMemFullStalls + m_profileRobFullStalls)/(double)m_profileActiveCycles);}

	inline bool isPStateActive() { return (cpu_pstate == CPU_PSTATE_ACTIVE); }
	inline bool isPStateLowPower() { return (cpu_pstate == CPU_PSTATE_LOWPOWER); }
	inline bool isPStateIdle() { return (cpu_pstate == CPU_PSTATE_IDLE); }

	//OoO capability
	/**
	* Search for a particular address in the ROB and return the entry id.
	*/
	int searchInROB(uint64_t addr);
	/*
	* Mark an OoO LD/ST transaction in ROB as completed (response from memory is back)
	*/
	int markTransactionCompleted(uint64_t addr);

/*	Stats::Scalar m_robFullStalls;
	Stats::Scalar m_memFullStalls;*/

	Stats::Scalar m_thisFrameRobFullStalls;
	Stats::Scalar m_thisFrameMemFullStalls;
	Stats::Scalar m_thisFrameIPFullStalls;

    Stats::Scalar m_idleStallsCount;
	Stats::Scalar m_fpsStallsCount;
	Stats::Scalar m_framesDisplayed;
	Stats::Scalar m_framesDropped;
	Stats::Scalar m_framesDroppedDueToRobStalls;
	Stats::Scalar m_framesDroppedDueToMemStalls;
	Stats::Scalar m_framesDroppedDueToIPStalls;

	Stats::Scalar m_FPS;

	Stats::Scalar m_idleCycles;
	Stats::Scalar m_committedInsns;
	Stats::Scalar m_cycles;
	Stats::Scalar m_memReqs;
	Stats::Scalar m_ipReqs;
	Stats::Scalar m_ipCallsPresentInTrace[IP_TYPE_END];
	Stats::Scalar m_ipResps;
	Stats::Scalar lines_read_cpu;

	//Stats::Formula m_coreIPC;

	Stats::Scalar m_activePStateCycles;
	Stats::Scalar m_lowpowerPStateCycles;
	Stats::Scalar m_idlePStateCycles;

/*	Stats::Scalar totalPowerConsumed;
	Stats::Scalar powerConsumed; //in the last 1 milli-seconds
*/
	long stat_m_cycles;
	long stat_m_idleCycles;
	long stat_m_committedInsns;
	long stat_m_memReqs;
	long stat_m_ipReqs;
	long stat_lines_read_cpu;
	long stat_m_framesDisplayed;
	long stat_m_framesDropped;
	long stat_m_FPS;
	long stat_m_robFullStalls;
	long stat_m_memFullStalls;
	long stat_m_IPFullStalls;
	long stat_m_fpsStallsCount;	
	long stat_m_activePStateCycles;
	long stat_m_lowpowerPStateCycles;
	long stat_m_idlePStateCycles;
	long m_frameNumber[IP_TYPE_END];
	long startCycle;

};

#endif //__GEMDROID_CORE_HH__

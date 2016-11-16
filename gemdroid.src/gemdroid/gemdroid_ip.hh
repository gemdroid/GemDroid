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

#ifndef __GEMDROID_IP_HH__
#define __GEMDROID_IP_HH__

#include "base/statistics.hh"

#include "gemdroid/gemdroid_defines.hh"

#define FRACTION_OF_IFRAMES 10
#define PROCESSING_CHUNK 16

// enter time 450 microsecs
#define IPACC_PSTATE_ENTER_TIME (48900*2)
// exit time 1 millisecs
#define IPACC_PSTATE_LOWPOWER_EXIT_TIME (54600*2)
// exit time 2 millisecs : from idle to active
#define IPACC_PSTATE_IDLE_EXIT_TIME (54600*2)

// enter time 150 microsecs
#define IPDEV_PSTATE_ENTER_TIME (48900*2)
// exit time 0.33 millisecs
#define IPDEV_PSTATE_LOWPOWER_EXIT_TIME (54600*2)
// exit time 1 millisecs: from idle to active
#define IPDEV_PSTATE_IDLE_EXIT_TIME (54600*2)

#define DC0_ADDR_START 2160000000
#define CAM_ADDR_START 2180000000
#define VD_ADDR_START 2200000000
#define VE_ADDR_START 2220000000
#define IMG_ADDR_START 2240000000
#define AE_ADDR_START 2260000000
#define NW_ADDR_START 2280000000
#define SND_ADDR_START 2300000000
#define AD_ADDR_START 2320000000
#define MMC_IN_ADDR_START 2340000000
#define MIC_ADDR_START 2360000000
#define MMC_OUT_ADDR_START 2380000000
#define DC1_ADDR_START 2400000000
#define GPU_ADDR_START 2420000000

#define DC_PROCESSING_TIME 1   //per cache line @ 400Mhz
#define NW_PROCESSING_TIME 1   //per cache line @ 400Mhz
#define SND_PROCESSING_TIME 1  //per cache line @ 400Mhz
#define MIC_PROCESSING_TIME 1  //per cache line   (2 bytes in 5500th of a second. 64 bytes in 32/5500sec) @ 400Mhz
#define CAM_PROCESSING_TIME 1  //per cache line 	@ 400Mhz
#define MMC_PROCESSING_TIME 10  //per cache line  @ 400Mhz

#define VD_PROCESSING_TIME (12)  //per cache line 	@ 800Mhz
#define VE_PROCESSING_TIME (18)  //per coding_ratio cache lines	@ 800Mhz
#define IMG_PROCESSING_TIME (16)  //per cache line	@ 800Mhz
#define AD_PROCESSING_TIME (12)  //per cache line	@ 800Mhz
#define AE_PROCESSING_TIME (8)  //per coding_ratio cache lines	@ 800Mhz

#define IPCLOCKS_IN_1MS (getIPFreq()*1000000)   //number of IP Clocks in 1msec when running a@ 800Mhz
#define IPCLOCKS_IN_10MS (10*IPCLOCKS_IN_1MS)

#define VD_MAX  (IPCLOCKS_IN_1MS / VD_PROCESSING_TIME 	)
#define VE_MAX  (IPCLOCKS_IN_1MS*VID_CODING_RATIO / VE_PROCESSING_TIME 	)	 //49800.0
//As AD,AE finish before a milli-second.. we need to compare wrt 44KHz of data per sec. Else, utilization is ~0.001to 0.009. For 1 mSec = 44 data chunks to be processed.
#define AD_MAX  (IPCLOCKS_IN_1MS / (44*AD_PROCESSING_TIME) 	)	 //(1600.0 / AUD_CODING_RATIO)
#define AE_MAX  (IPCLOCKS_IN_1MS*AUD_CODING_RATIO / (44*AE_PROCESSING_TIME) 	)	 //1600.0
#define IMG_MAX (IPCLOCKS_IN_1MS / IMG_PROCESSING_TIME 	)	 //13000.0
#define DC_MAX  (IPCLOCKS_IN_1MS / (DC_PROCESSING_TIME*5))	 //134589.0
#define CAM_MAX (IPCLOCKS_IN_1MS / CAM_PROCESSING_TIME 	)	 //70000.0
#define SND_MAX (IPCLOCKS_IN_1MS / SND_PROCESSING_TIME 	)	 //1600.0
#define NW_MAX  (IPCLOCKS_IN_1MS / NW_PROCESSING_TIME   )	 
#define MIC_MAX (IPCLOCKS_IN_1MS / MIC_PROCESSING_TIME 	)	 
#define MMC_MAX (IPCLOCKS_IN_1MS / MMC_PROCESSING_TIME 	)	 
#define GPU_MAX (IPCLOCKS_IN_1MS / 4.0)

#define DC_STATIC_PWR   0.1
#define SND_STATIC_PWR  0.05
#define MIC_STATIC_PWR  0.05
#define NW_STATIC_PWR   0.05
#define CAM_STATIC_PWR  0.1
#define MMC_STATIC_PWR  0.05
#define VD_STATIC_PWR 	0.1
#define VE_STATIC_PWR  	0.10
#define AD_STATIC_PWR 	0.05
#define AE_STATIC_PWR   0.05
#define IMG_STATIC_PWR  0.10
#define GPU_STATIC_PWR  0.35

#define V2F (1.057*1.057*0.8)  // 800Mhz maximum

#define VD_DYNAMIC_PWR_PER_CL 	(1/V2F)
#define VE_DYNAMIC_PWR_PER_CL 	(1.5/V2F)
#define AD_DYNAMIC_PWR_PER_CL 	(0.75/V2F)
#define AE_DYNAMIC_PWR_PER_CL 	(0.75/V2F)
#define IMG_DYNAMIC_PWR_PER_CL	(1.5/V2F)
#define DC_DYNAMIC_PWR_PER_CL 	(0.5/V2F)
#define CAM_DYNAMIC_PWR_PER_CL	(1/V2F)
#define SND_DYNAMIC_PWR_PER_CL	(0.0/V2F)
#define NW_DYNAMIC_PWR_PER_CL 	(0.2/V2F)
#define MIC_DYNAMIC_PWR_PER_CL	(0.0/V2F)
#define MMC_DYNAMIC_PWR_PER_CL	(0.3/V2F)
#define GPU_DYNAMIC_PWR_PER_CL	(4.0/V2F)

using namespace std;

class GemDroid;

enum ip_state
{
	ip_idle,
	ip_lowpower,
	ip_active
};

enum FrameType
{
	I_Frame,
	P_Frame,
	B_Frame
};

class GemDroidIP
{
protected:
	GemDroid *gemDroid;
	int ip_type;
	int ip_id;
	string desc;
	bool isDevice;
	int core_id; // Which core called this IP!

    int m_flowType;
    int m_flowId;
	int m_reqCount;
	int m_respCount;
	int m_dataSize;
	bool is_busy;
	uint64_t m_reqAddr;
	bool m_isRead;
	int m_frameNum;
	int m_cyclesToSkip;
	int ioLatency;
	long m_IPMemStallsFrame;

	//IP ACC stuff
	long m_reqOutAddr;
	int reqSent;
	int inputProcessedInCL;
	int	inputToBeProcessedInCL;
	int targetOutputChunkSizeInCL;
	// END

	int idleCycles;
	int cyclesToWake;
	long startCycle;

	int buffer_size;
	int power_state;
	/*bool is_change_state_up;
	bool is_change_state_down;
	int state_cycles_rem;*/

	double ip_static_power;
	int m_IPActiveInLast1ms; //in the last 1 milli-seconds
	int m_IPLowInLast1ms; //in the last 1 milli-seconds
	int  m_IPActivityIn1ms;
	int m_IPActiveInLast1us; //in the last 1 micro-seconds
	int m_IPLowInLast1us; //in the last 1 micro-seconds
	int  m_IPActivityIn1us;

    double voltage_freq_table[IP_DVFS_STATES][3];
    double optimal_freq;

    // double ipFreq;
    // double ipVoltage;
    int dvfsState;
    int dvfsCounter;
    int m_optDVFSState;
    double capacitance;
    double powerInLastEpoch;

	bool process();
	bool sendMemoryReq();
	bool updatePower();
	void IPActivityUpdate();

	inline void setPStateIdle() { assert(isPStateLowPower()); power_state = ip_idle; }
	inline void setPStateLowPower() { assert(isPStateActive()); power_state = ip_lowpower; }
	void setPStateActive();

	void streakCalculator();

    void printDVFSTable();
	int nextIPtoCall();
    int getIndexInTable(double freq);
    double getDynamicPower(double activity);
    double getStaticPower(double activePortio, double lowPortion, double idlePortion);
    int getMaxProcessible();
	long int m_IPActivityInDVFSEpoch;

private:

	bool isBusy() { return is_busy; }


public:
	 GemDroidIP();
	 void init(int ip_type, int id, bool isDevice, int ioLatency, int ip_freq, int opt_freq, GemDroid *gemDroid);
	 void regStats();
	 void resetStats();
	 void tick();
	 void printPeriodicStats();
	 double powerIn1ms(); //Return power consumed in the last 1 ms.
     double powerIn1us();

	 bool enqueueIPReq(int core_id, uint64_t addr, int size, bool isRead, int frameNum, int flowType, int flowId);
	 void memResponse(uint64_t addr, bool isRead);

	 void wakeUp();  //Wake up by another IP.

    void setMaxAllowedFreq();
	void setIPFreq(double freq); //Set IP frequency
    void setIPFreqInd(int ind);
    inline double getIPFreq(int ind) { if (ind >= IP_DVFS_STATES) ind = IP_DVFS_STATES -1; else if (ind < 0) ind = 0; return voltage_freq_table[ind][1]; }
    inline double getIPFreq() { return voltage_freq_table[dvfsState][1]; }
    inline int getIPFreqInd() { return dvfsState; }
    bool incFreq(int steps=1);
    bool decFreq(int steps=1);
    inline int getMaxIPFreqInd() { return IP_DVFS_STATES-1; }
    inline int getMinIPFreqInd() { return 0; }
    inline int getOptIPFreqInd() { return m_optDVFSState; }
    inline double getMaxIPFreq() { return voltage_freq_table[IP_DVFS_STATES-1][1]; }
    inline double getMinIPFreq() { return voltage_freq_table[0][1]; }
    inline double getOptIPFreq() { return voltage_freq_table[m_optDVFSState][1]; }
    void setMaxIPFreq();
    void setMinIPFreq();
    void setOptIPFreq();
    double getLoadInLastDVFSEpoch();
    void updateDVFS();
    double getPowerEst();
    double getFreqForPower(double power);
    double getFreqForSlackOptimal(double prevTime, double &slack);
    double getTimeEst(double oldTime, double oldFreq, double newFreq);
	double leewayAvailable(double maxEnergyAllowed, double minTimeNeeded, double lastTimeTook, double lastPowerTook, double &slack);
	double getEnergyEst(double time, double power, double newFreq);

	inline bool isPStateIdle() { return (power_state == ip_idle); }
	inline bool isPStateLowPower() { return (power_state == ip_lowpower); }
	inline bool isPStateActive() { return (power_state == ip_active); }

	Stats::Scalar ticks;
	Stats::Scalar m_CPUReqs;
	Stats::Scalar m_MemReqs;
	Stats::Scalar m_IPActiveCycles;
	Stats::Scalar m_IPIdleCycles;
	Stats::Scalar m_IPBusyStalls;
	Stats::Scalar m_IPLowPowerCycles;
	Stats::Scalar m_memRejected;
	Stats::Scalar m_IPMemStalls;  // in active state only.
	Stats::Scalar m_IPWorkingCycles;

	// Stats::Distribution idleStreaksinActivePState;
	long idleStreak;

	long stats_ticks;
	long stats_m_CPUReqs;
	long stats_m_MemReqs;
	long stats_m_IPBusyStalls;
	long stats_m_IPActiveCycles;
	long stats_m_IPLowPowerCycles;
	long stats_m_IPIdleCycles;
	long stats_m_IPWorkingCycles;
	long stats_m_memRejected;
	long stats_m_IPMemStalls;
};

#endif //__GEMDROID_IP_HH__

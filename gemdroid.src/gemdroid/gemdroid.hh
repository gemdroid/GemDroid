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

#ifndef __GEMDROID_HH__
#define __GEMDROID_HH__

#include "mem/abstract_mem.hh"
#include "sim/clocked_object.hh"
#include "base/statistics.hh"
#include "params/GemDroid.hh"

#include "gemdroid/gemdroid_defines.hh"
#include "gemdroid/gemdroid_core.hh"
#include "gemdroid/gemdroid_sa.hh"
#include "gemdroid/gemdroid_mem.hh"
#include "gemdroid/gemdroid_ip.hh"
#include "gemdroid/gemdroid_ip_gpu.hh"
#include "gemdroid/gemdroid_ip_encoder.hh"
#include "gemdroid/gemdroid_ip_decoder.hh"
#include "gemdroid/gemdroid_ip_nocoder.hh"
#include "gemdroid/gemdroid_ip_dma.hh"

#define PERIODIC_STATS (1000000 * (int) GEMDROID_FREQ) // 1ms
#define DVFS_PERIOD (1000000 * (int) GEMDROID_FREQ) // 1ms
#define POWER_CALC_PERIOD (1000000 * (int) GEMDROID_FREQ) // 1ms

#define MAX_FLOWS_IN_APP 5
#define MAX_IPS_IN_FLOW 5

#define DVFS_POWERCAP 7 // in Watts
#define DVFS_PRIORITIZE_CORE 1
#define MOTIVATION_GRAPHS 0

using namespace std;

extern bool true_fetch;
extern bool gemdroid_enable;

string flowIdToString(int flowId);
string pStateToString(int pstate);
string ipTypeToString(int type);
int findMax(double array[], int n); //Size of array = n
int findMin(double array[], int n);
bool isAllZeroes(double array[], int n);
void printArray(int array[], int n);

class GemDroid : public ClockedObject
{
private:
	 /**
	  * Progress the GemDroid cores one clock cycle.
	  */
	 void tick();

    /**
	  * Event to schedule clock ticks
	  */
	 EventWrapper<GemDroid, &GemDroid::tick> tickEvent;

	 string desc;

     int num_cpus;
     int core_freq; // in MHz
     int ip_freq; // in MHz
     int mem_freq; //in Ghz
     int num_ip_inst;
     long ticks;
     bool isPrintPeriodicStats;
     bool isPrintPeriodicStatsPower;
     double sweep_val1;
     double sweep_val2;
     bool perfectMemory;

     long powerCalcLastTick;
     long periodicStatsLastTick;
     long slackLastTick;
     long dvfsLastTick;
     long memLastTick;
     long cpuLastTick[MAX_CPUS];
     long ipLastTick[IP_TYPE_END][MAX_IPS];

	 //Variables for traces
     string em_gputrace_file_name;
	 string em_trace_file_name[MAX_CPUS];
     int app_id[MAX_CPUS];
     //3D table;
     //AppID (#defined in gemdroid_defines.h), flows, IPs in the flow.
     int flowTable[APP_ID_END][MAX_FLOWS_IN_APP][MAX_IPS_IN_FLOW];

	 long flowStartCycle[MAX_FLOWS][10000];
	 long ipProcessStartCycle[IP_TYPE_END][MAX_IPS];
	 bool frameStarted[IP_TYPE_END][MAX_IPS];

	 int ipIdRoundRobin;
	 int cpuIdRoundRobin;

     bool enableDVFS;
     int dvfsPeriod;
     int governor;
     int governor_timing;
     double powerInLastEpoch;
     int framesMissed;
     int framesToBeShown;

     bool doSlackDVFS;
     bool doIPSlackDVFS;
     int memFreqMultiplier;
     int cpuFreqMultipliers[MAX_CPUS];
     int ipFreqMultipliers[IP_TYPE_END][MAX_IPS];
     double m_avgPowerInFrameSum[IP_TYPE_END][MAX_IPS];
     int m_avgPowerInFrameCount[IP_TYPE_END][MAX_IPS];

     int optimal_freqs[IP_TYPE_END]; // store indices
     double optimal_energy[IP_TYPE_END];
     // double core_time_table[APP_ID_END][CORE_DVFS_STATES];
     // double core_energy_table[APP_ID_END][CORE_DVFS_STATES];
     double ip_energy_table[IP_TYPE_END][IP_DVFS_STATES];
     double time_pred_coeffs[IP_TYPE_END][4];

     int framenum_motivationgraphs;
     void powerCalculator1us();
	 void powerCalculator();
     GemDroidIP *getIPInstance(int ip_type, int id=0);

	//DVFS Related
     void initDVFS();
     void updateDVFS();
     void loadFlows(string fileName);
     void loadIPChars(string filename);
     double getLastSlack(int core_id, int flow_id);
     double getIPProcessingSize(int typeOfIP);
     double getIPProcessingLatency(int typeOfIP);
     bool isAtOptimal(int typeOfIP);
     bool dvfsMemory(double &slack);
     void setDVFSSlackFrequencies(double &slack, int ip_type);
     void dvfsMemScale(double slack);
     void dvfsCoScale(double slack);
     void dvfsCoScaleNew();
     void dvfsCoScaleNewNew();
     void dvfsLoadBased(double slack);
     void dvfsParetoEfficient(double slack);
     void dvfsMaxEnergyConsumer(double slack);
     void dvfsMaxPowerConsumer(double slack);
     void dvfsMaxEnergyPerTimeConsumer(double slack);
     void dvfsGreedyKnapsack(double slack);     
     void dvfsCoreOracle(double slack);
	 void dvfsFixedPriority(double slack);
     void dvfsDynamicProg(double slack);
     void dvfsDynamicProg1(double slack, int ip_accs[MAX_IPS_IN_FLOW], int num_accs, int ip_devs[MAX_IPS_IN_FLOW], int num_devs);
     void dvfsDynamicProg2(double slack, int ip_accs[MAX_IPS_IN_FLOW], int num_accs, int ip_devs[MAX_IPS_IN_FLOW], int num_devs);
     double getLastTimeForFlow(int app_id, int flow_id);

public:

	GemDroidCore gemdroid_core[MAX_CPUS];
	GemDroidMemory gemdroid_memory;
	GemDroidSA gemdroid_sa;
	GemDroidIP gemdroid_ip_dc[MAX_IPS];
	GemDroidIP gemdroid_ip_nw[MAX_IPS];
	GemDroidIP gemdroid_ip_snd[MAX_IPS];
	GemDroidIP gemdroid_ip_mic[MAX_IPS];
	GemDroidIP gemdroid_ip_cam[MAX_IPS];
	GemDroidIP gemdroid_ip_mmc_in[MAX_IPS];
	GemDroidIP gemdroid_ip_mmc_out[MAX_IPS];
	GemDroidIPDecoder gemdroid_ip_vd[MAX_IPS];
	GemDroidIPEncoder gemdroid_ip_ve[MAX_IPS];
	GemDroidIPDecoder gemdroid_ip_ad[MAX_IPS];
	GemDroidIPEncoder gemdroid_ip_ae[MAX_IPS];
    GemDroidIPNocoder gemdroid_ip_img[MAX_IPS];

    GemDroidIPGPU gemdroid_ip_gpu[MAX_IPS];
    // GemDroidIPDMA gemdroid_ip_dma[MAX_IPS];
    
    long appMemReqs[MAX_CPUS];
    long ipMemReqs[IP_TYPE_END];
    double ip_time_table[IP_TYPE_END][IP_DVFS_STATES];
    double lastTimeTook[IP_TYPE_END];
    double lastPowerTook[IP_TYPE_END];
    double lastEnergyTook[IP_TYPE_END];
    double lastFrequency[IP_TYPE_END];
    double lastMemFreq;
    double lastFlowTime;
    int dvfs_core;

    typedef GemDroidParams Params;
    GemDroid(const Params *p);
    ~GemDroid();
    void init();
    void startup();
    void regStats();
    void resetStats();
    void printPeriodicStats();

    inline bool isDevice(int ip_type) { if (ip_type == IP_TYPE_CPU) return false; if (ip_type < IP_TYPE_VD) return true; else return false; }
    void nextIPtoCall(int curr_ip_active, int core_id, int flow_type, int (&ips)[MAX_IPS_IN_FLOW]);
    bool enqueueIPReq(int sender_type, int sender_id, int core_id, int ip_type, uint64_t addr, int size, bool isRead, int frameNum, int flowType, int flowId);
    void enqueueIPResponse(int sender_type,  int sender_id, int core_id);
    bool enqueueCoreIPReq(int id, int ip_type, uint64_t addr, int size, bool isRead, int frameNum);
    bool memIPResponse(int ip_type, int ip_id, uint64_t addr, bool isRead);
    bool memCoreResponse(int type, int core_id, uint64_t addr, bool isRead);
    void tickIP(int type, int id);
    bool isIPCallDrop(int ip_type, long timeTook);
    int flowType(int array[]);
    void flow_identification(int core_id, int ip_type, int (&flows)[MAX_FLOWS_IN_APP]);
    int getLastIPInFlow(int flowId);
    int getFirstIPInFlow(int flowId);
    int getGPUFlowId();
    bool isFlowDrop(int flow_id, long timeTook);
    inline double getSweepVal1() { return sweep_val1; }
    inline double getSweepVal2() { return sweep_val2; }
    inline bool isPerfectMemory() {return perfectMemory;}

    bool has2ndFlow(int appID);
    int getIPAccsInFlow(int core_id, int flow_id, int (&ips)[MAX_IPS_IN_FLOW]);
    int getIPDevsInFlow(int core_id, int flow_id, int (&ips)[MAX_IPS_IN_FLOW]);
    int getIPsInFlow(int core_id, int flow_id, int (&ips)[MAX_IPS_IN_FLOW]);
    int getAppIdWithName(string appName);
    // double getCoreFreq() { if (core_freq == -1) return (GEMDROID_FREQ/GEMDROID_TO_CORE); else return (double) core_freq/1000; } // in GHz
    // double getIPFreq() { if (ip_freq == -1) return (GEMDROID_FREQ/CORE_TO_ACC_FREQ); else return (double) ip_freq/1000; } // in GHz
    inline int getGovernor() { return governor; }
    double getCoordinatedPower();
    inline long getTicks() { return ticks; }
    inline double getPowerInLastEpoch() { return powerInLastEpoch; }
    int getFlowId(int core_id, int ip_id);

    void markIPRequestStarted(int coreId, int ip_type, int ip_id, int frameNum);
    long markIPRequestCompleted(int coreId, int ip_type, int ip_id, int frameNum, int flowId);


    //DVFS Related
    double memScaledTime(int typeOfIP, int core_id, double availableBW, int ipFreqIndex);
    double memScaledTime(int typeOfIP, int core_id, int freqIndex);
    double memScaledTimeCPU(int core_id, double oldMemFreq, double newMemFreq);

    double getAvailBW(int core_id);
    void setDoSlackDVFSNextMs() {doSlackDVFS=true;}  //do Slack Based DVFS in the next milli-sec

	Stats::Scalar m_totalCommittedInsns;
	Stats::Distribution idleStreaksinActivePState[IP_TYPE_END][MAX_IPS];
	Stats::Average m_cyclesPerFrame[IP_TYPE_END][MAX_IPS];
	Stats::Average m_cyclesPerFrameInFlow[MAX_FLOWS];
	Stats::Scalar m_ipCallDrops[IP_TYPE_END];
	Stats::Scalar m_flowDrops[MAX_FLOWS];

    Stats::Scalar m_totalCoreEnergy[MAX_CPUS];
    Stats::Scalar m_totalIPEnergy[IP_TYPE_END];
    Stats::Scalar m_totalMemEnergy;
    Stats::Scalar m_totalPlatformEnergy;
    Stats::Scalar m_totalMilliSecs;
};

#endif //__GEMDROID_HH__

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
#include <float.h>

using namespace std;

int findMax(double array[], int n)
{
    double max_val = -DBL_MAX;
    int max_ind = -99999;

    for(int i=0; i < n; i++) {
        if(array[i] > max_val) {
            max_val = array[i];
            max_ind = i;    
        }
    }

    return max_ind;
}

int findMin(double array[], int n)
{
    double min_val = DBL_MAX;
    int min_ind = -99999;

    for(int i=0; i < n; i++) {
        if(array[i] != 0 && array[i] < min_val) {
            min_val = array[i];
            min_ind = i;    
        }
    }

    return min_ind;
}

bool isAllZeroes(double array[], int n)
{
    for(int i=0; i < n; i++) {
        if(array[i] != 0) {
          return false;
        }
    }

    return true;
}

void printArray(int array[], int n)
{
    for(int i=0; i < n; i++) {
        cout << array[i] << " ";
    }

    cout << endl;
}

bool GemDroid::isAtOptimal(int typeOfIP)
{
    if(typeOfIP == IP_TYPE_CPU) {
        if(gemdroid_core[dvfs_core].getCoreFreqInd() == gemdroid_core[dvfs_core].getOptCoreFreqInd())
            return true;
    }
    else if( typeOfIP >= IP_TYPE_DC && typeOfIP <= IP_TYPE_MMC_OUT ) { // Device
        return true;        //Devices are always at optimal.
    }
    else if( typeOfIP >= IP_TYPE_VD) { // Device
        GemDroidIP *inst = getIPInstance(typeOfIP);
        if(inst->getIPFreqInd() ==  inst->getOptIPFreqInd())
            return true;
    }
    else
        assert(0);

    return false;
}

double GemDroid::getIPProcessingLatency(int typeOfIP)
{
	if(typeOfIP == IP_TYPE_VD)
		return VD_PROCESSING_TIME;
	else if (typeOfIP == IP_TYPE_VE)
		return VE_PROCESSING_TIME;
	else if (typeOfIP == IP_TYPE_AE)
		return AE_PROCESSING_TIME;
	else if (typeOfIP == IP_TYPE_AD)
		return AD_PROCESSING_TIME;
	else if (typeOfIP == IP_TYPE_IMG)
		return IMG_PROCESSING_TIME;
	else if (typeOfIP == IP_TYPE_CAM)
		return CAM_PROCESSING_TIME;
	else if (typeOfIP == IP_TYPE_MMC_OUT || typeOfIP == IP_TYPE_MMC_IN)
		return MMC_PROCESSING_TIME;
	else
		return 1;	
}

double GemDroid::getIPProcessingSize(int typeOfIP)
{
	if(typeOfIP == IP_TYPE_VD || typeOfIP == IP_TYPE_VE)
		return VID_CODING_RATIO+1;
	else if (typeOfIP == IP_TYPE_AE || typeOfIP == IP_TYPE_AD)
		return AUD_CODING_RATIO + 1;
	else
		return 1;	
}

double GemDroid::memScaledTimeCPU(int core_id, double oldMemFreq, double newMemFreq)
{
    double memFraction = gemdroid_core[core_id].getProfileFractionOfMemInst();
    if (std::isnan(memFraction))
        memFraction = 0.20;
    double memTime = (memFraction) * lastTimeTook[IP_TYPE_CPU];
    double computeTime = lastTimeTook[IP_TYPE_CPU] - memTime;

    double newTime = computeTime + memTime * (oldMemFreq/newMemFreq);
    // cout << "DVFS Mem Fraction: " << memFraction << endl;

    return newTime;
}

double GemDroid::memScaledTime(int typeOfIP, int core_id, double availableBW, int ipFreqIndex)
{
    if (typeOfIP == IP_TYPE_CPU) {
        //memScaleCPU should never call this function.
        assert(0);
    }

	GemDroidIP *inst = getIPInstance(typeOfIP);
    if(typeOfIP == IP_TYPE_DC || typeOfIP == IP_TYPE_CAM) { // Device
        // cout << "DVFS Avail BW:" << availableBW << " DEV: " << typeOfIP << endl;
        return time_pred_coeffs[typeOfIP][0]*availableBW*availableBW*availableBW + time_pred_coeffs[typeOfIP][1]*availableBW*availableBW + time_pred_coeffs[typeOfIP][2]*availableBW + time_pred_coeffs[typeOfIP][3];
    }
    else if (typeOfIP >= IP_TYPE_NW && typeOfIP <= IP_TYPE_MMC_OUT ) {
        return lastTimeTook[typeOfIP];
    }
    else if (typeOfIP == IP_TYPE_GPU) {
        double memTime = (0.30) * lastTimeTook[IP_TYPE_GPU];
        double computeTime = lastTimeTook[IP_TYPE_GPU] - memTime;

        double newTime = computeTime + memTime * (gemdroid_memory.getMemFreq()/gemdroid_memory.getFreqForBandwidth(availableBW));
        return newTime;
    }
    else { //From IP_TYPE_VD to end
    	int ipProcessingLatency = getIPProcessingLatency(typeOfIP);
    	double dataAccessed = getIPProcessingSize(typeOfIP);
    	// double baseTime = ip_time_table[typeOfIP][ipFreqIndex];
    	double baseTime = inst->getTimeEst(lastTimeTook[typeOfIP], lastFrequency[typeOfIP], inst->getIPFreq(ipFreqIndex));
    
    	double ipBWRequirement = dataAccessed*CACHE_LINE_SIZE / (ipProcessingLatency / inst->getIPFreq(ipFreqIndex));
        //Returns in GBPS.
    
        // cout << "REQ ipBWRequirement" << ipBWRequirement << endl;
        // cout<<"DVFS. memScaledTime:"<<ipTypeToString(typeOfIP)<<" "<<ipBWRequirement<<" "<<availableBW<<" "<<baseTime<<endl;
    	if(availableBW > ipBWRequirement) {
    		return baseTime;
    	}
    	else {
    		return (ipBWRequirement/availableBW)*baseTime;
    	}
    }
}

double GemDroid::memScaledTime(int typeOfIP, int core_id, int ipFreqIndex)
{
	double availableBW = getAvailBW(core_id);
    // cout<<"DVFS. memScaledTime:"<<ipTypeToString(typeOfIP)<<" "<<freqIndex<<endl;
	return memScaledTime(typeOfIP, core_id, availableBW, ipFreqIndex);
}

//Returns in GBPS.
double GemDroid::getAvailBW(int core_id) //Get available bandwidth for this app.
{
    if(num_cpus == 1)
        return gemdroid_memory.getMaxBandwidth();

    //1.12 because of the constant amount of discrepancy from what DRAMSim2 calculates its bandwidth as. Not sure why DRAMSim numbers are off.
    //GemDroid numbers are perfect. Checked thoroughly. Agmark ISO 9001:2001.
    double appBW = ((appMemReqs[core_id]*64.0)/(dvfsPeriod*1.125))*10;
    double memBW = gemdroid_memory.getBandwidth();
    cout<<"\n DVFS. gemdroid_memory.getBandwidth()\t\t\t\t "<<gemdroid_memory.getBandwidth()\
    <<"\n DVFS. appMemReqs[core_id]*64.0 \t\t\t\t"<<appMemReqs[core_id]*64.0\
    <<"\n DVFS. ((appMemReqs[core_id]*64.0/(dvfsPeriod*1.125))*10 \t\t\t\t"<<((appMemReqs[core_id]*64.0)/(dvfsPeriod*1.125))*10\
    <<"\n DVFS. getAvailBW "<<(gemdroid_memory.getMaxBandwidth() - memBW) + appBW\
    <<endl;

	return (gemdroid_memory.getMaxBandwidth() - memBW) + appBW;
}

//For DVFS techniques we have: For "slack" we have, try to go as close as possible to optimal frequency for core or IPs (based on ip_type)
void GemDroid::setDVFSSlackFrequencies(double &slack, int ip_type)
{
    if(ip_type == IP_TYPE_CPU) {
        for(int i=0; i<num_cpus; i++) {
            double newFreq = gemdroid_core[i].getFreqForSlackOptimal(lastTimeTook[IP_TYPE_CPU], slack);
            cout<<"DVFS. Setting freq for CPU:"<<newFreq<<" "<<" Rem.slack "<<slack<<endl;
            gemdroid_core[i].setCoreFreq(newFreq);
        }
    }
    else if (ip_type >= IP_TYPE_VD && ip_type <= IP_TYPE_GPU) {
        assert (lastEnergyTook[ip_type] != 0);
        GemDroidIP *inst = getIPInstance(ip_type);
        double newFreq = inst->getFreqForSlackOptimal(lastTimeTook[ip_type], slack);
        cout<<"DVFS. Setting freq for IP:"<<ipTypeToString(ip_type)<<" "<<newFreq<<" "<<" Rem.slack "<<slack<<endl;
        inst->setIPFreq(newFreq);
    }
}

void GemDroid::updateDVFS()
{
    assert(enableDVFS);

    bool doFrameBoundaryDVFS = true;
    bool doIPBoundaryDVFS = false;
    double slack = 0;
    dvfs_core = 0;  //default value

    slack = getLastSlack(app_id[0], 0);

    if (slack == FPS_DEADLINE)
        return;

        //cout << "DVFS. Slack: " << slack << endl;

    if(governor_timing == GOVERNOR_TIMING_FRAME_BOUNDARIES) {
        if(doSlackDVFS)
            doFrameBoundaryDVFS = true;
        else
            doFrameBoundaryDVFS = false;
    }

    /*if(governor_timing == GOVERNOR_TIMING_FRAME_BOUNDARIES ) { //&& doSlackDVFS
        slack = getLastSlack(app_id[0], 0);
        cout << "DVFS. Slack: " << slack << endl;
        doSlackDVFS = false;
        doFrameBoundaryDVFS = true;
    }*/
/*     else if (governor_timing == GOVERNOR_TIMING_FIXED) {
            doFrameBoundaryDVFS = true;
     }*/
/*    else if (governor_timing == GOVERNOR_TIMING_IP_FRAME_BOUNDARIES && doIPSlackDVFS) {
        slack = getLastSlack(app_id[0], 0);
        cout << "DVFS. IP Slack: " << slack << endl;
        doIPSlackDVFS = false;
        doIPBoundaryDVFS = true;
    }*/

	for(int i=0; i<num_cpus; i++)
        gemdroid_core[i].updateDVFS();
   	for(int i=IP_TYPE_DC; i<IP_TYPE_DMA; i++)
   	    for(int j=0; j<num_ip_inst; j++) {
            GemDroidIP *inst = getIPInstance(i, j);
            inst->updateDVFS();
        }
    
    if (governor == GOVERNOR_TYPE_PERFORMANCE || governor == GOVERNOR_TYPE_POWERSAVE || governor == GOVERNOR_TYPE_OPTIMAL) {
        return;
    }
    else if (governor == GOVERNOR_TYPE_SLACK) {
        if (doFrameBoundaryDVFS) {
            dvfsFixedPriority(slack);
        }
        if (doIPBoundaryDVFS) {
        }
    }
    else if (governor == GOVERNOR_TYPE_CORE_ORACLE) {
        if (doFrameBoundaryDVFS) {
            dvfsCoreOracle(slack);
        }
        if (doIPBoundaryDVFS) {
        }
    }
    else if (governor == GOVERNOR_TYPE_SLACK_FAR_ENERGY) {
    	//Do DVFS after every frame boundary
        if (doFrameBoundaryDVFS) {
        	dvfsParetoEfficient(slack);
        }
        //Do DVFS after every IP completion within a frame
        if (doIPBoundaryDVFS) {
        }
    }
    else if (governor == GOVERNOR_TYPE_SLACK_MAXENERGY_CONSUMER) {
    	//Do DVFS after every frame boundary
        if (doFrameBoundaryDVFS) {
        	dvfsMaxEnergyConsumer(slack);
        }
        //Do DVFS after every IP completion within a frame
        if (doIPBoundaryDVFS) {
        }
    }
    else if (governor == GOVERNOR_TYPE_SLACK_MAXPOWER_CONSUMER) {
    	//Do DVFS after every frame boundary
        if (doFrameBoundaryDVFS) {
        	dvfsMaxPowerConsumer(slack);
        }
        //Do DVFS after every IP completion within a frame
        if (doIPBoundaryDVFS) {
        }
    }
    else if (governor == GOVERNOR_TYPE_SLACK_MAXENERGYPERTIME_CONSUMER) {
        //Do DVFS after every frame boundary
        if (doFrameBoundaryDVFS) {
            dvfsMaxEnergyPerTimeConsumer(slack);
        }
        //Do DVFS after every IP completion within a frame
        if (doIPBoundaryDVFS) {
        }
    }
    else if (governor == GOVERNOR_TYPE_SLACK_KNAPSACK) {
        //Do DVFS after every frame boundary
        if (doFrameBoundaryDVFS) {
            dvfsGreedyKnapsack(slack);
        }
        //Do DVFS after every IP completion within a frame
        if (doIPBoundaryDVFS) {
        }
    }
    else if (governor == GOVERNOR_TYPE_COSCALE) {
        //Do DVFS after every frame boundary
        if (doFrameBoundaryDVFS) {
            dvfsCoScaleNewNew();
            // dvfsCoScaleNew();
        }
        //Do DVFS after every IP completion within a frame
        if (doIPBoundaryDVFS) {
        }
    }
    else if (governor == GOVERNOR_TYPE_MEMSCALE) {
        //Do DVFS after every frame boundary
        if (doFrameBoundaryDVFS) {
            dvfsMemScale(slack);
        }
        //Do DVFS after every IP completion within a frame
        if (doIPBoundaryDVFS) {
        }
    }
    else if (governor == GOVERNOR_TYPE_DYNAMICPROG) {
        //Do DVFS after every frame boundary
        if (doFrameBoundaryDVFS) {
            dvfsDynamicProg(slack);
        }
    }
    else { 	// core utilization based DVFS schemes
        /*for governor == GOVERNOR_TYPE_ONDEMAND || GOVERNOR_TYPE_INTERACTIVE || GOVERNOR_TYPE_BUILDING || GOVERNOR_TYPE_POWERCAP*/
        dvfsLoadBased(slack);
    }
}

// returns true if next components can do DVFS (slack +ve)
bool GemDroid::dvfsMemory(double &slack)
{
    double mem_freq = gemdroid_memory.getMemFreq();
    double orig_mem_freq = gemdroid_memory.getMemFreq();
    double extra_time_estimate = 0;
    double correct_extra_time = 0;
    int core_id = 0;
    double newTime, oldTime;

    cout << "DVFS Mem" << " Slack left: " << slack << " CurrFreq: " << mem_freq << ". ";

    if (slack < 0) {
        if(orig_mem_freq < MAX_MEM_FREQ) {
            // if (orig_mem_freq < MAX_MEM_FREQ-0.1)
                // mem_freq += 0.2;
            // else
                mem_freq += 0.1;

            int ip, j = 0, ips[MAX_IPS_IN_FLOW];
            getIPsInFlow(0, 0, ips); //core_id = 0
            while ((ip = ips[j++]) != -1) {
                if (ip == IP_TYPE_CPU) {
                    newTime = memScaledTimeCPU(core_id,  orig_mem_freq, mem_freq);
                    oldTime = lastTimeTook[IP_TYPE_CPU];
                }
                else {
                    GemDroidIP *inst = getIPInstance(ip);
                   
                    newTime = memScaledTime(ip, core_id, gemdroid_memory.getMaxBandwidth(mem_freq), inst->getIPFreqInd());
                    oldTime = lastTimeTook[ip];
                    // oldTime = memScaledTime(ip, core_id, gemdroid_memory.getMaxBandwidth(orig_mem_freq), inst->getIPFreqInd());
                }
                // assert (newTime <= oldTime);
                if (newTime > oldTime);
                    newTime = oldTime;
                extra_time_estimate += (newTime - oldTime);
            }

            slack -= extra_time_estimate;

            gemdroid_memory.setMemFreq(mem_freq);
        }
        else
            cout<<" Slack -ve but Mem Already At Max"<<endl;

        cout<<" Increasing Mem_Freq = "<<mem_freq<< ". Slack left = " << slack <<endl;
        return false;
    }

    double min_mem = MIN_MEM_FREQ;
    if (governor == GOVERNOR_TYPE_SLACK_MAXENERGY_CONSUMER || governor == GOVERNOR_TYPE_SLACK_FAR_ENERGY || governor == GOVERNOR_TYPE_SLACK_MAXENERGY_CONSUMER)
        min_mem = MIN_MEM_FREQ+0.1;

    while (fabs(mem_freq - min_mem) > EPSILON) {
        mem_freq-=0.1;

        int ip, j = 0, ips[MAX_IPS_IN_FLOW];
        getIPsInFlow(0, 0, ips); //core_id = 0
        while ((ip = ips[j++]) != -1) {
            if (ip == IP_TYPE_CPU) {
                newTime = memScaledTimeCPU(core_id,  orig_mem_freq, mem_freq);
                oldTime = lastTimeTook[IP_TYPE_CPU];
            }
            else {
                GemDroidIP *inst = getIPInstance(ip);
               
                newTime = memScaledTime(ip, core_id, gemdroid_memory.getMaxBandwidth(mem_freq), inst->getIPFreqInd());
                oldTime = lastTimeTook[ip];
            }
            // assert (newTime >= oldTime);
            if (newTime < oldTime)
                newTime = oldTime;
            extra_time_estimate += (newTime - oldTime);
        }

        // cout << "DVFS Memory extra_time_estimate: " << extra_time_estimate << " at mem freq " << mem_freq << endl;
        if (extra_time_estimate > slack) {
            break;
        }
        else {
            gemdroid_memory.setMemFreq(mem_freq);
            correct_extra_time = extra_time_estimate;
            //cout<<"DVFS. Setting Mem_Freq = "<<mem_freq<< " to use " << slack << "ms slack" <<endl;
        }
    }
    
    slack -= correct_extra_time;
    cout<<"Setting freq for Mem "<<gemdroid_memory.getMemFreq()<< " Rem.slack " << slack << endl;
    return true;
}

void GemDroid::dvfsCoreOracle(double slack)
{
    int ips[MAX_IPS_IN_FLOW];
    int ip = -1, j = 0;
    getIPsInFlow(0, 0, ips); //core_id = 0
    // for(int i=IP_TYPE_VD; i<=IP_TYPE_GPU; i++) {
    while ((ip = ips[j++]) != -1) {
        if (lastEnergyTook[ip] != 0 && ip == IP_TYPE_CPU) {
            double cpu_freq = gemdroid_core[0].getFreqForSlackOptimal(lastTimeTook[IP_TYPE_CPU], slack);

            // cout << "DVFS. CPU PrevFreq: " << gemdroid_core[0].getCoreFreq() << " PrevTime: " << lastTimeTook[ip] << " NextFreq: " << cpu_freq << " Slack after: " << slack << endl;
            gemdroid_core[0].setCoreFreq(cpu_freq);
        }
    }
}

void GemDroid::dvfsFixedPriority(double slack)
{
    bool doNextDVFS = dvfsMemory(slack);

    if (slack > 0 && !doNextDVFS)
        return;

    int ips[MAX_IPS_IN_FLOW];
    int ip = -1, j = 0;
    getIPsInFlow(0, 0, ips); //core_id = 0
    // for(int i=IP_TYPE_VD; i<=IP_TYPE_GPU; i++) {
    while ((ip = ips[j++]) != -1) {
        if (lastEnergyTook[ip] != 0 && ip == IP_TYPE_CPU) {
            double cpu_freq = gemdroid_core[0].getFreqForSlackOptimal(lastTimeTook[IP_TYPE_CPU], slack);

            // cout << "DVFS. CPU PrevFreq: " << gemdroid_core[0].getCoreFreq() << " PrevTime: " << lastTimeTook[ip] << " NextFreq: " << cpu_freq << " Slack after: " << slack << endl;
            gemdroid_core[0].setCoreFreq(cpu_freq);
        }
        else if (lastEnergyTook[ip] != 0 && ip >= IP_TYPE_VD) {
	        GemDroidIP *inst = getIPInstance(ip);

            double ip_freq = inst->getFreqForSlackOptimal(lastTimeTook[ip], slack);

            // cout << "DVFS. IP_" << ipTypeToString(ip) << " NextFreq: " << ip_freq << " Slack after: " << slack << endl;
            // cout << "Slack after IP_" << ipTypeToString(i) << ": " << slack << " Prev Freq: " << inst->getIPFreq() << " Next Freq: " << ip_freq << endl;
            inst->setIPFreq(ip_freq);
        }
    }
}

void GemDroid::dvfsLoadBased(double slack)
{
    for(int i=0; i<num_cpus; i++) {
        double load = 0;
        
        // if(!gemdroid_core[i].isPStateIdle()) {
            load = gemdroid_core[i].getLoadInLastDVFSEpoch();

            if(load > 0.6) {
                // Ondemand or Interactive
                if (governor == GOVERNOR_TYPE_ONDEMAND || governor == GOVERNOR_TYPE_INTERACTIVE) {
                        gemdroid_core[i].setMaxCoreFreq();
                }
                // Building
                else if (governor == GOVERNOR_TYPE_BUILDING) {
                    gemdroid_core[i].incFreq(2);
                }
                // Coordinated
                else if (governor == GOVERNOR_TYPE_POWERCAP) {
                    gemdroid_core[i].setMaxAllowedFreq();
                }
            }
            else if (load < 0.2) {
                gemdroid_core[i].decFreq();
            }
        // }

        cout<<"DVFS. CPU"<<i<<" - Load: " << load << " Freq: "<<gemdroid_core[i].getCoreFreq()<<endl;
    }
    
    // IP ACC DVFS
    for(int i=IP_TYPE_VD; i<=IP_TYPE_GPU; i++) {
        for(int j=0; j<num_ip_inst; j++) {
            if (lastEnergyTook[i] == 0)
                continue;
            GemDroidIP *inst = getIPInstance(i, j);
            double load = 0;

            // if(!inst->isPStateIdle()) {
                load = inst->getLoadInLastDVFSEpoch();

                if (std::isnan(load))
                    continue;

                if(load > 0.6) {
                    if (governor == GOVERNOR_TYPE_ONDEMAND || governor == GOVERNOR_TYPE_INTERACTIVE) // Ondemand or Interactive
                        inst->setMaxIPFreq();
                    else if (governor == GOVERNOR_TYPE_BUILDING)
                        inst->incFreq(1);
                    else if (governor == GOVERNOR_TYPE_POWERCAP) // Coordinated
                        inst->setMaxAllowedFreq();
                }
                else if (load < 0.3) {
                    inst->decFreq();
                }
            // }

            cout<<"DVFS. IP_" << ipTypeToString(i) << " " << j << " - Load: " << load << " Freq: "<<inst->getIPFreq()<<endl;
        }
    }
    cout << "DVFS. Slack Mem" << " @ " << gemdroid_memory.getMemFreq() << endl;

    //Utilization based DVFS dont have memory scaling.

    /* double mem_latency = gemdroid_memory.getLastLatency();
    if (mem_latency > 1000)
        gemdroid_memory.setMaxMemFreq();
    else
        gemdroid_memory.decMemFreq(); */
}

void GemDroid::dvfsMemScale(double slack)
{
    for(int i=0; i<num_cpus; i++) {
        double load = 0;
        load = gemdroid_core[i].getLoadInLastDVFSEpoch();
        if(load >= 0.1) {
            gemdroid_core[i].setMaxCoreFreq();
        }
        else if (load < 0.1) {
                    gemdroid_core[i].setMinCoreFreq();
        }
        cout<<"DVFS. CPU"<<i<<" - Load: " << load << " Freq: "<<gemdroid_core[i].getCoreFreq()<<endl;
    }

    // IP ACC DVFS
    for(int i=IP_TYPE_VD; i<=IP_TYPE_GPU; i++) {
        for(int j=0; j<num_ip_inst; j++) {
            GemDroidIP *inst = getIPInstance(i, j);
            double load = 0;

            // if(!inst->isPStateIdle()) {
                load = inst->getLoadInLastDVFSEpoch();

                if(load > 0.5) {
                    inst->setMaxIPFreq();
                }
                else if (load < 0.1) {
                    inst->setMinIPFreq();
                }
            // }

            cout<<"DVFS. IP_" << ipTypeToString(i) << " " << j << " - Load: " << load << " Freq: "<<inst->getIPFreq()<<endl;
        }
    }

    cout << "DVFS. Slack Mem" << " @ " << gemdroid_memory.getMemFreq() << endl;\
    dvfsMemory(slack);
}

void GemDroid::dvfsCoScaleNewNew()
{
    double max_memfreq_setsofar=0;
    double mem_freq_min = gemdroid_memory.getMinMemFreq();
    double cpu_freq_min = gemdroid_core[0].getOptCoreFreqInd();
    double mem_freq_max = gemdroid_memory.getMaxMemFreq();
    double cpu_freq_max = gemdroid_core[0].getMaxCoreFreqInd();
    // double ip_freq_min = 0;
    int cpu_freq = gemdroid_core[0].getCoreFreqInd();
    double mem_freq = gemdroid_memory.getMemFreq();
    double mem = gemdroid_memory.getMemFreq();
    int cpu = gemdroid_core[0].getCoreFreqInd();
    int ipFreqIndex;
    double cpu_time = 0, cpu_energy = 0;
    double ip_time = 0;
    double memory_energy, memory_time;

    int ips[MAX_IPS_IN_FLOW];
    int ip = -1, j = 0;
    getIPsInFlow(0, 0, ips); //core_id = 0
    while ((ip = ips[j++]) != -1) {
        if (ip == IP_TYPE_CPU && lastTimeTook[ip] > 0) {        //For CPUs
            if(gemdroid_core[0].isPStateActive()) {
                cout << "DVFS " << "lastTimeTook CPU: " << lastTimeTook[IP_TYPE_CPU] << " lastEnergyTook CPU: " << lastEnergyTook[IP_TYPE_CPU] << " @ " << lastFrequency[ip] << " and Mem @ " << gemdroid_memory.getMemFreq() << endl;
                double last_memory_energy = (FPS_DEADLINE+FPS_DEADLINE_SAFETYNET) * gemdroid_memory.powerIn1ms();
                double last_cpu_energy = lastEnergyTook[IP_TYPE_CPU];
                double cpu_min_base_time = memScaledTimeCPU(0, gemdroid_memory.getMemFreq(), gemdroid_memory.getMaxMemFreq());
                double cpu_min_time = gemdroid_core[0].getTimeEst(cpu_min_base_time, gemdroid_core[0].getCoreFreq(), gemdroid_core[0].getMaxCoreFreq());
                cout << "DVFS cpu_min_time:" << cpu_min_time*getSweepVal1() << endl;

                if(lastTimeTook[IP_TYPE_CPU] > getSweepVal2()*cpu_min_time) {
                    if (mem_freq < mem_freq_max)
                        mem_freq += 0.1;
                    if (cpu_freq < cpu_freq_max)
                        cpu_freq++;
                }
                else {
                    while (1) {
                        //Set to numbers
                        memory_energy = last_memory_energy;
                        memory_time = cpu_min_time;
                        cpu_energy = last_cpu_energy;
                        cpu_time = cpu_min_time;

                        //If mem freq can be reduced
                        if(fabs(mem - mem_freq_min) > EPSILON) {                        
                            mem -= 0.1;
                            memory_energy = gemdroid_memory.getEnergyEst(gemdroid_memory.getMemFreq(), last_memory_energy, mem);
                            cout << "DVFS " << "Mem Energy @ " << mem << " : " << memory_energy << endl;                        
                            memory_time = memScaledTimeCPU(0, gemdroid_memory.getMemFreq(), mem);
                            cout << "DVFS Mem Time cpu's freq: " << lastFrequency[IP_TYPE_CPU] << " " << memory_time << endl;
                        }
                        //If cpu freq can be reduced
                        if(cpu > cpu_freq_min) {
                            cpu--;
                            cpu_time = gemdroid_core[0].getTimeEst(memory_time, lastFrequency[IP_TYPE_CPU], gemdroid_core[0].getCoreFreq(cpu));
                            cpu_energy = gemdroid_core[0].getEnergyEst(memory_time, lastPowerTook[IP_TYPE_CPU], gemdroid_core[0].getCoreFreq(cpu));                        
                        }
                        else if (fabs(mem - mem_freq_min) < EPSILON) {
                            break;
                        }

                        if ((last_memory_energy - memory_energy) > (last_cpu_energy - cpu_energy)) {
                            if (memory_time < cpu_min_time*getSweepVal1())
                                mem_freq = mem;
                            else {
                                if (cpu_time < cpu_min_time*getSweepVal1())
                                    cpu_freq = cpu;
                                else
                                    break;
                            }
                        }
                        else {
                            if (cpu_time < cpu_min_time*getSweepVal1())
                                cpu_freq = cpu;
                            else {
                                if (memory_time < cpu_min_time*getSweepVal1())
                                    mem_freq = mem;
                                else
                                    break;
                            }
                        }
                    }
                }

                cout << "DVFS at " << mem_freq << " " << gemdroid_core[0].getCoreFreq(cpu_freq) << endl;
                if(mem_freq > max_memfreq_setsofar)
                    max_memfreq_setsofar = mem_freq;             //gemdroid_memory.setMemFreq(mem_freq);
                gemdroid_core[0].setCoreFreqInd(cpu_freq);
            }
        }
        else if(ip >= IP_TYPE_VD && lastTimeTook[ip] > 0) {     //For IPs
            GemDroidIP *inst = getIPInstance(ip);

            if (!inst->isPStateActive())
                continue;

            cout << "DVFS " << "lastTimeTook " << ipTypeToString(ip) << ": " << lastTimeTook[ip] << " lastEnergyTook: " << lastEnergyTook[ip] << " @ " << lastFrequency[ip] << " and Mem @ " << gemdroid_memory.getMemFreq() << endl;
            ipFreqIndex = inst->getIPFreqInd();
            double ip_freq_ind=0;
            double last_ip_energy = lastEnergyTook[ip];
            double ip_min_base_time = memScaledTime(ip, 0, gemdroid_memory.getMaxBandwidth(gemdroid_memory.getMaxMemFreq()), inst->getIPFreqInd());
            double ip_min_time = inst->getTimeEst(ip_min_base_time, inst->getIPFreq(), inst->getMaxIPFreq());
            double ip_freq_min = inst->getOptIPFreqInd();
            double ip_freq_max = inst->getMaxIPFreqInd();
            double last_memory_energy = (FPS_DEADLINE+FPS_DEADLINE_SAFETYNET) * gemdroid_memory.powerIn1ms();
            double ip_energy = last_ip_energy;

            // cout << "DVFS ip_min_time:" << ip_min_time*1.10000 << endl;
            cout << "DVFS ip_min_time:" << ip_min_time*getSweepVal1() << endl;

            if(lastTimeTook[ip] > getSweepVal2()*ip_min_time) {
                if (mem_freq < mem_freq_max)
                    mem_freq += 0.1;
                if (ip_freq_ind < ip_freq_max)
                    ip_freq_ind++;
            }
            else {
                while(1) {
                    //Set to numbers
                    memory_energy = last_memory_energy;
                    memory_time = ip_min_time;
                    ip_energy = last_ip_energy;
                    ip_time = ip_min_time;

                    //Should we reduce memory freq or IP freq? Thats what we decide frist. 
                    //We decide that based on the amount of energy savings provided by IP or memory.

                    //If mem freq can be reduced
                    if( fabs(mem - mem_freq_min) > EPSILON) {                    
                        mem -= 0.1;
                        memory_energy = gemdroid_memory.getEnergyEst(gemdroid_memory.getMemFreq(), last_memory_energy, mem);
                        cout << "DVFS " << "Mem Energy @ " << mem << " : " << memory_energy << endl;                        

                        memory_time = memScaledTime(ip, 0, gemdroid_memory.getMaxBandwidth(mem), inst->getIPFreqInd());
                        if (mem < lastMemFreq)
                            memory_time += memory_time*0.10*(lastMemFreq - mem);

                        // double ip_base_energy = lastEnergyTook[ip_type];
                        cout << "DVFS     IP BaseTime IP_Freq: " << lastFrequency[ip] << " " << memory_time << endl;
                    }

                    if(ipFreqIndex > ip_freq_min) {     //if ipFreqIndex > ip_freq_min->which is 0
                        ipFreqIndex--;
                        ip_time = inst->getTimeEst(memory_time, lastFrequency[ip], inst->getIPFreq(ipFreqIndex));
                        ip_energy = inst->getEnergyEst(memory_time, lastPowerTook[ip], inst->getIPFreq(ipFreqIndex));

                        cout << "DVFS      IP_Freq:" << inst->getIPFreq(ipFreqIndex) << " IP Time: " << ip_time << " IP Energy: " << ip_energy << endl;
                    }
                    else if (fabs(mem - mem_freq_min) < EPSILON) {
                        break;
                    }

                    if ((last_memory_energy - memory_energy) > (last_ip_energy - ip_energy)) {
                        if (memory_time < ip_min_time*getSweepVal1())
                            mem_freq = mem;
                        else {
                            if (ip_time < ip_min_time*getSweepVal1())
                                ip_freq_ind = ipFreqIndex;
                            else {
                                break;
                            }
                        }
                    }
                    else { //If ip_energy is greater than memory_energy, then save from IP.
                        if (ip_time < ip_min_time*getSweepVal1())
                            ip_freq_ind = ipFreqIndex;
                        else {
                            if (memory_time < ip_min_time*getSweepVal1())
                                mem_freq = mem;
                            else {
                                break;
                            }
                        }
                    }
                }
            }
            
            if(mem_freq > max_memfreq_setsofar)
                max_memfreq_setsofar = mem_freq; 

            //gemdroid_memory.setMemFreq(mem_freq);
            inst->setIPFreqInd(ip_freq_ind);
            cout << "DVFS " << mem_freq << " " << inst->getIPFreq(ip_freq_ind) << endl;
        }
        else if(ip < IP_TYPE_VD && lastTimeTook[ip] > 0) {     //For Devices
            GemDroidIP *inst = getIPInstance(ip);

            if (!inst->isPStateActive())
                continue;
            
            double last_memory_energy = (FPS_DEADLINE+FPS_DEADLINE_SAFETYNET) * gemdroid_memory.powerIn1ms();
            cout << "DVFS " << "lastTimeTook " << ipTypeToString(ip) << ": " << lastTimeTook[ip] << " lastEnergyTook: " << lastEnergyTook[ip] << " @ " << lastFrequency[ip] << " and Mem @ " << gemdroid_memory.getMemFreq() << endl;
            double ip_min_time = memScaledTime(ip, 0, gemdroid_memory.getMaxBandwidth(gemdroid_memory.getMaxMemFreq()), inst->getIPFreqInd());
            cout << "DVFS dev_min_time:" << ip_min_time*getSweepVal1() << endl;

            if(lastTimeTook[ip] > getSweepVal2()*ip_min_time) {
                if (mem_freq < mem_freq_max)
                    mem_freq += 0.1;
            }
            else {
                while(1) {
                    //Set to numbers 
                    memory_energy = last_memory_energy;
                    memory_time = ip_min_time;

                    //If mem freq can be reduced
                    if( fabs(mem - mem_freq_min) > EPSILON) {
                        mem -= 0.1;
                        memory_energy = gemdroid_memory.getEnergyEst(gemdroid_memory.getMemFreq(), last_memory_energy, mem);
                        cout << "DVFS " << "Mem Energy @ " << mem << " : " << memory_energy << endl;
                        memory_time = memScaledTime(ip, 0, gemdroid_memory.getMaxBandwidth(mem), inst->getIPFreqInd());
                        cout << "DVFS     Dev BaseTime IP_Freq: " << lastFrequency[ip] << " " << memory_time << endl;
                    }
                    else
                        break;
                    
                    if (memory_time < ip_min_time*getSweepVal1())
                        mem_freq = mem;
                    else
                        break;
                }
            }
            
            if(mem_freq > max_memfreq_setsofar)
                max_memfreq_setsofar = mem_freq; 

            //gemdroid_memory.setMemFreq(mem_freq);
            cout << "DVFS Mem " << mem_freq << endl;
        }
    }
    if (max_memfreq_setsofar != 0) {
        cout << "DVFS Mem freq set to " << max_memfreq_setsofar << endl;
        gemdroid_memory.setMemFreq(max_memfreq_setsofar);
    }
}

void GemDroid::dvfsCoScaleNew()
{
    double energy = 0, minEnergy = 9999;
    double max_memfreq_setsofar=0;
    double mem_freq_max = gemdroid_memory.getMaxMemFreq();
    double mem_freq_min = gemdroid_memory.getMinMemFreq();
    double cpu_freq_max = gemdroid_core[0].getMaxCoreFreqInd();
    double cpu_freq_min = gemdroid_core[0].getOptCoreFreqInd();
    int cpu_freq = gemdroid_core[0].getCoreFreqInd();
    double ip_time = 0, ip_energy = 0;
    double mem_freq = gemdroid_memory.getMemFreq();
    double mem = gemdroid_memory.getOptMemFreq();
    double cpu_time = 0, cpu_energy = 0;
    double memory_energy;

    int ips[MAX_IPS_IN_FLOW];
    int ip = -1, j = 0;
    getIPsInFlow(0, 0, ips); //core_id = 0
    // for(int i=IP_TYPE_VD; i<=IP_TYPE_GPU; i++) {
    while ((ip = ips[j++]) != -1) {
        if (ip == IP_TYPE_CPU && lastTimeTook[ip] > 0) {        //For CPUs
            // for(int i=0;i<num_cpus;i++) {
                if(gemdroid_core[0].isPStateActive()) {
                    cout << "DVFS " << "lastTimeTook CPU: " << lastTimeTook[IP_TYPE_CPU] << " lastEnergyTook CPU: " << lastEnergyTook[IP_TYPE_CPU] << " @ " << lastFrequency[ip] << " and Mem @ " << gemdroid_memory.getMemFreq() << endl;
                    double cpu_min_base_time = memScaledTimeCPU(0, gemdroid_memory.getMemFreq(), gemdroid_memory.getMaxMemFreq());
                    double cpu_min_time = gemdroid_core[0].getTimeEst(cpu_min_base_time, gemdroid_core[0].getCoreFreq(), gemdroid_core[0].getMaxCoreFreq());
                    // cout << "DVFS cpu_min_time:" << cpu_min_time*1.10000 << endl;
                    cout << "DVFS cpu_min_time:" << cpu_min_time*getSweepVal1() << endl;
                    for (mem=mem_freq_max; mem>=mem_freq_min; mem-=0.1) {
                        // Estimate memory energy at this mem frequency
                        //
                        // Estimate cpu_base_time and energy at max freq for this mem freq
                        //
                        //
                        // Estimate cpu times at different cpu frequencies using cpu_base_time

                        double last_memory_energy = (FPS_DEADLINE+FPS_DEADLINE_SAFETYNET) * gemdroid_memory.powerIn1ms();
                        memory_energy = gemdroid_memory.getEnergyEst(gemdroid_memory.getMemFreq(), last_memory_energy, mem);

                        cout << "DVFS " << "Mem Energy @ " << mem << " : " << memory_energy << endl;

                        double cpu_base_time = memScaledTimeCPU(0, gemdroid_memory.getMemFreq(), mem);

                        // double cpu_base_energy = lastEnergyTook[IP_TYPE_CPU];
                        //double cpu_base_energy = gemdroid_core[0].getEnergyEst(cpu_base_time, lastPowerTook[IP_TYPE_CPU], gemdroid_core[0].getCoreFreq());
                        cout << "DVFS   CPU BaseTime CPU_Freq: " << lastFrequency[IP_TYPE_CPU] << " " << cpu_base_time << endl;

                        for (int cpu = cpu_freq_max; cpu >= cpu_freq_min; cpu--) {
                            cpu_time = gemdroid_core[0].getTimeEst(cpu_base_time, lastFrequency[IP_TYPE_CPU], gemdroid_core[0].getCoreFreq(cpu));
                            cpu_energy = gemdroid_core[0].getEnergyEst(cpu_base_time, lastPowerTook[IP_TYPE_CPU], gemdroid_core[0].getCoreFreq(cpu));

                            energy = memory_energy + cpu_energy;
                            cout << "DVFS    CPU_Freq:" << gemdroid_core[0].getCoreFreq(cpu) << " CPU Time: " << cpu_time << " CPU Energy: " << cpu_energy << endl;
                         
                            // if (cpu_time > cpu_min_time*1.1000)
                            if (cpu_time > cpu_min_time*getSweepVal1())
                                break;

                            if (energy < minEnergy) {
                                minEnergy = energy;
                                mem_freq = mem;
                                cpu_freq = cpu;
                                cout << "DVFS " << "Updating min energy value " << mem_freq << " " << gemdroid_core[0].getCoreFreq(cpu_freq) << endl;
                            }
                        }
                    }
                    cout << "DVFS Min Energy " << minEnergy << " is at " << mem_freq << " " << gemdroid_core[0].getCoreFreq(cpu_freq) << endl;
                    if(mem_freq > max_memfreq_setsofar)
                        max_memfreq_setsofar = mem_freq;             //gemdroid_memory.setMemFreq(mem_freq);
                    gemdroid_core[0].setCoreFreqInd(cpu_freq);
                }
            //}
        }
        else if(ip >= IP_TYPE_VD && lastTimeTook[ip] > 0) {     //For IPs
            GemDroidIP *inst = getIPInstance(ip);
            double ip_freq_max = inst->getMaxIPFreqInd();
            double ip_freq_min = inst->getOptIPFreqInd();
            int ip_freq = inst->getIPFreqInd();

            if (!inst->isPStateActive())
                continue;

            cout << "DVFS " << "lastTimeTook " << ipTypeToString(ip) << ": " << lastTimeTook[ip] << " lastEnergyTook: " << lastEnergyTook[ip] << " @ " << lastFrequency[ip] << " and Mem @ " << gemdroid_memory.getMemFreq() << endl;

            double ip_min_base_time = memScaledTime(ip, 0, gemdroid_memory.getMaxBandwidth(gemdroid_memory.getMaxMemFreq()), inst->getIPFreqInd());
            double ip_min_time = inst->getTimeEst(ip_min_base_time, inst->getIPFreq(), inst->getMaxIPFreq());

            // cout << "DVFS ip_min_time:" << ip_min_time*1.10000 << endl;
            cout << "DVFS ip_min_time:" << ip_min_time*getSweepVal1() << endl;

            for (mem=mem_freq_max; mem>=mem_freq_min; mem-=0.1) {
                // Estimate memory energy at this mem frequency
                // Estimate device times and device energies at this mem frequency
                // Estimate cpu_base_time and energy at max freq for this mem freq
                // Estimate ip_base_time and energy at max freq for this mem freq
                //
                // Estimate cpu times at different cpu frequencies using cpu_base_time
                // Estimate ip times at different ip frequencies using ip_base_time

                double last_memory_energy = (FPS_DEADLINE+FPS_DEADLINE_SAFETYNET) * gemdroid_memory.powerIn1ms();
                memory_energy = gemdroid_memory.getEnergyEst(gemdroid_memory.getMemFreq(), last_memory_energy, mem);

                cout << "DVFS " << "Mem Energy @ " << mem << " : " << memory_energy << endl;

                double ip_base_time = memScaledTime(ip, 0, gemdroid_memory.getMaxBandwidth(mem), inst->getIPFreqInd());
                if (mem < lastMemFreq)
                    ip_base_time += ip_base_time*0.15*(lastMemFreq - mem);

                // double ip_base_energy = lastEnergyTook[ip_type];
                cout << "DVFS     IP BaseTime IP_Freq: " << lastFrequency[ip] << " " << ip_base_time << endl;

                for (int ipfreqIndex = ip_freq_max; ipfreqIndex >= ip_freq_min; ipfreqIndex--) {
                    ip_time = inst->getTimeEst(ip_base_time, lastFrequency[ip], inst->getIPFreq(ipfreqIndex));
                    ip_energy = inst->getEnergyEst(ip_base_time, lastPowerTook[ip], inst->getIPFreq(ipfreqIndex));

                    energy = memory_energy + ip_energy;

                    cout << "DVFS      IP_Freq:" << inst->getIPFreq(ipfreqIndex) << " IP Time: " << ip_time << " IP Energy: " << ip_energy << endl;
            
                    // cout << "DVFS       Mem " << mem << " @ " << " " << inst->getIPFreq(ipfreqIndex) << " Time: " << time << " Energy: " << energy << endl;

                    // if (ip_time > ip_min_time*1.10000)
                    if (ip_time > ip_min_time*getSweepVal1())
                        break;
            
                    if (energy < minEnergy) {
                        minEnergy = energy;
                        mem_freq = mem;
                        ip_freq = ipfreqIndex;
                        cout << "DVFS " << "Updating min energy value " << mem_freq << " " << inst->getIPFreq(ip_freq) << endl;
                    }
                }
            }
            
            if(mem_freq > max_memfreq_setsofar)
                max_memfreq_setsofar = mem_freq; 

            //gemdroid_memory.setMemFreq(mem_freq);
            inst->setIPFreqInd(ip_freq);
            cout << "DVFS Min Energy " << minEnergy << " is at " << mem_freq << " " << inst->getIPFreq(ip_freq) << endl;
        }
        else if(ip < IP_TYPE_VD && lastTimeTook[ip] > 0) {     //For Devices
            GemDroidIP *inst = getIPInstance(ip);
            int ip_freq = inst->getIPFreqInd();

            if (!inst->isPStateActive())
                continue;

            cout << "DVFS " << "lastTimeTook " << ipTypeToString(ip) << ": " << lastTimeTook[ip] << " lastEnergyTook: " << lastEnergyTook[ip] << " @ " << lastFrequency[ip] << " and Mem @ " << gemdroid_memory.getMemFreq() << endl;

            double ip_min_time = memScaledTime(ip, 0, gemdroid_memory.getMaxBandwidth(gemdroid_memory.getMaxMemFreq()), inst->getIPFreqInd());

            // cout << "DVFS ip_min_time:" << ip_min_time*1.10000 << endl;
            cout << "DVFS dev_min_time:" << ip_min_time*getSweepVal1() << endl;

            for (mem=mem_freq_max; mem>=mem_freq_min; mem-=0.1) {
                // Estimate memory energy at this mem frequency
                // Estimate device times and device energies at this mem frequency
                // Estimate cpu_base_time and energy at max freq for this mem freq
                // Estimate ip_base_time and energy at max freq for this mem freq
                //
                // Estimate cpu times at different cpu frequencies using cpu_base_time
                // Estimate ip times at different ip frequencies using ip_base_time

                double last_memory_energy = (FPS_DEADLINE+FPS_DEADLINE_SAFETYNET) * gemdroid_memory.powerIn1ms();
                memory_energy = gemdroid_memory.getEnergyEst(gemdroid_memory.getMemFreq(), last_memory_energy, mem);

                cout << "DVFS " << "Mem Energy @ " << mem << " : " << memory_energy << endl;

                double ip_base_time = memScaledTime(ip, 0, gemdroid_memory.getMaxBandwidth(mem), inst->getIPFreqInd());

                cout << "DVFS     Dev BaseTime IP_Freq: " << lastFrequency[ip] << " " << ip_base_time << endl;

                if (ip_base_time > ip_min_time*getSweepVal1())
                    break;
            
                if (memory_energy < minEnergy) {
                    minEnergy = memory_energy;
                    mem_freq = mem;
                    cout << "DVFS " << "Updating min energy value " << mem_freq << " " << inst->getIPFreq(ip_freq) << endl;
                }
            }
            
            if(mem_freq > max_memfreq_setsofar)
                max_memfreq_setsofar = mem_freq; 

            //gemdroid_memory.setMemFreq(mem_freq);
            cout << "DVFS Min Energy for device " << minEnergy << " is at " << mem_freq << endl;
        }
    }
    if (max_memfreq_setsofar != 0) {
        cout << "DVFS Mem freq set to " << max_memfreq_setsofar << endl;
        gemdroid_memory.setMemFreq(max_memfreq_setsofar);
    }
}

void GemDroid::dvfsCoScale(double slack)
{
    double orig_sum_freq = 0;
    double new_sum_freq = 0;

    for(int i=0; i<num_cpus; i++) {
        orig_sum_freq = gemdroid_core[i].getCoreFreq();
    }

    setDVFSSlackFrequencies(slack, IP_TYPE_CPU);    
    //cout<<"DVFS. CPU"<<i<<" - Load: " << load << " Freq: "<<gemdroid_core[i].getCoreFreq()<<endl;
    for(int i=0; i<num_cpus; i++) {
        new_sum_freq = gemdroid_core[i].getCoreFreq();
    }


    // IP ACC DVFS
    for(int i=IP_TYPE_VD; i<=IP_TYPE_GPU; i++) {
        for(int j=0; j<num_ip_inst; j++) {
            GemDroidIP *inst = getIPInstance(i, j);
            double load = 0;

            // if(!inst->isPStateIdle()) {
                load = inst->getLoadInLastDVFSEpoch();

                if(load > 0.5) {
                    inst->setMaxIPFreq();
                }
                else if (load < 0.1) {
                    inst->setMinIPFreq();
                }
            // }

            cout<<"DVFS. IP_" << ipTypeToString(i) << " " << j << " - Load: " << load << " Freq: "<<inst->getIPFreq()<<endl;
        }
    }

    cout << "DVFS. Slack Mem" << " @ " << gemdroid_memory.getMemFreq() << endl;
    
    double memBW  = gemdroid_memory.getBandwidth();;
    cout<<"DVFS. MemBW-Lat: "<<memBW<<" "<<gemdroid_memory.getLastLatency()<<endl;

    if(new_sum_freq > orig_sum_freq) {
        //Core Freqs have been increased in total. So, memory freq has to be decreased.

        if(memBW > gemdroid_memory.getMaxBandwidth()*0.50 || gemdroid_memory.getLastLatency() > 150) {
        }
        else {
            if(gemdroid_memory.getMemFreq() > MIN_MEM_FREQ) {
                mem_freq -= 0.1;
                gemdroid_memory.setMemFreq(mem_freq);
            }
        }
    }
    else {
        //Core Freqs have been decreased in total. So, memory freq has to be increased.   
        if(memBW > gemdroid_memory.getMaxBandwidth()*0.50 || gemdroid_memory.getLastLatency() > 150) {
            if(gemdroid_memory.getMemFreq() < MAX_MEM_FREQ) {
                mem_freq += 0.2;
                gemdroid_memory.setMemFreq(mem_freq);
            }
        }
    }
    cout<<"DVFS. Mem_Freq = "<<mem_freq<<endl;

    //Utilization based DVFS dont havememory scaling.

    //dvfsMemory(slack);
    /*double mem_latency = gemdroid_memory.getLastLatency();
    if (mem_latency > 100)
        gemdroid_memory.setMaxMemFreq();
    else
        gemdroid_memory.decMemFreq();*/
}


void GemDroid::dvfsParetoEfficient(double slack)
{
    double diffEnergy[IP_TYPE_END]={};

    for(int i=0; i<num_cpus; i++) {
        double optimalFreq = gemdroid_core[i].getOptCoreFreq();
        double optimalEnergy = gemdroid_core[i].getEnergyEst(lastTimeTook[IP_TYPE_CPU], lastPowerTook[IP_TYPE_CPU], optimalFreq);

        diffEnergy[IP_TYPE_CPU] = lastEnergyTook[IP_TYPE_CPU] - optimalEnergy;
        if(diffEnergy[IP_TYPE_CPU] <= 0)
            diffEnergy[IP_TYPE_CPU] = 0.01;

        // cout << "DVFS. Slack CPU" << i << " @ " << gemdroid_core[i].getCoreFreq() << " " << lastEnergyTook[IP_TYPE_CPU] << " " << optimalEnergy << " " << diffEnergy[IP_TYPE_CPU] << endl;
        cout << "DVFS. Slack CPU" << i << " @ " << gemdroid_core[i].getCoreFreq() << ": " << diffEnergy[IP_TYPE_CPU] << endl;
    }

    int ips[MAX_IPS_IN_FLOW];
    int i = -1, j = 0;
    getIPsInFlow(0, 0, ips); //core_id = 0
    // for(int i=IP_TYPE_VD; i<=IP_TYPE_GPU; i++) {
    while ((i = ips[j++]) != -1) {
        if (lastEnergyTook[i] != 0 && i >= IP_TYPE_VD) {
            diffEnergy[i] = lastEnergyTook[i] - optimal_energy[i];
            if(diffEnergy[i] <= 0)
                diffEnergy[i] = 0.01;

            if(i == IP_TYPE_AD || i == IP_TYPE_AE) {
            	diffEnergy[i] *= AD_AE_RATIO_TO_VD;		//44000Hz audio sounds in 1 sec, with 60FPS.
            }

	        GemDroidIP *inst = getIPInstance(i);
            // cout << "DVFS. Slack " << ipTypeToString(i) << " @ " << inst->getIPFreq() << " " << lastEnergyTook[i] << " " << optimal_energy[i] << " " << diffEnergy[i] << endl;
            cout << "DVFS. Slack " << ipTypeToString(i) << " @ " << inst->getIPFreq() << ": " << diffEnergy[i] << endl;
        }
    }
    cout << "DVFS. Mem" << " @ " << gemdroid_memory.getMemFreq() << endl;

    bool doNextDVFS = dvfsMemory(slack);

    if (slack > 0 && !doNextDVFS)
        return;

    if (slack > 0) {
        int max_index=findMax(diffEnergy, IP_TYPE_END);
        while(max_index > -1 && diffEnergy[max_index] > 0){		//should be a valid index, as well as the difference_energy value should be more than 0.
        	//IP with maximum energy difference is in max_index.
	    	diffEnergy[max_index] = 0;
            setDVFSSlackFrequencies(slack, max_index);
	    	max_index=findMax(diffEnergy, IP_TYPE_END);		
        }
    }
    else {
        int min_index=findMin(diffEnergy, IP_TYPE_END);
        while(min_index > -1 && diffEnergy[min_index] > 0) {		//should be a valid index, as well as the difference_energy value should be more than 0.
        	//IP with minimum energy difference is in min_index.
	    	diffEnergy[min_index] = 0;
            setDVFSSlackFrequencies(slack, min_index);
	    	min_index=findMin(diffEnergy, IP_TYPE_END);		
            if (slack > 0)
                return;
        }
    }
}

void GemDroid::dvfsMaxEnergyConsumer(double slack)
{
    double lastEnergy[IP_TYPE_END]={};

    for(int i=0; i<num_cpus; i++) {
        lastEnergy[IP_TYPE_CPU] = lastEnergyTook[IP_TYPE_CPU];
        if(lastEnergy[IP_TYPE_CPU] <= 0)
            lastEnergy[IP_TYPE_CPU] = 0.01;

        cout << "DVFS. Slack CPU" << i << " @ " << gemdroid_core[i].getCoreFreq() << ": " << lastEnergy[IP_TYPE_CPU] << endl;
    }

    int ips[MAX_IPS_IN_FLOW];
    int i = -1, j = 0;
    getIPsInFlow(0, 0, ips); //core_id = 0
    // for(int i=IP_TYPE_VD; i<=IP_TYPE_GPU; i++) {
    while ((i = ips[j++]) != -1) {
        if (lastEnergyTook[i] != 0 && i >= IP_TYPE_VD) {
            lastEnergy[i] = lastEnergyTook[i];

            if(i == IP_TYPE_AD || i == IP_TYPE_AE) {
            	lastEnergy[i] *= AD_AE_RATIO_TO_VD;		//44000Hz audio sounds in 1 sec, with 60FPS.
            }
            if(lastEnergy[i] <= 0)
                lastEnergy[i] = 0.01;

	        GemDroidIP *inst = getIPInstance(i);
            cout << "DVFS. Slack " << ipTypeToString(i) << " @ " << inst->getIPFreq() << ": " << lastEnergy[i] << endl;
        }
    }
    cout << "DVFS. Mem" << " @ " << gemdroid_memory.getMemFreq() << endl;

    bool doNextDVFS = dvfsMemory(slack);

    if (slack > 0 && !doNextDVFS)
        return;

    if (slack > 0) {
        int max_index=findMax(lastEnergy,IP_TYPE_END);
        while(max_index > -1 && lastEnergy[max_index] > 0.000005) {		//should be a valid index, as well as the difference_energy value should be more than 0.
        	//cout<<"DVFS. Setting freq for IP:"<<ipTypeToString(max_index)<<endl;
	    	lastEnergy[max_index] = 0;
            setDVFSSlackFrequencies(slack, max_index);
	    	max_index=findMax(lastEnergy, IP_TYPE_END);
        }
    }
    else {
        int min_index=findMin(lastEnergy, IP_TYPE_END);
        while(min_index > -1 && lastEnergy[min_index] > 0){		//should be a valid index, as well as the difference_energy value should be more than 0.
        	//IP with minimum energy difference is in min_index.
	    	lastEnergy[min_index] = 0;
            setDVFSSlackFrequencies(slack, min_index);
	    	min_index=findMin(lastEnergy, IP_TYPE_END);		
            if (slack > 0)
                return;
        }
    }
}

void GemDroid::dvfsMaxPowerConsumer(double slack)
{
    double lastPower[IP_TYPE_END]={};

    for(int i=0; i<num_cpus; i++) {
        lastPower[IP_TYPE_CPU] = lastPowerTook[IP_TYPE_CPU];

        cout << "DVFS. Slack CPU" << i << " @ " << gemdroid_core[i].getCoreFreq() << " " << lastEnergyTook[IP_TYPE_CPU] << " " << lastPower[IP_TYPE_CPU] << endl;
    }

    int ips[MAX_IPS_IN_FLOW];
    int i = -1, j = 0;
    getIPsInFlow(0, 0, ips); //core_id = 0
    // for(int i=IP_TYPE_VD; i<=IP_TYPE_GPU; i++) {
    while ((i = ips[j++]) != -1) {
        if (lastEnergyTook[i] != 0 && i >= IP_TYPE_VD) {
            lastPower[i] = lastPowerTook[i];

	        GemDroidIP *inst = getIPInstance(i);
            cout << "DVFS. Slack " << ipTypeToString(i) << " @ " << inst->getIPFreq() << " " << lastEnergyTook[i] << " " << lastPower[i] << endl;
        }
    }

    cout << "DVFS. Mem" << " @ " << gemdroid_memory.getMemFreq() << endl;

    bool doNextDVFS = dvfsMemory(slack);

    if (slack > 0 && !doNextDVFS)
        return;

    if (slack > 0) {
        int max_index=findMax(lastPower,IP_TYPE_END);
        while(max_index > -1 && lastPower[max_index] > EPSILON)		//should be a valid index, as well as the difference_energy value should be more than 0.
        {
        	//IP with maximum energy consumed is in max_index.
        	//cout<<"DVFS. Setting freq for IP:"<<ipTypeToString(max_index)<<endl;
	    	lastPower[max_index] = 0;
            setDVFSSlackFrequencies(slack, max_index);
            max_index=findMax(lastPower, IP_TYPE_END);		
        }
    }
    else {
        int min_index=findMin(lastPower, IP_TYPE_END);
        while(min_index > -1 && lastPower[min_index] > 0) {		//should be a valid index, as well as the difference_energy value should be more than 0.
	    	lastPower[min_index] = 0;
            setDVFSSlackFrequencies(slack, min_index);
	    	min_index=findMin(lastPower, IP_TYPE_END);		
            if (slack > 0)
                return;
        }
    }
}

void GemDroid::dvfsMaxEnergyPerTimeConsumer(double slack)
{
    double lastEnergyPerTime[IP_TYPE_END]={};
    double diffEnergy[IP_TYPE_END]={};
    double diffTime[IP_TYPE_END]={};
    int ips[MAX_IPS_IN_FLOW];
    int i = -1, j = 0;

    getIPsInFlow(0, 0, ips); //core_id = 0
    while ((i = ips[j++]) != -1) {
        if (lastEnergyTook[i] != 0 && i == IP_TYPE_CPU) {
            double optimalFreq = gemdroid_core[0].getOptCoreFreq();
            double optimalEnergy = gemdroid_core[0].getEnergyEst(lastTimeTook[IP_TYPE_CPU], lastPowerTook[IP_TYPE_CPU], optimalFreq);
            double optimalTime = gemdroid_core[0].getTimeEst(lastTimeTook[IP_TYPE_CPU], gemdroid_core[0].getCoreFreq(), optimalFreq);
    
            lastEnergyPerTime[IP_TYPE_CPU] = (lastEnergyTook[IP_TYPE_CPU] - optimalEnergy) / (lastTimeTook[IP_TYPE_CPU] - optimalTime);
            if (lastTimeTook[IP_TYPE_CPU] - optimalTime == 0)
                lastEnergyPerTime[IP_TYPE_CPU] = 100;
            else if (lastEnergyPerTime[IP_TYPE_CPU] < 0)
                lastEnergyPerTime[IP_TYPE_CPU] *= -1;
    
            cout << "DVFS. CPU" << " @ " << gemdroid_core[0].getCoreFreq() << ": " << lastEnergyPerTime[IP_TYPE_CPU] << endl;
        }
        else if (lastEnergyTook[i] != 0 && i >= IP_TYPE_VD) {
	        GemDroidIP *inst = getIPInstance(i);

            diffEnergy[i] = lastEnergyTook[i] - optimal_energy[i];
            // diffTime[i] = ip_time_table[i][optimal_freqs[i]] - lastTimeTook[i];
            diffTime[i] = lastTimeTook[i] - ip_time_table[i][inst->getOptIPFreqInd()];
            if(diffEnergy[i] < 0)
                diffEnergy[i] = 0.01;
            if(diffTime[i] < 0)
                diffTime[i] = 0.01;

            lastEnergyPerTime[i] = diffEnergy[i] / diffTime[i];

            if(i == IP_TYPE_AD || i == IP_TYPE_AE) {
            	lastEnergyPerTime[i] *= AD_AE_RATIO_TO_VD;		//44000Hz audio sounds in 1 sec, with 60FPS.
            }

            cout << "DVFS. " << ipTypeToString(i) << " @ " << inst->getIPFreq() << " " << lastEnergyPerTime[i] << endl;
        }
    }
    cout << "DVFS. Mem" << " @ " << gemdroid_memory.getMemFreq() << endl;

    bool doNextDVFS = dvfsMemory(slack);

    if (slack > 0 && !doNextDVFS)
        return;

    if (slack > 0) {
        int max_index=findMax(lastEnergyPerTime, IP_TYPE_END);
        while(max_index > -1 && lastEnergyPerTime[max_index] > 0) {		//should be a valid index, as well as the difference_energy value should be more than 0.
        	//IP with maximum energy difference is in max_index.
	    	lastEnergyPerTime[max_index] = 0;
            setDVFSSlackFrequencies(slack, max_index);
	    	max_index=findMax(lastEnergyPerTime, IP_TYPE_END);		
        }
    }
    else {
        int min_index=findMin(lastEnergyPerTime, IP_TYPE_END);
        while(min_index > -1 && lastEnergyPerTime[min_index] > 0){		//should be a valid index, as well as the difference_energy value should be more than 0.
	    	lastEnergyPerTime[min_index] = 0;
            setDVFSSlackFrequencies(slack, min_index);
	    	min_index=findMin(lastEnergyPerTime, IP_TYPE_END);		
            if (slack > 0)
                return;
        }
    }
}

void GemDroid::dvfsGreedyKnapsack(double slack)
{
    double lastEnergyPerTime[IP_TYPE_END]={};
    double diffEnergy[IP_TYPE_END]={};
    double diffTime[IP_TYPE_END]={};

    int ips[MAX_IPS_IN_FLOW];
    int i = -1, j = 0;
    getIPsInFlow(0, 0, ips); //core_id = 0
    while ((i = ips[j++]) != -1) {
        if (lastEnergyTook[i] != 0 && i == IP_TYPE_CPU) {
            double optimalFreq = gemdroid_core[0].getOptCoreFreq();
            double optimalEnergy = gemdroid_core[0].getEnergyEst(lastTimeTook[IP_TYPE_CPU], lastPowerTook[IP_TYPE_CPU], optimalFreq);
            double optimalTime = gemdroid_core[0].getTimeEst(lastTimeTook[IP_TYPE_CPU], gemdroid_core[0].getCoreFreq(), optimalFreq);
    
            lastEnergyPerTime[IP_TYPE_CPU] = (lastEnergyTook[IP_TYPE_CPU] - optimalEnergy) / (lastTimeTook[IP_TYPE_CPU] - optimalTime);
            if (lastTimeTook[IP_TYPE_CPU] - optimalTime == 0)
                lastEnergyPerTime[IP_TYPE_CPU] = 100;
            else if (lastEnergyPerTime[IP_TYPE_CPU] < 0)
                lastEnergyPerTime[IP_TYPE_CPU] *= -1;
    
            cout << "DVFS. CPU" << " @ " << gemdroid_core[0].getCoreFreq() << ": " << lastEnergyPerTime[IP_TYPE_CPU] << endl;
        }
        else if (lastEnergyTook[i] != 0 && i >= IP_TYPE_VD) {
	        GemDroidIP *inst = getIPInstance(i);

            diffEnergy[i] = lastEnergyTook[i] - optimal_energy[i];
            // diffTime[i] = ip_time_table[i][optimal_freqs[i]] - lastTimeTook[i];
            diffTime[i] = lastTimeTook[i] - ip_time_table[i][inst->getOptIPFreqInd()];
            if(diffEnergy[i] < 0)
                diffEnergy[i] = 0.01;
            if(diffTime[i] < 0)
                diffTime[i] = 0.01;

            lastEnergyPerTime[i] = diffEnergy[i] / diffTime[i];

            if(i == IP_TYPE_AD || i == IP_TYPE_AE) {
            	lastEnergyPerTime[i] *= AD_AE_RATIO_TO_VD;		//44000Hz audio sounds in 1 sec, with 60FPS.
            }

            cout << "DVFS. " << ipTypeToString(i) << " @ " << inst->getIPFreq() << " " << lastEnergyPerTime[i] << endl;
        }
    }
    cout << "DVFS. Mem" << " @ " << gemdroid_memory.getMemFreq() << endl;

    double lastMemFreq = gemdroid_memory.getMemFreq();

    if(slack < 0) {
        int min_index=findMin(lastEnergyPerTime, IP_TYPE_END);
        while(min_index > -1 && lastEnergyPerTime[min_index] > 0){      //should be a valid index, as well as the difference_energy value should be more than 0.
            lastEnergyPerTime[min_index] = 0;
            setDVFSSlackFrequencies(slack, min_index);
            min_index=findMin(lastEnergyPerTime, IP_TYPE_END);      
            if (slack > 0)
                break;
        }
        if (slack < 0)
            dvfsMemory(slack);
        return;
    }

    dvfsMemory(slack);

    if (fabs(gemdroid_memory.getMemFreq() - MIN_MEM_FREQ) < EPSILON && slack > 0) {
        int max_index=findMax(lastEnergyPerTime, IP_TYPE_END);
        while(max_index > -1 && lastEnergyPerTime[max_index] > 0) {		//should be a valid index, as well as the difference_energy value should be more than 0.
        	//IP with maximum energy difference is in max_index.
	    	lastEnergyPerTime[max_index] = 0;
            setDVFSSlackFrequencies(slack, max_index);
	    	max_index=findMax(lastEnergyPerTime, IP_TYPE_END);		
        }

        return;
    }

    while (gemdroid_memory.getMemFreq() > MIN_MEM_FREQ+EPSILON) {
        //While slack > 0,
        //Find max, and go as much as possible towards optimal, SLACK PERMITTING!
        //Next, go to the min guy and request him (with a please) if he can go to a higher freq.

        bool reducedMem = false;
        double last_memory_energy = (FPS_DEADLINE+FPS_DEADLINE_SAFETYNET) * gemdroid_memory.powerIn1ms();
        double curr_memory_energy = gemdroid_memory.getEnergyEst(lastMemFreq, last_memory_energy, gemdroid_memory.getMemFreq());
        double next_memory_energy = gemdroid_memory.getEnergyEst(lastMemFreq, last_memory_energy, gemdroid_memory.getMemFreq()-0.1);
        double memory_energy_saving = curr_memory_energy - next_memory_energy;
        double curr_mem_freq = gemdroid_memory.getMemFreq();
        double next_mem_freq = gemdroid_memory.getMemFreq() - 0.1;
        double extra_cpu_time = 0, extra_ip_time = 0;

        int ips[MAX_IPS_IN_FLOW];
        int i = -1, j = 0;
        getIPsInFlow(0, 0, ips); //core_id = 0
        while ((i = ips[j++]) != -1) {
            if (i == IP_TYPE_CPU) {
                double curr_cpu_time = memScaledTimeCPU(0, lastMemFreq, curr_mem_freq);
                double next_cpu_time = memScaledTimeCPU(0, lastMemFreq, next_mem_freq);
                extra_cpu_time += next_cpu_time - curr_cpu_time;
				assert (extra_cpu_time >= 0);
            }
            else {
                GemDroidIP *inst = getIPInstance(i);
                double curr_ip_time = memScaledTime(i, 0, gemdroid_memory.getMaxBandwidth(curr_mem_freq), inst->getIPFreqInd());
                double next_ip_time = memScaledTime(i, 0, gemdroid_memory.getMaxBandwidth(next_mem_freq), inst->getIPFreqInd());
                extra_ip_time += next_ip_time - curr_ip_time;
				assert (extra_ip_time >= 0);
            }
        }
        double memory_extra_time = extra_cpu_time + extra_ip_time - slack;
		// assert (memory_extra_time >= 0);

        cout << "DVFS Mem energy_saving: " << memory_energy_saving << " extra_time: " << memory_extra_time << endl;

        int min_index=findMin(lastEnergyPerTime, IP_TYPE_END);
        if(min_index == IP_TYPE_CPU) {
            int currFreqInd = gemdroid_core[dvfs_core].getCoreFreqInd();
            double currEnergy = lastEnergyTook[min_index];
            double currTime = memScaledTimeCPU(0, lastMemFreq, gemdroid_memory.getMemFreq());
            double diffEnergyToNextStep;
            double diffTimeToNextStep;

            for(i=currFreqInd+1; i<=gemdroid_core[dvfs_core].getMaxCoreFreqInd(); i++) {
                double nextEnergy = gemdroid_core[dvfs_core].getEnergyEst(lastTimeTook[min_index], lastPowerTook[min_index], gemdroid_core[dvfs_core].getCoreFreq(i));
                diffEnergyToNextStep = nextEnergy - currEnergy;

                double nextTime = gemdroid_core[dvfs_core].getTimeEst(currTime, lastFrequency[IP_TYPE_CPU], gemdroid_core[dvfs_core].getCoreFreq(i));
                diffTimeToNextStep = currTime - nextTime;
                if(diffTimeToNextStep < 0) {
                    cout<<"Core Time estimate is incorrect\n";
                    assert(0);
                }

				cout << "DVFS CPU @ " << gemdroid_core[dvfs_core].getCoreFreq(i) << " diffEnergy: " << diffEnergyToNextStep << " diffTime: " << diffTimeToNextStep << endl;

                if (diffEnergyToNextStep > memory_energy_saving)
                    break;

                if (diffTimeToNextStep > memory_extra_time && diffEnergyToNextStep < memory_energy_saving) {
                    gemdroid_memory.decMemFreq();
                    gemdroid_core[dvfs_core].setCoreFreqInd(i);
                    cout << "DVFS Setting mem to " << gemdroid_memory.getMemFreq() << " and Core to " << gemdroid_core[dvfs_core].getCoreFreq() << endl;
                    reducedMem = true;
                    break;
                }
            }
        }
        else if (min_index != -99999) { // min is IP
            GemDroidIP *inst = getIPInstance(min_index);

            int currFreqInd = inst->getIPFreqInd();
            double currEnergy = lastEnergyTook[min_index];
            double currTime = memScaledTime(min_index, 0, gemdroid_memory.getMaxBandwidth(), currFreqInd);
            double diffEnergyToNextStep;
            double diffTimeToNextStep;

            for(i=currFreqInd+1; i<=inst->getMaxIPFreqInd(); i++) {
                double nextEnergy = inst->getEnergyEst(lastTimeTook[min_index], lastPowerTook[min_index], inst->getIPFreq(i));
                diffEnergyToNextStep = nextEnergy - currEnergy;

                double nextTime = inst->getTimeEst(currTime, lastFrequency[min_index], inst->getIPFreq(i));
                diffTimeToNextStep = currTime - nextTime;
                if(diffTimeToNextStep < 0) {
                    cout<<"IP Time estimate is incorrect\n";
                    assert(0);
                }

				cout << "DVFS IP " << ipTypeToString(min_index) << " @ " << inst->getIPFreq(i) << " diffEnergy: " << diffEnergyToNextStep << " diffTime: " << diffTimeToNextStep << endl;

                if (diffEnergyToNextStep > memory_energy_saving)
                    break;

                if (diffTimeToNextStep > memory_extra_time && diffEnergyToNextStep < memory_energy_saving) {
                    gemdroid_memory.decMemFreq();
                    inst->setIPFreqInd(i);
                    cout << "DVFS Setting mem to " << gemdroid_memory.getMemFreq() << " and IP " << ipTypeToString(min_index) << " to " << inst->getIPFreq() << endl;
                    reducedMem = true;
                    break;
                }
            }
        }
        if (reducedMem == false)
            break;
    }
}

void GemDroid::dvfsDynamicProg(double slack)
{
    int ip_accs[MAX_IPS_IN_FLOW];
    int ip_devs[MAX_IPS_IN_FLOW];
    int num_accs = getIPAccsInFlow(0, 0, ip_accs); //core_id = 0
    int num_devs = getIPDevsInFlow(0, 0, ip_devs); //core_id = 0

    if (num_accs == 1) {
        dvfsDynamicProg1(slack, ip_accs, num_accs, ip_devs, num_devs);
    }
    else if (num_accs == 2) {
        dvfsDynamicProg2(slack, ip_accs, num_accs, ip_devs, num_devs);
    }
    else {
        cout << "FATAL: dvfsDynamicProg(): num_ips < 2 or > 4" << endl;
        assert(0);
    }
}

void GemDroid::dvfsDynamicProg1(double slack, int ip_accs[MAX_IPS_IN_FLOW], int num_accs, int ip_devs[MAX_IPS_IN_FLOW], int num_devs)
{
    int ip_type = ip_accs[0];
    double time = 0, energy = 0, minEnergy = 9999;
    GemDroidIP *inst = getIPInstance(ip_type);
    double mem_freq_max = gemdroid_memory.getMaxMemFreq();
    double mem_freq_min = gemdroid_memory.getMinMemFreq();
    mem_freq_min = gemdroid_memory.getMinMemFreq() + 0.3;
    double ip_freq_max = inst->getMaxIPFreqInd();
    // double ip_freq_min = inst->getOptIPFreqInd();
    double ip_freq_min = inst->getIPFreqInd()-2;
    int ip_freq = ip_freq_max;
    double mem_freq = gemdroid_memory.getMaxMemFreq();
    double mem = gemdroid_memory.getOptMemFreq();
    double ip_time = 0, ip_energy = 0;
    double memory_energy;

    cout << "DVFS " << "lastTimeTook dev1: " << lastTimeTook[ip_devs[0]] << " dev2: " << lastTimeTook[ip_devs[1]] << " IP: " << lastTimeTook[ip_type] << " @ " << lastFrequency[ip_type] << " and Mem @ " << gemdroid_memory.getMemFreq() << endl;

    if (lastTimeTook[ip_type] == 0)
        return;

    for (mem=mem_freq_max; mem>=mem_freq_min; mem-=0.1) {
        double dev_time = 0;
        double dev_energy = 0;
        double last_memory_energy = (FPS_DEADLINE+FPS_DEADLINE_SAFETYNET) * gemdroid_memory.powerIn1ms();
        memory_energy = gemdroid_memory.getEnergyEst(gemdroid_memory.getMemFreq(), last_memory_energy, mem);

        cout << "DVFS " << "Mem Energy @ " << mem << " : " << memory_energy << endl;

        for(int i=0; i<num_devs; i++) {
            // dev_time += lastTimeTook[ip_devs[i]];
            dev_time += memScaledTime(ip_devs[i], 0, gemdroid_memory.getMaxBandwidth(mem), gemdroid_ip_dc[0].getIPFreqInd());
            dev_energy += lastEnergyTook[ip_devs[i]];
        }
        cout << "DVFS  Mem @ " << mem << " - Dev Time: " << dev_time << " Dev Energy: " << dev_energy << endl;

        if (dev_time > FPS_DEADLINE)
            break;

        double ip_base_time = memScaledTime(ip_type, 0, gemdroid_memory.getMaxBandwidth(mem), inst->getIPFreqInd());
        if (mem < lastMemFreq)
            ip_base_time += ip_base_time*0.1*(lastMemFreq - mem);
        //double ip_base_energy = lastEnergyTook[ip_type];
        cout << "DVFS     IP BaseTime IP_Freq: " << lastFrequency[ip_type] << " " << ip_base_time << endl;

        for (int ip = ip_freq_max; ip >= ip_freq_min; ip--) {
            ip_time = inst->getTimeEst(ip_base_time, lastFrequency[ip_type], inst->getIPFreq(ip));
            ip_energy = inst->getEnergyEst(ip_base_time, lastPowerTook[ip_type], inst->getIPFreq(ip));
            // ip_energy = inst->getEnergyEst(lastTimeTook[ip_type], lastPowerTook[ip_type], inst->getIPFreq(ip));

            time = dev_time + ip_time;
            energy = memory_energy + dev_energy + ip_energy;

            cout << "DVFS      IP_Freq:" << inst->getIPFreq(ip) << " IP Time: " << ip_time << " IP Energy: " << ip_energy << endl;
        
            // cout << "DVFS       Mem " << mem << " @ " << inst->getIPFreq(ip) << " Time: " << time << " Energy: " << energy << endl;

            if (time > FPS_DEADLINE)
                break;
        
            if (energy <= minEnergy) {
                minEnergy = energy;
                mem_freq = mem;
                ip_freq = ip;
                cout << "DVFS " << "Updating min energy value " << mem_freq << " " << inst->getIPFreq(ip_freq) << endl;
            }
        }
    }

    if (app_id[0] == APP_ID_ANGRYBIRDS || app_id[0] == APP_ID_MPGAME) {
        mem_freq = 0.5;
        ip_freq = 0.3;
    }

    cout << "DVFS Min Energy " << minEnergy << " is at " << mem_freq << " " << inst->getIPFreq(ip_freq) << endl;

    gemdroid_memory.setMemFreq(mem_freq);
    inst->setIPFreqInd(ip_freq);
}

void GemDroid::dvfsDynamicProg2(double slack, int ip_accs[MAX_IPS_IN_FLOW], int num_accs, int ip_devs[MAX_IPS_IN_FLOW], int num_devs)
{
    int ip_type = ip_accs[1];
    double time = 0, energy = 0, minEnergy = 9999;
    GemDroidIP *inst = getIPInstance(ip_type);
    double mem_freq_max = gemdroid_memory.getMaxMemFreq();
    double mem_freq_min = gemdroid_memory.getMinMemFreq();
    double cpu_freq_max = gemdroid_core[0].getMaxCoreFreqInd();
    // double cpu_freq_min = gemdroid_core[0].getOptCoreFreqInd();
    double cpu_freq_min = gemdroid_core[0].getCoreFreqInd()-2;
    double ip_freq_max = inst->getMaxIPFreqInd();
    // double ip_freq_min = inst->getOptIPFreqInd();
    double ip_freq_min = inst->getIPFreqInd()-2;
    int cpu_freq = cpu_freq_max;
    int ip_freq = ip_freq_max;
    double mem_freq = gemdroid_memory.getMaxMemFreq();
    double mem = gemdroid_memory.getOptMemFreq();
    double cpu_time = 0, cpu_energy = 0;
    double ip_time = 0, ip_energy = 0;
    double memory_energy;

    cout << "DVFS " << "lastTimeTook dev1: " << lastTimeTook[ip_devs[0]] << " dev2: " << lastTimeTook[ip_devs[1]] << " CPU: " << lastTimeTook[IP_TYPE_CPU] << " IP: " << lastTimeTook[ip_type] << endl;
    cout << "DVFS " << "lastEnergyTook dev1: " << lastEnergyTook[ip_devs[0]] << " dev2: " << lastEnergyTook[ip_devs[1]] << " CPU: " << lastEnergyTook[IP_TYPE_CPU] << " IP: " << lastEnergyTook[ip_type] << endl;

    if (lastTimeTook[ip_type] == 0 || lastTimeTook[IP_TYPE_CPU] == 0)
        return;

    for (mem=mem_freq_max; mem>=mem_freq_min; mem-=0.1) {
        double dev_time = 0;
        double dev_energy = 0;
        double last_memory_energy = (FPS_DEADLINE+FPS_DEADLINE_SAFETYNET) * gemdroid_memory.powerIn1ms();
        memory_energy = gemdroid_memory.getEnergyEst(gemdroid_memory.getMemFreq(), last_memory_energy, mem);

        cout << "DVFS " << "Mem Energy @ " << mem << " : " << memory_energy << endl;

        for(int i=0; i<num_devs; i++) {
            // dev_time += lastTimeTook[ip_devs[i]];
            dev_time += memScaledTime(ip_devs[i], 0, gemdroid_memory.getMaxBandwidth(mem), gemdroid_ip_dc[0].getIPFreqInd());
            dev_energy += lastEnergyTook[ip_devs[i]];
        }
        cout << "DVFS  Mem @ " << mem << " - Dev Time: " << dev_time << " Dev Energy: " << dev_energy << endl;

        if (dev_time > FPS_DEADLINE)
            break;

        double cpu_base_time = memScaledTimeCPU(0, gemdroid_memory.getMemFreq(), mem);
        // double cpu_base_energy = lastEnergyTook[IP_TYPE_CPU];
//        double cpu_base_energy = gemdroid_core[0].getEnergyEst(cpu_base_time, lastPowerTook[IP_TYPE_CPU], gemdroid_core[0].getCoreFreq());
        cout << "DVFS   CPU BaseTime CPU_Freq: " << lastFrequency[IP_TYPE_CPU] << " " << cpu_base_time << endl;

        for (int cpu = cpu_freq_max; cpu >= cpu_freq_min; cpu--) {
            cpu_time = gemdroid_core[0].getTimeEst(cpu_base_time, lastFrequency[IP_TYPE_CPU], gemdroid_core[0].getCoreFreq(cpu));
            cpu_energy = gemdroid_core[0].getEnergyEst(cpu_base_time, lastPowerTook[IP_TYPE_CPU], gemdroid_core[0].getCoreFreq(cpu));

            cout << "DVFS    CPU_Freq:" << gemdroid_core[0].getCoreFreq(cpu) << " CPU Time: " << cpu_time << " CPU Energy: " << cpu_energy << endl;
         
            if (cpu_time + dev_time > FPS_DEADLINE)
                break;

            double ip_base_time = memScaledTime(ip_type, 0, gemdroid_memory.getMaxBandwidth(mem), inst->getIPFreqInd());
            if (mem < lastMemFreq)
                ip_base_time += ip_base_time*0.1*(lastMemFreq - mem);
            // double ip_base_energy = lastEnergyTook[ip_type];
            cout << "DVFS     IP BaseTime IP_Freq: " << ip_base_time <<  " @ " << lastFrequency[ip_type] << endl;

            for (int ip = ip_freq_max; ip >= ip_freq_min; ip--) {
                ip_time = inst->getTimeEst(ip_base_time, lastFrequency[ip_type], inst->getIPFreq(ip));
                ip_energy = inst->getEnergyEst(ip_base_time, lastPowerTook[ip_type], inst->getIPFreq(ip));

                time = dev_time + cpu_time + ip_time;
                energy = memory_energy + dev_energy + cpu_energy + ip_energy;

                cout << "DVFS      IP_Freq:" << inst->getIPFreq(ip) << " IP Time: " << ip_time << " IP Energy: " << ip_energy << endl;
        
                cout << "DVFS       Mem " << mem << " @ " << gemdroid_core[0].getCoreFreq(cpu) << " " << inst->getIPFreq(ip) << " Time: " << time << " Energy: " << energy << endl;

                if (time > FPS_DEADLINE)
                    break;
        
                if (energy < minEnergy) {
                    minEnergy = energy;
                    mem_freq = mem;
                    cpu_freq = cpu;
                    ip_freq = ip;
                    cout << "DVFS " << "Updating min energy value " << mem_freq << " " << gemdroid_core[0].getCoreFreq(cpu_freq) << " " << inst->getIPFreq(ip_freq) << endl;
                }
            }
        }
    }

    cout << "DVFS Min Energy " << minEnergy << " is at " << mem_freq << " " << gemdroid_core[0].getCoreFreq(cpu_freq) << " " << inst->getIPFreq(ip_freq) << endl;

    gemdroid_memory.setMemFreq(mem_freq);
    gemdroid_core[0].setCoreFreqInd(cpu_freq);
    inst->setIPFreqInd(ip_freq);
}

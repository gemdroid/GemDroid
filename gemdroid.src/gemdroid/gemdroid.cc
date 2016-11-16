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
#include "gemdroid/gemdroid.hh"

#include <cstdlib>
#include <ctime>

bool true_fetch;
bool gemdroid_enable;

using namespace std;

//Given an IP type as int, returns a string with IP name.
string ipTypeToString(int type)
{
	switch(type){
	case IP_TYPE_CPU:
		return "CPU";
	case IP_TYPE_DC:
		return "DC";
	case IP_TYPE_NW:
		return "NW";
	case IP_TYPE_SND:
		return "SND";
	case IP_TYPE_MIC:
		return "MIC";
	case IP_TYPE_CAM:
		return "CAM";
	case IP_TYPE_MMC_IN:
		return "MMC_IN";
	case IP_TYPE_MMC_OUT:
		return "MMC_OUT";
	case IP_TYPE_VD:
		return "VD";
	case IP_TYPE_VE:
		return "VE";
	case IP_TYPE_AD:
		return "AD";
	case IP_TYPE_AE:
		return "AE";
	case IP_TYPE_IMG:
		return "IMG";
	case IP_TYPE_GPU:
		return "GPU";
	case IP_TYPE_DMA:
		return "DMA";
	case IP_TYPE_CACHE:
		return "CACHE";
	default:
		return "UNKNOWN";
	}
}

//Given PState (power-state) as int, returns PState string.
string pStateToString(int pstate)
{
	if (pstate == ip_idle)
		return "IDLE";
	else if (pstate == ip_lowpower)
		return "LOWPOWER";
	else if (pstate == ip_active)
		return "ACTIVE";
	else
		return "UNKNOWN";
}

string flowIdToString(int flowId) 
{
	switch(flowId)
	{
		case 1: return "VD_DC_1";
		case 2: return "AD_SND_2";
		case 3: return "CAM_IMG_DC_3";
		case 4: return "CAM_VE_MMC_4";
		case 5: return "MIC_AE_MMC_5";
		case 6: return "CAM_IMG_DC_6";
		case 7: return "CAM_IMG_MMC_7";
		case 8: return "MMC_AD_SND_8";
		case 9: return "CAM_IMG_DC_9";
		case 10: return "AD_SND_10";
		case 11: return "MMC_IMG_DC_11";
		case 12: return "MIC_AE_MMC_12";
		case 13: return "MMC_VD_DC_13";
		case 14: return "MMC_AD_SND_14";
		case 15: return "GPU_DC1_15";
		case 16: return "CAM_VE_16";
		case 17: return "VD_DC_17";
		case 18: return "MIC_AE_18";
		case 19: return "AD_SND_19";
		case 20: return "GPU_DC1_20";
		default: return "UNKNOWN";
	}
}

bool GemDroid::has2ndFlow(int appID)
{
    if((appID == APP_ID_YOUTUBE) ||\
         (appID == APP_ID_PHOTOCAPTURE) ||\
         (appID == APP_ID_AUDIOPLAY) ||\
         (appID == APP_ID_GALLERY) ||\
         (appID == APP_ID_AUDIORECORD) ||\
         (appID == APP_ID_VIDPLAYER) ||\
         (appID == APP_ID_OTHER) )
        return false;
    else
        return true;
}


GemDroid::GemDroid(const Params *p):
		ClockedObject(p),
		tickEvent(this),
		gemdroid_memory(0, p->deviceConfigFile, p->systemConfigFile, p->filePath,
		            p->traceFile, p->range.size() / 1024 / 1024, p->perfect_memory, p->enableDebug, this),
		gemdroid_sa(0, this)
{
    ticks = 0;
    desc = "GemDroid";

	gemdroid_enable = p->enable_gemdroid;
	std::cout << "Gemdroid Enable: " << gemdroid_enable << std::endl;

	if(!gemdroid_enable) {
		// If Gemdroid not enabled simulation should continue with original benchmark
		true_fetch = true;
		return;
	}
    framenum_motivationgraphs = 0;

	perfectMemory = p->perfect_memory;
    powerCalcLastTick = 0;
    periodicStatsLastTick = 0;
    slackLastTick = 0;
    dvfsLastTick = 0;
    memLastTick = 0;
    framesToBeShown = 0;

	num_cpus = p->num_cpu_traces;
	core_freq = p->core_freq;
	ip_freq = p->ip_freq;
	mem_freq = p->mem_freq;
    governor = p->governor;
    governor_timing = p->governor_timing;
	num_ip_inst = p->num_ip_instances;
    em_trace_file_name[0] = p->cpu_trace1;
    em_trace_file_name[1] = p->cpu_trace2;
    em_trace_file_name[2] = p->cpu_trace3;
    em_trace_file_name[3] = p->cpu_trace4;

    em_gputrace_file_name = p->gpu_trace;
    isPrintPeriodicStats = !(p->no_periodic_stats);
    isPrintPeriodicStatsPower = false;
    // cout << "Core Freq set as " << core_freq << endl;
    // cout << "IP ACC Freq set as " << ip_freq << endl;

	sweep_val1 = p->sweep_val1;
	sweep_val2 = p->sweep_val2;
    cout << "Sweep Value 1 set is " << getSweepVal1() << "." << endl;
    cout << "Sweep Value 2 set is " << getSweepVal2() << ". Currently used for IP_TYPE in motivation_tick." << endl; 

    //GemDroid Memory
    gemdroid_memory.setMemFreq(mem_freq/1000.0);  //param in Ghz

    if(num_cpus)
    	true_fetch = true;

	for(int i=0; i<IP_TYPE_END; i++) {
		for(int j=0; j<MAX_IPS; j++) {
            ipLastTick[i][j] = 0;
			frameStarted[i][j] = false;
            m_avgPowerInFrameSum[i][j] = 0;
            m_avgPowerInFrameCount[i][j] = 0;
        }
	}

    loadIPChars("ipchars.txt");

    loadFlows("flows.txt");

	//adds - trace file open
    for(int i=0; i<num_cpus; i++) {
        cpuLastTick[i] = 0;
        app_id[i] = getAppIdWithName(em_trace_file_name[i]);
    	gemdroid_core[i].init(i, em_trace_file_name[i], app_id[i], p->core_freq, optimal_freqs[IP_TYPE_CPU], p->issue_width, this);
    }

    ipIdRoundRobin = IP_TYPE_NW;
    cpuIdRoundRobin = 0;
    for(int i=0; i<num_ip_inst; i++) {
    	gemdroid_ip_dc[i].init(IP_TYPE_DC, i, true, DC_PROCESSING_TIME, p->dev_freq, optimal_freqs[IP_TYPE_DC],  this);
		gemdroid_ip_nw[i].init(IP_TYPE_NW, i, true,NW_PROCESSING_TIME, p->dev_freq, optimal_freqs[IP_TYPE_NW], this);
		gemdroid_ip_snd[i].init(IP_TYPE_SND, i, true, SND_PROCESSING_TIME, p->dev_freq, optimal_freqs[IP_TYPE_SND], this);
		gemdroid_ip_mic[i].init(IP_TYPE_MIC, i, true, MIC_PROCESSING_TIME, p->dev_freq, optimal_freqs[IP_TYPE_MIC], this);
		gemdroid_ip_cam[i].init(IP_TYPE_CAM, i, true, CAM_PROCESSING_TIME, p->dev_freq, optimal_freqs[IP_TYPE_CAM], this);
        gemdroid_ip_mmc_in[i].init(IP_TYPE_MMC_IN, i, true, MMC_PROCESSING_TIME, p->dev_freq, optimal_freqs[IP_TYPE_MMC_IN], this);
        gemdroid_ip_mmc_out[i].init(IP_TYPE_MMC_OUT, i, true, MMC_PROCESSING_TIME, p->dev_freq, optimal_freqs[IP_TYPE_MMC_OUT], this);

		gemdroid_ip_vd[i].init(IP_TYPE_VD, i, false, VD_PROCESSING_TIME, 24, 64, VID_CODING_RATIO, 1, true, p->ip_freq, optimal_freqs[IP_TYPE_VD], this);
        gemdroid_ip_ve[i].init(IP_TYPE_VE, i, false, VE_PROCESSING_TIME, 24, 64, VID_CODING_RATIO, PROCESSING_CHUNK, false, p->ip_freq, optimal_freqs[IP_TYPE_VE], this);
        gemdroid_ip_ad[i].init(IP_TYPE_AD, i, false, AD_PROCESSING_TIME, 24, 64, AUD_CODING_RATIO, 1, true, p->ip_freq, optimal_freqs[IP_TYPE_AD], this);
        gemdroid_ip_ae[i].init(IP_TYPE_AE, i, false, AE_PROCESSING_TIME, 24, 64, AUD_CODING_RATIO, PROCESSING_CHUNK, false, p->ip_freq, optimal_freqs[IP_TYPE_AE], this);
        gemdroid_ip_img[i].init(IP_TYPE_IMG, i, false, IMG_PROCESSING_TIME, 16, PROCESSING_CHUNK*2, 1, PROCESSING_CHUNK, false, p->ip_freq, optimal_freqs[IP_TYPE_IMG], this);
    }
	gemdroid_ip_gpu[0].init(IP_TYPE_GPU, 0, false, em_gputrace_file_name /* GPU trace file*/, p->ip_freq, optimal_freqs[IP_TYPE_GPU], this);
    if (gemdroid_ip_gpu[0].isEnabled())
    	gemdroid_ip_dc[num_ip_inst].init(IP_TYPE_DC, num_ip_inst, true, DC_PROCESSING_TIME, p->dev_freq, optimal_freqs[IP_TYPE_DC], this);

	initDVFS();
}

void GemDroid::loadFlows(string fileName)
{
    ifstream flowsFile;
    string line;
    int app_id, ip_id;
    int flowId = 0;
    int last_app_id = -1;

    flowsFile.open(fileName);

    for(int i=0; i<APP_ID_END; i++) {
        for(int j=0; j<MAX_FLOWS_IN_APP; j++) {
            for(int k=0; k<MAX_IPS_IN_FLOW; k++) {
                flowTable[i][j][k] = -1;
            }
        }
    }

    while(getline(flowsFile, line)) {
        istringstream iss(line);
        if (line == "")
            break;

        iss >> app_id;
        if (app_id != last_app_id)
            flowId = 0;
        int pos = 0;
        do {
            iss >> ip_id;
            flowTable[app_id][flowId][pos++] = ip_id;
        } while (ip_id != -1);
        flowId++;
        last_app_id = app_id;
    }

    flowsFile.close();

    cout << "Flow information" << endl;
    for(int i=0; i<APP_ID_END; i++) {
        cout << i << endl;
        for(int j=0; j<MAX_FLOWS_IN_APP; j++) {
            cout << j << ": ";
            for(int k=0; k<MAX_IPS_IN_FLOW; k++) {
                cout << flowTable[i][j][k] << " ";
            }
            cout << endl;
        }
    }
    cout << endl;
}

void GemDroid::initDVFS()
{
	enableDVFS = true;
    doSlackDVFS = false;
    doIPSlackDVFS = false;
    framesMissed = 0;

    if(governor >= END_OF_GOVERNOR) {
        cout<<"\n Incorrect Governor number provided!";
        assert(0);
    }

    if (governor == GOVERNOR_TYPE_DISABLED) {
        enableDVFS = false;
    }
    else if (governor == GOVERNOR_TYPE_OPTIMAL) { // Optimal Freqs
        for(int i=0; i<num_cpus; i++)
    	    gemdroid_core[i].setOptCoreFreq();
   	    for(int i=IP_TYPE_VD; i<IP_TYPE_DMA; i++) {
   	        for(int j=0; j<num_ip_inst; j++) {
                GemDroidIP *inst = getIPInstance(i, j);
                inst->setOptIPFreq();
            }
        }
        gemdroid_memory.setMemFreq(0.5);
        enableDVFS = false;
    }
    else if (governor == GOVERNOR_TYPE_PERFORMANCE) { // Performance
        for(int i=0; i<num_cpus; i++)
    	    gemdroid_core[i].setMaxCoreFreq();
   	    for(int i=IP_TYPE_VD; i<IP_TYPE_DMA; i++) {
   	        for(int j=0; j<num_ip_inst; j++) {
                GemDroidIP *inst = getIPInstance(i, j);
                inst->setMaxIPFreq();
            }
        }
        gemdroid_memory.setMaxMemFreq();
        enableDVFS = false;
    }
    else if (governor == GOVERNOR_TYPE_POWERSAVE) { // Power Saving
        for(int i=0; i<num_cpus; i++)
    	    gemdroid_core[i].setCoreFreq(0.5);
   	    for(int i=IP_TYPE_VD; i<IP_TYPE_DMA; i++) {
   	        for(int j=0; j<num_ip_inst; j++){
                GemDroidIP *inst = getIPInstance(i, j);
                inst->setIPFreq(0.2);
            }
        }
        gemdroid_memory.setMemFreq(0.5);
        enableDVFS = false;
    }
    else if (governor == GOVERNOR_TYPE_ONDEMAND || governor == GOVERNOR_TYPE_INTERACTIVE || governor == GOVERNOR_TYPE_BUILDING || governor == GOVERNOR_TYPE_CORE_ORACLE) {
        for(int i=0; i<num_cpus; i++)
    	    gemdroid_core[i].setMaxCoreFreq();
   	    for(int i=IP_TYPE_VD; i<IP_TYPE_DMA; i++) {
   	        for(int j=0; j<num_ip_inst; j++) {
                GemDroidIP *inst = getIPInstance(i, j);
                inst->setMaxIPFreq();
            }
        }
        gemdroid_memory.setMemFreq(0.9);
    }
    else if (governor == GOVERNOR_TYPE_SLACK ||\
    		 governor == GOVERNOR_TYPE_SLACK_FAR_ENERGY ||\
    		  governor == GOVERNOR_TYPE_SLACK_KNAPSACK ||\
    		   governor == GOVERNOR_TYPE_DYNAMICPROG ||\
    		    governor == GOVERNOR_TYPE_SLACK_MAXENERGY_CONSUMER ||\
    		     governor == GOVERNOR_TYPE_SLACK_MAXPOWER_CONSUMER ||\
    		      governor == GOVERNOR_TYPE_SLACK_MAXENERGYPERTIME_CONSUMER ) { // Slack Governor
        for(int i=0; i<num_cpus; i++)
    	    gemdroid_core[i].setOptCoreFreq();
   	    for(int i=IP_TYPE_VD; i<IP_TYPE_DMA; i++) {
   	        for(int j=0; j<num_ip_inst; j++){
                GemDroidIP *inst = getIPInstance(i, j);
                inst->setOptIPFreq();
            }
        }
        gemdroid_memory.setOptMemFreq();
    }

    /* if(num_cpus == 1) {
    	if(governor_timing == GOVERNOR_TIMING_IP_FRAME_BOUNDARIES) {
    		cerr<<"Not implemented yet!"<<endl;
			assert(0);
    	}
    }
	else {
		if(governor_timing != GOVERNOR_TIMING_FIXED_16MS) {
			cerr<<"You can use GOVERNOR_TIMING_FIXED_16MS ONLY for multiple applications!"<<endl;
			assert(0);
		}
	} */
    
/*    if (governor == GOVERNOR_TYPE_ONDEMAND ||\
             governor == GOVERNOR_TYPE_PERFORMANCE ||\
              governor == GOVERNOR_TYPE_POWERSAVE ||\
               governor == GOVERNOR_TYPE_BUILDING ||\
                governor == GOVERNOR_TYPE_INTERACTIVE ||\
                 governor == GOVERNOR_TYPE_POWERCAP ) {
        
        assert(governor_timing == GOVERNOR_TIMING_FIXED_10MS);

                    }*/

    /* if (governor_timing == GOVERNOR_TIMING_FIXED_10MS)
        dvfsPeriod = 10 * MILLISEC;
    else if (governor_timing == GOVERNOR_TIMING_FIXED_1MS)
        dvfsPeriod = MILLISEC;
	else if (governor_timing == GOVERNOR_TIMING_FIXED_16MS)
    	dvfsPeriod = 16*MILLISEC;
    else
        dvfsPeriod = MILLISEC; */

    dvfsPeriod = governor_timing * MILLISEC;

    for(int i=0; i<num_cpus; i++)
        cpuFreqMultipliers[i] = GEMDROID_FREQ / gemdroid_core[i].getCoreFreq();

    for(int i=IP_TYPE_DC; i<IP_TYPE_DMA; i++) {
    	for(int j=0; j<num_ip_inst; j++) {
    		GemDroidIP *inst = getIPInstance(i, j);
            ipFreqMultipliers[i][j] = GEMDROID_FREQ / inst->getIPFreq();
        }
    }

    memFreqMultiplier = GEMDROID_FREQ / gemdroid_memory.getMemFreq();
}

void GemDroid::loadIPChars(string filename)
{
    ifstream ipcharsFile;
    string dummy;

    ipcharsFile.open(filename);

    if (!ipcharsFile.good()) {
        cout << "Cannot open ipchars file " << filename << endl;
        assert(0);
    }

    /* ipcharsFile >> dummy;
    for(int i=0; i<3; i++) {
        for(int j=0; j<CORE_DVFS_STATES; j++)
           ipcharsFile >> core_time_table[i][j];
        ipcharsFile >> dummy;
    }

    for(int i=0; i<3; i++) {
        for(int j=0; j<CORE_DVFS_STATES; j++)
           ipcharsFile >> core_energy_table[i][j];
        ipcharsFile >> dummy;
    } */

    ipcharsFile >> dummy;
    for(int i=IP_TYPE_VD; i<=IP_TYPE_GPU; i++) {
        if (i == IP_TYPE_AD || i == IP_TYPE_AE)
            continue;
        for(int j=0; j<IP_DVFS_STATES; j++)
            ipcharsFile >> ip_time_table[i][j];
        ipcharsFile >> dummy;
    }

    for(int i=IP_TYPE_VD; i<=IP_TYPE_GPU; i++) {
        if (i == IP_TYPE_AD || i == IP_TYPE_AE)
            continue;
        for(int j=0; j<IP_DVFS_STATES; j++)
            ipcharsFile >> ip_energy_table[i][j];
        ipcharsFile >> dummy;
    }

    ipcharsFile.close();

    ip_time_table[IP_TYPE_DC][2] = 3.167;

    // coeffs for time prediction
    // time_pred_coeffs[IP_TYPE_CPU][0] = -0.0198; time_pred_coeffs[IP_TYPE_DC][1] = 0.6805; time_pred_coeffs[IP_TYPE_DC][2] = -7.4836; time_pred_coeffs[IP_TYPE_CPU][3] = 28.823;
    time_pred_coeffs[IP_TYPE_DC][0] = -0.0115; time_pred_coeffs[IP_TYPE_DC][1] = 0.3574; time_pred_coeffs[IP_TYPE_DC][2] = -3.8969; time_pred_coeffs[IP_TYPE_DC][3] = 18.473;
    time_pred_coeffs[IP_TYPE_CAM][0] = -0.0115; time_pred_coeffs[IP_TYPE_CAM][1] = 0.3574; time_pred_coeffs[IP_TYPE_CAM][2] = -3.8969; time_pred_coeffs[IP_TYPE_CAM][3] = 18.473;

    // Set optimal frequencies
    for (int i=0; i<IP_TYPE_END; i++)
        optimal_freqs[i] = 1; // 300 MHz

    optimal_freqs[IP_TYPE_CPU] = 6; //1000 MHz
    optimal_freqs[IP_TYPE_VD] = 1; // 300
    optimal_freqs[IP_TYPE_VE] = 2; // 400
    optimal_freqs[IP_TYPE_AD] = 1; // 300
    optimal_freqs[IP_TYPE_AE] = 2; // 400
    optimal_freqs[IP_TYPE_IMG] = 2; // 400
    optimal_freqs[IP_TYPE_GPU] = 1; // 300

    // optimal_energy[IP_TYPE_CPU] = core_energy_table[0][optimal_freqs[IP_TYPE_CPU]];
    optimal_energy[IP_TYPE_CPU] = 0;
    optimal_energy[IP_TYPE_VD] = ip_energy_table[IP_TYPE_VD][optimal_freqs[IP_TYPE_VD]];
    optimal_energy[IP_TYPE_VE] = ip_energy_table[IP_TYPE_VE][optimal_freqs[IP_TYPE_VE]];
    optimal_energy[IP_TYPE_AD] = ip_energy_table[IP_TYPE_AD][optimal_freqs[IP_TYPE_AD]];
    optimal_energy[IP_TYPE_AE] = ip_energy_table[IP_TYPE_AE][optimal_freqs[IP_TYPE_IMG]];
    optimal_energy[IP_TYPE_IMG] = ip_energy_table[IP_TYPE_IMG][optimal_freqs[IP_TYPE_IMG]];
    optimal_energy[IP_TYPE_GPU] = ip_energy_table[IP_TYPE_GPU][optimal_freqs[IP_TYPE_GPU]];

    cout << "Optimal Freq and Energy" << endl;
    for (int i=0; i<IP_TYPE_END; i++)
        cout << ipTypeToString(i) << " " << optimal_freqs[i] << " " << optimal_energy[i] << endl;

    /* cout << "Core Time and Energy Table for different apps" << endl;
    for(int i=0; i<APP_ID_END-1; i++) {
        for(int j=0; j<CORE_DVFS_STATES; j++)
           cout << core_time_table[i][j] << " " << core_energy_table[i][j] << endl;
       cout << endl;
    } */

    cout << "IP Time and Energy Table" << endl;
    for(int i=IP_TYPE_VD; i<=IP_TYPE_GPU; i++) {
        if (i == IP_TYPE_AD || i == IP_TYPE_AE)
            continue;
        cout << ipTypeToString(i) << endl;
        for(int j=0; j<IP_DVFS_STATES; j++)
           cout << ip_time_table[i][j] << " " << ip_energy_table[i][j] << endl;
        cout << endl;
    }
}

GemDroid::~GemDroid()
{
}

void GemDroid::init()
{
}

void GemDroid::startup()
{
	if(!gemdroid_enable)
		return;

    // kick off the clock ticks
    schedule(tickEvent, clockEdge());
}


void GemDroid::regStats()
{
	for(int i=0; i<num_cpus; i++)
		gemdroid_core[i].regStats();

	gemdroid_sa.regStats();
	gemdroid_memory.regStats();
 
    for(int i=IP_TYPE_DC; i<IP_TYPE_DMA; i++) {
        for(int j=0; j<num_ip_inst; j++) {
	        GemDroidIP *inst = getIPInstance(i, j);
            inst->regStats();
        }
	}
	if (gemdroid_ip_gpu[0].isEnabled()) {
		// gemdroid_ip_gpu[0].regStats();
		gemdroid_ip_dc[1].regStats();
	}

    // Local stats
    m_totalCommittedInsns
        .name("GemDroid.total_committed_insns")
        .desc("GemDroid: Total Number of Instructions committed across all cores")
        .flags(Stats::display);

	for(int i=0; i<MAX_FLOWS; i++) {
		string str  = flowIdToString(i);
		m_cyclesPerFrameInFlow[i].name(str + ".cyclesPerFrame").desc("GemDroid: Average number of cycles in this flow per frame frame").flags(Stats::display);
		m_flowDrops[i].name(str + ".frameDrops").desc("GemDroid: Average number of frame drops in this flow").flags(Stats::display);
	}

	for(int i=0; i<IP_TYPE_END; i++) {
			stringstream str;
			string str1 = ".m_ipCallDrops";
			str << ipTypeToString(i);
			m_ipCallDrops[i].name(str.str() + str1).desc("GemDroid: Number of ip calls rejected").flags(Stats::display);

		for(int j=0; j<num_ip_inst; j++) {
			stringstream nstr;
			string str1 = "idleStreaksinActivePState_";
			string str2 = "cyclesPerFrame";
			nstr << ipTypeToString(i);
			nstr << "_";
			nstr << j;
			idleStreaksinActivePState[i][j].init(1, 12000, 100).name(str1 + nstr.str()).desc("GemDroid: Idle Cycle Streaks in IP "+nstr.str()).flags(Stats::nozero);
			m_cyclesPerFrame[i][j].name(str2 + nstr.str()).desc("GemDroid: Number of cycles spent in this frame").flags(Stats::display);
			// m_avgPowerInFrame[i][j].name(str.str()).desc("GemDroid: Avg Power");
		}
	}
	if (gemdroid_ip_gpu[0].isEnabled()) {
		idleStreaksinActivePState[IP_TYPE_DC][num_ip_inst].init(1, 1000, 10).name("idleStreaksinActivePState_DC_1").desc("GemDroid: Idle Cycle Streaks in IP DC_1").flags(Stats::nozero);
		m_cyclesPerFrame[IP_TYPE_DC][num_ip_inst].name("cyclesPerFrame_DC_1").desc("GemDroid: Number of cycles spent in this frame").flags(Stats::display);
		m_ipCallDrops[num_ip_inst].name("DC_1.m_ipCallDrops").desc("GemDroid: Number of ip calls rejected").flags(Stats::display);
	}

	for(int i=0; i<IP_TYPE_END; i++)
		for(int j=num_ip_inst; j<MAX_IPS; j++) {
			stringstream str;
			string str1 = "idleStreaksinActivePState_";
			string str2 = "cyclesPerFrame";
			str << ipTypeToString(i);
			str << "_";
			str << j;
			if (gemdroid_ip_gpu[0].isEnabled() && i == IP_TYPE_DC && j == 1)
				; // DC 1 already initialized
			else {
				idleStreaksinActivePState[i][j].init(1, 1000, 10).name(str1 + str.str()).desc("GemDroid: Idle Cycle Streaks in IP"+str.str()).flags(Stats::nozero);
				m_cyclesPerFrame[i][j].name(str2 + str.str()).desc("GemDroid: Number of cycles spent in this frame").flags(Stats::nozero);
			}
		}

    for(int i=0;i<num_cpus; i++) {
        stringstream nstr;
        string str1 = "GemDroidCore";
        string str2 = ".total_energy";
        nstr << i;
        nstr << "_";
        nstr << str2;

        m_totalCoreEnergy[i]
            .name(str1 + nstr.str())
            .desc("GemDroid: Total energy consumed by the core")
            .flags(Stats::display);
    }

    m_totalMemEnergy
        .name("GemDroid.total_memory_energy")
        .desc("GemDroid: Total Energy Consumed by Memory")
        .flags(Stats::display);

    m_totalPlatformEnergy
        .name("GemDroid.total_platform_energy")
        .desc("GemDroid: Total Energy Consumed by Platform")
        .flags(Stats::display);

    m_totalMilliSecs
        .name("GemDroid.total_milli_secs")
        .desc("GemDroid: Total Number Of Milliseconds so far")
        .flags(Stats::display);

    for(int j=0; j<IP_TYPE_END; j++) {
        stringstream nstr;
        string str1 = "GemDroid.ipenergy.";
        nstr << ipTypeToString(j);
        nstr << "_";
        m_totalIPEnergy[j]
            .name(str1 + nstr.str())
            .desc("GemDroid: Total Energy Consumed by IP"+nstr.str())
            .flags(Stats::display);        
    }    
}

void GemDroid::resetStats()
{
	for(int i=0; i<num_cpus; i++)
		gemdroid_core[i].resetStats();

	gemdroid_sa.resetStats();
	gemdroid_memory.resetStats();

    for(int i=IP_TYPE_DC; i<IP_TYPE_DMA; i++) {
        for(int j=0; j<num_ip_inst; j++) {
	        GemDroidIP *inst = getIPInstance(i, j);
            inst->resetStats();
        }
    }

	if (gemdroid_ip_gpu[0].isEnabled()) {
		gemdroid_ip_dc[1].resetStats();
    }

	m_totalCommittedInsns = 0;
	for(int i=0; i<num_ip_inst; i++)
		for(int j=0; j<MAX_IPS; j++)
			idleStreaksinActivePState[i][j].reset();

	for(int i=0; i<MAX_FLOWS; i++) {
		m_cyclesPerFrameInFlow[i] = 0;
	}  
}

void GemDroid::printPeriodicStats()
{
    cout << "#########################" << endl;
	cout<<desc<<".m_totalCommittedInsns: "<<m_totalCommittedInsns.value()<<endl;
	cout<<desc<<".milliSecs: "<<ticks/(double) MILLISEC<<endl;

	for(int i=0; i<num_cpus; i++)
		gemdroid_core[i].printPeriodicStats();

	gemdroid_sa.printPeriodicStats();
	// gemdroid_memory.printPeriodicStats();

    for(int i=IP_TYPE_DC; i<IP_TYPE_DMA; i++) {
        if(i == IP_TYPE_NW || i == IP_TYPE_SND  || i == IP_TYPE_MIC || i == IP_TYPE_MMC_IN  || i == IP_TYPE_MMC_OUT)
            continue;
        for(int j=0; j<num_ip_inst; j++) {
	        //GemDroidIP *inst = getIPInstance(i, j);
            switch(i)
            {
                case IP_TYPE_VD:
                    gemdroid_ip_vd[j].printPeriodicStats();
                    break;
                case IP_TYPE_VE:
                    gemdroid_ip_ve[j].printPeriodicStats();
                    break;
                case IP_TYPE_AD:
                    gemdroid_ip_ad[j].printPeriodicStats();
                    break;
                case IP_TYPE_AE:
                    gemdroid_ip_ae[j].printPeriodicStats();
                    break;
                case IP_TYPE_IMG:
                    gemdroid_ip_img[j].printPeriodicStats();
                    break;
                case IP_TYPE_GPU:
                    gemdroid_ip_gpu[j].printPeriodicStats();
                    break;
            }
/*            inst->printPeriodicStats();*/
        }
    }

	if (gemdroid_ip_gpu[0].isEnabled()) {
		gemdroid_ip_dc[num_ip_inst].printPeriodicStats();
	}
}

void GemDroid::tickIP(int type, int id)
{
    switch (type) {
    case IP_TYPE_DC:
        gemdroid_ip_dc[id].tick();
        break;
    case IP_TYPE_NW:
        gemdroid_ip_nw[id].tick();
        break;
    case IP_TYPE_SND:
        gemdroid_ip_snd[id].tick();
        break;
    case IP_TYPE_MIC:
        gemdroid_ip_mic[id].tick();
        break;
    case IP_TYPE_CAM:
        gemdroid_ip_cam[id].tick();
        break;
    case IP_TYPE_MMC_IN:
        gemdroid_ip_mmc_in[id].tick();
        break;
    case IP_TYPE_MMC_OUT:
        gemdroid_ip_mmc_out[id].tick();
        break;
    case IP_TYPE_VD:
        gemdroid_ip_vd[id].tick();
        break;
    case IP_TYPE_VE:
        gemdroid_ip_ve[id].tick();
        break;
    case IP_TYPE_AD:
        gemdroid_ip_ad[id].tick();
        break;
    case IP_TYPE_AE:
        gemdroid_ip_ae[id].tick();
        break;
    case IP_TYPE_IMG:
        gemdroid_ip_img[id].tick();
        break;
    case IP_TYPE_GPU:
        gemdroid_ip_gpu[id].tick();
        break;
    case IP_TYPE_DMA:
    case IP_TYPE_CACHE:
        break;
    default: 
        cout<<"FATAL: Unknown IP ticked!"<<endl;
        assert (1);
    }
}

inline GemDroidIP *GemDroid::getIPInstance(int type, int id)
{
    switch (type) {
    case IP_TYPE_DC:
        return &gemdroid_ip_dc[id];
    case IP_TYPE_NW:
        return &gemdroid_ip_nw[id];
    case IP_TYPE_SND:
        return &gemdroid_ip_snd[id];
    case IP_TYPE_MIC:
        return &gemdroid_ip_mic[id];
    case IP_TYPE_CAM:
        return &gemdroid_ip_cam[id];
    case IP_TYPE_MMC_IN:
        return &gemdroid_ip_mmc_in[id];
    case IP_TYPE_MMC_OUT:
        return &gemdroid_ip_mmc_out[id];
    case IP_TYPE_VD:
        return &gemdroid_ip_vd[id];
    case IP_TYPE_VE:
        return &gemdroid_ip_ve[id];
    case IP_TYPE_AD:
        return &gemdroid_ip_ad[id];
    case IP_TYPE_AE:
        return &gemdroid_ip_ae[id];
    case IP_TYPE_IMG:
        return &gemdroid_ip_img[id];
    case IP_TYPE_GPU:
        return &gemdroid_ip_gpu[id];
    case IP_TYPE_DMA:
    case IP_TYPE_CACHE:
    default: 
        cout<<"FATAL: getIPInstance(): Unknown IP instance asked:" << type <<endl;
        assert (0);
    }

    return NULL;
}

double GemDroid::getLastTimeForFlow(int app_id, int flow_id)
{
    int i = 0;
    int ip = -1;
    double time = 0;

    while ((ip = flowTable[app_id][flow_id][i++]) != -1)
        time += lastTimeTook[ip];

    // if(flowTable[app_id][0][IP_TYPE_CPU] != 0)
        // time += lastTimeTook[IP_TYPE_CPU];

    return time;
}

double GemDroid::getLastSlack(int app_id, int flow_id) // Predict slack in this frame based on previous history
{
    // cout << "Slack: " << ipTypeToString(ip_type) << " " << lastTimeTook[ip_type] << endl;
    double slack=FPS_DEADLINE - getLastTimeForFlow(app_id, flow_id);

    return slack;
}

void GemDroid::tick()
{
	//std::cout<<"\n GemDroid Called"<<endl;
	ticks++;

    //if (ticks % PERIODIC_STATS == 0) {
    if(ticks - periodicStatsLastTick >= PERIODIC_STATS)
    {
        cout << "Tot_Millisecs: " << m_totalMilliSecs.value() << endl;

        if (isPrintPeriodicStats)
            printPeriodicStats();

        gemdroid_memory.printPeriodicStats();
       
        assert(PERIODIC_STATS == POWER_CALC_PERIOD);

        m_totalMilliSecs++;
        if(isPrintPeriodicStatsPower)
            powerCalculator();
        
        for(int i=0; i<num_cpus; i++)
            appMemReqs[i] = 0;

        periodicStatsLastTick = ticks;
    }

	// if (ticks % MICROSEC == 0) {
    if(ticks - powerCalcLastTick >= MICROSEC) {
        powerCalculator1us();

        powerCalcLastTick = ticks;
    }

    // if (ticks % (16*MILLISEC) == 0) {
    if (ticks - slackLastTick >= (16*MILLISEC)) {
        double slack = getLastSlack(app_id[0], 0);
        framesToBeShown++;
        cout << "DVFS. Slack: " << slack << endl;
        if(slack < (-1*FPS_DEADLINE_SAFETYNET))
            framesMissed++;
        if(has2ndFlow(app_id[0])) {
            double slack2 = getLastSlack(app_id[0], 1);
            cout << "DVFS. Slack2: " << slack2 << endl;
            // if(slack2 < (-1*FPS_DEADLINE_SAFETYNET))
                // framesMissed++;
        }


        int sumFramesDropped=0;
        for(int i = 0; i<num_cpus;i++)
            sumFramesDropped += gemdroid_core[i].m_framesDropped.value();

        cout<< "FramesMissed: "<<framesMissed<<endl;
        cout<< "FramesDropped: "<<sumFramesDropped+framesMissed<<endl;
        cout<< "FramesToBeShown: "<<framesToBeShown<<endl;

        slackLastTick = ticks;
    }

	// if (enableDVFS && ticks % dvfsPeriod == 0) {
	if (enableDVFS && (ticks - dvfsLastTick >= dvfsPeriod)) {
		cout<<"DVFS. UpdateDVFS @ time: "<<ticks/MILLISEC*1.0<<endl;
		updateDVFS();

	    for(int i=0; i<num_cpus; i++)
            cpuFreqMultipliers[i] = GEMDROID_FREQ / gemdroid_core[i].getCoreFreq();

	    for(int i=IP_TYPE_DC; i<IP_TYPE_DMA; i++)
   	    	for(int j=0; j<num_ip_inst; j++) {
   	    		GemDroidIP *inst = getIPInstance(i, j);
                ipFreqMultipliers[i][j] = GEMDROID_FREQ / inst->getIPFreq();
            }

        memFreqMultiplier = GEMDROID_FREQ / gemdroid_memory.getMemFreq();

	    for(int i=IP_TYPE_CPU; i<=IP_TYPE_GPU; i++) {
	    	if(i == IP_TYPE_CPU)
	    		cout<<"BWAttained by CPU: "<<ipMemReqs[i]*64.0/1000000/(lastTimeTook[IP_TYPE_CPU])<<endl;
	        ipMemReqs[i] = 0;
	    }

        dvfsLastTick = ticks;
	}

	for(int i=0; i<num_cpus; i++) {
	    // if(ticks % cpuFreqMultipliers[i] == 0) {
	    if(ticks - cpuLastTick[i] >= cpuFreqMultipliers[i]) {
			gemdroid_core[i].tick();

            cpuLastTick[i] = ticks;
        }
    }

 	for(int i=IP_TYPE_DC; i<IP_TYPE_GPU; i++){
   		for(int j=0; j<num_ip_inst; j++){
  			// if(ticks % ipFreqMultipliers[i][j] == 0) {
  			if(ticks - ipLastTick[i][j] >= ipFreqMultipliers[i][j]) {
  				// inst->tick();
   				tickIP(i, j);

                ipLastTick[i][j] = ticks;
   			}
   		}
   	}

  	if (gemdroid_ip_gpu[0].isEnabled()) {
        // if(ticks % ipFreqMultipliers[IP_TYPE_GPU][0] == 0) {
        if(ticks - ipLastTick[IP_TYPE_GPU][0] >= ipFreqMultipliers[IP_TYPE_GPU][0]) {
  			gemdroid_ip_gpu[0].tick();
		    gemdroid_ip_dc[num_ip_inst].tick();

            ipLastTick[IP_TYPE_GPU][0] = ticks;
  		}
  	}

	// if(ticks % memFreqMultiplier == 0) {
	if(ticks - memLastTick >= memFreqMultiplier) {
		gemdroid_sa.tick();
		gemdroid_memory.tick();

        memLastTick = ticks;
    }

	schedule(tickEvent, curTick() + (1/GEMDROID_FREQ) * SimClock::Int::ns);
}

bool GemDroid::memIPResponse(int ip_type, int ip_id, uint64_t addr, bool isRead)
{
	assert (ip_type != IP_TYPE_CPU);

	if (ip_type == IP_TYPE_DC)
		gemdroid_ip_dc[ip_id].memResponse(addr, isRead);
	else if (ip_type == IP_TYPE_SND)
		gemdroid_ip_snd[ip_id].memResponse(addr, isRead);
	else if (ip_type == IP_TYPE_MIC)
		gemdroid_ip_mic[ip_id].memResponse(addr, isRead);
	else if (ip_type == IP_TYPE_NW)
		gemdroid_ip_nw[ip_id].memResponse(addr, isRead);
	else if (ip_type == IP_TYPE_CAM)
		gemdroid_ip_cam[ip_id].memResponse(addr, isRead);
	else if (ip_type == IP_TYPE_MMC_IN)
		gemdroid_ip_mmc_in[ip_id].memResponse(addr, isRead);
	else if (ip_type == IP_TYPE_MMC_OUT)
		gemdroid_ip_mmc_out[ip_id].memResponse(addr, isRead);
	else if (ip_type == IP_TYPE_VD)
		gemdroid_ip_vd[ip_id].memResponse(addr, isRead);
	else if (ip_type == IP_TYPE_VE)
		gemdroid_ip_ve[ip_id].memResponse(addr, isRead);
	else if (ip_type == IP_TYPE_AD)
		gemdroid_ip_ad[ip_id].memResponse(addr, isRead);
	else if (ip_type == IP_TYPE_AE)
		gemdroid_ip_ae[ip_id].memResponse(addr, isRead);
	else if (ip_type == IP_TYPE_IMG)
		gemdroid_ip_img[ip_id].memResponse(addr, isRead);

	return true;
}

bool GemDroid::memCoreResponse(int type, int core_id, uint64_t addr, bool isRead)
{
	// Mark the OoO memory transaction as completed once we receive response back from the core.

	int is_success = gemdroid_core[core_id].markTransactionCompleted(addr);

	if( is_success == -1)
		cout<<"FATAL ERROR: Response back, but no transaction found when searched for "<<hex<<addr<<dec<<"("<<addr<<")!"<<endl;
	else if(is_success == -2)
		cout<<"FATAL: Response back, but no transaction found in empty ROB!"<<endl;
	else
		return true; // cout<<"Dequeueing "<<addr<<endl;

	return false;
}

bool GemDroid::enqueueIPReq(int sender_type, int sender_id, int core_id, int ip_type, uint64_t addr, int size, bool isRead, int frameNum, int flowType, int flowId)
{
	// TODO: Add scheduling between multiple IP instances (instead of always instance 0)
	switch(ip_type)	{
	case IP_TYPE_DC:
		if (sender_type == IP_TYPE_GPU) // core writing to Framebuffer mmap'ed area
			return gemdroid_ip_dc[1].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		else	//Another IP (GPU or Video-dec) writing to Framebuffer mmap'ed area
			return gemdroid_ip_dc[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
	case IP_TYPE_NW:
		return gemdroid_ip_nw[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
	case IP_TYPE_SND:
		return gemdroid_ip_snd[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
	case IP_TYPE_MIC:
		return gemdroid_ip_mic[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
	case IP_TYPE_CAM:
		return gemdroid_ip_cam[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
	case IP_TYPE_MMC_IN:
		return gemdroid_ip_mmc_in[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
	case IP_TYPE_MMC_OUT:
		return gemdroid_ip_mmc_out[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
	case IP_TYPE_VD:
		return gemdroid_ip_vd[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
	case IP_TYPE_VE:
		return gemdroid_ip_ve[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
	case IP_TYPE_AD:
		return gemdroid_ip_ad[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
	case IP_TYPE_AE:
		return gemdroid_ip_ae[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
	case IP_TYPE_IMG:
		return gemdroid_ip_img[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
/*	case IP_TYPE_DMA:
		return gemdroid_ip_dma[0].enqueueIPReq(core_id, addr, DC0_ADDR_START, size, frameNum, flowId);
		break;*/
	case IP_TYPE_GPU:
		return gemdroid_ip_gpu[0].enqueueIPReq(core_id, addr, size, isRead, frameNum, flowType, flowId);
		break;
	default:
		cout << "Wrong IP Type" << ip_type << "Addr: "<<addr<<endl;
		break;
	}
	assert(0);
	return false;
}

int GemDroid::flowType(int array[])
{
    int j;

    for(j=0; j<MAX_IPS_IN_FLOW; j++) {
        if (array[j] == -1)
            break;
    }

    return array[j-1];
}

int GemDroid::getIPAccsInFlow(int core_id, int flow_id, int (&ips)[MAX_IPS_IN_FLOW])
{
    int appid = app_id[core_id];
    int i, j=0;

    for(int i=0; i<MAX_IPS_IN_FLOW; i++)
        ips[i] = -1;

    for(i=0; i<MAX_IPS_IN_FLOW; i++) {
        if (flowTable[appid][flow_id][i] != -1 && !isDevice(flowTable[appid][flow_id][i])) {
            ips[j++] = flowTable[appid][flow_id][i];
        }
    }

    return j;
}

int GemDroid::getIPDevsInFlow(int core_id, int flow_id, int (&ips)[MAX_IPS_IN_FLOW])
{
    int appid = app_id[core_id];
    int i, j=0;

    for(int i=0; i<MAX_IPS_IN_FLOW; i++)
        ips[i] = -1;

    for(i=0; i<MAX_IPS_IN_FLOW; i++) {
        if (flowTable[appid][flow_id][i] != -1 && isDevice(flowTable[appid][flow_id][i])) {
            ips[j++] = flowTable[appid][flow_id][i];
        }
    }

    return j;
}

int GemDroid::getIPsInFlow(int core_id, int flow_id, int (&ips)[MAX_IPS_IN_FLOW])
{
    int appid = app_id[core_id];
    int i;

    for(int i=0; i<MAX_IPS_IN_FLOW; i++)
        ips[i] = -1;

    for(i=0; i<MAX_IPS_IN_FLOW; i++) {
        if (flowTable[appid][flow_id][i] == -1)
            break;
        ips[i] = flowTable[appid][flow_id][i];
    }

    return i;
}

int GemDroid::getFlowId(int core_id, int ip_id)
{
    int appid = app_id[core_id];

    for(int i=0; i<MAX_FLOWS_IN_APP; i++) {
        for(int j=0; j<MAX_IPS_IN_FLOW; j++) {
            if (flowTable[appid][i][i] == ip_id)
                return i;
        }
    }

    return -1;
}

void GemDroid::nextIPtoCall(int curr_ip_active, int core_id, int flow_type, int (&ips)[MAX_IPS_IN_FLOW])
{
    int appid = app_id[core_id];
    int count = 0;

    for(int i=0; i<MAX_IPS_IN_FLOW; i++)
        ips[i] = -1;

    for(int i=0; i<MAX_FLOWS_IN_APP; i++) {
        if (flowTable[appid][i][0] == -1)
            break;

        for(int j=0; j<MAX_IPS_IN_FLOW-1; j++) {
            if (flowTable[appid][i][j] == curr_ip_active && flow_type == flowType(flowTable[appid][i]))
                ips[count++] = flowTable[appid][i][j+1];
        }
    }

    // cout << "Next IP after " << ipTypeToString(curr_ip_active) << " " << ipTypeToString(ips[0]) << " " << ipTypeToString(ips[1]) << endl;

	/* if(curr_ip_active == IP_TYPE_CPU) {

		if(flow_type == IP_TYPE_SND) {
			if(em_trace_file_name[core_id].find("video-record") != string::npos) {
				return -1;
            }
            else if(em_trace_file_name[core_id].find("video-play") != string::npos) {
				return IP_TYPE_MMC_IN;
			}
		}

		if((em_trace_file_name[core_id].find("gallery") != string::npos) || (em_trace_file_name[core_id].find("video-play") != string::npos) || (em_trace_file_name[core_id].find("audio-play") != string::npos))
			return IP_TYPE_MMC_IN;
		else if(em_trace_file_name[core_id].find("youtube") != string::npos)
			return IP_TYPE_VD;
		else if(em_trace_file_name[core_id].find("skype") != string::npos)
			return IP_TYPE_VD;
		else if(em_trace_file_name[core_id].find("video-record") != string::npos)
			return -1;
		else if(em_trace_file_name[core_id].find("DISP") != string::npos)
			return IP_TYPE_DC;
	}
	else if(curr_ip_active == IP_TYPE_VE) {
		if(em_trace_file_name[core_id].find("skype") != string::npos)
			return IP_TYPE_NW;
	}	
	else if(curr_ip_active == IP_TYPE_AE) {
		if(em_trace_file_name[core_id].find("skype") != string::npos)
			return IP_TYPE_NW;
	}
	else if(curr_ip_active == IP_TYPE_MIC) {
		if(em_trace_file_name[core_id].find("skype") != string::npos)
			return IP_TYPE_AE;
	}
	else if(curr_ip_active == IP_TYPE_AD) {
		if(em_trace_file_name[core_id].find("skype") != string::npos)
			return IP_TYPE_SND;
	}	
	else if(curr_ip_active == IP_TYPE_VD) {
		if(em_trace_file_name[core_id].find("skype") != string::npos)
			return IP_TYPE_DC;
	}	

	else if(curr_ip_active == IP_TYPE_CAM) {
		if(flow_type == IP_TYPE_DC) {
			if(em_trace_file_name[core_id].find("argame") != string::npos)
				return IP_TYPE_IMG;
			if(em_trace_file_name[core_id].find("video-record") != string::npos)
				return IP_TYPE_IMG;
			if(em_trace_file_name[core_id].find("photo-capture") != string::npos)
				return IP_TYPE_IMG;
		}
		else if(flow_type == IP_TYPE_CAM) {
			if(em_trace_file_name[core_id].find("argame") != string::npos)
				return IP_TYPE_IMG;
			if(em_trace_file_name[core_id].find("video-record") != string::npos)
				return IP_TYPE_VE;
			if(em_trace_file_name[core_id].find("skype") != string::npos)
				return IP_TYPE_VE;
		}
	}
	else if(curr_ip_active == IP_TYPE_IMG) {
		if(em_trace_file_name[core_id].find("photo-capture") != string::npos)
			return IP_TYPE_MMC_OUT;
	}
	else if(curr_ip_active == IP_TYPE_MMC_IN) {

		if(flow_type == IP_TYPE_SND) {
			if(em_trace_file_name[core_id].find("video-play") != string::npos) {
				return IP_TYPE_AD;
			}
		}

		if(em_trace_file_name[core_id].find("gallery") != string::npos)
			return IP_TYPE_IMG;
		else if(em_trace_file_name[core_id].find("video-play") != string::npos)
			return IP_TYPE_VD;
		else if(em_trace_file_name[core_id].find("audio-play") != string::npos)
			return IP_TYPE_AD;
	}

	// assert(0);
	return -1; */
}

/* bool GemDroid::isIPCallDrop(int ip_type, long timeTook)
{
	if(ip_type == IP_TYPE_VE && timeTook > 8311666)
		return true;
	else if(ip_type == IP_TYPE_VD && timeTook > 21473608)
		return true;
	else if(ip_type == IP_TYPE_CAM && timeTook > 14180969)
		return true;
	else if(ip_type == IP_TYPE_IMG && timeTook > 22029763)
		return true;
	else if(ip_type == IP_TYPE_AD && timeTook > 6832)
		return true;
	else if(ip_type == IP_TYPE_AE && timeTook > 10487)
		return true;
	else if(ip_type == IP_TYPE_DC && timeTook > 11009496)
		return true;
	else if(ip_type == IP_TYPE_MIC && timeTook > 39319)
		return true;
	else if(ip_type == IP_TYPE_SND && timeTook > 6835)
		return true;

	return false;
}

bool GemDroid::isFlowDrop(int flow_id, long timeTook)
{
	if(flow_id == 1 && timeTook > 384868884)
		return true;
	else if(flow_id == 2 && timeTook > 78667)
		return true;
	else if(flow_id == 3 && timeTook > 40677199)
		return true;
	else if(flow_id == 4 && timeTook > 12306919)
		return true;
	else if(flow_id == 5 && timeTook > 20735)
		return true;
	else if(flow_id == 6 && timeTook > 60851662)
		return true;
	else if(flow_id == 7 && timeTook > 76516738)
		return true;
	else if(flow_id == 8 && timeTook > 22104)
		return true;
	else if(flow_id == 9 && timeTook > 7979386)
		return true;
	else if(flow_id == 10 && timeTook > 57005)
		return true;
	else if(flow_id == 11 && timeTook > 77284342)
		return true;
	else if(flow_id == 12 && timeTook > 20315)
		return true;
	else if(flow_id == 13 && timeTook > 17041869)
		return true;
	else if(flow_id == 14 && timeTook > 10701)
		return true;
	else if(flow_id == 15 && timeTook > 75105051)
		return true;
	else if(flow_id == 16 && timeTook > 11674795)
		return true;
	else if(flow_id == 17 && timeTook > 401417124)
		return true;
	else if(flow_id == 18 && timeTook > 401296338)
		return true;
	else if(flow_id == 19 && timeTook > 32697)
		return true;
	else if(flow_id == 20 && timeTook > 1)
		return true;

	return false;
} */

void GemDroid::flow_identification(int core_id, int ip_type, int (&flows)[MAX_FLOWS_IN_APP])
{
    int appid = app_id[core_id];
    int count = 0;

    for(int i=0; i<MAX_FLOWS_IN_APP; i++) {
	    flows[i] = -1;
    }

    for(int i=0; i<MAX_FLOWS_IN_APP; i++) {
        if (flowTable[appid][i][0] == -1)
            break;

        for(int j=0; j<MAX_IPS_IN_FLOW; j++) {
            if (flowTable[appid][i][j] == ip_type) {
                flows[count++] = i;
            }
        }
    }

    // cout << "GemDroid::flow_identification " << ipTypeToString(ip_type) << " " << flows[0] << " " << flows[1] << endl;

	/* if(em_trace_file_name[core_id].find("youtube") != string::npos) {
		if(ip_type == IP_TYPE_VD)
			flow_id1 = 1;
		if(ip_type == IP_TYPE_AD)
			flow_id1 = 2;
		if(ip_type == IP_TYPE_DC)
			flow_id1 = 1;
	}
	else if(em_trace_file_name[core_id].find("video-record") != string::npos) {
		if(ip_type == IP_TYPE_CAM) {
			flow_id1 = 3;
			flow_id2 = 4;
		}
		// not else if
		if(ip_type == IP_TYPE_IMG)
			flow_id1 = 3;
		if(ip_type == IP_TYPE_VE)
			flow_id1 = 4;

		if(ip_type == IP_TYPE_MIC)
			flow_id1 = 5;
		if(ip_type == IP_TYPE_AE)
			flow_id1 = 5;

		if(ip_type == IP_TYPE_DC)
			flow_id1 = 3;
		if(ip_type == IP_TYPE_SND)
			flow_id1 = 4;
	}
	else if(em_trace_file_name[core_id].find("photo") != string::npos) {
		if(ip_type == IP_TYPE_CAM) {
			flow_id1 = 6;
			flow_id2 = 7;
		}
		if(ip_type == IP_TYPE_IMG) {
			flow_id1 = 6;
			flow_id2 = 7;
		}

		if(ip_type == IP_TYPE_DC)
			flow_id1 = 6;
		if(ip_type == IP_TYPE_MMC_OUT)
			flow_id1 = 7;
	}
	else if(em_trace_file_name[core_id].find("audio-play") != string::npos) {
		if(ip_type == IP_TYPE_MMC_IN || ip_type == IP_TYPE_AD) {
			flow_id1 = 8;
		}
		if(ip_type == IP_TYPE_SND)
			flow_id1 = 8;
	}
	else if(em_trace_file_name[core_id].find("argame") != string::npos) {
		if(ip_type == IP_TYPE_CAM || ip_type == IP_TYPE_IMG) {
			flow_id1 = 9;
		}
		if(ip_type == IP_TYPE_AD || ip_type == IP_TYPE_SND) {
			flow_id1 = 10;
		}
		if(ip_type == IP_TYPE_GPU)
		 	flow_id1 = 20;

		if(ip_type == IP_TYPE_DC)
			flow_id1 = 9;
		if(ip_type == IP_TYPE_SND)
			flow_id1 = 10;
	}
	else if(em_trace_file_name[core_id].find("angry") != string::npos) {
		if(ip_type == IP_TYPE_GPU)
		 	flow_id1 = 15;
		if(ip_type == IP_TYPE_AD || ip_type == IP_TYPE_SND) {
			flow_id1 = 10;
		}

		if(ip_type == IP_TYPE_DC)
			flow_id1 = 15;
	}
	else if(em_trace_file_name[core_id].find("gallery") != string::npos) {
		if(ip_type == IP_TYPE_MMC_IN || ip_type == IP_TYPE_IMG) {
			flow_id1 = 11;
		}

		if(ip_type == IP_TYPE_DC)
			flow_id1 = 11;
	}
	else if(em_trace_file_name[core_id].find("audio-record") != string::npos) {
		if(ip_type == IP_TYPE_MIC || ip_type == IP_TYPE_AE) {
			flow_id1 = 12;
		}
		if(ip_type == IP_TYPE_MMC_OUT)
			flow_id1 = 11;
	}
	else if(em_trace_file_name[core_id].find("video-play") != string::npos) {
		if(ip_type == IP_TYPE_MMC_IN) {
			flow_id1 = 13;
			flow_id2 = 14;
		}

		if(ip_type == IP_TYPE_VD) {
			flow_id1 = 13;
		}

		if(ip_type == IP_TYPE_AD) {
			flow_id1 = 14;
		}

		if(ip_type == IP_TYPE_DC) {
			flow_id1 = 13;
		}

		if(ip_type == IP_TYPE_SND) {
			flow_id1 = 14;
		}
	}
	else if(em_trace_file_name[core_id].find("skype") != string::npos) {
		if(ip_type == IP_TYPE_CAM) {
			flow_id1 = 16;
			flow_id2 = 16;
		}
		if(ip_type == IP_TYPE_VE)
			flow_id1 = 16;
		if(ip_type == IP_TYPE_VD)
			flow_id1 = 17;
		if(ip_type == IP_TYPE_MIC)
			flow_id1 = 18;
		if(ip_type == IP_TYPE_AE)
			flow_id1 = 18;
		if(ip_type == IP_TYPE_AD)
			flow_id1 = 19;

		if(ip_type == IP_TYPE_DC)
			flow_id1 = 17;

		if(ip_type == IP_TYPE_SND)
			flow_id1 = 19;
	}
	else 
	{
		cout<<"\n Incorrect trace file name.. "<<endl;
		assert(0);
    } */
   // assert (flow_id1 != -1);  // Network IP calls this.
}

/* int GemDroid::getLastIPInFlow(int flowId)
{
	switch(flowId)
	{
		case 1: return IP_TYPE_DC;
		case 2: return IP_TYPE_SND;
		case 3: return IP_TYPE_DC;
		case 4: return IP_TYPE_MMC_OUT;
		case 5: return IP_TYPE_MMC_OUT;
		case 6: return IP_TYPE_DC;
		case 7: return IP_TYPE_MMC_OUT;
		case 8: return IP_TYPE_SND;
		case 9: return IP_TYPE_DC;
		case 10: return IP_TYPE_SND;
		case 11: return IP_TYPE_DC;
		case 12: return IP_TYPE_MMC_OUT;
		case 13: return IP_TYPE_DC;
		case 14: return IP_TYPE_SND;
		case 15: return IP_TYPE_DC; // GPUs
		case 16: return IP_TYPE_VE; // Skype
		case 17: return IP_TYPE_DC; // Skype
		case 18: return IP_TYPE_AE; // Skype
		case 19: return IP_TYPE_AD; // Skype
		case 20: return IP_TYPE_DC; // argame
		default: return -1;
	}
}

int GemDroid::getFirstIPInFlow(int flowId)
{
	switch(flowId)
	{
		case 1: return IP_TYPE_VD;
		case 2: return IP_TYPE_AD;
		case 3: return IP_TYPE_CAM;
		case 4: return IP_TYPE_CAM;
		case 5: return IP_TYPE_MIC;
		case 6: return IP_TYPE_CAM;
		case 7: return IP_TYPE_CAM;
		case 8: return IP_TYPE_MMC_IN;
		case 9: return IP_TYPE_CAM;
		case 10: return IP_TYPE_AD;
		case 11: return IP_TYPE_MMC_IN;
		case 12: return IP_TYPE_MIC;
		case 13: return IP_TYPE_MMC_IN;
		case 14: return IP_TYPE_MMC_IN;
		case 15: return IP_TYPE_GPU;
		case 16: return IP_TYPE_CAM; // Skype
		case 17: return IP_TYPE_VD; // Skype
		case 18: return IP_TYPE_MIC; // Skype
		case 19: return IP_TYPE_SND; // Skype
		case 20: return IP_TYPE_GPU; // argame
		default: return -1;
	}
}

int GemDroid::getGPUFlowId()
{
	if(em_trace_file_name[0].find("argame") != string::npos)
		return 20;
	else if(em_trace_file_name[0].find("angrybirds") != string::npos)
		return 15;

	return 20;
} */

int GemDroid::getAppIdWithName(string appName)
{
    if(appName.find("youtube") != string::npos)
        return APP_ID_YOUTUBE;
    else if(appName.find("video-record") != string::npos)
        return APP_ID_VIDRECORD;
    else if(appName.find("photo-capture") != string::npos)
        return APP_ID_PHOTOCAPTURE;
    else if(appName.find("audio-play") != string::npos)
        return APP_ID_AUDIOPLAY;
    else if(appName.find("argame") != string::npos)
        return APP_ID_ARGAME;
    else if(appName.find("gallery") != string::npos)
        return APP_ID_GALLERY;
    else if(appName.find("audio-record") != string::npos)
        return APP_ID_AUDIORECORD;
    else if(appName.find("video-play") != string::npos)
        return APP_ID_VIDPLAYER;
    else if(appName.find("angrybirds") != string::npos)
        return APP_ID_ANGRYBIRDS;
    else if(appName.find("skype") != string::npos)
        return APP_ID_SKYPE;
    else if(appName.find("facebook") != string::npos)
        return APP_ID_FACEBOOK;
    else if(appName.find("mpgame") != string::npos)
        return APP_ID_MPGAME;

    return APP_ID_OTHER;
}

void GemDroid::markIPRequestStarted(int core_id, int ip_type, int ip_id, int frameNum)
{
	ipProcessStartCycle[ip_type][ip_id] = ticks;
	frameStarted[ip_type][ip_id] = true;

	/* int flowid1=-1,flowid2=-1;
	flow_identification(core_id, ip_type,flowid1,flowid2);

	int firstIP = getFirstIPInFlow(flowid1);

	if(flowid1 != -1 && firstIP == ip_type)
		flowStartCycle[flowid1][frameNum] = ticks;
	//not else if
	if(flowid2 != -1 && firstIP == ip_type)
		flowStartCycle[flowid2][frameNum] = ticks; */

    // cout<<ticks<< " DBG2: " << ipTypeToString(ip_type) << " Started processing frame "<<frameNum << " at "<<ipProcessStartCycle[ip_type][ip_id] / 10000000.0 <<endl;

	/* cout<<ticks<< " DBG2: Flow1 " << flowIdToString(flowid1) << " Started processing frame " << frameNum << " at " <<flowStartCycle[flowid1][frameNum]<<endl;
	if (flowid2 != -1)
		cout<<ticks<< " DBG2: Flow2 " << flowIdToString(flowid2) << " Started processing frame " << frameNum << " at " <<flowStartCycle[flowid2][frameNum]<<endl;*/
}

long GemDroid::markIPRequestCompleted(int coreId, int ip_type, int ip_id, int frameNum, int flowId)
{
	// assert(flowId != -1);
	// assert(frameNum > 0);

	//Should keep track of how much time each frame took.
    double pwrAvg;

	if (frameStarted[ip_type][ip_id] == false)
		return -1;

	long timeTook = ticks - ipProcessStartCycle[ip_type][ip_id];
	frameStarted[ip_type][ip_id] = false;
	m_cyclesPerFrame[ip_type][ip_id] = timeTook;
    lastTimeTook[ip_type] = timeTook / (double) MILLISEC;

	// if(isIPCallDrop(ip_type, timeTook))
		// m_ipCallDrops[ip_type]++;

    /* if (app_id[coreId] == APP_ID_VIDPLAYER) {
        if (ip_type == IP_TYPE_CPU || ip_type == IP_TYPE_VD) {
            doIPSlackDVFS = true;
        }
    }
    else if (app_id[coreId] == APP_ID_VIDRECORD) {
        if (ip_type == IP_TYPE_CPU || ip_type == IP_TYPE_CAM || ip_type == IP_TYPE_IMG) {
            doIPSlackDVFS = true;
        }
    } */

    lastMemFreq = gemdroid_memory.getMemFreq();
    cout << " Time took by IP Mem @ " << lastMemFreq << endl;
    if (ip_type == IP_TYPE_CPU) {
        pwrAvg = m_avgPowerInFrameSum[ip_type][coreId] / m_avgPowerInFrameCount[ip_type][coreId];
        lastPowerTook[ip_type] = pwrAvg;
        lastEnergyTook[ip_type] = pwrAvg * lastTimeTook[ip_type];
        lastFrequency[ip_type] = gemdroid_core[coreId].getCoreFreq();

	    cout<<"DBG1Time took by IP " << ipTypeToString(ip_type) << " " << ip_id << " for frame " << frameNum << " is " << lastTimeTook[ip_type] << " ms @ " << gemdroid_core[coreId].getCoreFreq() << " with Avg Power: " << pwrAvg << " W and Avg Energy: " << lastEnergyTook[ip_type] << " mJ" << endl;
        m_avgPowerInFrameSum[ip_type][coreId] = 0;
        m_avgPowerInFrameCount[ip_type][coreId] = 0;
    }
    else {
        GemDroidIP *inst = getIPInstance(ip_type, ip_id);

        pwrAvg = m_avgPowerInFrameSum[ip_type][ip_id] / m_avgPowerInFrameCount[ip_type][ip_id];
        lastPowerTook[ip_type] = pwrAvg;
        lastEnergyTook[ip_type] = pwrAvg * lastTimeTook[ip_type];
        lastFrequency[ip_type] = inst->getIPFreq();

        cout<< "DBG1Time took by IP " << ipTypeToString(ip_type) << " " << ip_id << " for frame " << frameNum << " is " << lastTimeTook[ip_type]  << " ms @ " << inst->getIPFreq() << " with Avg Power: " << pwrAvg << " W and Avg Energy: " << lastEnergyTook[ip_type] << " mJ" << endl;
        m_avgPowerInFrameSum[ip_type][ip_id] = 0;
        m_avgPowerInFrameCount[ip_type][ip_id] = 0;
    }

    /* if (ip_type == IP_TYPE_CPU || ip_type == IP_TYPE_NW)
        return timeTook;

	 // cout<<"DBG2: " << ipTypeToString(ip_type) << " Curr Tick: "<<ticks<<" start cycle:  "<<ipProcessStartCycle[ip_type][ip_id]<<endl;

	long timeTookForFlow;
	int lastIp = getLastIPInFlow(flowId);

	// cout<<ticks<<"FlowId = "<<flowId << endl;

	assert(lastIp != -1);

	if (ip_type == lastIp) {
		timeTookForFlow = ticks - flowStartCycle[flowId][frameNum];
        if(flowId == 1 || flowId == 3 || flowId == 6 || flowId == 9 || flowId == 13) {
            lastFlowTime = timeTookForFlow / (double) MILLISEC;
        }
		m_cyclesPerFrameInFlow[flowId] = timeTookForFlow;
        cout<<" DBG2: @"<<ticks/MILLISEC*1.0<<"ms, time took for flow " << flowIdToString(flowId) << " for frame " << frameNum << " is " << timeTookForFlow / (double) MILLISEC << "ms" << endl;

		if (isFlowDrop(flowId, timeTookForFlow))
			m_flowDrops[flowId]++;
	} */

	return timeTook;
}

void GemDroid::powerCalculator1us()
{
    double power = 0;

	for(int i=0; i<num_cpus; i++) {
        power = gemdroid_core[i].powerIn1us();
	    if (frameStarted[IP_TYPE_CPU][i]) {
            m_avgPowerInFrameSum[IP_TYPE_CPU][i] += power;
            m_avgPowerInFrameCount[IP_TYPE_CPU][i]++;
        }
	}

	for(int type=IP_TYPE_DC; type<IP_TYPE_DMA; type++) {
		for(int j=0; j<num_ip_inst; j++) {
            GemDroidIP *inst = getIPInstance(type, j);
            power = inst->powerIn1us();
	        if (frameStarted[type][j]) {
                if (type == IP_TYPE_GPU) {
                    power = gemdroid_ip_gpu[0].powerIn1us();
                }
                m_avgPowerInFrameSum[type][j] += power;
                m_avgPowerInFrameCount[type][j]++;
                // if (type == IP_TYPE_VD)
                    // cout << "BBB VD us" << power << endl;
            }
        }
    }

    if (gemdroid_ip_gpu[0].isEnabled()) {
        m_avgPowerInFrameSum[IP_TYPE_DC][1] += gemdroid_ip_dc[1].powerIn1us();
        m_avgPowerInFrameCount[IP_TYPE_DC][1]++;
    }
}

void GemDroid::powerCalculator()
{
	// long one_millisec = GEMDROID_FREQ*1000000;
    double core_power = 0;
    double sa_power = 0;
    double memory_power = 0;
    double dev_power = 0;
    double ip_power = 0;
    double gpu_power = 0;
    double black_box_platform_power = 0.3 + (0.2*gemdroid_sa.getActivity());  //Idle platform
	double screen_power = 0.30;

	// if(ticks % one_millisec == 0) {
        double power;
		for(int i=0; i<num_cpus; i++) {
            power = gemdroid_core[i].powerIn1ms();
            m_totalCoreEnergy[i] += power;
			core_power += power;

			// m_avgPowerInFrame[IP_TYPE_CPU][i] = power;
		}

		for(int type=IP_TYPE_DC; type<IP_TYPE_DMA; type++) {
			for(int j=0; j<num_ip_inst; j++) {
                GemDroidIP *inst = getIPInstance(type, j);
                double power = inst->powerIn1ms();

                if (type == IP_TYPE_GPU)
                    gpu_power += power;
                else if (type > IP_TYPE_MMC_OUT)
                    ip_power += power;
                else
                    dev_power += power;

                m_totalIPEnergy[type] += power;
   			}
		}
        if (gemdroid_ip_gpu[0].isEnabled())
            ip_power += gemdroid_ip_dc[1].powerIn1ms();

		sa_power = gemdroid_sa.powerIn1ms();
		memory_power = gemdroid_memory.powerIn1ms();

        m_totalPlatformEnergy += black_box_platform_power + screen_power + sa_power;
        m_totalMemEnergy += memory_power;
        
        cout<<"SumOfCores power in last 1 ms: "<< core_power << endl;
        cout << "SA Power: " << sa_power << endl;
        cout << "Memory Power: " << memory_power << " @ " << gemdroid_memory.getMemFreq() <<endl;
        cout << "Dev Power: " << dev_power << endl;
        cout << "IP Power: " << ip_power << endl;


        cout << "GPU Power: " << gpu_power << endl;
        cout << "Platform Power: " << black_box_platform_power + screen_power << endl;


        double total_power = core_power + sa_power + memory_power + dev_power + ip_power + gpu_power + screen_power + black_box_platform_power;
        cout << "Total Power: " << total_power << endl;
        powerInLastEpoch = total_power;

        //enerJ
        double total_enerj=0.0;        
        
        cout<<"Enerj Core: ";
        for(int i = 0; i<num_cpus;i++) {
            cout<<m_totalCoreEnergy[i].value()<<" ";
            total_enerj += m_totalCoreEnergy[i].value();
        }
        cout<<endl;
        cout<<"Enerj IP: " ;

        for(int i = 0; i<IP_TYPE_END;i++) {
            cout<<m_totalIPEnergy[i].value()<<" ";
            total_enerj += m_totalIPEnergy[i].value();
        }
        cout<<endl;                
        cout<<"Enerj Memory: " << m_totalMemEnergy.value()<<endl;
        cout<<"Enerj Platform: " << m_totalPlatformEnergy.value()<<endl;
        total_enerj += m_totalMemEnergy.value() + m_totalPlatformEnergy.value();
        cout<<"Enerj Total: " << total_enerj <<endl;

        cout<<"Enerj Total no Platform: " << total_enerj - m_totalPlatformEnergy.value() << endl;

	// }
}

double GemDroid::getCoordinatedPower()
{
    return DVFS_POWERCAP - powerInLastEpoch;

    /* double powerUsed = 0;

	for(int i=0; i<num_cpus; i++) {
        powerUsed += gemdroid_core[i].getPowerEst();
	}

   	for(int i=IP_TYPE_DC; i<=IP_TYPE_GPU; i++)
   	    for(int j=0; j<num_ip_inst; j++) {
            GemDroidIP *inst = getIPInstance(i, j);
            powerUsed += inst->getPowerEst();
        }

    return (POWERCAP - powerUsed - 1.5); // 2 for memory + sa + blackbox + screen */
}

GemDroid* GemDroidParams::create()
{
    return new GemDroid(this);
}

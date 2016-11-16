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

using namespace std;

GemDroidCore::GemDroidCore()
{
	lines_read_cpu = 0;
	number_of_instr_executed_OoO = 0;
	deadlocks_faced = 0;
	outstanding_transactions_size = 0;
	type_of_application = CORE_BOUND;
	needToLookAhead = true;

	idleCycles = 0;
	cyclesToWake = 0;
	idleStreak = 0;

	ticks = 0;
    fpsStalls = 0;
    lastDCTick = 0;
    audFpsStalls = 0;
    lastSNDTick = 0;

    m_idleCycles = 0;
	m_committedInsns = 0;
	m_cycles = 0;
	m_memReqs = 0;
	m_ipReqs = 0;
	m_ipResps = 0;
	lines_read_cpu = 0;
    m_framesDisplayed = 0;
    m_framesDropped = 0;
	m_framesDroppedDueToRobStalls = 0;
	m_framesDroppedDueToMemStalls = 0;
	m_fpsStallsCount = 0;
	m_FPS = 0;
    m_readFBLine = false;

/*	m_robFullStalls = 0;*/
	m_idleStallsCount = 0;
	idleStalls = 0;
    qemu_to_60FPS_speedratio = 0;
	m_thisFrameRobFullStalls = 0;
	m_thisFrameMemFullStalls = 0;
/*	m_memFullStalls = 0;*/

	m_activePStateCycles = 0;
	m_lowpowerPStateCycles = 0;
	m_idlePStateCycles = 0;

	stat_m_cycles = 0;
	stat_m_idleCycles = 0;
	stat_m_committedInsns = 0;
	stat_m_memReqs = 0;
	stat_m_ipReqs = 0;
	stat_lines_read_cpu = 0;
	stat_m_framesDisplayed = 0;
	stat_m_framesDropped = 0;
	stat_m_FPS = 0;
	stat_m_robFullStalls = 0;
	stat_m_memFullStalls = 0;
	stat_m_fpsStallsCount = 0;
	stat_m_activePStateCycles = 0;
	stat_m_lowpowerPStateCycles = 0;
	stat_m_idlePStateCycles = 0;
	startCycle = 0;

	m_thisMilliSecActivePStateCycles 	=  0;
	m_thisMilliSecLowpowerPStateCycles 	=  0;
	m_thisMilliSecIdlePStateCycles 		=  0;
	m_thisMilliSecInstructionsCommitted =  0;
	m_thisMilliSecRobFullStalls 		=  0;
	m_thisDVFSEpochInstructionsCommitted = 0;
    m_thisMilliSecIdleStalls            =  0;

	m_thisMicroSecActivePStateCycles 	=  0;
	m_thisMicroSecLowpowerPStateCycles 	=  0;
	m_thisMicroSecIdlePStateCycles 		=  0;
	m_thisMicroSecInstructionsCommitted =  0;
	m_thisMicroSecRobFullStalls 		=  0;

	for(int i=0; i<IP_TYPE_END; i++) {
		m_ipCallsPresentInTrace[i] = 0;
		m_frameNumber[i] = 0;
	}

	outstanding_transactions.clear();
	outstanding_transactions.reserve(MAX_TRANSACTIONS+2);

    // Core voltage and Freqs
    voltage_freq_table[0][0] = 0.925			; voltage_freq_table[0][1] = 0.300;
    voltage_freq_table[1][0] = 0.9485714286		; voltage_freq_table[1][1] = 0.400;
    voltage_freq_table[2][0] = 0.9721428571		; voltage_freq_table[2][1] = 0.500;
    voltage_freq_table[3][0] = 0.9957142857		; voltage_freq_table[3][1] = 0.600;
    voltage_freq_table[4][0] = 1.0192857143		; voltage_freq_table[4][1] = 0.700;
    voltage_freq_table[5][0] = 1.0428571429		; voltage_freq_table[5][1] = 0.800;
    voltage_freq_table[6][0] = 1.0664285714		; voltage_freq_table[6][1] = 0.900;
    voltage_freq_table[7][0] = 1.09				; voltage_freq_table[7][1] = 1.000;
    voltage_freq_table[8][0] = 1.1135714286		; voltage_freq_table[8][1] = 1.100;
    voltage_freq_table[9][0] = 1.1371428571		; voltage_freq_table[9][1] = 1.200;
    voltage_freq_table[10][0] =1.1607142857		; voltage_freq_table[10][1] = 1.400;
    voltage_freq_table[11][0] =1.1842857143		; voltage_freq_table[11][1] = 1.600;
    voltage_freq_table[12][0] =1.255			; voltage_freq_table[12][1] = 1.800;
    // voltage_freq_table[13][0]	=					1.250; voltage_freq_table[13][1] = 2.001;

    // Calculate Power Values
    for(int i=0; i<CORE_DVFS_STATES; i++) {
        // voltage_freq_table[i][2] = CORE_STATIC_CONST * voltage_freq_table[i][1];
        voltage_freq_table[i][2] = CORE_STATIC_PWR;
        voltage_freq_table[i][3] = CORE_CAPACITANCE * voltage_freq_table[i][0] * voltage_freq_table[i][0] * voltage_freq_table[i][1];
    }

    dvfsCounter = 0;
}

void GemDroidCore::printDVFSTable()
{
    for(int i=0; i<CORE_DVFS_STATES; i++)
        cout << i << ": " << voltage_freq_table[i][0] << " " << voltage_freq_table[i][1] << " " << voltage_freq_table[i][2] << " " << voltage_freq_table[i][3] << endl;
}

void GemDroidCore::regStats()
{
	m_cycles.name(desc + ".cycles").desc("GemDroid: Number of cycles").flags(Stats::display);
	m_idleCycles.name(desc + ".idleCycles").desc("GemDroid: Number of idle cycles").flags(Stats::display);
	m_committedInsns.name(desc + ".committedInsns").desc("GemDroid: Number of committed instructions").flags(Stats::display);
	m_memReqs.name(desc + ".memReqs").desc("GemDroid: Number of memory requests from this core").flags(Stats::display);
	m_ipReqs.name(desc + ".ipCalls").desc("GemDroid: Number of ip calls from this core").flags(Stats::display);
	m_ipResps.name(desc + ".ipResponses").desc("GemDroid: Number of ip responses to this core").flags(Stats::display);
	lines_read_cpu.name(desc + ".traceLinesRead").desc("GemDroid: Number of lines read from the trace").flags(Stats::display);
    m_idleStallsCount.name(desc + ".idleStalls").desc("GemDroid: Number of core idle stalls").flags(Stats::display);
    m_fpsStallsCount.name(desc + ".fpsStalls").desc("GemDroid: Number of core stalls for FPS").flags(Stats::display);
    m_framesDisplayed.name(desc + ".framesDisplayed").desc("GemDroid: Number of frames displayed").flags(Stats::display);
    m_framesDropped.name(desc + ".framesDropped").desc("GemDroid: Number of frames dropped").flags(Stats::display);
    m_framesDroppedDueToRobStalls.name(desc + ".framesDroppedDueToRobStalls").desc("GemDroid: Number of frames dropped due to ROB Stalls").flags(Stats::display);
    m_framesDroppedDueToMemStalls.name(desc + ".framesDroppedDueToMemStalls").desc("GemDroid: Number of frames dropped due to Memory Stalls").flags(Stats::display);
    m_framesDroppedDueToIPStalls.name(desc + ".framesDroppedDueToIPStalls").desc("GemDroid: Number of frames dropped due to IP Stalls").flags(Stats::display);

    m_FPS.name(desc + ".FPS").desc("GemDroid: Frames Per Second (FPS)").flags(Stats::display);
/*	m_robFullStalls.name(desc + ".m_robFullStalls").desc("GemDroid: number of cycles stalled for ROB full").flags(Stats::display);
	m_memFullStalls.name(desc + ".m_memFullStalls").desc("GemDroid: number of cycles stalled for Mem full").flags(Stats::display);
*/	m_thisFrameRobFullStalls.name(desc + ".m_thisFrameRobFullStalls").desc("GemDroid: For this frame, number of cycles stalled for ROB full").flags(Stats::display);
	m_thisFrameMemFullStalls.name(desc + ".m_thisFrameMemFullStalls").desc("GemDroid: For this frame, number of cycles stalled for Mem full").flags(Stats::display);
	m_thisFrameIPFullStalls.name(desc + ".m_thisFrameIPFullStalls").desc("GemDroid: For this frame, number of cycles stalled for IP full").flags(Stats::display);

	m_activePStateCycles.name(desc + ".m_activePStateCycles").desc("GemDroid: Number of cycles spent in active state").flags(Stats::display);
	m_lowpowerPStateCycles.name(desc + ".m_lowpowerPStateCycles").desc("GemDroid: Number of cycles spent in lowpower state").flags(Stats::display);
	m_idlePStateCycles.name(desc + ".m_idlePStateCycles").desc("GemDroid: Number of cycles spent in idle state").flags(Stats::display);
	/*m_coreIPC
        .name(desc + ".indiv_core_ipc")
        .desc("IPC: IPC of this Core")
        .precision(6);
	m_coreIPC =  m_committedInsns / m_cycles;*/
    for(int i=0; i<IP_TYPE_END; i++) {
   		m_ipCallsPresentInTrace[i].name(desc + "." + ipTypeToString(i) + ".ipCallsPresentInTrace").desc("GemDroid: Number of ip calls present in trace").flags(Stats::display);
	}
}

void GemDroidCore::resetStats()
{
	m_idleCycles = 0;
	m_committedInsns = 0;
	m_cycles = 0;
	m_memReqs = 0;
	m_ipReqs = 0;
	m_ipResps = 0;
	lines_read_cpu = 0;
    m_fpsStallsCount = 0;
    m_framesDisplayed = 0;
    m_framesDropped = 0;
    m_framesDroppedDueToRobStalls = 0;
    m_framesDroppedDueToMemStalls = 0;

    m_FPS = 0;
/*	m_robFullStalls = 0;
	m_memFullStalls = 0;*/
	m_thisFrameRobFullStalls = 0;
	m_thisFrameMemFullStalls = 0;

	m_activePStateCycles = 0;
	m_lowpowerPStateCycles = 0;
	m_idlePStateCycles = 0;
}

void GemDroidCore::init(int id, std::string trace_file, int app_id, int core_freq, int optDVFSState, int issue_width, GemDroid *gemDroid)
{
	core_id=id;
	this->gemDroid = gemDroid;
	this->issue_width = issue_width;
    this->app_id = app_id;
	desc="GemDroid.Core_";
	desc += (char)(id+'0');

	if(trace_file.find("DISP") != string::npos)
		type_of_application = DISPLAY_BOUND;
	else if(trace_file.find("VID") != string::npos)
		type_of_application = VIDEO_PLAYBACK;
	else if(trace_file.find("AUD") != string::npos)
		type_of_application = AUDIO_PLAYBACK;

	cout << desc <<":\t"<< trace_file <<" Type:(0 for CORE_BOUND, 1 for DISPLAY_BOUND and 2 for VIDEO_PLAYBACK and 3 for AUDIO_PLAYBACK) " <<type_of_application<<std::endl;

	em_trace_file.open(trace_file, std::iostream::in);
	lookahead_trace_file.open(trace_file, std::iostream::in);
	if (!em_trace_file.good() || !lookahead_trace_file.good()) {
		inform("Cannot open trace file ");
		assert(0);
	}
	else {
		inform("Opened trace file ");
		if(type_of_application != CORE_BOUND)
	    	setIdleRatio();
	}

	setPStateActive();
    setCoreFreq(core_freq/1000.0); // Set core frequency

    this->optDVFSState = optDVFSState;

    cout << "Core Volt-Freq Table" << endl;
    printDVFSTable();
}

void GemDroidCore::printPeriodicStats()
{
	cout<<"-=-=-=-=-=-=-=-=-=-=-=-"<<endl;

	//print overall stat.
	if(0) {
		cout<<desc<<".m_cycles: "				<<m_cycles.value()<<endl;
		cout<<desc<<".m_idleCycles: "			<<m_idleCycles.value()<<endl;
		cout<<desc<<".m_committedInsns: "		<<m_committedInsns.value()<<endl;
		cout<<desc<<".m_memReqs: "				<<m_memReqs.value()<<endl;
		cout<<desc<<".m_ipReqs: "				<<m_ipReqs.value()<<endl;
		//cout<<desc<<".m_ipCallsPresentInTrace: "  <<m_ipCallsPresentInTrace[i].value()<<endl;
		cout<<desc<<".m_ipResps: "				<<m_ipResps.value()<<endl;
		cout<<desc<<".TRACE.lines_read_cpu: "	<<lines_read_cpu.value()<<endl;
		cout<<desc<<".m_framesDisplayed: "		<<m_framesDisplayed.value()<<endl;
		cout<<desc<<".m_framesDropped: "		<<m_framesDropped.value()<<endl;
		double seconds = (1 / voltage_freq_table[dvfsState][1]) * ticks /(1000 * 1000 * 1000);
		// cout << "Seconds = " << seconds << endl;
		m_FPS = m_framesDisplayed.value() / seconds;
		cout<<desc<<".m_framesDroppedDueToRobStalls: "		<<m_framesDroppedDueToRobStalls.value()<<endl;
		cout<<desc<<".m_framesDroppedDueToMemStalls: "		<<m_framesDroppedDueToMemStalls.value()<<endl;
		cout<<desc<<".m_framesDroppedDueToIPStalls: "		<<m_framesDroppedDueToIPStalls.value()<<endl;
		cout<<desc<<".m_FPS: "					<<m_FPS.value()<<endl;
/*		cout<<desc<<".m_robFullStalls: "		<<m_robFullStalls.value()<<endl;
		cout<<desc<<".m_memFullStalls: "		<<m_memFullStalls.value()<<endl;*/


		cout<<desc<<".m_fpsStallsCount: "		<<m_fpsStallsCount.value()<<endl;
		cout<<desc<<".m_activePStateCycles: "	<<m_activePStateCycles.value()<<endl;
		cout<<desc<<".m_lowpowerPStateCycles: "	<<m_lowpowerPStateCycles.value()<<endl;
		cout<<desc<<".m_idlePStateCycles: "		<<m_idlePStateCycles.value()<<endl;		
	}
	else {
		// This interval per-phase stats
/*		cout<<desc<<".m_framesDroppedDueToRobStalls: "		<<m_framesDroppedDueToRobStalls.value()<<endl;
		cout<<desc<<".m_framesDroppedDueToMemStalls: "		<<m_framesDroppedDueToMemStalls.value()<<endl;
		cout<<desc<<".m_framesDroppedDueToIPStalls: "		<<m_framesDroppedDueToIPStalls.value()<<endl;
		cout<<desc<<".m_ipResps: "				<<m_ipResps.value()<<endl;*/
		std::cout.precision (1);
		if(isPStateIdle())
			cout<<desc<<".State: "<<"********IDLE********"<<endl;
		else if(isPStateActive())
			cout<<desc<<".State: "<<"********ACTIVE********"<<endl;
		else
			cout<<desc<<".State: "<<"********LOWPOWER********"<<endl;

		cout<<desc<<".m_cycles: "				<<m_cycles.value() - stat_m_cycles<<" "\
				<<m_activePStateCycles.value() - stat_m_activePStateCycles <<" "\
				<<m_lowpowerPStateCycles.value() - stat_m_lowpowerPStateCycles <<" "\
				<<m_idlePStateCycles.value() - stat_m_idlePStateCycles <<" "<<endl;

/*		cout<<desc<<".m_activePStateCycles: "	<<m_activePStateCycles.value() - stat_m_activePStateCycles<<endl;
		cout<<desc<<".m_lowpowerPStateCycles: "	<<m_lowpowerPStateCycles.value() - stat_m_lowpowerPStateCycles<<endl;
		cout<<desc<<".m_idlePStateCycles: "		<<m_idlePStateCycles.value() - stat_m_idlePStateCycles<<endl;
*/
		cout<<desc<<".m_committedInsns: "		<<m_committedInsns.value() - stat_m_committedInsns<<endl;
		//cout<<desc<<".m_idleCycles: "			<<m_idleCycles.value() - stat_m_idleCycles<<endl;
		cout<<desc<<".m_frames: "		<<m_framesDisplayed.value()<<" "<<m_framesDropped.value()<<endl;		
		double seconds = (1 / voltage_freq_table[dvfsState][1]) * ticks /(1000 * 1000 * 1000);
		// cout << "Seconds = " << seconds << endl;
		m_FPS = m_framesDisplayed.value() / seconds;
		cout<<desc<<".m_FPS(global): "			<<m_FPS.value()<<endl;
		cout<<desc<<".FPS_STALLS Rem(ms): "	<<(double)fpsStalls/MILLISEC<<endl;
		cout<<desc<<".idle_STALLS : "	<<(double)idleStalls<<endl;
		cout<<desc<<".thisMilliSecIdleStalls : "	<<(double)m_thisMilliSecIdleStalls<<endl;
		cout<<desc<<".stalls:"<<m_thisFrameRobFullStalls.value()<<" "<< m_thisFrameMemFullStalls.value() <<" "<< m_thisFrameIPFullStalls.value()<<endl;
/*
		cout<<desc<<".m_thisFrameRobFullStalls: "		<<m_thisFrameRobFullStalls.value()<<endl;
		cout<<desc<<".m_thisFrameMemFullStalls: "		<<m_thisFrameMemFullStalls.value()<<endl;
		cout<<desc<<".m_thisFrameIPFullStalls: "		<<m_thisFrameIPFullStalls.value()<<endl;*/
/*		cout<<desc<<".m_robFullStalls: "		<<m_robFullStalls.value() - stat_m_robFullStalls<<endl;
		cout<<desc<<".m_memFullStalls: "		<<m_memFullStalls.value() - stat_m_memFullStalls<<endl;
*/		//cout<<desc<<".m_fpsStallsCount: "		<<m_fpsStallsCount.value() - stat_m_fpsStallsCount<<endl;
		cout<<desc<<".Reqs: "				<<m_memReqs.value() - stat_m_memReqs<<" "<<m_ipReqs.value() - stat_m_ipReqs<<endl;
		//cout<<desc<<".m_memBWRequested: "		<< (m_memReqs.value() - stat_m_memReqs)*CACHE_LINE_SIZE/125000.0<<" GBPS"<<endl;
		//cout<<desc<<".m_ipReqs: "				<<m_ipReqs.value() - stat_m_ipReqs<<endl;
		//cout<<desc<<".m_ipReqs(total): "				<<m_ipReqs.value()<<endl;
		//cout<<desc<<".m_ipCallsPresentInTrace(total): "	 <<m_ipCallsPresentInTrace[i].value()<<endl;
		// cout<<desc<<".TRACE.lines_read_cpu: "	<<lines_read_cpu.value() - stat_lines_read_cpu<<endl;
		cout<<desc<<".TRACEdone: "	<<lines_read_cpu.value()<<endl;
	}

	//Set stats to latest value. Updating it..
	stat_m_cycles 				= m_cycles.value();
	stat_m_idleCycles 			= m_idleCycles.value();
	stat_m_committedInsns 		= m_committedInsns.value();
	stat_m_memReqs 				= m_memReqs.value();
	stat_m_ipReqs				= m_ipReqs.value();
	stat_lines_read_cpu 		= lines_read_cpu.value();
	stat_m_framesDisplayed 		= m_framesDisplayed.value();
	stat_m_framesDropped 		= m_framesDropped.value();
	stat_m_FPS 					= m_FPS.value();
/*	stat_m_robFullStalls			= m_robFullStalls.value();
	stat_m_memFullStalls 			= m_memFullStalls.value();*/
	stat_m_fpsStallsCount			= m_fpsStallsCount.value();
	stat_m_activePStateCycles		= m_activePStateCycles.value();
	stat_m_lowpowerPStateCycles 	= m_lowpowerPStateCycles.value();
	stat_m_idlePStateCycles 		= m_idlePStateCycles.value();
}

/*	Conceptual Description:
 *	In GemDroid core, we run instruction traces, and we assume 1-cycle latency for all instructions.
 *	So, we need to make sure that when we have an Out of Order core, we go out of order and execute a bunch of instructions,
 *	making them all ready to be committed and removed from ROB in just 1 cycle. Even though this is not possible in reality, because each instruction
 *	will take 1 cycle for write back (or the final stage of commit), we have to deal with this approximation.
 */

/*  Because a LD/ST is blocking at the head, we go OoO and execute other compute instructions that are following it.
 *	All the instructions executed now, will "all be committed" in just 1 cycle once the head is committed.
 *
 * 	We go from i=0, because, in case the first entry is an INSTR_CPU, and it has not been fully
 * 	executed yet, then we commit "issue_width" number of instructions.
 *  Executed issue_width number of instructions from transaction at <i>
 *  We are breaking out of the external i loop to stop committing more compute instructions
*/
 
void GemDroidCore::process()
{
	/* If head transaction is ready to commit,
	 * then commit it!
	 * And, remove it from the head of ROB.
	 */

	int ROB_check_head = 0;

	for(int j=0; j<issue_width; j++) {
		if(outstanding_transactions_size > 0) {
			if(ROB_DEBUG) {
				cout<<"Size of ROB: "<<outstanding_transactions_size<<endl;
				cout<<"Num of OoO commits: "<<number_of_instr_executed_OoO<<endl;
				cout<<"Head of ROB: "<<outstanding_transactions[0].getTransactionType()<<endl;
				outstanding_transactions[0].print();
			}

			// full transaction is ready to commit if INSTR or ready to execute if LD/ST
			if(outstanding_transactions[0].getTransactionType() != INSTR_CPU) {
				if(outstanding_transactions[0].isTransactionReadyToCommit()) {
					idleCycles = 0;
					streakCalculator();
					commitHeadTransaction();
					updateStatInstructionCommit();	
					continue;
				}
				else if(!outstanding_transactions[0].isIssuedToMem()) {
					if(gemDroid->gemdroid_sa.enqueueCoreMemRequest(core_id, outstanding_transactions[0].getAddr(), outstanding_transactions[0].isRead())) {
						outstanding_transactions[0].setInsertedTick(ticks);
						outstanding_transactions[0].issueToMem();
					}
					else {
						//cout << "FATAL: Core unable to inject a memory request" << endl;
					}
					continue;
				}
			}
			else if(outstanding_transactions[0].getTransactionType() == INSTR_CPU){  //outstanding_transactions[0].getNumOfInstructions() == 0 &&
				number_of_instr_executed_OoO -= outstanding_transactions[0].getNumOfCommittedInstructions();
				// cout<<"OoO executes: "<<number_of_instr_executed_OoO<<endl;
				outstanding_transactions[0].reset_instr_committed();
				idleCycles = 0;
				streakCalculator();

				if(outstanding_transactions[0].getNumOfInstructions() == 0) {
					outstanding_transactions[0].commit_transaction();
					commitHeadTransaction(); //No instructions are committed here. Only the transaction is removed from the head of ROB.
				}
			}

			// whole transaction is NOT ready to commit
			if(number_of_instr_executed_OoO < ROB_SIZE && outstanding_transactions_size > 0) {
				if(outstanding_transactions[0].getTransactionType() == INSTR_CPU && outstanding_transactions[0].getNumOfInstructions() > 0) {
					/* If a compute instruction is in the head of the ROB,
					 * then it can be committed every cycle without checking if the
					 * number_of_instr_executed_OoO is less than ROB size.
					 */
						outstanding_transactions[0].commit_instruction();
						number_of_instr_executed_OoO++;
						updateStatInstructionCommit();
						streakCalculator();
					continue;
				}
				else if(!outstanding_transactions[0].isTransactionReadyToCommit() && outstanding_transactions[0].getTransactionType() != INSTR_CPU) {
					ROB_check_head++;
					if(ROB_check_head >= outstanding_transactions_size)
						break;
					if(outstanding_transactions[ROB_check_head].getTransactionType() != INSTR_CPU && !outstanding_transactions[ROB_check_head].isIssuedToMem()) {
						if(gemDroid->gemdroid_sa.enqueueCoreMemRequest(core_id, outstanding_transactions[ROB_check_head].getAddr(), outstanding_transactions[ROB_check_head].isRead())) {							
							outstanding_transactions[ROB_check_head].issueToMem();
							outstanding_transactions[ROB_check_head].setInsertedTick(ticks);
							streakCalculator();
						}
						else {
							m_thisFrameMemFullStalls++;
							if(flagProfile)
								m_profileMemFullStalls++;	
							//cout << "Core unable to inject a memory request" << endl;
						}
					}
					else if(outstanding_transactions[ROB_check_head].getTransactionType() == INSTR_CPU && outstanding_transactions[ROB_check_head].getNumOfInstructions() > 0) {
						outstanding_transactions[ROB_check_head].commit_instruction();
						number_of_instr_executed_OoO++;
						updateStatInstructionCommit();
						streakCalculator();
					}
					else {
						j--;
						continue;
					}
				}
			}
			else {
				//Head of ROB is not ready to commit. But already executed ROB number of entries. SO, stall the CPU till head commits.
				/*m_robFullStalls++;*/
				m_thisFrameRobFullStalls++;
				if(flagProfile)
					m_profileRobFullStalls++;

				m_thisMicroSecRobFullStalls++;
				m_thisMilliSecRobFullStalls++;

				idleCycles++;
				idleStreak++;
			}
		}
		else {
				idleCycles++;
				idleStreak++;
		}
	}
}




void GemDroidCore::inOrderProcess()
{

	for(int j=0; j<issue_width; j++) {
		if(outstanding_transactions_size > 0) {
			if(ROB_DEBUG) {
				cout<<"Size of ROB: "<<outstanding_transactions_size<<endl;
				cout<<"Num of OoO commits: "<<number_of_instr_executed_OoO<<endl;
				cout<<"Head of ROB: "<<outstanding_transactions[0].getTransactionType()<<endl;
				outstanding_transactions[0].print();
			}

			// whole transaction is NOT ready to commit
			if(outstanding_transactions[0].getTransactionType() == INSTR_CPU) {
				if(outstanding_transactions[0].getNumOfInstructions() > 0) {
					/* If a compute instruction is in the head of the ROB,
					* then it can be committed every cycle without checking if the
					* number_of_instr_executed_OoO is less than ROB size.
					*/
					outstanding_transactions[0].commit_instruction();
					updateStatInstructionCommit();
					//streakCalculator();
					continue;
				}
				if(outstanding_transactions[0].getNumOfInstructions() == 0) {
					outstanding_transactions[0].commit_transaction();
					commitHeadTransaction(); //No instructions are committed here. Only the transaction is removed from the head of ROB.
					continue;
				}
			}

			// full transaction is ready to commit if INSTR or ready to execute if LD/ST
			if(outstanding_transactions[0].getTransactionType() != INSTR_CPU) {
				if(outstanding_transactions[0].isTransactionReadyToCommit()) {
					idleCycles = 0;
					//streakCalculator();
					commitHeadTransaction();
					updateStatInstructionCommit();	
					continue;
				}
				else if(!outstanding_transactions[0].isIssuedToMem()) {
					if(gemDroid->gemdroid_sa.enqueueCoreMemRequest(core_id, outstanding_transactions[0].getAddr(), outstanding_transactions[0].isRead())) {
						outstanding_transactions[0].setInsertedTick(ticks);
						outstanding_transactions[0].issueToMem();
					}
					else {
						//cout << "FATAL: Core unable to inject a memory request" << endl;
						m_thisFrameMemFullStalls++;
						if(flagProfile)
							m_profileMemFullStalls++;	
					}
					continue;
				}
				else {
					//Head of queue is not ready to commit. So, stall the CPU till head commits.
					/*m_robFullStalls++;*/
					m_thisFrameRobFullStalls++;
					if(flagProfile)
						m_profileRobFullStalls++;

					m_thisMicroSecRobFullStalls++;
					m_thisMilliSecRobFullStalls++;

					idleCycles++;
				}
			}
		}
		else {
				idleCycles++;
		}
	}
}


void GemDroidCore::checkDeadlock()
{
	if(outstanding_transactions_size > 0 && outstanding_transactions[0].getTransactionType() != INSTR_CPU && outstanding_transactions[0].isIssuedToMem() && !outstanding_transactions[0].isTransactionReadyToCommit()) {
		if(ticks - outstanding_transactions[0].getInsertedTick() > DEADLOCK_PERIOD) {
			cout<<"FATAL: number:"<<deadlocks_faced<<" for address "<<outstanding_transactions[0].getAddr() <<" is at the head of ROB for > DEADLOCK_PERIOD"<<endl;
			//assert("DeadLock period reached.");
			outstanding_transactions[0].commit_transaction();
			commitHeadTransaction();
			deadlocks_faced++;
		}
	}
}

int GemDroidCore::getIndexInTable(double freq)
{
    for(int i = 0; i<CORE_DVFS_STATES; i++)
        if(freq == voltage_freq_table[i][1])
            return i;

    return -1;
}

bool GemDroidCore::incFreq(int steps)
{
   if (dvfsState < CORE_DVFS_STATES-1) {
        dvfsState += steps;
        return true;
    }

    return false;
}

bool GemDroidCore::decFreq(int steps)
{
    if (gemDroid->getGovernor() == GOVERNOR_TYPE_INTERACTIVE && dvfsCounter > 0)
        return false;

    if (dvfsState > steps-1) {
        dvfsState -= steps;
        return true;
    }

    return false;
}

void GemDroidCore::setMaxCoreFreq()
{
    dvfsState = CORE_DVFS_STATES - 1;

    if(gemDroid->getGovernor() == GOVERNOR_TYPE_INTERACTIVE)
    	dvfsCounter = INTERACTIVE_GOVERNOR_LAT;
}

void GemDroidCore::setMinCoreFreq()
{
    dvfsState = 0;
}

void GemDroidCore::setOptCoreFreq()
{
    dvfsState = optDVFSState;
}

void GemDroidCore::setCoreFreqInd(int ind)
{
    assert(ind > -1 && ind < CORE_DVFS_STATES);

    dvfsState = ind;
}

void GemDroidCore::setCoreFreq(double freq)
{
    int ind = getIndexInTable(freq);

    assert(ind > -1 && ind < CORE_DVFS_STATES);

    dvfsState = ind;
}

void GemDroidCore::updateDVFS()
{
    if(gemDroid->getGovernor() == GOVERNOR_TYPE_INTERACTIVE)
    	dvfsCounter--;
}

void GemDroidCore::setMaxAllowedFreq()
{
    double powerAllowed = gemDroid->getCoordinatedPower() - powerInLastEpoch;
    int freq = getFreqForPower(powerAllowed);

    // cout << "CPU" << core_id << ": Power Left = " << powerAllowed << " Frequency selected " << freq << endl;
    return setCoreFreq(voltage_freq_table[freq][1]);
}

void GemDroidCore::setPStateActive()
{
    if(gemDroid->getGovernor() == GOVERNOR_TYPE_INTERACTIVE)
        setMaxCoreFreq();
    else if(gemDroid->getGovernor() == GOVERNOR_TYPE_POWERCAP) {
        setOptCoreFreq();
        // setMaxAllowedFreq();
    }

    cpu_pstate = CPU_PSTATE_ACTIVE;
    gemDroid->markIPRequestStarted(core_id, IP_TYPE_CPU, core_id, 0);
    idleCycles = 0;
}

double GemDroidCore::getFreqForSlackOptimal(double prevTime, double &slack)
{
    double currFreq = getCoreFreq();
    int optFreqInd = getOptCoreFreqInd();
    double newFreq = currFreq;
    double newTime;
    double scaledTime = gemDroid->memScaledTimeCPU(0, gemDroid->lastMemFreq, gemDroid->gemdroid_memory.getMemFreq());

    cout << "DVFS. CPU" << core_id << " Slack left: " << slack << " CurrFreq: " << currFreq << " CurrTime: " << prevTime << ". ";

    if (slack < 0) {
        newFreq = getCoreFreq(dvfsState+2);
        cout << "-ve Slack. Increasing frequency by 2 steps to freq " << newFreq << " for time " << getTimeEst(scaledTime, getCoreFreq(), newFreq) << ". ";
    }
    else if (dvfsState > optFreqInd) {  // slack > 0 here
        int k;

        // try to reduce frequency and calculate times
        for(k=dvfsState-1; k>=optFreqInd; k--) {
            if (getTimeEst(scaledTime, getCoreFreq(), getCoreFreq(k)) > prevTime + slack)
                break;
        }

        // if (k == CORE_DVFS_STATES-1)
            // k = CORE_DVFS_STATES-2;

        newFreq = getCoreFreq(k+1);
        assert (newFreq > 0);

        if (k+1 < dvfsState)
            cout << "Reducing frequency to " << newFreq << " for time " << getTimeEst(scaledTime, getCoreFreq(), newFreq) << endl;
        else
            cout << endl;
    }
    else {
    	newFreq = getCoreFreq(optFreqInd);
        if (newFreq != currFreq)
            cout << "Moving to optimal. Increasing frequency to optFreqInd " << newFreq << " for time " << getTimeEst(scaledTime, getCoreFreq(), newFreq) << ". ";
        else
            cout << endl;
    }

    newTime = getTimeEst(scaledTime, getCoreFreq(), newFreq);

    slack -= (newTime - prevTime);
    cout << " Slack left = " << slack << endl;
    return newFreq;
}

double GemDroidCore::getFreqForPower(double power)
{
    for(int i=0; i<CORE_DVFS_STATES; i++)
        if(voltage_freq_table[i][2] + voltage_freq_table[i][3] < power)
            return voltage_freq_table[i][1];

    return voltage_freq_table[CORE_DVFS_STATES-1][1];
}

void GemDroidCore::tick()
{
	ticks++;
	m_cycles++;

	// if((long)lines_read_cpu.value() % 1000 == 0)
		// cout << core_id << " read " << lines_read_cpu.value() << " from trace" << endl;

	if (ticks % (DEADLOCK_PERIOD))
		checkDeadlock();

	if(isPStateActive()) {
		m_activePStateCycles++;
		m_thisMilliSecActivePStateCycles++;
		m_thisMicroSecActivePStateCycles++;
	}
	else if (isPStateLowPower()) {
		m_lowpowerPStateCycles++;
		m_thisMilliSecLowpowerPStateCycles++;
		m_thisMicroSecLowpowerPStateCycles++;
	}
	else {
		m_idlePStateCycles++;
		m_thisMilliSecIdlePStateCycles++;
		m_thisMicroSecIdlePStateCycles++;
	}

	if (cyclesToWake > 0) {
		cyclesToWake--;
		if (cyclesToWake == 0) {
			setPStateActive();
		}
		return;
	}

    if (isPStateActive()) {
        if (idleCycles >= CORE_PSTATE_LOWPOWER_ENTER_TIME) {
    		setPStateLowPower();
    	    idleCycles = 0;
    	}
    }
    else if (isPStateLowPower() && ENABLE_CORE_IDLE) {
        if (idleCycles >= CORE_PSTATE_IDLE_ENTER_TIME) {
    		setPStateIdle();
    	    idleCycles = 0;
    	}
    }

    if(idleStalls > 0) {
      m_idleStallsCount++;
      m_thisMilliSecIdleStalls++;
      idleStalls--;
      return;
    }


	// Wait for fpsStalls if there are any
    if(fpsStalls > 0) {
        m_fpsStallsCount++;
        fpsStalls -= GEMDROID_FREQ / getCoreFreq();
        idleCycles++;
		idleStreak++;

        // Last stall for FPS. Display frame.
		if (fpsStalls <= 0) {
			m_framesDisplayed++;
			startCycle = 0; //when fps_stalls are zero is when next frame is going to be started.
			//gemDroid->setDoSlackDVFSNextMs();
			flagProfile = true;
			profileStartGemDroidCycle = gemDroid->getTicks();
			m_profileRobFullStalls = 0;
			m_profileMemFullStalls = 0;
			m_profileStartCPUCycle = ticks;

			lastDCTick = gemDroid->getTicks();
			/*if(!gemDroid->enqueueCoreIPReq(core_id, IP_TYPE_DC, DC0_ADDR_START, FRAME_SIZE, true, m_frameNumber-1))
				cout << "FATAL: Core unable to inject a DC ip request" << endl;
*/
			if (isPStateLowPower()) {
				cyclesToWake = CORE_PSTATE_LOWPOWER_EXIT_TIME;
			}
			else if (isPStateIdle()) {
				cyclesToWake = CORE_PSTATE_IDLE_EXIT_TIME;
			}
            else {
                gemDroid->markIPRequestStarted(core_id, IP_TYPE_CPU, core_id, 0);
            }
		}

        return;
    }

    if(audFpsStalls > 0) {
    	audFpsStalls -= GEMDROID_FREQ / getCoreFreq();
        idleCycles++;
		idleStreak++;
		if (audFpsStalls <= 0) {
			//m_framesDisplayed++;
			startCycle = 0; //when fps_stalls are zero is when next frame is going to be started.
			lastSNDTick = gemDroid->getTicks();

			if (isPStateLowPower()) {
				cyclesToWake = CORE_PSTATE_LOWPOWER_EXIT_TIME;
			}
			else if (isPStateIdle()) {
				cyclesToWake = CORE_PSTATE_IDLE_EXIT_TIME;
			}
		}
		return;
    }


    if(flagProfile && gemDroid->getTicks() >= profileStartGemDroidCycle+300*MICROSEC ) {    
    	m_profileActiveCycles = ticks - m_profileStartCPUCycle;
		flagProfile = false;
    }

	if(IN_ORDER)
		inOrderProcess();
    else
		process();

	if(number_of_instr_executed_OoO < ROB_SIZE && outstanding_transactions_size < MAX_TRANSACTIONS)
		readLine();
}

// returns -1 when not found and -2 when rob size is 0
int GemDroidCore::searchInROB(uint64_t addr)
{
	for(int i=0; i<outstanding_transactions_size && i < MAX_TRANSACTIONS; i++) {
	   if(addr == outstanding_transactions[i].getAddr() && !outstanding_transactions[i].isTransactionReadyToCommit())
		   return i;
	}
	if(outstanding_transactions_size == 0)
	   return -2;

	return -1;
}

int GemDroidCore::markTransactionCompleted(uint64_t addr)
{
	// assert(outstanding_transactions.size() <= MAX_TRANSACTIONS);

	for(int i=0; i<outstanding_transactions_size && i < MAX_TRANSACTIONS; i++) {
	   if(addr == outstanding_transactions[i].getAddr()) {
		   outstanding_transactions[i].commit_transaction();
		   return i;
	   }
	}
	if(outstanding_transactions_size == 0)
	   return -2;

	return -1;
}

void GemDroidCore::commitHeadTransaction()
{
	assert(outstanding_transactions[0].isTransactionReadyToCommit());
	assert(!outstanding_transactions.empty());

	outstanding_transactions.erase(outstanding_transactions.begin());
	outstanding_transactions_size--;
	if(outstanding_transactions_size<0) {
		cout<<" outstanding_transactions_size is less than 0!"<<endl;
		assert(0);
	}
}

void GemDroidCore::updateStatInstructionCommit()
{
	m_committedInsns++;
	gemDroid->m_totalCommittedInsns++;

	m_thisMilliSecInstructionsCommitted++;
	m_thisDVFSEpochInstructionsCommitted++;
	m_thisMicroSecInstructionsCommitted++;
}

void GemDroidCore::addInstructions(long insns)
{
	GemDroidOoOTransaction new_entry(INSTR_CPU, insns, -1);
	outstanding_transactions.push_back(new_entry);
	outstanding_transactions_size++;
	if(outstanding_transactions_size>MAX_TRANSACTIONS) {
		cout<<" outstanding_transactions_size is greater than MAX_TRANSACTIONS!"<<endl;
		assert(0);
	}
}

void GemDroidCore::readLine()
{
	string op;
	string dummy;
	long insns;
    long wait;
	int size;
	uint64_t addr;

    assert(isPStateActive());

	em_trace_file>>op;
	lines_read_cpu++;

	if (op == "CPU") { // CPU line
		em_trace_file>>insns;
		em_trace_file>>wait;

		if(qemu_to_60FPS_speedratio)
        	idleStalls = wait*getCoreFreq()/qemu_to_60FPS_speedratio;
        else 
        	idleStalls = 0;

        addInstructions(insns);
	}
	else if (op == "MMU_ld" || op == "MMU_st") {
		string str;
		em_trace_file>>hex>>addr>>dec;
		if (addr > 1024*1024*1024)  {
			cout << "FATAL: Read more than 1GB address in Gemdroid core" << hex << addr << dec << endl;
		}
		if(core_id<2)
			addr += (core_id*1024*1024*1024);
		else
			addr += ((core_id/6)*1024*1024*1024);
		// cout << core_id << "  " << hex << addr << dec << endl;
		em_trace_file>>size;

		processMMURequest(op, addr);
	}
	else if (op == "CPUSummary") {
        em_trace_file>>dummy>>dummy>>dummy>>dummy;
    }
	else if (op == "END") {
		cout << endl <<"*** Read END from trace file " << core_id <<"  *** " << endl << endl;
		true_fetch = false;
	}
	else if (op == "") {
		cout << "Trace file not valid" << endl;
		//assert(0);
	}
	else {  // IP Line
		em_trace_file>>hex>>addr>>dec;
	    em_trace_file>>size;

		processIPCall(op, addr, size);
        if(op.find("FB-UP") != string::npos && (type_of_application != CORE_BOUND) && needToLookAhead) {
          setIdleRatio();
        }
	}
}

void GemDroidCore::setIdleRatio()
{
    long cpu_working_ticks = 0;
    long cpu_idle_time_ns = 0;
    long lines = 0;

    while(true) {
      string op;
      string dummy;
      long insns;
      long wait;

      lookahead_trace_file>>op;
      lines++;
      if (op == "CPU") { // CPU line
        lookahead_trace_file>>insns;
        lookahead_trace_file>>wait;
        
        cpu_working_ticks += insns;
        cpu_idle_time_ns += wait;      	
      }
      else if (op == "MMU_ld" || op == "MMU_st") {
        lookahead_trace_file>>dummy>>dummy;
      }
      else if (op == "CPUSummary") {
        lookahead_trace_file>>dummy>>dummy>>dummy>>dummy;
      }
      else if (op == "END") {
      	cout<<"JOOMLAAAA"<<op<<endl;
      	needToLookAhead = false;
      }
      else if (op == "") {
        cout << "lookahead: Trace file not valid "<<lines<< endl;
      	needToLookAhead = false;
      	break;
        //assert(0);
      }
      else {  // IP Line
        lookahead_trace_file>>dummy>>dummy;

        if(op.find("FB-UP") != string::npos) {
          break;
        }
      }
    }

    if ((cpu_idle_time_ns == 0) && (cpu_working_ticks == 0)) {
      qemu_to_60FPS_speedratio = 0;
    }
    else {
      qemu_to_60FPS_speedratio = (cpu_idle_time_ns*getCoreFreq())/(getCoreFreq()*1000000*16 - cpu_working_ticks);
    }

    cout<<"Hz: "<<getCoreFreq()*pow(10,9)<<endl;
    cout<<"CPU working: "<<cpu_working_ticks<<endl;
    cout<<"CPU idle(ns): "<<cpu_idle_time_ns<<endl;
    cout<<"CPU idle(ticks) original: "<<cpu_idle_time_ns*getCoreFreq()<<endl;
    cout<<"left ticks: "<<getCoreFreq()*1000000*16 - cpu_working_ticks<<endl;
    cout<<"Set idle time ratio from "<<lines_read_cpu.value()<<" to "<<lines_read_cpu.value()+lines<<": "<<qemu_to_60FPS_speedratio<<endl;
    return;
}

void GemDroidCore::processMMURequest(string op, uint64_t addr)
{
	int index = searchInROB(addr);
	int is_read = false;

	if(op == "MMU_ld")
		is_read = true;
	else if(op == "MMU_st")
		is_read = false;
	else
		cout<<"\n FATAL! Incorrect MMU Request sent\n";

	// cout << "CPU tried: " << ticks << " enqueued " << addr << endl;

	if( index == -1 || index == -2) {
		m_memReqs++;
		if (is_read) {
			GemDroidOoOTransaction new_entry(INSTR_MMU_LD, 0, addr);
			outstanding_transactions.push_back(new_entry);
			outstanding_transactions_size++;
			if(outstanding_transactions_size>MAX_TRANSACTIONS) {
				cout<<" outstanding_transactions_size is greater than MAX_TRANSACTIONS!"<<endl;
				assert(0);
			}
		}
		else {
			GemDroidOoOTransaction new_entry(INSTR_MMU_ST, 0, addr);
			outstanding_transactions.push_back(new_entry);
			outstanding_transactions_size++;
			if(outstanding_transactions_size>MAX_TRANSACTIONS) {
				cout<<" outstanding_transactions_size is greater than MAX_TRANSACTIONS!"<<endl;
				assert(0);
			}
		}
	}
}

void GemDroidCore::processIPCall(string op, uint64_t addr, int size)
{
	bool isRead = true;
	int ip_master;  //Who is the master injecting packets into the memory controller?
	int frameNum;
    int flowType = IP_TYPE_END;
    int ips[MAX_IPS_IN_FLOW];
    string dummy;

    if (op.find("FB-UP") != string::npos) {
    	bool shouldFrameDrop = isFrameDrop();
        m_readFBLine = true;
        flowType = IP_TYPE_DC;

    	if(shouldFrameDrop) {
    		m_ipCallsPresentInTrace[IP_TYPE_DC]++;
			addr = DC0_ADDR_START;
			ip_master = IP_TYPE_DC;
			size = FRAME_SIZE;
    	}
    	else {
			gemDroid->nextIPtoCall(IP_TYPE_CPU, core_id, IP_TYPE_DC, ips);

			if(ips[0] == IP_TYPE_VD) {
				m_ipCallsPresentInTrace[IP_TYPE_VD]++;
				addr = VD_ADDR_START;
				ip_master = IP_TYPE_VD;
				size = FRAME_SIZE/VID_CODING_RATIO;
			}
            else if(ips[0] == IP_TYPE_IMG) {
				m_ipCallsPresentInTrace[IP_TYPE_IMG]++;
				addr = IMG_ADDR_START;
				ip_master = IP_TYPE_IMG;
				size = FRAME_SIZE/2;
			}
            else if(ips[0] == IP_TYPE_VE) {
				m_ipCallsPresentInTrace[IP_TYPE_VE]++;
				addr = VE_ADDR_START;
				ip_master = IP_TYPE_VE;
				size = FRAME_SIZE;
			}
			else if(ips[0] == IP_TYPE_MMC_IN) {
				m_ipCallsPresentInTrace[IP_TYPE_MMC_IN]++;
				addr = MMC_IN_ADDR_START;
				ip_master = IP_TYPE_MMC_IN;
				size = FRAME_SIZE/VID_CODING_RATIO;
				isRead = false;
			}
			else if(ips[0] == IP_TYPE_DC) {
				m_ipCallsPresentInTrace[IP_TYPE_DC]++;
				addr = DC0_ADDR_START;
				ip_master = IP_TYPE_DC;
				size = FRAME_SIZE;
			}
			else {
	            gemDroid->markIPRequestCompleted(core_id, IP_TYPE_CPU, core_id, m_frameNumber[IP_TYPE_DC]+1, 0);
				return;
            }
    	}

	    gemDroid->markIPRequestCompleted(core_id, IP_TYPE_CPU, core_id, m_frameNumber[ip_master]+1, 0);
    }
    else if (op.find("NW") != string::npos) {
    	m_ipCallsPresentInTrace[IP_TYPE_NW]++;
   		addr = NW_ADDR_START;
   		size = 1562;
    	ip_master = IP_TYPE_NW;
		isRead = false;
    }
    else if (op.find("SND-IN") != string::npos) {
    	
    	isAudioFrameDrop();

		gemDroid->nextIPtoCall(IP_TYPE_CPU, core_id, IP_TYPE_SND, ips);
        flowType = IP_TYPE_SND;

		if(ips[0] == IP_TYPE_MMC_IN) {
			//For audio-play trace
			m_ipCallsPresentInTrace[IP_TYPE_MMC_IN]++;
			m_ipCallsPresentInTrace[IP_TYPE_AD]++;
			m_ipCallsPresentInTrace[IP_TYPE_SND]++;
			addr = MMC_IN_ADDR_START;
			size = AUD_FRAME_SIZE/AUD_CODING_RATIO;
			ip_master = IP_TYPE_MMC_IN;
			isRead = false;
		}
        else if (ips[0] == -1) {
            return;
        }
		else {
			//For youtube trace
			m_ipCallsPresentInTrace[IP_TYPE_AD]++;
			m_ipCallsPresentInTrace[IP_TYPE_SND]++;
			addr = AD_ADDR_START;
			size = AUD_FRAME_SIZE;
			ip_master=IP_TYPE_AD;
		}
    }
    else if (op.find("SND-OUT") != string::npos) {
    	m_ipCallsPresentInTrace[IP_TYPE_MIC]++;
    	m_ipCallsPresentInTrace[IP_TYPE_AE]++;
   		addr = MIC_ADDR_START;
   		size = AUD_FRAME_SIZE;
    	ip_master=IP_TYPE_MIC;
    	isRead = false;	
        flowType = IP_TYPE_MIC;
    }
    else if (op.find("CAM") != string::npos) {
    	//For ar game trace
        if (!m_readFBLine)
            return;

        m_readFBLine = false;
    	m_ipCallsPresentInTrace[IP_TYPE_CAM]++;
    	m_ipCallsPresentInTrace[IP_TYPE_IMG]++;
    	m_ipCallsPresentInTrace[IP_TYPE_DC]++;
   		addr = CAM_ADDR_START;
   		size = FRAME_SIZE/1; // Cam is only 1080p
    	ip_master=IP_TYPE_CAM;
    	isRead = false;  
        flowType = IP_TYPE_CAM;

        em_trace_file>>dummy;
        em_trace_file>>dummy;
        em_trace_file>>dummy;
    }
    else {
    	return; // ignore DC-OUT
    }

	if(gemDroid->gemdroid_sa.isIPReqLimitReached(ip_master)) {
		m_thisFrameIPFullStalls++;
        // cout << "IP LIMIT REACHED" << ipTypeToString(ip_master) << endl;
		// gemDroid->m_ipCallDrops[ip_master]++;
		return;
	}

    // eventually this IP call will be serviced. So count it
    m_ipReqs++;
	frameNum = m_frameNumber[ip_master]++;

   	// if(!gemDroid->enqueueCoreIPReq(core_id, ip_master, addr, size, isRead, frameNum)) {
    int flows[MAX_FLOWS_IN_APP];
	gemDroid->flow_identification(core_id, ip_master, flows);
	if(!gemDroid->gemdroid_sa.enqueueCoreIPRequest(core_id, ip_master, addr, size, isRead, frameNum, flowType, flows[0])) {
   		cout << "FATAL: Core unable to inject " << ip_master << "ip request" << endl;
	}
}

void GemDroidCore::streakCalculator()
{
	if(idleStreak > 0) {
		gemDroid->idleStreaksinActivePState[IP_TYPE_CPU][core_id].sample(idleStreak);
		idleStreak=0; //Reset streak of rejects
	}
}

//Returns true when a frame has to be dropped
bool GemDroidCore::isFrameDrop()
{
  	startCycle = 0; //Simply setting here. Has to be set when fps_stalls are zero as well.
	m_thisFrameRobFullStalls=0;
	m_thisFrameMemFullStalls=0;
	m_thisFrameIPFullStalls=0;

	//gemDroid->m_cyclesPerFrame[IP_TYPE_CPU][core_id][m_frameNumber[IP_TYPE_DC]] = ticks - startCycle;
    //double gemDroidMultiplier = GEMDROID_FREQ / voltage_freq_table[CORE_DVFS_STATES-1][1];

	//If core is fast, we stall till we reach required FPS, and then issue the DC call.
	//Meanwhile, when the core is stalling, video decoder is invoked for next frame.
	if ((type_of_application == DISPLAY_BOUND || type_of_application == VIDEO_PLAYBACK)) {
		int ticksFromLastDC = gemDroid->getTicks() - lastDCTick;
		m_frameNumber[IP_TYPE_DC]++;

		/* cout<<" isFrameDrop? "<<endl<<" ticksFromLastDC "<< ticksFromLastDC\
			<<endl <<" FPS_TICKS: "<< FPS_TICKS\
			<<endl <<" lastDCTick: "<<lastDCTick\
			<<endl <<" ticks: "<<ticks<<endl; */

		if(lastDCTick == 0) {
			fpsStalls = FPS_TICKS - ticks;
			// fpsStalls=1;
            cout << "@"<<gemDroid->getTicks()/MILLISEC*1.0<<"ms FPS STALLS of: " << fpsStalls / MILLISEC*1.0 << " ms" << endl;
		    return false;
        }

		if(ticksFromLastDC < FPS_TICKS) {
			fpsStalls = FPS_TICKS - ticksFromLastDC;
            
            cout << "@"<<gemDroid->getTicks()/MILLISEC*1.0<<"ms FPS STALLS of: " << fpsStalls / MILLISEC*1.0 << " ms" << endl;
            /*cout << "LOWPOWER TIME: " << (CORE_PSTATE_LOWPOWER_ENTER_TIME + CORE_PSTATE_LOWPOWER_EXIT_TIME)*gemDroidMultiplier << endl;;
            cout << "IDLE TIME: " << (CORE_PSTATE_IDLE_ENTER_TIME + CORE_PSTATE_IDLE_EXIT_TIME)*gemDroidMultiplier << endl;*/
            /* if (fpsStalls > (CORE_PSTATE_LOWPOWER_ENTER_TIME + CORE_PSTATE_LOWPOWER_EXIT_TIME)*gemDroidMultiplier)
                setPStateLowPower();
            if (fpsStalls > (CORE_PSTATE_IDLE_ENTER_TIME + CORE_PSTATE_IDLE_EXIT_TIME)*gemDroidMultiplier)
                setPStateIdle(); */

            cout << "FPS STALLS " << fpsStalls << endl;

			return false;
		}
		else { //Frame was not shown within QOS deadline. So, counter reset, and the frame is dropped.
			//One frame is skipped from decoding data.
			lastDCTick = gemDroid->getTicks();
			m_framesDropped++;
			cout<<"Dropper! ROB Stalls:"<<m_thisFrameRobFullStalls.value()<<"  Mem Stalls:"<<m_thisFrameMemFullStalls.value()<<endl;

			if(m_thisFrameRobFullStalls.value() > m_thisFrameMemFullStalls.value() && m_thisFrameRobFullStalls.value() > m_thisFrameIPFullStalls.value())
				m_framesDroppedDueToRobStalls++;
			else if(m_thisFrameMemFullStalls.value() > m_thisFrameRobFullStalls.value() && m_thisFrameMemFullStalls.value() > m_thisFrameIPFullStalls.value() )
				m_framesDroppedDueToMemStalls++;
			else 
				m_framesDroppedDueToIPStalls++;

			return true;
		}
	}

	//For a non-display or video bound application, there is no concept of frame drop.
	return false;
}


//Returns true when a audio frame has to be dropped
bool GemDroidCore::isAudioFrameDrop()
{
 	//gemDroid->m_cyclesPerFrame[IP_TYPE_CPU][core_id][m_frameNumber[IP_TYPE_DC]] = ticks - startCycle;

	if(type_of_application == AUDIO_PLAYBACK) {
    	
    	//double gemDroidMultiplier = GEMDROID_FREQ / voltage_freq_table[CORE_DVFS_STATES-1][1];
		int ticksFromLastSND = gemDroid->getTicks() - lastSNDTick;
		m_frameNumber[IP_TYPE_SND]++;

		 if(lastSNDTick == 0) {
		 	audFpsStalls = 1;
		 	return false;
		 }

	 	if(ticksFromLastSND < AUD_FPS_TICKS) {
	 		audFpsStalls = AUD_FPS_TICKS - ticksFromLastSND;	 		
            return false;
	 	}
	 	else { //Frame was not shown within QOS deadline. So, counter reset, and the frame is dropped.
			//One frame is skipped from decoding data.
			lastSNDTick = gemDroid->getTicks();
			cout<<"AudDropper! ROB Stalls:"<<m_thisFrameRobFullStalls.value()<<"  Mem Stalls:"<<m_thisFrameMemFullStalls.value()<<endl;
			return true;
		}
	}

	return false;
}

double GemDroidCore::getStaticPower(double activePortion, double lowPortion, double idlePortion)
{
	double staticPower = voltage_freq_table[dvfsState][2] * activePortion + voltage_freq_table[dvfsState][2]/3 * lowPortion;

    return staticPower;
}

double GemDroidCore::getDynamicPower(double coreActivity, double coreStallActivity)
{
	//DynPower = CV^2F
    if (coreActivity > 1)
        coreActivity = 1;
    double dynamicPowerConsumedInThisMS = coreActivity * voltage_freq_table[dvfsState][3] +  coreStallActivity * voltage_freq_table[dvfsState][3]/3;
    // Stall Power is 1/3rd of full power (instruction committed). From Pentium pro power breakdown, SIGMETRICS 2001

    return dynamicPowerConsumedInThisMS;
}

double GemDroidCore::powerIn1us()
{
	long one_microsec = getCoreFreq()*1000; //if 2 ghz core, we have 2K ticks in 1 microsecs

    double staticPowerConsumedInThisuS = getStaticPower((double)m_thisMicroSecActivePStateCycles/one_microsec, (double)m_thisMicroSecLowpowerPStateCycles/one_microsec, (double)m_thisMicroSecIdlePStateCycles/one_microsec);

    double dynamicPowerConsumedInThisuS = getDynamicPower((double)m_thisMicroSecInstructionsCommitted / (issue_width*one_microsec), m_thisMicroSecRobFullStalls/(issue_width*one_microsec));

    double powerInLastus = staticPowerConsumedInThisuS + dynamicPowerConsumedInThisuS;

	// cout<<"CPU"<< core_id <<" us power: Static: "<<  staticPowerConsumedInThisuS <<" Dynamic: "<< dynamicPowerConsumedInThisuS << " Total = "<< powerInLastus <<endl;

	m_thisMicroSecActivePStateCycles 	=  0;
	m_thisMicroSecLowpowerPStateCycles 	=  0;
	m_thisMicroSecIdlePStateCycles 		=  0;
	m_thisMicroSecInstructionsCommitted =  0;
	m_thisMicroSecRobFullStalls 		=  0;

    return powerInLastus;
}

double GemDroidCore::powerIn1ms()
{
	long one_millisec = getCoreFreq()*1000000; //if 2 ghz core, we have 2M ticks in 1 mili secs
 
	// Power of core is composed into two parts - static and dynamic
	// if core was in active state for one-millisecond, then we use the max power that can be consumed by the core - which is around 0.75W.

	// cout << "one_millisec: " << one_millisec << endl;

	// cout<<"m_thisMilliSecInstructionsCommitted: "<<m_thisMilliSecInstructionsCommitted << endl;

    double staticPowerConsumedInThisMS = getStaticPower((double)m_thisMilliSecActivePStateCycles/one_millisec, (double)m_thisMilliSecLowpowerPStateCycles/one_millisec, (double)m_thisMilliSecIdlePStateCycles/one_millisec);
    double dynamicPowerConsumedInThisMS = getDynamicPower((double)m_thisMilliSecInstructionsCommitted / (issue_width*one_millisec), m_thisMilliSecRobFullStalls/(issue_width*one_millisec));

	cout<<"CPU" << core_id << " Active: "<<  (double)m_thisMilliSecActivePStateCycles/one_millisec <<" LowPower: "<< (double)m_thisMilliSecLowpowerPStateCycles/one_millisec << " Idle: " << (double)m_thisMilliSecIdlePStateCycles/one_millisec <<endl;
	cout<<"CPU" << core_id << " Static: "<<  staticPowerConsumedInThisMS <<" Dynamic: "<< dynamicPowerConsumedInThisMS << " Total: " << staticPowerConsumedInThisMS + dynamicPowerConsumedInThisMS <<endl;

	m_thisMilliSecActivePStateCycles 	=  0;
	m_thisMilliSecLowpowerPStateCycles 	=  0;
	m_thisMilliSecIdlePStateCycles 		=  0;
	m_thisMilliSecInstructionsCommitted =  0;
	m_thisMilliSecRobFullStalls 		=  0;
    m_thisMilliSecIdleStalls            =  0;

    powerInLastEpoch = staticPowerConsumedInThisMS + dynamicPowerConsumedInThisMS;

	return powerInLastEpoch;
}

double GemDroidCore::getLoadInLastDVFSEpoch()
{
	double ticks_in_epoch = (gemDroid->lastTimeTook[IP_TYPE_CPU]*1000000) * voltage_freq_table[dvfsState][1];

	// cout<<"ticks_in_epoch" << ticks_in_epoch<<endl;

	double core_util = m_thisDVFSEpochInstructionsCommitted / (ticks_in_epoch*issue_width);

	m_thisDVFSEpochInstructionsCommitted = 0;

	return core_util;
}

double GemDroidCore::getPowerEst()
{
	if(isPStateActive())
		return voltage_freq_table[dvfsState][2]/2 + voltage_freq_table[dvfsState][3]/2;
	return 0;
}

double GemDroidCore::getTimeEst(double time, double oldFreq, double newFreq)
{
    return time * (oldFreq/newFreq);
}

double GemDroidCore::getEnergyEst(double time, double power, double newFreq)
{
    // double energy = time * power;
    double dynPower = power - CORE_STATIC_PWR;
    double activity = dynPower / voltage_freq_table[dvfsState][3]; // Denominator is the max dynamic power at the curr freq

    int ind = getIndexInTable(newFreq);
    double newTime = time * (getCoreFreq()/newFreq);
    double newDynPower = activity * voltage_freq_table[ind][3];
    double newStatEnergy = newTime * CORE_STATIC_PWR;
    double newDynEnergy = newTime * newDynPower;
    double newEnergy = newStatEnergy + newDynEnergy;

    // cout << "Slack CPU" << " " << activity << " " << time << " " << power << " " << energy << endl;

    return newEnergy;
}

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

GemDroidSA::GemDroidSA(int id, GemDroid *gemDroid)
{
	sa_id = id;
	this->gemDroid = gemDroid;
	desc = "GemDroid.SA_";
	desc += (char) (id + '0');

	cout << "Instantiated " << desc << std::endl;


	ticks = 0;
	numRejected = 0;
	numIPReqs = 0;
	numIPMemReqs = 0;
	numMemCoreResponse = 0;
	numIPCoreResponse = 0;
	numMemIPResponse = 0;
	numCoreMemReqs = 0;

	dynamicActivity = 0;
	
	stats_ticks = 0;
	stats_numCoreMemReqs = 0;
	stats_numIPReqs = 0;
	stats_numIPMemReqs = 0;
	stats_numMemCoreResponse = 0;
	stats_numIPCoreResponse = 0;
	stats_numMemIPResponse = 0;
	stats_numRejected = 0;
}

void GemDroidSA::regStats()
{
	ticks.name(desc + ".cycles").desc("GemDroid SA: Number of cycles").flags(Stats::display);
	numCoreMemReqs.name(desc + ".numCoreMemReqs").desc("GemDroid SA: Number of memory requests from core").flags(Stats::display);
	numIPReqs.name(desc + ".numCoreIPReqs").desc("GemDroid SA: Number of IP requests").flags(Stats::display);
	numIPMemReqs.name(desc + ".numIPMemReqs").desc("GemDroid SA: Number of memory requests from IP").flags(Stats::display);
	numMemCoreResponse.name(desc + ".numMemCoreResponse").desc("GemDroid SA: Number of memory response to core").flags(Stats::display);
	numIPCoreResponse.name(desc + ".numIPCoreResponse").desc("GemDroid SA: Number of IP responses to core").flags(Stats::display);
	numMemIPResponse.name(desc + ".numMemIPResponse").desc("GemDroid SA: Number of memory responses to IP").flags(Stats::display);
	numRejected.name(desc + ".numRejected").desc("GemDroid SA: Number of IP memory requests rejected by SA").flags(Stats::display);
}

void GemDroidSA::resetStats()
{
	ticks = 0;
	numRejected = 0;
	numIPReqs = 0;
	numIPMemReqs = 0;
	numMemCoreResponse = 0;
	numIPCoreResponse = 0;
	numMemIPResponse = 0;
	numCoreMemReqs = 0;

	stats_ticks = 0;
	stats_numCoreMemReqs = 0;
	stats_numIPReqs = 0;
	stats_numIPMemReqs = 0;
	stats_numMemCoreResponse = 0;
	stats_numIPCoreResponse = 0;
	stats_numMemIPResponse = 0;
	stats_numRejected = 0;
}

void GemDroidSA::printPeriodicStats()
{

	cout << "-=-=-=-=-=-=-=-=-=-=-=-" << endl;
	//Total stats
	if (0) {
		cout << desc << ".m_cycles: " << ticks.value() << endl;
		cout << desc << ".numCoreMemReqs: " << numCoreMemReqs.value() << endl;
		cout << desc << ".numCoreIPReqs: " << numIPReqs.value() << endl;
		cout << desc << ".numIPMemReqs: " << numIPMemReqs.value() << endl;
		cout << desc << ".numMemCoreResponse: " << numMemCoreResponse.value() << endl;
		cout << desc << ".numIPCoreResponse: " << numIPCoreResponse.value() << endl;
		cout << desc << ".numMemIPResponse: " << numMemIPResponse.value() << endl;
		cout << desc << ".numRejected: " << numRejected.value() << endl;
		//cout << "Average Mem Transaction Queue size: " << (double) totalQueueSize / cyclesElapsed << endl;
	} else { //Per Phase stats
		cout << desc << ".m_cycles: " << ticks.value() - stats_ticks << endl;
		cout << desc << ".numMemReqs: " << numCoreMemReqs.value() - stats_numCoreMemReqs << " " <<  numIPMemReqs.value() - stats_numIPMemReqs << endl;
		//cout << desc << ".numIPReqs: " << numIPReqs.value() - stats_numIPReqs << endl;
		//cout << desc << ".numIPMemReqs: " <<
		//cout << desc << ".numMemCoreResponse: " << numMemCoreResponse.value() - stats_numMemCoreResponse << endl;
		//cout << desc << ".numIPCoreResponse: " << numIPCoreResponse.value() - stats_numIPCoreResponse << endl;
		//cout << desc << ".numMemIPResponse: " << numMemIPResponse.value() - stats_numMemIPResponse << endl;
		//cout << desc << ".nemRejected: " << numRejected.value() - stats_numRejected << endl;
	}

	//queue Sizes
	//cout << "Average Mem Transaction Queue size: " << (double) totalQueueSize / PERIODIC_STATS << endl;
	totalQueueSize = 0;
/*	cout << desc << ".CoretoMem: " << coreMemReq.size() << endl;
	cout << desc << ".IPtoMem: " << ipMemReq.size() << endl;
	cout << desc << ".MemtoCore: " << memCoreResp.size() << endl;
	cout << desc << ".MemtoIP: " << memIpResp.size() << endl;*/
	
	cout << desc<<" ";//".IP" << ipTypeToString(i)
	for (int i = 1; i < IP_TYPE_DMA; i++)
		cout<<" " << ipReq[i].size();
	cout<<endl;

	list<GemDroidMemMsg>::iterator it;
	for (it = ipMemReq.begin(); it != ipMemReq.end(); it++) {
		if (it->getIsRead())
			readMemCounter[it->getIpType()]++;
		else
			writeMemCounter[it->getIpType()]++;
	}

	/*cout << desc << ".IPtoMemRead: "; //<<coreMemReq.size()<<endl;
	for (int i = 1; i < IP_TYPE_END; i++) {
		cout << readMemCounter[i] << "\t";
		readMemCounter[i] = 0;
	}
	cout << endl;

	cout << desc << ".IPtoMemWrite: "; //<<coreMemReq.size()<<endl;
	for (int i = 1; i < IP_TYPE_END; i++) {
		cout << writeMemCounter[i] << "\t";
		writeMemCounter[i] = 0;
	}
	cout << endl;
*/
	stats_ticks = ticks.value();

	stats_numCoreMemReqs = numCoreMemReqs.value();
	stats_numIPReqs = numIPReqs.value();
	stats_numIPMemReqs = numIPMemReqs.value();
	stats_numMemCoreResponse = numMemCoreResponse.value();
	stats_numIPCoreResponse = numIPCoreResponse.value();
	stats_numMemIPResponse = numMemIPResponse.value();
	stats_numRejected = numRejected.value();
}

void GemDroidSA::sendMemoryRequests()
{
	if (!coreMemReq.empty()) {
		GemDroidMemMsg request = coreMemReq.front();
		if (gemDroid->gemdroid_memory.enqueueMemReq(IP_TYPE_CPU, request.getId(), request.getCoreId(), request.getAddr(), request.getIsRead())) {
			coreMemReq.pop_front();
			updateActivityCountIn1Ms();
			return;
		} else {
			; // cout<<"SA not able to inject Core Mem Request"<<endl;
			return;  //Anyway, no space in memQueue, so dont even try to inject.
		}
	}

	if (!ipMemReq.empty()) {
		GemDroidMemMsg request = ipMemReq.front();
		if (gemDroid->gemdroid_memory.enqueueMemReq(request.getIpType(), request.getId(), request.getCoreId(), request.getAddr(), request.getIsRead())) {
			ipMemReq.pop_front();
			updateActivityCountIn1Ms();
			return;
		} else
			; // cout<<"SA not able to inject IP Mem Request"<<endl;
	}
}

void GemDroidSA::sendMemoryResponses()
{

	if (gemDroid->isPerfectMemory()) {
		if (!memCoreResp.empty()) {
			GemDroidMemMsg response = memCoreResp.front();
			gemDroid->memCoreResponse(response.getIpType(), response.getId(), response.getAddr(), response.getIsRead());
			memCoreResp.pop_front();
			m_cyclesToSkip = 0;
			updateActivityCountIn1Ms();
			return;
		}

		if (!memIpResp.empty()) {
			GemDroidMemMsg response = memIpResp.front();
			gemDroid->memIPResponse(response.getIpType(), response.getId(), response.getAddr(), response.getIsRead());
			memIpResp.pop_front();
			m_cyclesToSkip = 0;
			updateActivityCountIn1Ms();
			return;
		}
		return;
	}

	if (!memCoreResp.empty()) {
		GemDroidMemMsg response = memCoreResp.front();
		gemDroid->memCoreResponse(response.getIpType(), response.getId(), response.getAddr(), response.getIsRead());
		memCoreResp.pop_front();
		if (response.getIsRead())
			m_cyclesToSkip = MEM_RESP_TRANSMIT_CYCLES;
		else
			m_cyclesToSkip = 1;
		updateActivityCountIn1Ms(MEM_RESP_TRANSMIT_CYCLES);
		return;
	}

	if (!memIpResp.empty()) {
		GemDroidMemMsg response = memIpResp.front();
		gemDroid->memIPResponse(response.getIpType(), response.getId(), response.getAddr(), response.getIsRead());
		memIpResp.pop_front();
		if (response.getIsRead())
			m_cyclesToSkip = MEM_RESP_TRANSMIT_CYCLES;
		else
			m_cyclesToSkip = 1;
		updateActivityCountIn1Ms(MEM_RESP_TRANSMIT_CYCLES);
		return;
	}
}

void GemDroidSA::sendIPRequests()
{
	for (int i = 1; i < IP_TYPE_END; i++)
		if (!ipReq[i].empty()) {
			GemDroidIPRequest request = ipReq[i].front();
			if (gemDroid->enqueueIPReq(request.getSenderType(), request.getSenderId(), request.getCoreId(), request.getIpType(), request.getAddr(), request.getSize(), request.getIsRead(), request.getFrameNum(), request.getFlowType(), request.getFlowId())) {
				// gemDroid->gemdroid_core[request.getCoreId()].markIPRequestStarted(request.getIpType(), 0, request.getFrameNum());
				ipReq[i].pop_front();
				updateActivityCountIn1Ms();
				return;
			} else
				; // cout<<"SA not able to inject into IP "<< request.getIpType() << " a Request"<<endl;
		}
}

void GemDroidSA::sendIPResponses()
{
	if (!ipCoreResp.empty()) {
		// GemDroidIPResponse response = ipCoreResp.front();
		// gemDroid->gemdroid_core[response.getCoreId()].markIPRequestCompleted(response.getSenderType(), response.getSenderId(), response.getFrameNum());
		ipCoreResp.pop_front();
		updateActivityCountIn1Ms();
		return;
	}
}

void GemDroidSA::process()
{
	sendIPResponses();

	for (int i=0; i<gemDroid->gemdroid_memory.getNumChannels(); i++) {
		sendMemoryRequests();
	}
	
	sendIPRequests();

	if (m_cyclesToSkip > 0) {
		m_cyclesToSkip--;
		return;
	}

	for (int i=0; i<gemDroid->gemdroid_memory.getNumChannels(); i++) {
		sendMemoryResponses();
	}
}

void GemDroidSA::tick()
{
	ticks++;

	totalQueueSize += coreMemReq.size() + ipMemReq.size();

	process();
}

bool GemDroidSA::enqueueCoreMemRequest(int id, uint64_t addr, bool isRead)
{
	if (coreMemReq.size() + ipMemReq.size() > MAX_MEM_REQS) {
		numRejected++;
		return false;
	}

	if (memCoreResp.size() + memIpResp.size() > MAX_MEM_RESPS) {
		numRejected++;
		return false;
	}

	numCoreMemReqs++;
	GemDroidMemMsg request(IP_TYPE_CPU, id, id, addr, isRead, false);  //ip id is also core_id here.
	coreMemReq.push_back(request);

	return true;
}

bool GemDroidSA::enqueueCoreIPRequest(int receiverCoreId, int iptype, uint64_t addr, int size, bool isRead, int frameNum, int flowType, int flowId)
{
	assert(iptype != 0);
	assert(iptype != IP_TYPE_END);

	if (iptype <= 0 || iptype >= IP_TYPE_END)
		assert(1);

	numIPReqs++;
	GemDroidIPRequest request(IP_TYPE_CPU, receiverCoreId, receiverCoreId, iptype, addr, size, isRead, frameNum, flowType, flowId);

	assert(ipReq[iptype].size() < MAX_IP_OUTSTANDING_REQS);

	ipReq[iptype].push_back(request);

	return true;
}

// This function will be used for an IP (GPU, VD) to call another IP (DC, DMA) respectively.
bool GemDroidSA::enqueueIPIPRequest(int sendertype, int senderid, int receiverCoreId, int iptype, uint64_t addr, int size, bool isRead, int frameNum, int flowType, int flowId)
{
	assert(iptype != 0);
	assert(iptype != IP_TYPE_END);

	if (iptype <= 0 || iptype >= IP_TYPE_END)
		assert(1);

	if (isIPReqLimitReached(iptype)) {
		// gemDroid->m_ipCallDrops[iptype]++;
		return false;
	}

	numIPReqs++;
	GemDroidIPRequest request(sendertype, senderid, receiverCoreId, iptype, addr, size, isRead, frameNum, flowType, flowId);

	ipReq[iptype].push_back(request);

	return true;
}

bool GemDroidSA::enqueueIPMemRequest(int ip_type, int ip_id, int core_id, uint64_t addr, bool isRead)
{
	if (ipMemReq.size() > MAX_IP_MEM_REQS) {
		numRejected++;
		return false;
	}

	if (memCoreResp.size() + memIpResp.size() > MAX_MEM_RESPS) {
		numRejected++;
		return false;
	}

	numIPMemReqs++;

	GemDroidMemMsg request(ip_type, ip_id, core_id, addr, isRead, false);
	ipMemReq.push_back(request);

	return true;
}

bool GemDroidSA::enqueueIPResponse(int senderIPType, int senderIPId, int receiverCoreId, int frameNum, int flowType, int flowId)
{
	if (senderIPType <= IP_TYPE_CPU || senderIPType >= IP_TYPE_END)
		assert(1);

	// GemDroidIPResponse response(senderIPType, senderIPId, receiverCoreId, frameNum);
	// ipCoreResp.push_back(response);
	// long timeTook = gemDroid->gemdroid_core[receiverCoreId].markIPRequestCompleted(senderIPType, senderIPId, frameNum, flowId);
	long timeTook = gemDroid->markIPRequestCompleted(receiverCoreId, senderIPType, senderIPId, frameNum, flowId);
    timeTook++;
	numIPCoreResponse++;

	if(MOTIVATION_GRAPHS)
		return true;

	//Find if another IP is to be called in the pipeline/flow.

	// if(gemDroid->isIPCallDrop(senderIPType, timeTook))
		// ; //return true;

    int flows[MAX_FLOWS_IN_APP];
	gemDroid->flow_identification(receiverCoreId, senderIPType, flows);
	int flowid1 = flows[0];
	int flowid2 = flows[1];
    int ips[MAX_IPS_IN_FLOW];

	if (senderIPType == IP_TYPE_CAM) {
		gemDroid->nextIPtoCall(senderIPType, receiverCoreId, IP_TYPE_DC, ips);
		if (ips[0] == IP_TYPE_IMG)
			if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_IMG, CAM_ADDR_START, 4*FRAME_SIZE/2, true, frameNum, flowType, flowid1))
			    cout << "Dropper! CAM IP unable to inject to IMG IP" << endl;
		gemDroid->nextIPtoCall(senderIPType, receiverCoreId, IP_TYPE_MMC_OUT, ips);
		if (ips[0] == IP_TYPE_VE)
			if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_VE, CAM_ADDR_START, 4*FRAME_SIZE/1, true, frameNum, flowType, flowid2))
				cout << "Dropper! CAM IP unable to inject to VE IP" << endl;
		gemDroid->nextIPtoCall(senderIPType, receiverCoreId, IP_TYPE_NW, ips);
		if (ips[0] == IP_TYPE_VE)
			if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_VE, CAM_ADDR_START, 4*FRAME_SIZE/1, true, frameNum, flowType, flowid2))
				cout << "Dropper! CAM IP unable to inject to VE IP" << endl;
	} else if (senderIPType == IP_TYPE_MIC) {
		if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_AE, MIC_ADDR_START, 4*AUD_FRAME_SIZE, true, frameNum, flowType, flowid1))
			cout << "Dropper! MIC IP unable to inject to AE IP" << endl;
	} else if (senderIPType == IP_TYPE_VD) {
		if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_DC, VD_ADDR_START + FRAME_SIZE, 4*FRAME_SIZE, true, frameNum, flowType, flowid1))
			cout << "Dropper! IP VD unable to inject a DC ip request" << endl;
	} else if (senderIPType == IP_TYPE_AD) {
		if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_SND, AD_ADDR_START + AUD_FRAME_SIZE, 4*AUD_FRAME_SIZE, true, frameNum, flowType, flowid1))
			cout << "Dropper! IP AD unable to inject a SND ip request" << endl;
	} else if (senderIPType == IP_TYPE_IMG) {
		if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_DC, IMG_ADDR_START + FRAME_SIZE, 4*FRAME_SIZE, true, frameNum, flowType, flowid1))
			cout << "Dropper! IP IMG unable to inject a DC ip request" << endl;
		//If photocapture, then call MMC as well. Else, move on.
		// if (gemDroid->nextIPtoCall(senderIPType, receiverCoreId, IP_TYPE_DC, ips) != -1) {
		gemDroid->nextIPtoCall(senderIPType, receiverCoreId, IP_TYPE_MMC_OUT, ips);
		if (ips[0] != -1) {
			if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_MMC_OUT, IMG_ADDR_START + FRAME_SIZE, FRAME_SIZE, false, frameNum, flowType, flowid2))
				cout << "Dropper! IP IMG unable to inject a MMC ip request" << endl;
		}
	} else if (senderIPType == IP_TYPE_VE) {
		// int next_ip1 = gemDroid->nextIPtoCall(senderIPType, receiverCoreId, IP_TYPE_VE, ips);
		gemDroid->nextIPtoCall(senderIPType, receiverCoreId, IP_TYPE_VE, ips);
		int next_ip1 = ips[0];
		if(next_ip1 == IP_TYPE_NW)
			return true;

		if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_MMC_OUT, VE_ADDR_START + FRAME_SIZE, FRAME_SIZE / (VID_CODING_RATIO), false, frameNum, flowType, flowid1))
			cout << "Dropper! IP VE unable to inject a MMC ip request" << endl;
	} else if (senderIPType == IP_TYPE_AE) {
		// int next_ip1 = gemDroid->nextIPtoCall(senderIPType, receiverCoreId, IP_TYPE_AE, ips);
		gemDroid->nextIPtoCall(senderIPType, receiverCoreId, IP_TYPE_AE, ips);
		int next_ip1 = ips[0];
		if(next_ip1 == IP_TYPE_NW)
			return true;
		if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_MMC_OUT, AE_ADDR_START + AUD_FRAME_SIZE, AUD_FRAME_SIZE / AUD_CODING_RATIO, false, frameNum, flowType, flowid1))
			cout << "Dropper! IP AE unable to inject a MMC ip request" << endl;
	} else if (senderIPType == IP_TYPE_MMC_IN) {
		int next_ip;

        gemDroid->nextIPtoCall(senderIPType, receiverCoreId, flowType, ips);

		next_ip = ips[0];

		if (next_ip == IP_TYPE_AD) {
			if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_AD, MMC_IN_ADDR_START, 4*AUD_FRAME_SIZE / AUD_CODING_RATIO, true, frameNum, flowType, flowid1))
				cout << "Dropper! IP MMC unable to inject a AD ip request" << endl;
		} else if (next_ip == IP_TYPE_IMG) {
			if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_IMG, MMC_IN_ADDR_START, 4*FRAME_SIZE/2, true, frameNum, flowType, flowid1))
				cout << "Dropper! IP MMC unable to inject a IMG ip request" << endl;
		} else if (next_ip == IP_TYPE_VD) {
			if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_VD, MMC_IN_ADDR_START, 4*FRAME_SIZE / VID_CODING_RATIO, true, frameNum, flowType, flowid1))
				cout << "Dropper! IP MMC unable to inject a VD ip request" << endl;
		}
	} else if (senderIPType == IP_TYPE_GPU) {
		if (!enqueueIPIPRequest(senderIPType, senderIPId, receiverCoreId, IP_TYPE_DC, DC1_ADDR_START + FRAME_SIZE, 4*FRAME_SIZE, true, frameNum, flowType, flowid1))
			cout << "Dropper! IP GPU unable to inject a DC1 ip request" << endl;
	}

	return true;
}

void GemDroidSA::memResponse(uint64_t addr, bool isRead, int sender_type, int sender_id)
{
	// int ip_type;
	// int senderIPId = findIPFromMap(addr, &ip_type);

	GemDroidMemMsg response(sender_type, sender_id, 0, addr, isRead, true);

	// response for core
	if (sender_type == IP_TYPE_CPU) {
		// All CPU addresses are below 2GB
		assert(addr < 2 * 1024 * 1024 * 1024L);
		memCoreResp.push_back(response);
		numMemCoreResponse++;
	} else if (sender_type == -1) {
		cout << "FATAL!";
		assert(1);
	} else {
		// All IP addresses are above 2GB
		assert(addr > 2 * 1024 * 1024 * 1024L);
		memIpResp.push_back(response);
		numMemIPResponse++;
	}
}

bool GemDroidSA::isIPReqLimitReached(int ip_type)
{
//	 for(int i=1; i<IP_TYPE_END; i++)
	if (ipReq[ip_type].size() >= MAX_IP_OUTSTANDING_REQS)
		return true;

	return false;
}

double GemDroidSA::powerIn1ms()
{
	//CV2F
	//V2F = 1.1 * 1.1 * 0.5 = 0.605
	//If max = 0.5W, C = 0.5 / 0.604
	double dynamicPowerin1ms = ((0.5 / 0.605) * (dynamicActivity/750000.0))  *  1.1  *  1.1  *  0.5;   //CV2F;
	double staticPowerin1ms = 0.025;

	dynamicActivity = 0;
	return (staticPowerin1ms + dynamicPowerin1ms);
}

double GemDroidSA::getActivity()
{
	return (dynamicActivity/750000.0);	
}


void GemDroidSA::updateActivityCountIn1Ms(int activity_count)
{
	dynamicActivity+=activity_count;
}

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

#ifndef GEMDROID_REQUEST_HH_
#define GEMDROID_REQUEST_HH_

string ipTypeToString(int);

class GemDroidMemMsg
{
private:
	int ip_type;
    int id;
    uint64_t addr;
    bool isRead;
    bool isResponse;
    int core_id;

public:
    GemDroidMemMsg(int type, int id, int core_id, uint64_t addr, bool isRead, bool isResponse) { this->ip_type = type; this->id = id; this->core_id = core_id; this->addr = addr; this->isRead = isRead; this->isResponse = isResponse; }
    inline uint64_t getIpType() { return ip_type; }
    inline uint64_t getId() { return id; }
    inline uint64_t getAddr() { return addr; }
    inline bool getIsRead() { return isRead; }
    inline bool getIsResponse() { return isResponse; }
    inline void setIpType(int ip_type) { this->ip_type = ip_type; }
    inline int getCoreId() { return core_id; }
    inline void print() { cout << "GemDroidMemMsg:  IpType: " <<  ipTypeToString(ip_type) << " ipId: " << id << " addr: " << addr << " isRead: " << isRead << " isResponse: " << isResponse << endl; }
};

class GemDroidIPRequest
{
private:
	int sendertype;
    int iptype;
    int sender_id;
    int core_id;
    uint64_t addr;
    int size;
    bool isRead;
    int frameNum;
    int flowType;
    int flowId;

public:
    GemDroidIPRequest(int sendertype, int sender_id, int core_id, int iptype, uint64_t addr, int size, bool isRead, int frameNum, int flowType, int flowId) { this->sendertype = sendertype; this->sender_id = sender_id; this->iptype = iptype; this->core_id = core_id; this->addr = addr; this->size = size; this->isRead = isRead; this->frameNum = frameNum; this->flowType = flowType; this->flowId = flowId; }
    inline int getSenderType() { return sendertype; }
    inline int getIpType() { return iptype; }
    inline int getSenderId() { return sender_id; }
    inline int getCoreId() { return core_id; }
    inline uint64_t getAddr() { return addr; }
    inline int getSize() { return size; }
    inline bool getIsRead() { return isRead; }
    inline int getFrameNum() { return frameNum; }
    inline int getFlowType() { return flowType; }
    inline int getFlowId() { return flowId; }
};

class GemDroidIPResponse
{
private:
	int sender_type;
    int sender_id;
    int core_id;
    int frame_num;
public:
    GemDroidIPResponse(int sender_type, int sender_id, int core_id, int frame_num) { this->sender_type = sender_type; this->sender_id = sender_id; this->core_id = core_id; this->frame_num = frame_num; }
    inline int getSenderType() { return sender_type; }
    inline int getSenderId() { return sender_id; }
    inline int getCoreId() { return core_id; }
    inline int getFrameNum() { return frame_num; }
};

#endif /* GEMDROID_REQUEST_HH_ */

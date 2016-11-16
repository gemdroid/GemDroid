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

#ifndef GEMDROID_CORE_UTIL_HH_
#define GEMDROID_CORE_UTIL_HH_

#include <iomanip>

using namespace std;

enum INSTR_TYPE
{
	INSTR_CPU,
	INSTR_MMU_LD,
	INSTR_MMU_ST,
	INSTR_OTHERS
};

class GemDroidOoOTransaction
{
private:
	int transaction_type;
	long long number_of_instructions;
	long long instr_executed_ooo;
	uint64_t addr;
	bool can_be_committed;
	long inserted_tick;
	bool isIssued;

public:

	GemDroidOoOTransaction(int transaction_type, long long number_of_instructions, uint64_t addr)
	{
		this->transaction_type = transaction_type;
		this->number_of_instructions = number_of_instructions;
		this->addr = addr;

		instr_executed_ooo = 0;
		can_be_committed = false;
		inserted_tick = -1;
		isIssued = false;

		if(transaction_type == INSTR_CPU)
			assert (addr == -1);
		if(transaction_type == INSTR_MMU_LD || transaction_type == INSTR_MMU_ST)
				assert (addr != 1);
		if(transaction_type == INSTR_MMU_LD || transaction_type == INSTR_MMU_ST)
			assert (number_of_instructions == 0);
	}

	inline uint64_t getAddr() { return addr; }
	inline long getInsertedTick() { return inserted_tick; }
	inline void setInsertedTick(long tick) { inserted_tick = tick;}
	inline bool isRead(){ assert (transaction_type != INSTR_CPU); return (transaction_type == INSTR_MMU_LD);}
	inline long long getNumOfInstructions() { return number_of_instructions; }
	inline long long getNumOfCommittedInstructions() { return instr_executed_ooo; }
	inline int getTransactionType() { return transaction_type; }
	inline void commit_transaction() { can_be_committed = true; }
	inline void commit_instruction() { assert (number_of_instructions); number_of_instructions--; instr_executed_ooo++; }
	inline void reset_instr_committed() { instr_executed_ooo = 0;}
	inline bool isTransactionReadyToCommit() { return can_be_committed; }
	inline void issueToMem() { isIssued = true; }
	inline bool isIssuedToMem() { return isIssued; }
	void print()
	{
		cout<< "\t Transaction type : \t"<<transaction_type<<endl;
		cout<< "\t Num of insn: \t\t"<<number_of_instructions<<endl;
		cout<< "\t Instructions Committed OoO: "<<instr_executed_ooo<<endl;
		cout<< "\t Address: \t"<<std::setw(20)<<addr<<endl;
		cout<< "\t Can_be_committed: "<<std::setw(20)<<can_be_committed<<endl;
	}
};

#endif /* GEMDROID_CORE_UTIL_HH_ */

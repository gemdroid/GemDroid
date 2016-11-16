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

#ifndef GEMDROID_DEFINES_HH_
#define GEMDROID_DEFINES_HH_

#define DBGPRINT cout<<ticks.value()<<" DBG2: "<<ipTypeToString(ip_type)

#define CACHE_LINE_SIZE 64
#define MAX_FLOWS 21
#define MAX_IPS 4   //MAX 4 IP instances of each type
#define MAX_CPUS 4

#define TARGET_FPS 60
#define FPS_DEADLINE_SAFETYNET 1
#define FPS_DEADLINE ((1000/TARGET_FPS)-FPS_DEADLINE_SAFETYNET)

#define GEMDROID_FREQ (10.0)  // 10 GHz
// #define GEMDROID_TO_CORE 25 // 400 MHz
// #define CORE_TO_ACC_FREQ 20 // 500 MHz
// #define CORE_TO_DEV_FREQ 20 // 500 MHz
#define GEMDROID_TO_SA_FREQ 20 // 500 MHz
#define GEMDROID_TO_MEM_FREQ 12 // 833 MHz

#define MILLISEC (1000000 * (int) GEMDROID_FREQ)
#define MICROSEC (1000 * (int) GEMDROID_FREQ)
 
#define MAX_MEM_REQS 256
#define MAX_IP_MEM_REQS (MAX_MEM_REQS*0.9)
#define MAX_MEM_RESPS 256
#define MAX_IP_OUTSTANDING_REQS 10
#define MEM_RESP_TRANSMIT_CYCLES 2
#define PEFECT_MEM_LATENCY 1

#define MAX_MEM_FREQ 1.0
#define MIN_MEM_FREQ 0.3

#define EPSILON 0.000005

#define INTERACTIVE_GOVERNOR_LAT 5

// #define FRAME_SIZE ((1920*1080*32)/8)
// #define FRAME_SIZE ((2560*1600*32)/8)
#define FRAME_SIZE ((4096*2160*24)/8)
// #define AUD_FRAME_SIZE (4*1024)
#define AUD_FRAME_SIZE (16*1024)

#define VID_CODING_RATIO 16
#define AUD_CODING_RATIO 8

#define CORE_DVFS_STATES 13
#define IP_DVFS_STATES 7

#define AD_AE_RATIO_TO_VD 12

enum IP_TYPE
{
	IP_TYPE_CPU = 0,
	IP_TYPE_DC,
	IP_TYPE_NW,
	IP_TYPE_SND,
	IP_TYPE_MIC,
	IP_TYPE_CAM,
	IP_TYPE_MMC_IN,
	IP_TYPE_MMC_OUT,
	IP_TYPE_VD, // 8
	IP_TYPE_VE,
	IP_TYPE_AD,
	IP_TYPE_AE,
	IP_TYPE_IMG,
	IP_TYPE_GPU, // 13
	IP_TYPE_DMA,
	IP_TYPE_CACHE,
	IP_TYPE_END // 16
};

enum GOVERNOR_TYPE
{
    GOVERNOR_TYPE_DISABLED = 0,
    GOVERNOR_TYPE_ONDEMAND,                 //1  Go to highest when load comes and come down in steps 
    GOVERNOR_TYPE_PERFORMANCE,              //2 Always stay is highest dvfs state
    GOVERNOR_TYPE_POWERSAVE,                //3 Always stay is lowest dvfs state
    GOVERNOR_TYPE_BUILDING,                 //4 Slowly increments freq as long as load is high
    GOVERNOR_TYPE_INTERACTIVE,              //5 Goes to highest dvfs state when becoming active and stays there for atleast INTERACTIVE_GOVERNOR_LAT ms
    GOVERNOR_TYPE_POWERCAP,                 //6 Doesn't allow the power to go beyond given limit
    GOVERNOR_TYPE_SLACK,                    //7 Predicts slack based on history and identifies correct frequencies; always prioritize core (or IPs) based on DVFS_PRIORITIZE_CORE flag.
    GOVERNOR_TYPE_SLACK_FAR_ENERGY,            //8 Predicts slack based on history and identifies correct frequencies according to which component is farthest away from their optimal freqs.
    GOVERNOR_TYPE_SLACK_KNAPSACK,           //9 Predicts slack based on history and identifies correct frequencies according to (energy_difference / time difference).
    GOVERNOR_TYPE_SLACK_MAXENERGY_CONSUMER, //10 
    GOVERNOR_TYPE_SLACK_MAXPOWER_CONSUMER,  //11
    GOVERNOR_TYPE_SLACK_MAXENERGYPERTIME_CONSUMER,  //12 Max Energy per Time
    GOVERNOR_TYPE_COSCALE,                          //13 CoScale
    GOVERNOR_TYPE_MEMSCALE,                          //14 MemScale
    GOVERNOR_TYPE_OPTIMAL,                           //15 All components at optimal freqs.
    GOVERNOR_TYPE_DYNAMICPROG,                           //16 Try all possible frequencies for all IPs and select least energy combination, slack allowing.
    GOVERNOR_TYPE_CORE_ORACLE,                           //17
    END_OF_GOVERNOR
};

enum GOVERNOR_TIMING
{
    GOVERNOR_TIMING_FIXED_1MS = 1,
    GOVERNOR_TIMING_FIXED_10MS,
    GOVERNOR_TIMING_FIXED_16MS, //3
    GOVERNOR_TIMING_FRAME_BOUNDARIES,
    GOVERNOR_TIMING_IP_FRAME_BOUNDARIES  //5
};

enum APP_ID
{
    APP_ID_YOUTUBE = 0,
    APP_ID_VIDRECORD,
    APP_ID_PHOTOCAPTURE,
    APP_ID_AUDIOPLAY,
    APP_ID_ARGAME,
    APP_ID_GALLERY,
    APP_ID_AUDIORECORD,  // 6
    APP_ID_VIDPLAYER,
    APP_ID_ANGRYBIRDS,
    APP_ID_SKYPE,
    APP_ID_FACEBOOK,
    APP_ID_MPGAME,
    APP_ID_OTHER,
    APP_ID_END // 11
};

#endif /* GEMDROID_DEFINES_HH_ */

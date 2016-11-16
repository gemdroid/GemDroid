# Copyright (c) 2013 ARM Limited
# Copyright (c) 2016 The Pennsylvania State University
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Contact: Shulin Zhao (suz53@cse.psu.edu)

from ClockedObject import ClockedObject
from m5.params import *
#from AbstractMemory import *

class GemDroid(ClockedObject):
    type = 'GemDroid'
    cxx_header = "gemdroid/gemdroid.hh"
    enable_gemdroid = Param.Bool(False, "GemDroid functionalities enabled or disabled?")
    num_cpu_traces = Param.Int(1, "Number of CPU traces") 
    governor = Param.Int(1, "Voltage & Frequency governer for SOC")
    governor_timing = Param.Int(1, "When to do DVFS")
    core_freq = Param.Int(900, "Core Freq in MHz")
    issue_width = Param.Int(1, "Issue Width of the cores")
    mem_freq = Param.Int(500, "Mem Freq in MHz")
    dev_freq = Param.Int(400, "IP Freq in MHz")
    ip_freq = Param.Int(300, "IP Freq in MHz")
    num_ip_instances = Param.Int(1, "Number of IP instances")
    cpu_trace1 = Param.String("none", "file from which cpu mem trace1 is read")
    cpu_trace2 = Param.String("none", "file from which cpu mem trace2 is read")
    cpu_trace3 = Param.String("none", "file from which cpu mem trace3 is read")
    cpu_trace4 = Param.String("none", "file from which cpu mem trace4 is read")
    gpu_trace = Param.String("none", "file from which gpu mem trace is read")
    perfect_memory = Param.Bool(False, "Use a perfect memory")
    no_periodic_stats = Param.Bool(False, "Print periodic stats from GemDroid")
    sweep_val1 = Param.Float(1, "Value to use for the current sweep variable1")
    sweep_val2 = Param.Float(1, "Value to use for the current sweep variable2")
   
    deviceConfigFile = Param.String("ini/LPDDR3_micron_32M_8B_x8_sg15.ini",
                                    "Device configuration file")
    systemConfigFile = Param.String("gemdroid.ini",
                                    "Memory organisation configuration file")
    filePath = Param.String("ext/dramsim2/DRAMSim2/",
                            "Directory to prepend to file names")
    traceFile = Param.String("", "Output file for trace generation")
    enableDebug = Param.Bool(True, "Enable DRAMSim2 debug output")
    range = Param.AddrRange('4096MB', "Address range (potentially interleaved)")

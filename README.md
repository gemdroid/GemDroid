# GemDroid
## Build
	git clone https://github.com/gemdroid/GemDroid.git
	cd GemDroid/
	hg clone http://repo.gem5.org/gem5
	cd gem5/
	hg update -r 10231
	rm -rf src/
	cp -r ../gemdroid.src ./src
	cp -r ../gemdroid.needed/gemdroid.dramsim2/* ./ext/dramsim2/
	mkdir traces
	cp your_trace.trace tarces/
	cp ../gemdroid.needed/*.txt ./
	scons build/ARM/gem5.debug 
	
## Run
	build/ARM/gem5.debug -d results/test configs/example/se.py -n 1 --cpu-type=timing --caches --l2cache --num-dirs=1 --gemdroid --cpu_trace1 traces/your_trace.trace --num_cpu_traces=1 --device_config=ini/your_device_config.ini --system_config=your_system_config.ini -c tests/test-progs/hello/bin/arm/linux/hello

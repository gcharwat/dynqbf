cudd_dir=$(CURDIR)/../cudd-3.0.0
cudd_lib=$(cudd_dir)/cudd/.libs/libcudd.a

htd_dir=$(CURDIR)/../htd-lib/current/
htd_lib=$(htd_dir)/lib/libhtd.a

# Set to false to disable depqbf dependency scheme integration (option --dep-scheme standard)
depqbf_enabled=true
depqbf_dir=$(CURDIR)/../depqbf
depqbf_lib=$(depqbf_dir)/libqdpll.a


export PATH := /opt/local/bin:$(PATH)

.PHONY: all
all: release

.PHONY: release
release:
	mkdir -p build/release
	cd build/release && \
	cmake ../../src \
		$(cmake_extra_options) \
		-DCMAKE_BUILD_TYPE=release \
		-Dcudd_dir=$(cudd_dir) \
		-Dcudd_lib=$(cudd_lib) \
		-Dhtd_dir=$(htd_dir) \
		-Dhtd_lib=$(htd_lib) \
		-Ddepqbf_enabled=$(depqbf_enabled) \
		-Ddepqbf_dir=$(depqbf_dir) \
		-Ddepqbf_lib=$(depqbf_lib) \
	&& $(MAKE)

.PHONY: debug
debug:
	mkdir -p build/debug
	cd build/debug && \
	cmake ../../src \
		$(cmake_extra_options) \
		-DCMAKE_BUILD_TYPE=debug \
		-Dcudd_dir=$(cudd_dir) \
		-Dcudd_lib=$(cudd_lib) \
		-Dhtd_dir=$(htd_dir) \
		-Dhtd_lib=$(htd_lib) \
		-Ddepqbf_enabled=$(depqbf_enabled) \
		-Ddepqbf_dir=$(depqbf_dir) \
		-Ddepqbf_lib=$(depqbf_lib) \
	&& $(MAKE)

.PHONY: gprof
gprof:
	mkdir -p build/gprof
	cd build/gprof && \
	cmake ../../src \
		$(cmake_extra_options) \
		-DCMAKE_BUILD_TYPE=gprof \
		-Dcudd_dir=$(cudd_dir) \
		-Dcudd_lib=$(cudd_lib) \
		-Dhtd_dir=$(htd_dir) \
		-Dhtd_lib=$(htd_lib) \
		-Ddepqbf_enabled=$(depqbf_enabled) \
		-Ddepqbf_dir=$(depqbf_dir) \
		-Ddepqbf_lib=$(depqbf_lib) \
	&& $(MAKE)
	
.PHONY: profiler
profiler:
	mkdir -p build/profiler
	cd build/profiler && \
	cmake ../../src \
		$(cmake_extra_options) \
		-DCMAKE_BUILD_TYPE=profiler \
		-Dcudd_dir=$(cudd_dir) \
		-Dcudd_lib=$(cudd_lib) \
		-Dhtd_dir=$(htd_dir) \
		-Dhtd_lib=$(htd_lib) \
		-Ddepqbf_enabled=$(depqbf_enabled) \
		-Ddepqbf_dir=$(depqbf_dir) \
		-Ddepqbf_lib=$(depqbf_lib) \
	&& $(MAKE)

.PHONY: clean
clean:
	rm -rf build

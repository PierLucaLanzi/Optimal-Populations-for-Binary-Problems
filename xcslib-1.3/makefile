#	MAKEFILE FOR XCS
#
#	generates different versions of XCS
#
#	multiplexer:	standard XCS with multiplexer problems
#
#	maze:		standard XCS with maze problems
#

BUILD_DIR := ./build
EXECUTABLES=`find . -maxdepth 1 -type f -perm +a+x`
OBJECTS=$(wildcard ./*.o ./extra/*.o ./pf/*.o)
DOCS=$(wildcard ./docs/html/*.html ./docs/html/*.png)
BAK = $(wildcard ./*~)
EXECUTION=$(wildcard pop_state.* statistics.* *.gz popul.* steps.* report.* AVERAGE.* average.*)


clean:
	@make -f make/xcs.make clean 
	@rm -f executables/*
	@make -f make/xcs.make clean 
	@\rm -f $(OBJECTS) $(BAK) $(EXECUTABLES) *.gz
	@\rm -f example/*/*.gz statistics.* trace.* steps.* perf.* popul.* report.* system_error.* AVERAGE.* average.*


.PHONY: docs clean clean-all

INSTALL=$(HOME)/bin

################################################################################
# TERNARY REPRESENTATION
################################################################################

bf:
	make clean
	make -f make/xcs.make ENVIRONMENT_VERSION=bf ENVIRONMENT=bf_env

woods:
	make clean
	make -f make/xcs.make ENVIRONMENT_VERSION=woods ENVIRONMENT=woods_env ACTIONS=binary_action

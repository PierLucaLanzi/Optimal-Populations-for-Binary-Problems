##
##
##
##	MAKEFILE FOR BUILDING XCS IN XCSLIB 1.2 
##
##
##
BUILD_DIR := ./build
SRC_DIRS := ./src
INCLUDE_DIRS := ./include
EXEC_DIR := ./executables

CXX = g++
OPT = -std=c++17
### suffix for the executable
ENVIRONMENT_VERSION = mp

### class to define environmental state
INPUTS = binary_inputs

### environment definition
ENVIRONMENT = multiplexer_env

### classifier definitions
ACTIONS = boolean_action
CONDITIONS = ternary_condition
CLASSIFIERS = xcs
MODEL = xcs

# header files to be included during compilation
EXTSYS_INCLUDE = \
	-D __DET_INCLUDE__='"$(INPUTS).h"' \
	-D __COND_INCLUDE__='"$(CONDITIONS).h"' \
	-D __ACT_INCLUDE__='"$(ACTIONS).h"' \
	-D __CLS_INCLUDE__='"$(CLASSIFIERS)_classifier.h"' \
	-D __MOD_INCLUDE__='"$(CLASSIFIERS)_classifier_system.h"' 

ENV_INCLUDE = \
	-D __ENV_INCLUDE__='"$(ENVIRONMENT).h"' 
	
ENV_CLASSDEF = \
	-D __ENVIRONMENT__=$(ENVIRONMENT)
	
EXTSYS_CLASSDEF = \
	-D __INPUTS__=$(INPUTS) \
	-D __ACTION__=$(ACTIONS) \
	-D __CONDITION__=$(CONDITIONS) \
	-D __CLASSIFIER__=$(CLASSIFIERS)_classifier \
	-D __MODEL__=$(MODEL)_classifier_system 

### class which runs the experiments
EXPERIMENT_MANAGER = experiment_mgr

INC_DIRS := $(shell find $(INCLUDE_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CXXFLAGS := $(INC_FLAGS) $(EXTCXXFLAGS) $(OPT) $(USERFLAGS) $(EXTSYS_CLASSDEF) $(ENV_CLASSDEF) $(EXTSYS_INCLUDE) $(ENV_INCLUDE) 

##########################################################
#	Utilities & extras
##########################################################
UTILITY := $(SRC_DIRS)/utility/xcs_utility.cpp \
		$(SRC_DIRS)/utility/xcs_random.cpp \
		$(SRC_DIRS)/utility/xcs_configuration_manager.cpp \
		$(SRC_DIRS)/utility/xcs_statistics.cpp \

EXTRAS := $(SRC_DIRS)/utility/generic.cpp

##########################################################
#	Core files that define the environment input/outputs
#	and the classifier structure
##########################################################
CORE := $(SRC_DIRS)/$(MODEL)/$(CLASSIFIERS)_classifier.cpp \
		$(SRC_DIRS)/inputs/$(INPUTS).cpp \
		$(SRC_DIRS)/actions/$(ACTIONS).cpp \
		$(SRC_DIRS)/environments/$(ENVIRONMENT).cpp \
		$(SRC_DIRS)/conditions/$(CONDITIONS).cpp \
		$(UTILITY) \
		$(EXTRAS)

CORE_OBJS := $(CORE:%=$(BUILD_DIR)/%.o)

##########################################################
#	Core + XCS + experiment files 
#	needed to build XCS executable.
##########################################################
SRCS := $(SRC_DIRS)/$(MODEL)/xcs_main.cpp \
		$(SRC_DIRS)/experiments/$(EXPERIMENT_MANAGER).cpp \
		$(SRC_DIRS)/$(MODEL)/$(CLASSIFIERS)_classifier_system.cpp \
		$(CORE)

SRCS_OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

##########################################################
#	Core + XCS + experiment files 
#	needed to build XCS executable.
##########################################################
# MATCH_SRCS := $(SRC_DIRS)/match.cpp
# MATCH_OBJS := $(MATCH_SRCS:%=$(BUILD_DIR)/%.o)


TARGET_EXEC := $(MODEL)$(XCS_VERSION)-$(ENVIRONMENT_VERSION)

# The final build step.
$(EXEC_DIR)/$(TARGET_EXEC): $(SRCS_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(SRCS_OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)/*
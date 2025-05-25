/*!
 * \file experiment_mgr.h
 *
 * \author Pier Luca Lanzi
 *
 * \version 0.01
 *
 * \date 2002/05/18
 * 
 * \brief defines the methods for the experiment manager 
 *
 */


#include "xcs_definitions.h"
#include "xcs_random.h"
#include "xcs_configuration_manager.h"

#ifndef __EXPERIMENT_MGR__
#define __EXPERIMENT_MGR__

/*!
 * \class experiment_mgr experiment_mgr.hpp
 *
 * \brief implements the experiment manager 
 *
 */

class experiment_mgr
{

public:
	//================================================================================
	//
	//
	//	PUBLIC METHODS
	//
	//
	//================================================================================
	
	//! name of the class that implements the experiment manager
	string class_name() const { return string("experiment_mgr"); };

	//! tag used to access the configuration file
	string tag_name() const { return string("experiments"); };

	//! class constructor; it reads the class parameters through the configuration manager
	experiment_mgr(xcs_configuration_manager &xcs_config, t_classifier_system *xcs, t_environment *environment, bool verbose=true);

	//! set the parameters from the configuration file
	void set_parameters(xcs_configuration_manager & xcs_config);

	//! print the current parameter setting 
	void print_parameters(ostream&);

	//! perform the experiments
	void perform_experiments();

	//! print the flags for save various experiment statistics
	void print_save_options(ostream &output) const;

	//! save the state of the experiment
	void save_state(const unsigned long, const bool, unsigned long problem_no = 0) const;

	//! restore the state of the experiment
	bool restore_state(const unsigned long);

private:
	//================================================================================
	//
	//
	//	PRIVATE VARIABLES
	//
	//
	//================================================================================
	
	long	current_experiment;		//!< the experiment currently running
	long	first_experiment;		//!< number of the first experiment to run
	long	no_experiments;			//!< numbero of total experiments to run
	
	unsigned long	current_problem;		//!< the problem being currently executed
	unsigned long	first_learning_problem;		//!< first problem executed
	unsigned long	no_learning_problems;		//!< number of problems executed
	unsigned long	no_condensation_problems;	//!< number of problems executed in condensation
	unsigned long	no_test_problems;		//!< number of test problems executed at the end
	unsigned long	no_max_steps;		//!< maximum number of step per problem

	unsigned long	current_no_test_problems;	//!< number of test problems solved so far

	bool	flag_save_experiment_final_state;		//!< true if the state of the system will be saved at the end of the experiment
	long	save_experiment_interval;	//! the experiment status is saved every "save_interval" problems

	bool	flag_save_final_population;		//!< true if the state of the agent must be saved when an experiment ends
	long	save_population_interval;	//! the experiment status is saved every "save_interval" problems

	bool	flag_trace;					//!< true if the experiment outputs on the trace file
	bool	flag_test_environment;		//!< true if the system will be tested on the whole environment
	bool	flag_save_time_report;		//!< true if execution time is traced
	bool	flag_save_avf; 				//!< true if saves the action-value function

	string		extension;			//!< file extension for the experiment files
	
	// bool			flag_compact_mode;				//!< false if the statistics of every problem is saved
	unsigned long	save_stats_every;				//!< number of problems on which the average is computed and the statistics is reported
	// double			compact_average_steps;
	// double			compact_average_reward_sum;
	// double			compact_average_size;

	unsigned long teletransportation_interval;	//! number of steps between teletransportation

	t_classifier_system *xcs;
	t_environment *environment;

	//================================================================================
	//
	//
	//	PRIVATE METHODS
	//
	//
	//================================================================================
	
	const static std::vector<std::string> configuration_parameters;

 private:

	//! save the agent state for experiment \emph expNo
	void save_population(const unsigned long expNo, const unsigned long problem_no=0) const;

	//! save time report
	void save_time_report(timer &timer_overall, std::vector<double> &experiment_time, std::vector<double> &problem_time);

	//! save action value function 
	void save_avf(const unsigned long expNo, const unsigned long problem_no=0) const;

};
#endif

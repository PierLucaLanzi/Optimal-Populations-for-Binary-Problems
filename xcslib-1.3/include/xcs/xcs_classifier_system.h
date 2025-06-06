/*!
 * \file xcs_classifier_system.hpp
 *
 * \brief implements the XCS classifier system
 *
 */

#include <list>
#include "xcs_definitions.h"
#include "xcs_random.h"
#include "xcs_statistics.h"
#include "xcs_configuration_manager.h"

using namespace std;

#ifndef __XCS_CLASSIFIER_SYSTEM__
#define __XCS_CLASSIFIER_SYSTEM__

//! covering strategy
typedef enum { 
 	COVERING_STANDARD,		//! perform covering based on the average prediction of [M] and [P]
	COVERING_ACTION_BASED	//! perform covering based on the number of actions in [M]
} t_covering_strategy;

//! deletion strategy
typedef enum { 
 	XCS_DELETE_RWS_SETBASED,			//! delete with RWS according to actionset size
 	XCS_DELETE_RWS_FITNESS,				//! delete with RWS according to actionset size and fitness
	XCS_DELETE_RANDOM,					//! random delete
	XCS_DELETE_RANDOM_WITH_ACCURACY,	//! random delete based on numerosity+accuracy enhancement
	// XCS_DELETE_TSES,		//! delete with TS according to the maximization of error
	// XCS_DELETE_TSE,			//! delete with TS according to the maximization of error with threshold
	// XCS_DELETE_TSF,			//! delete with TS according to the minimization of fitness
	// XCS_DELETE_TSFS,		//! delete with TS according to the minimization of fitness without threshold
	// XCS_DELETE_TSSS			//! delete with TS according to the maximization of action set size
} t_delete_strategy;

//! selection strategy
typedef enum { 
	XCS_SELECT_RWS_FITNESS,		//! select offspring classifiers based on RWS and fitness
	XCS_SELECT_TS_FITNESS,		//! offspring selection based on TS and fitness maximization
	XCS_SELECT_TS_ERROR,		//! offspring selection based on TS and error minimization
} t_offspring_selection_strategy;

//! action selection strategy
typedef enum { 
 	ACTION_SELECTION_DETERMINISTIC,	// sceglie azione con predizione migliore	
	ACTION_SELECTION_RANDOM,			// sceglie azione a caso
	ACTION_SELECTION_EGREEDY, 	// Uniform con prob. biased deterministico
	ACTSEL_PROPORTIONAL, 	// Prob. proporzionale alla predizione
} t_action_selection;

//! population init strategy
typedef enum {
	POPULATION_INIT_RANDOM, 		// random population
	POPULATION_INIT_EMPTY, 		// empty population
	POPULATION_INIT_LOAD, 		// empty population
	POPULATION_INIT_SOLUTION
} t_classifier_set_init;

/*!
 * \brief implements the elements of the prediction array
 */
class t_system_prediction {
 public:
        double		payoff;		//! action payoff
        t_action	action;		//! action
        double		sum;		//! fitness sum to normalize action payoff
	unsigned long 	n;		//! number of classifiers that advocate the action

	t_system_prediction()
	{
		payoff = 0;
		sum = 0;
		n = 0;
	};

	int operator==(const t_system_prediction& p2) const
	{	// due predizioni sono 'uguali' se si riferiscono alla stessa azione
		return (action == p2.action);
	}

	int operator==(const t_action& act) const
	{	// due predizioni sono 'uguali' se si riferiscono alla stessa azione
		return (action == act);
	}
};

	 

/*!
 * \class xcs_classifier_system
 * \brief implements the XCS classifier system
 */
class xcs_classifier_system
{
 public:
	//! name of the class that implements XCS
	string class_name() const {return string("xcs_classifier_system");};

	//! tag used to access the configuration file
	string tag_name() const {return string("classifier_system");};

	//! class constructor
	xcs_classifier_system(xcs_configuration_manager& xcs_config, t_environment *environment);


void set_parameters(xcs_configuration_manager & xcs_config);
void print_parameters(ostream& output) const;

/**
 * methods used from the experiment manager
 */

//@{

//! defines what has to be done when a new experiment begins
void begin_experiment();

//! defines what has to be done when the current experiment begins
void end_experiment();

//! defines what has to be done when a new problem begins.
void begin_problem();

//! defines what must be done when the current problem ends
void end_problem();

//! return the size of memory that is used during learning, i.e., the number of macro classifiers in [P]
unsigned long size() const { return population.size(); };

//! return the current system error
double get_system_error() const { return system_error; };

//! writes trace information on an output stream;
/*!
 * it is called by the experiment manager just before the end_problem method
 * \sa end_problem
 * \sa \class experiment_mgr
 */
void trace(ostream &output) const {};

//!	return the statistics of the current experiment
xcs_statistics statistics() const { return stats; };

//!	restore XCS state from an input stream
void restore_state(istream &input);

//!	save the experiment state to an output stream
void save_state(ostream &output);

//!	save population
void save_population(ostream &ouput);

//@}

public:
//================================================================================
//
//
//	PUBLIC TYPES
//
//
//================================================================================

//! pointer to a classifier
typedef t_classifier *t_classifier_ptr;

//! set of classifiers
/*!
 * \type t_classifier_set
 * \brief represent a set of classifiers
 */
typedef vector<t_classifier *> t_classifier_set;

//! index in set of classifiers
/*!
 * \type t_set_iterator
 * \brief represent an iterator over a set of classifiers
 */
typedef vector<t_classifier *>::iterator t_set_iterator;
typedef vector<t_classifier *>::const_iterator t_set_const_iterator;

bool update_during_test_problems() const { return flag_update_test; }
	
private:
	//================================================================================
	//
	//
	//	PERFORMANCE COMPONENT
	//
	//
	//================================================================================
						
	bool			init;					//! true, if the class was initialized
	t_environment	*environment;			//! link to the environment

	//! experiment parameters
	unsigned long		total_steps;			//! total number of steps 
	unsigned long		total_learning_steps;	//! total number of exploration steps
	unsigned long		total_time;				//! total time passed (should be the same as total steps);
	unsigned long		problem_steps;			//! total number of steps within the single problem
	double				total_reward;			//! total reward gained during the problem
	double				system_error;			//! difference between predicted payoff and reward received (it is used only in single step problems)

	//! [P] parameters
	unsigned long		max_population_size;	//! maximum number of micro classifiers
	unsigned long		population_size;		//! population size
	unsigned long		macro_size;				//! number of macroclassifiers
	t_classifier_set_init	population_init;	//! init strategy for [P]
	string			population_init_file;		//! file containing a population to be used to init [P]

	//! classifier parameters
	double			init_prediction;		//! initial prediction for newly created classifiers
	double			init_error; 			//! initial predition error for newly created classifiers
	double			init_fitness; 			//! initial fitness for newly created classifiers
	double			init_set_size;			//! initial set size for newly created classifiers
	unsigned long	init_no_updates;		//! initial number of updates for newly created classifiers

	//! covering strategy
	t_covering_strategy	covering_strategy;	//! strategy for create covering classifiers
	double			fraction_for_covering;	//! original covering parameter in Wilson's 1995 paper
	double	 		tetha_nma;				//! minimum number of actions in [M]

	//! action selection
	t_action_selection	action_selection_strategy;	//! strategy for action selection
	double				prob_random_action;			//! probability of random action

	///================================================================================
	///
	///
	///	REINFORCEMENT COMPONENT
	///
	///
	///================================================================================
						
	//! fitness computation
	double			epsilon_zero;				//! epsilon zero parameter, determines the threshold
	double			alpha; 						//! alpha parameter, determines the start of the decay
	double			vi;							//! vi parameter, determines the decay rate
	// bool			use_exponential_fitness;	//! if true exponential fitness is used


	//================================================================================
	//
	//
	//	DISCOVERY COMPONENT
	//
	//
	//================================================================================
						
	//! GA parameters
	bool			flag_discovery_component;	//! true if the GA is on
	double			theta_ga;			//! threshold for GA activation
	double			prob_crossover;			//! probability to apply crossover
	double			prob_mutation;			//! probability to apply mutation
	bool			flag_ga_average_init;		//! if true offspring parameters are averages
	bool			flag_error_update_first;	//! if true, prediction error is updated first

	bool			flag_update_test;		//! true if update is performed during test

	bool			use_ga;				//! true if GA is on
	bool			use_crossover;			//! true if crosseover is used
	bool			use_mutation;			//! true if mutation is used

	//! subsumption deletion parameters
	bool			flag_ga_subsumption;		//! true if GA subsumption is on
	bool			flag_gaa_subsumption;		//! true if Butz GA subsumption is used
	bool			flag_as_subsumption;		//! true if AS subsumption is on
	double			theta_sub;			//! threshold for subsumption activation
	double	 		theta_as_sub;			//! threshold for subsumption activation
	bool			flag_cover_average_init;

	//! delete parameters
	t_delete_strategy	delete_strategy;		//! deletion strategy
	bool			flag_delete_with_accuracy;	//! deletion strategy: true, if T3 is used; false if T1 is used
	double			theta_del;					//! theta_del parameter for deletion
	double			delta_del;					//! delta parameter for deletion

	//================================================================================
	//
	//
	//	OTHER PRIVATE FUNCTIONS
	//
	//
	//================================================================================
						
	xcs_statistics stats;					//! classifier system statistics

	const static std::vector<std::string> configuration_parameters;

#ifdef __NICHE_TRACKING__	
	unsigned long max_niche_queue_size;
#endif

 public:
	//================================================================================
	//
	//
	//	REINFORCEMENT COMPONENT
	//
	//
	//================================================================================
						
	double	learning_rate;					//! beta parameter
	double	discount_factor;				//! gamma parameter, the discount factor

	//================================================================================
	//
	//
	//	METHODS FOR THE DISCOVERY COMPONENT
	//
	//
	//================================================================================

 private:
	//! private methods for GA
	//@{
	bool	need_ga(t_classifier_set &action_set, const bool flag_explore);
	void	select_offspring(t_classifier_set&, t_set_iterator&, t_set_iterator&);
	//void	genetic_algorithm(t_classifier_set &action_set, const t_state& detectors, const bool flag_condensation);
	void	genetic_algorithm(t_classifier_set &action_set, const t_state& detectors, bool flag_condensation=false);
	void	update_timestamp(t_classifier_set &action_set);
	void	condensation(t_classifier_set &action_set);

	//@}

 private:

	//! variables for [P], [M], [A], and [A]-1
	//@{
	t_classifier_set 				population;			//! population [P]
	t_classifier_set				match_set;			//! match set [M]
	t_classifier_set				action_set;			//! action set [A]
	t_classifier_set				previous_action_set;		//! action set at previous time step [A]-1
	//@}

	vector<double>					select;				//! vector for roulette wheel selection
	vector<double>					error;

	t_state					previous_input;			//! input at t-1
	t_state					current_input;			//! current input at time t
	vector<t_system_prediction>			prediction_array;		//! prediction array P(.) 
	vector<unsigned long>				available_actions;		//! actions in the prediction array that have a not null prediction, and thus are available for selection

	double		previous_reward;						//! reward received at previous time step


	//! methods for setting the parameters from the configuration file
	//@{
	void	set_init_strategy(string);				
	void	set_exploration_strategy(const string& );
	void	set_deletion_strategy(const string& deletion_strategy);
	void	set_covering_strategy(const string& strategy);			//! set the covering strategy, which can be "standard" or "nma"
	//@}

	//! methods for [P]
	//@{
	void	clear_population();				//! empty [P]
	void    init_classifier_set();				//! init [P] according to the selected strategy (i.e., empty or random)

	void	erase_population();				//! erase [P]
	void	insert_classifier(const t_classifier& cs);		//! insert a classifier in [P]
	void	delete_classifier();				//! delete a classifier from [P]

	//! init the classifier parameters
	void	init_classifier(t_classifier& classifier, bool average=false);	
	//void	init_classifier(t_classifier& classifier, bool average);	
	//@}

	//! methods for covering
	//@{
	bool	need_covering_standard(t_classifier_set, const t_state&);	//! true if covering is needed according to Wilson 1995

	void	covering(const t_state& detectors);				//! covering
	bool	perform_covering(t_classifier_set&, const t_state&);		//! perform covering in [M]

	bool	perform_standard_covering(t_classifier_set&, const t_state&); //! perform covering according to Wilson 1995
	bool	need_standard_covering(t_classifier_set&, const t_state&);	//! true if standard covering is needed

	bool	perform_nma_covering(t_classifier_set&, const t_state&);	//! perform covering according to Butz and Wilson 2001

	//@}
	
	//!	build the prediction array P(.) from [M]
	void	build_prediction_array();

	//! 	print the prediction array P(.) to an output stream
	void	print_prediction_array(ostream&) const;

	//! select an action
	void	select_action(const t_action_selection, t_action&);
	
	//!	build [A] based on the selected action a
	void	build_action_set(const t_action&);

	//! methods for distributing the reinforcement among classifiers
	//@{
	void	update_set(const double, t_classifier_set&);
	void	update_fitness(t_classifier_set&);
	//@}

	//! true if classifier \emph first subsume classifier \emph second
	bool 	subsume(const t_classifier& first, const t_classifier& second);


 public:
	//@{
	/*! 
	 * find the classifiers that are AS subsumed in the current set
	 * \param set input classifiers
	 * \param set classifiers that are AS subsumed
	 */
	void	do_as_subsumption(t_classifier_set &set);

	//! Butz subsumption: check whether a classifier is subsumed by any classifier in [A] 
	void ga_a_subsume(t_classifier_set&, const xcs_classifier&, t_set_iterator&);

	//!	find the "most general classifier in the set
	//t_set_iterator find_most_general(t_classifier_set&) const;
	t_set_iterator find_most_general(t_classifier_set&) const;

	//! 	find the classifiers in the set that are subsumed by classifier
	void	find_as_subsumed(t_set_iterator, t_classifier_set&, t_classifier_set&) const;

	//!	perform AS subsumption, deleting the subsumed classifiers and increasing the numerosity of most general classifier
	void	as_subsume(t_set_iterator, t_classifier_set &set);

	//@}
 public:
	//!	a step of a problem
	void	step(const bool exploration_mode,const bool condensationMode);

	//!  build the match set [M]; it returns the number of microclassifiers that match the sensory configuration
	unsigned long	match(const t_state& detectors);

 private:
	//! true if the problem is solved in exploration (Wilson 1995), i.e., XCS is in learning (Butz 2001)
	bool		exploration_mode;	
 public:
	//! total number of learning steps
	unsigned long	learning_time() const {return total_learning_steps;};
	
	//! total number of steps
	unsigned long	time() const {return total_steps;};

	//! total number of steps within the current problem
	unsigned long	problem_time() const {return problem_steps;};
 
	//! return true if the XCS is solving the problem in exploration
	bool	in_exploration() {return exploration_mode;};

	//! set the exploration mode: if mode is true problems will be solved in exploration
	void	exploration(bool mode) {exploration_mode = mode;};

	/*! 
	 * TRAIN/TEST FUNCTIONS
	 */
	void train(t_state&, t_action&, double) {};
	void test(t_state&, t_action&, double) {};

 private:
	bool	classifier_could_subsume(const t_classifier &classifier, double epsilon_zero, double theta_sub) const
		{ return ((classifier.experience>theta_sub) && (classifier.error<epsilon_zero)); };

 private:
	//! create the prediction array based on the action used
	void	create_prediction_array();

	//! clear the prediction array based
	void	init_prediction_array();

	//@{ \defgroup procedures to trace execution
	
	//! print a set of classifiers
	void print_set(t_classifier_set &set, ostream& output);

	//! check the consistency of various parameters in [P]
	void	check(string,ostream&);
	//@}
	
	bool comp_num(const t_classifier& cl1, const t_classifier& cl2) const {return (cl1.numerosity>cl2.numerosity);};

	class comp_numerosity {
  	  public:
	  int operator()(const xcs_classifier_system::t_classifier_ptr &first, const xcs_classifier_system::t_classifier_ptr &second)
	  {
	    return (first->numerosity>second->numerosity);
	  };
	};

	class comp_experience {
  	  public:
	  int operator()(const xcs_classifier_system::t_classifier_ptr &first, const xcs_classifier_system::t_classifier_ptr &second)
	  {
	    return (first->experience>second->experience);
	  };
	};

	void select_random_action(t_action& action) const;
	void select_best_action(t_action& action) const;

 private:
 	//! new random init procedure
	void init_population_random();

 	//! new random init procedure
	void init_population_load(string);

	//! load a solution specified with condition-action-prediction (fitness, error and other parameters are initialized)
	void init_population_solution(string);


 private:
	//! new configuration parameters 2003/05/06

	bool			flag_use_mam;
	bool			flag_ga_tournament_selection;
	bool			flag_tournament_on_fitness;
	double 			tournament_size;

	//! select offspring classifier through tournament selection
	void			select_offspring_ts(t_classifier_set&, t_set_iterator&);
 private:
	/*!
	    selection strategies for deletion
         */
	//! random deletion
	t_set_iterator select_delete_random(t_classifier_set &set);

	//! roulette wheel
	t_set_iterator select_delete_rw(t_classifier_set &set);

	/*!
	    gradient descent
         */

 	//! true if gradient descent is used	
	bool	flag_use_gradient_descent;

	t_set_iterator select_proportional(t_classifier_set&, const vector<double>&, double) const;

 public:
	//! returns the specificity in a set
	double	specificity(const t_classifier_set &set) const;

	//! returns the average gradient in a set
	double	average_gradient(const t_classifier_set &set) const;

	std::vector<double> predict(t_state inputs);
};
#endif

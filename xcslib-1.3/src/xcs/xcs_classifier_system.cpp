/*! \file	xcs_classifier_system.cpp
 *  \brief	Implementation of the XCS classifier system as described in Butz & Wilson paper.
 *
 */

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "xcs_classifier_system.h"

using namespace std;

const std::vector<std::string> xcs_classifier_system::configuration_parameters = {"population size", "epsilon zero", "theta GA", "initial population", "crossover probability", "mutation probability", "learning rate", "discount factor", "discovery component", "vi", "alpha", "prediction init", "error init", "fitness init", "set size init", "exploration strategy", "deletion strategy", "theta delete", "theta GA sub", "theta AS sub", "GA subsumption", "GAA subsumption", "AS subsumption", "update during test", "update error first", "tournament selection", "tournament size", "gradient descent", "niche queue max size"};

xcs_classifier_system::xcs_classifier_system(xcs_configuration_manager& xcs_config, t_environment *environment)
{
	this->environment = environment;

	//! look for the init section in the configuration file
	if (!xcs_config.exist(tag_name()))
	{
		xcs_utility::error(class_name(), "constructor", "section <" + tag_name() + "> not found", 1);	
	}

	// check if the configuration file contains only allowed keywords
	xcs_config.check_parameters(tag_name(), configuration_parameters);

	//! set the parameters from the configuration file
    set_parameters(xcs_config);

    //! reserve memory for [P], [M], [A], [A]-1
	match_set.reserve(max_population_size);
	action_set.reserve(max_population_size);
	previous_action_set.reserve(max_population_size);
	select.reserve(max_population_size);

	//! create the prediction array
	create_prediction_array();
	
	//! check subsumption settings
	t_condition	cond;

	if (flag_as_subsumption && !cond.allow_as_subsumption())
	{
		xcs_utility::error( class_name(), "constructor", "AS subsumption requested but condition class does not allow", 1);
	}

	if (flag_ga_subsumption && !cond.allow_ga_subsumption())
	{
		xcs_utility::error( class_name(), "constructor", "GA subsumption requested but condition class does not allow", 1);
	}	
}

void xcs_classifier_system::set_parameters(xcs_configuration_manager &xcs_config)
{
	try 
	{
		max_population_size = xcs_config.Value(tag_name(), "population size");
	} catch (...) {
        xcs_utility::error(class_name(), "constructor", "attribute \'population size\' not found in <" + tag_name() + ">", 1);
	}

	try 
	{
        epsilon_zero = xcs_config.Value(tag_name(), "epsilon zero");
	} catch (...) {
        xcs_utility::error(class_name(), "constructor", "attribute \'epsilon zero\' not found in <" + tag_name() + ">", 1);
	}

	// str_discovery_component = (string)xcs_config.Value(tag_name(), "discovery component", "on");
	xcs_utility::set_flag(xcs_config.Value(tag_name(), "discovery component", "on"), flag_discovery_component);

	// if the discovery component it is used, it is mandatory to specify its parameters
	if (flag_discovery_component)
	{
		try 
		{
			theta_ga = xcs_config.Value(tag_name(), "theta GA");

		} catch (...) {
			xcs_utility::error(class_name(), "constructor", "attribute \'theta GA\' not found in <" + tag_name() + ">", 1);
		}

		try 
		{
			prob_crossover = xcs_config.Value(tag_name(), "crossover probability");
		} catch (...) {
			xcs_utility::error(class_name(), "constructor", "attribute \'crossover probability\' not found in <" + tag_name() + ">", 1);
		}

		try 
		{
			prob_mutation = xcs_config.Value(tag_name(), "mutation probability");
		} catch (...) {
			xcs_utility::error(class_name(), "constructor", "attribute \'mutation probability\' not found in <" + tag_name() + ">", 1);
		}
	} else {
		theta_ga = xcs_config.Value(tag_name(), "theta GA", 25.0);
		prob_crossover = xcs_config.Value(tag_name(), "crossover probability", 0.8);
		prob_mutation = xcs_config.Value(tag_name(), "mutation probability", 0.04);
	}



	//! typical values that never change in all configurations
	learning_rate = xcs_config.Value(tag_name(), "learning rate", 0.2);
	discount_factor = xcs_config.Value(tag_name(), "discount factor",0.7);

	set_covering_strategy(xcs_config.Value(tag_name(), "covering strategy", "standard"));


	//! 
	vi = xcs_config.Value(tag_name(), "vi", 5.0);
	alpha = xcs_config.Value(tag_name(), "alpha", 0.1);

	init_prediction = xcs_config.Value(tag_name(), "prediction init", 10.0);
	init_error = xcs_config.Value(tag_name(), "error init", 0.0);
	init_fitness = xcs_config.Value(tag_name(), "fitness init", 0.01);
	init_set_size = xcs_config.Value(tag_name(), "set size init", 1.0);

	// str_pop_init = (string)xcs_config.Value(tag_name(), "initial population", "empty");
    set_init_strategy(xcs_config.Value(tag_name(), "initial population", "empty"));

	string str_exploration = (string)xcs_config.Value(tag_name(), "exploration strategy", "random");
    set_exploration_strategy(str_exploration.c_str());

    string str_deletion_strategy = (string)xcs_config.Value(tag_name(), "deletion strategy", "accuracy-based");
    set_deletion_strategy(str_deletion_strategy.c_str());

	theta_del = xcs_config.Value(tag_name(), "theta delete", 20.0);
	theta_sub = xcs_config.Value(tag_name(), "theta GA sub", 20.0);
	theta_as_sub = xcs_config.Value(tag_name(), "theta AS sub", 100.0);

    xcs_utility::set_flag(xcs_config.Value(tag_name(), "GA subsumption", "off"), flag_ga_subsumption);

	//! if the GA Subsumption is activated also the GAA Subsubmption (introduced in the XCS algorithmic description is activated)
	string str_gaa_sub = (string)xcs_config.Value(tag_name(), "GAA subsumption", flag_ga_subsumption?"on":"off");

    xcs_utility::set_flag(string(str_gaa_sub), flag_gaa_subsumption);

	// string str_as_sub = (string)xcs_config.Value(tag_name(), "AS subsumption", "off");
    xcs_utility::set_flag(xcs_config.Value(tag_name(), "AS subsumption", "off"), flag_as_subsumption);

    string str_update_test = (string)xcs_config.Value(tag_name(), "update during test", "on");
	xcs_utility::set_flag(string(str_update_test), flag_update_test);

    // str_error_first = (string)xcs_config.Value(tag_name(), "update error first", "on");
    xcs_utility::set_flag(xcs_config.Value(tag_name(), "update error first", "on"), flag_error_update_first);

    // string str_use_mam = (string)xcs_config.Value(tag_name(), "use MAM", "on");
    xcs_utility::set_flag(xcs_config.Value(tag_name(), "use MAM", "on"), flag_use_mam);

    // string str_ga_ts = (string)xcs_config.Value(tag_name(), "tournament selection");
    xcs_utility::set_flag(xcs_config.Value(tag_name(), "tournament selection", "off"), flag_ga_tournament_selection);
    tournament_size = xcs_config.Value(tag_name(), "tournament size", 0.4);

    // string str_use_gd = (string)xcs_config.Value(tag_name(), "gradient descent", "off");
	xcs_utility::set_flag(xcs_config.Value(tag_name(), "gradient descent", "off"), flag_use_gradient_descent);

	//! constant parameters 
	delta_del = 0.1;
    flag_cover_average_init = false;
    flag_ga_average_init = false;
}

void xcs_classifier_system::print_parameters(ostream& OUTPUT) const
{
	OUTPUT << "<" << tag_name() << ">" << endl;
	OUTPUT << "\t" << "population size = " << max_population_size << endl;
	OUTPUT << "\t" << "epsilon zero = " << epsilon_zero << endl;
	OUTPUT << "\t" << "theta GA = " << theta_ga << endl;
	OUTPUT << "\t" << "crossover probability = " << prob_crossover << endl;
	OUTPUT << "\t" << "mutation probability = " << prob_mutation << endl;
	OUTPUT << "\t" << "learning rate = " << learning_rate << endl;
	OUTPUT << "\t" << "discount factor = " << discount_factor << endl;
	OUTPUT << "\t" << "discovery component = " << (flag_discovery_component?"on":"off") << endl;
	OUTPUT << "\t" << "vi = " << vi << endl;
	OUTPUT << "\t" << "alpha = " << alpha << endl;
	OUTPUT << "\t" << "prediction init = " << init_prediction << endl;
	OUTPUT << "\t" << "error init = " << init_error << endl;
	OUTPUT << "\t" << "fitness init = " << init_fitness << endl;
	OUTPUT << "\t" << "set size init = " << init_set_size << endl;

	string str_exploration_strategy;
	stringstream str_epsilon;

	switch (action_selection_strategy)
	{
		case ACTION_SELECTION_EGREEDY:
			str_epsilon << std::fixed << std::setprecision(2) << prob_random_action;
			str_exploration_strategy = "epsilon-greedy " + str_epsilon.str();
			break;
		case ACTION_SELECTION_RANDOM:
			str_exploration_strategy = "random";
			break;
		case ACTSEL_PROPORTIONAL:
			str_exploration_strategy = "proportional";
			break;
		case ACTION_SELECTION_DETERMINISTIC:
			str_exploration_strategy = "deterministic";
			break;
	}

	OUTPUT << "\t" << "exploration strategy = " << str_exploration_strategy << endl;

	string str_deletion_strategy; 

	switch (delete_strategy)
	{
		case XCS_DELETE_RWS_SETBASED:
			str_deletion_strategy = "standard // proportional to action set size";
			break;
		case XCS_DELETE_RWS_FITNESS:
			str_deletion_strategy = "accuracy-based // proportional to action set size and fitness";
			break;
		case XCS_DELETE_RANDOM_WITH_ACCURACY:
				str_deletion_strategy = "random-with-accuracy // random with accuracy score";
			break;
		case XCS_DELETE_RANDOM:
			str_deletion_strategy = "random // completely random";
			break;		
	}

	OUTPUT << "\t" << "deletion strategy = " << str_deletion_strategy << endl;

	OUTPUT << "\t" << "theta delete = " << theta_del << endl;
	OUTPUT << "\t" << "theta GA sub = " << theta_sub << endl;
	OUTPUT << "\t" << "theta AS sub = " << theta_as_sub << endl;

	OUTPUT << "\t" << "GA subsumption = " << (flag_ga_subsumption?"on":"off") << endl;
	OUTPUT << "\t" << "GAA subsumption = " << (flag_gaa_subsumption?"on":"off") << endl;
	OUTPUT << "\t" << "AS subsumption = " << (flag_as_subsumption?"on":"off") << endl;

	OUTPUT << "\t" << "update during test = " << (flag_update_test?"on":"off") << endl;
	OUTPUT << "\t" << "update error first = " << (flag_error_update_first?"on":"off") << endl;

	OUTPUT << "\t" << "use MAM = " << (flag_use_mam?"on":"off") << endl;
	OUTPUT << "\t" << "update error first = " << (flag_error_update_first?"on":"off") << endl;

	OUTPUT << "\t" << "tournament selection = " << (flag_ga_tournament_selection?"on":"off") << endl;
	OUTPUT << "\t" << "tournament size = " << tournament_size << endl;

	OUTPUT << "\t" << "gradient descent = " << (flag_use_gradient_descent?"on":"off") << endl;
	OUTPUT << "</" << tag_name() << ">" << endl;
}

void	
xcs_classifier_system::set_exploration_strategy(const string& exploration_strategy)
{
	if (exploration_strategy=="proportional")
	{
		action_selection_strategy = ACTSEL_PROPORTIONAL;
	} else if (exploration_strategy=="random") {
		action_selection_strategy = ACTION_SELECTION_RANDOM;
	} else if (exploration_strategy=="deterministic") {
		action_selection_strategy = ACTION_SELECTION_DETERMINISTIC;
	} else if (xcs_utility::trim(exploration_strategy.substr(0,exploration_strategy.find(" ")))=="epsilon-greedy") {
		action_selection_strategy = ACTION_SELECTION_EGREEDY;
		string str_epsilon = xcs_utility::trim(exploration_strategy.substr(exploration_strategy.find(" ")+1));
		prob_random_action = std::stof(str_epsilon);

		if ((prob_random_action<=0.) || (prob_random_action>1.))
		{	
			xcs_utility::error(class_name(),"set_exploration_strategy",
				"epsilon-greedy value out of range (0.0,1.0]",1);
		}
	} else {
		xcs_utility::error(class_name(),"set_exploration_strategy",
			"Unrecognized action selection policy \'"+exploration_strategy+"\'",1);
	}
}

void
xcs_classifier_system::set_deletion_strategy(const string& deletion_strategy)
{
	if (deletion_strategy=="standard")
	{
		flag_delete_with_accuracy = false;
 		delete_strategy = XCS_DELETE_RWS_SETBASED;	//! delete with RWS according to actionset size
	} else if (deletion_strategy=="accuracy-based")
	{
		flag_delete_with_accuracy = true;
 		delete_strategy = XCS_DELETE_RWS_FITNESS;	//! delete with RWS according to actionset size and fitness
	} else if (deletion_strategy=="random-with-accuracy")
	{
		delete_strategy = XCS_DELETE_RANDOM;		//! random delete with accuracy score
		flag_delete_with_accuracy = true;
	} else if (deletion_strategy=="random")
	{
		delete_strategy = XCS_DELETE_RANDOM;		//! random delete
	} else {
		//! unrecognized deletion strategy
		xcs_utility::error(class_name(),"set_deletion_strategy", "unrecognized deletion strategy '" + string(deletion_strategy) + "'", 1);
	}
}

void
xcs_classifier_system::init_classifier_set()
{
	// t_classifier classifier;
	switch (population_init)
	{
		//! [P] = {}
		case POPULATION_INIT_EMPTY:
			erase_population();
			break;

		//! fill [P] with random classifiers
		case POPULATION_INIT_RANDOM:
			init_population_random();
			break;

		//! fill [P] with classifiers save in a file
		case POPULATION_INIT_LOAD:
			init_population_load(population_init_file);
			break;

		//! fill [P] with classifiers save in a file
		case POPULATION_INIT_SOLUTION:
			init_population_solution(population_init_file);
			break;

		default:
			xcs_utility::error(class_name(),"init_classifier_set", "init strategy unknown" , 1);
	}
};
				
				
bool	compare_cl(t_classifier *clp1, t_classifier *clp2) {return *clp1<*clp2;};
void
xcs_classifier_system::insert_classifier(const t_classifier& new_cl)
{
	///CHECK
	assert(new_cl.actionset_size>=0);
	assert(new_cl.numerosity==1);

	//new_cl.condition.check_limits();
	///END CHECK

	/// keep a sorted index of classifiers
	t_classifier *clp = new t_classifier;
	*clp = new_cl;

	clp->time_stamp = total_steps;
	clp->experience = 0;

	t_set_iterator	pp;

	pp = lower_bound(population.begin(),population.end(),clp,compare_cl);
	if ((pp!=population.end()))
	{
		if ( (**pp)!=(*clp) )
		{
			clp->generate_id();
			clp->time_stamp = total_steps;

			population.insert(pp,clp);
			macro_size++;
		}
		else {
			(**pp).numerosity++;
			delete clp;
		}
	} else {

		clp->generate_id();
		clp->time_stamp = total_steps;

		population.insert(pp,clp);
		macro_size++;
	}
	population_size++;
}

//! build [M]
unsigned long	
xcs_classifier_system::match(const t_state& detectors)
{
	t_set_iterator			pp;		/// iterator for visiting [P]
	unsigned long			match_set_size = 0;		/// number of micro classifiers in [M]

	match_set.clear();				/// [M] = {}

#ifdef __FAST_BINARY_MATCHING__
        istringstream   INPUTS(detectors.string_value()); 
        bitset<__BIT_CONDITION_SIZE__>    input;

        // string is = detectors.string_value();

        INPUTS >> input;

        for(pp=population.begin();pp!=population.end();pp++)
        {  

                if ((**pp).condition.match(input))
                {     
                        match_set.push_back(*pp); 
                        sz += (**pp).numerosity;
                }     
        }  
#else 	
	for(pp=population.begin();pp!=population.end();pp++)
	{
		if ((**pp).match(detectors))
		{
			match_set.push_back(*pp);
			match_set_size += (**pp).numerosity;
       	}
   	}
#endif

	return match_set_size;
}

//! perform covering on [M], only if needed
bool
xcs_classifier_system::perform_covering(t_classifier_set &match_set, const t_state& detectors)
{
	switch (covering_strategy)
	{
		//! perform covering according to Wilson 1995
		case COVERING_STANDARD:
			return perform_standard_covering(match_set, detectors);
			break;

		//! covering strategy as in Butz and Wilson 2001
		case COVERING_ACTION_BASED:
			return perform_nma_covering(match_set, detectors);
			break;
		default:
			xcs_utility::error(class_name(),"perform_covering", "covering strategy not allowed", 1);
			exit(-1);
	}
}

//! perform covering based on the number of actions in [M]
bool
xcs_classifier_system::perform_standard_covering(t_classifier_set &match_set, const t_state& detectors)
{
	if ((match_set.size()==0) || need_standard_covering(match_set, detectors))
	{
		t_classifier	classifier;

		//! create a covering classifier
		classifier.cover(detectors);

		//! init classifier parameters
		init_classifier(classifier);


		//! insert the new classifier in [P]
		insert_classifier(classifier);

		//! delete another classifier from [P] if necessary
		delete_classifier();
		
		//! signal that a covering operation took place
		return true;
	}
	return false;
}

bool
xcs_classifier_system::need_standard_covering(t_classifier_set &match_set, const t_state& detectors)
{
	//unsigned long	i;
	t_set_iterator	pp;					//! iterator for visiting [P]
	//unsigned long	sz = 0;					//! number of micro classifiers in [M]
	double		average_prediction;			//!	average prediction in [P]
	double		total_match_set_prediction;		//!	total prediction in [M]

	if (match_set.size()==0)
		return true;

	average_prediction = 0;
	total_match_set_prediction = 0.;

	for(pp=population.begin();pp!=population.end();pp++)
	{
		average_prediction += (*pp)->prediction * (*pp)->numerosity;
		//cout << (*pp)->prediction << "*" << (*pp)->numerosity << "\t";
	}
	//cout << endl << endl;

	average_prediction = average_prediction/population_size;

	for(pp=match_set.begin(); pp!=match_set.end();pp++)
	{
		total_match_set_prediction += (*pp)->prediction * (*pp)->numerosity;
	}

	//cerr << "==> " << total_match_set_prediction << "<=" << fraction_for_covering << " x " << average_prediction << endl;;

	return (total_match_set_prediction<=fraction_for_covering*average_prediction);
}

void	
xcs_classifier_system::build_prediction_array()
{
	t_set_iterator					mp;
	vector<t_system_prediction>::iterator		pr;	
	t_system_prediction				prediction;

	//! clear P(.)
	init_prediction_array();

	//! scan [M] and build the prediction array
	for(mp=match_set.begin(); mp!=match_set.end(); mp++ )
	{
		//!	look whether the action was already found in the prediction array
		pr = find(prediction_array.begin(), prediction_array.end(), ((**mp).action));

		if (pr==prediction_array.end())
		{	
			xcs_utility::error(class_name(),"build_prediction_array", "action not found in prediction array", 1);
			exit(-1);
			/*!	the action was not previously found
			 *	thus prediction array is initialized with the
			 *	classifier values
			 */
			prediction.payoff = (**mp).prediction * (**mp).fitness;
			prediction.sum = (**mp).fitness;
			prediction.n = 1;
			//!	add the element to the prediction array
			prediction_array.push_back(prediction);
		} else {  
			/*!	the action was already in the array
			 *	thus the corresponding value is updated 
			 *	with the classifier values
			 */
			pr->payoff += (**mp).prediction * (**mp).fitness;
			pr->sum += (**mp).fitness;
			pr->n++;
		}
	}

	available_actions.clear();
	for(pr=prediction_array.begin(); pr!=prediction_array.end(); pr++ )
	{
		if (pr->n!=0)
		{
			available_actions.push_back((pr - prediction_array.begin()));
			pr->payoff /= pr->sum;
		}
	}
};

void	xcs_classifier_system::select_action(const t_action_selection policy, t_action& act)
{
	assert(available_actions.size()>0);

	switch(policy)
	{
		//! select the action with the highest payoff
		case ACTION_SELECTION_DETERMINISTIC:
			select_best_action(act);
			break;
			
		//! biased action selection
		case ACTION_SELECTION_EGREEDY:
			if (xcs_random::random()<prob_random_action)
				select_random_action(act);
			else
				select_best_action(act);
			
			break;

		case ACTION_SELECTION_RANDOM:
			select_random_action(act);
			break;

		default: 
			xcs_utility::error(class_name(),"select_action", "action selection strategy not allowed", 1);
	};
};

void 
xcs_classifier_system::select_best_action(t_action& action) const
{
	assert(available_actions.size()>0);

	// select best without shuffling
	long no_actions = available_actions.size();

	long random_action_index = xcs_random::dice(no_actions);

	long best_action_index = random_action_index;

	for(int i=1; i<no_actions; i++)
	{
		long next_action_index = (random_action_index+i)%no_actions;

		if (prediction_array[best_action_index].payoff<=prediction_array[next_action_index].payoff)
		{
			best_action_index = next_action_index;
		}
	}

	action = prediction_array[best_action_index].action;

	// random_shuffle(available_actions.begin(),available_actions.end());
	// std::shuffle(available_actions.begin(), available_actions.end(), xcs_random::rng());
	// 
	// best = available_actions.begin();
	// for(ap=available_actions.begin(); ap!=available_actions.end(); ap++)
	// {
	// 	if ((prediction_array[*best].payoff) < (prediction_array[*ap].payoff))
	// 		best = ap;
	// }
	// act = prediction_array[*best].action;

}

void 
xcs_classifier_system::select_random_action(t_action& action) const 
{
	// assert(available_actions.size()>0);
	action = prediction_array[available_actions[xcs_random::dice(available_actions.size())]].action;
	// stat_rnd++;
}

void
xcs_classifier_system::update_set(const double P, t_classifier_set &action_set)
{
	t_set_iterator	clp;
	double		set_size = 0;
	double		fitness_sum = 0;	//! sum of classifier fitness in [A]

	//! update the experience of classifiers in [A]
	//! estimate the action set size
	for(clp=action_set.begin(); clp != action_set.end(); clp++)
	{
		(**clp).experience++;
		set_size += (**clp).numerosity;
		fitness_sum += (**clp).fitness;	//! sums up classifier fitness for gradient descent
	}

	for(clp=action_set.begin(); clp!= action_set.end(); clp++)
	{
		//! prediction error is updated first if required (i.e., flag_error_update is true)
		if (flag_error_update_first)
		{
			//! update the classifier prediction error
			if (!flag_use_mam || ((**clp).experience>(1/learning_rate)))
			{
				(**clp).error += learning_rate*(fabs(P-(**clp).prediction)-(**clp).error);
			} else {
				(**clp).error += (fabs(P-(**clp).prediction)-(**clp).error)/(**clp).experience;
			}
		}

		//! update the classifier prediction

		if (flag_use_gradient_descent)
		{
			//! update the classifier prediction with gradient descent
			(**clp).prediction +=
				learning_rate*(P - (**clp).prediction) * ((**clp).fitness/fitness_sum);
		} else {
			//! usual update of classifier prediction
			if (!flag_use_mam || ((**clp).experience>(1/learning_rate)))
			{
				(**clp).prediction += learning_rate*(P - (**clp).prediction);
			} else {
				(**clp).prediction += (P - (**clp).prediction)/(**clp).experience;
			}
		}

		if (!flag_error_update_first)
		{
			//! update the classifier prediction error
			if (!flag_use_mam || ((**clp).experience>(1/learning_rate)))
			{
				(**clp).error += learning_rate*(fabs(P-(**clp).prediction)-(**clp).error);
			} else {
				(**clp).error += (fabs(P-(**clp).prediction)-(**clp).error)/(**clp).experience;
			}
		}


		//! update the classifier action set size estimate
		if (!flag_use_mam || ((**clp).experience>(1/learning_rate)))
		{
			(**clp).actionset_size += learning_rate*(set_size - (**clp).actionset_size);
		} else {
			(**clp).actionset_size += (set_size - (**clp).actionset_size)/(**clp).experience;
		}
	}

	//! update fitness
	update_fitness(action_set);

	//! do AS subsumption
	if (flag_as_subsumption)
	{	
		do_as_subsumption(action_set);
	}
}

void
xcs_classifier_system::update_fitness(t_classifier_set &action_set)
{
	t_set_iterator 			as;
	double				ra;
	vector<double>			raw_accuracy;
	vector<double>::iterator	rp;
	double				accuracy_sum = 0;

	raw_accuracy.clear();

	for(as = action_set.begin(); as !=action_set.end(); as++)
	{
		if ((**as).error<epsilon_zero)
			ra = (**as).numerosity;
		else 
			ra = alpha*(pow(((**as).error/epsilon_zero),-vi)) * (**as).numerosity;

		raw_accuracy.push_back(ra);
		accuracy_sum += ra;
	}

//	//! use mam for fitness
//
// 	for(as = action_set.begin(), rp=raw_accuracy.begin(); as!=action_set.end(); as++,rp++)
// 	{
// 		if (!flag_use_mam || ((**as).experience>(1/learning_rate)))
// 		{
// 			(**as).fitness += learning_rate*((*rp)/accuracy_sum - (**as).fitness);
// 		} else {
// 			(**as).fitness += ((*rp)/accuracy_sum - (**as).fitness)/(**as).experience;
// 		}
// 	}

	for(as = action_set.begin(), rp=raw_accuracy.begin(); as!=action_set.end(); as++,rp++)
	{
		(**as).fitness += learning_rate*((*rp)/accuracy_sum - (**as).fitness);
	}

}

bool
xcs_classifier_system::subsume(const t_classifier &first, const t_classifier &second)
{
	bool	result;
	
	result = (classifier_could_subsume(first, epsilon_zero, theta_sub)) && (first.subsume(second));

	if (result)
		stats.no_subsumption++;
	
	return result;
}

bool
xcs_classifier_system::need_ga(t_classifier_set &action_set, const bool flag_explore)
{
	double		average_set_stamp = 0;
	unsigned long	size = 0;

	if (!flag_explore) return false;

	t_set_iterator 	as;	

	for(as=action_set.begin(); as!=action_set.end(); as++)
	{
		average_set_stamp += (**as).time_stamp * (**as).numerosity;
		size += (**as).numerosity;
	}

	average_set_stamp = average_set_stamp / size;

	if (total_steps<average_set_stamp)
	{
		cout << "TOTSTEPS = " << total_steps << endl;
		cout << "AVGTS = " << average_set_stamp << endl;
	}

	if (total_steps<average_set_stamp)
		cerr << "NEEDGA " << total_steps << " " << average_set_stamp << endl; 
	assert(total_steps>=average_set_stamp);
	return ((total_steps - average_set_stamp)>=theta_ga);
}

// void
// xcs_classifier_system::condensation(t_classifier_set &action_set)
// {
// 	t_set_iterator 	parent1;
// 	t_set_iterator	parent2;

// 	t_classifier	offspring1;
// 	t_classifier	offspring2;

// 	update_timestamp(action_set);	
// }


void xcs_classifier_system::update_timestamp(t_classifier_set &action_set)
{
	//! set the time stamp of classifiers in [A]
	for(t_set_iterator as=action_set.begin(); as!=action_set.end(); as++)
	{
		(**as).time_stamp = total_steps;
	}
}
void
xcs_classifier_system::genetic_algorithm(t_classifier_set &action_set, const t_state& detectors, bool flag_condensation)
{
	t_set_iterator 	parent1;
	t_set_iterator	parent2;

	t_classifier	offspring1;
	t_classifier	offspring2;

	update_timestamp(action_set);

	// condensation always applies roulette-wheel-selection
	if (flag_condensation)
	{
		select_offspring(action_set, parent1, parent2);
	} else {
		//! select offspring classifiers
		if (flag_ga_tournament_selection)
		{
			select_offspring_ts(action_set, parent1);
			select_offspring_ts(action_set, parent2);
		} else {
			select_offspring(action_set, parent1, parent2);
		}
	}

	//! the GA is activated only if condensation is off
	if (!flag_condensation)
	{	
		offspring1 = (**parent1);
		offspring2 = (**parent2);

		offspring1.numerosity = offspring2.numerosity = 1;
		offspring1.experience = offspring2.experience = 1;

		if (xcs_random::random()<prob_crossover)
		{
			offspring1.recombine(offspring2);

			//! classifier parameters are inited from parents' averages
			if (flag_ga_average_init)
			{
				init_classifier(offspring1,true);
				init_classifier(offspring2,true);
				offspring1.prediction = offspring2.prediction = ((**parent1).prediction+(**parent2).prediction)/2;
			} else {
				offspring1.prediction = offspring2.prediction = ((**parent1).prediction+(**parent2).prediction)/2;
				offspring1.error = offspring2.error = ((**parent1).error+(**parent2).error)/2;
				offspring1.fitness = offspring2.fitness = ((**parent1).fitness+(**parent2).fitness)/2;
				offspring1.time_stamp = offspring2.time_stamp = total_steps;
				offspring1.actionset_size = offspring2.actionset_size = ((**parent2).actionset_size + (**parent2).actionset_size)/2;

			}
		}

		offspring1.mutate(prob_mutation,detectors);
		offspring2.mutate(prob_mutation,detectors);

		//! offsprings are penalized through the reduction of fitness
		offspring1.fitness = offspring1.fitness * 0.1;
		offspring2.fitness = offspring2.fitness * 0.1;

		t_condition	cond;
	
		if (cond.allow_ga_subsumption() && flag_ga_subsumption)
		{
			if (subsume(**parent1, offspring1))
			{	//! parent1 subsumes offspring1
				(**parent1).numerosity++;
				population_size++;
			} else if (subsume(**parent2, offspring1))
			{	//! parent2 subsumes offspring1
				(**parent2).numerosity++;
				population_size++;
			} else {
				//! neither of the parent subsumes offspring1
				if (!flag_gaa_subsumption)
				{
					//! if the usual GA subsumption is used, offspring classifier is inserted
					insert_classifier(offspring1);
				} else {
					//! if Martin's GA subsumption is used, offspring classifier is compared to the classifiers in [A]
					t_set_iterator 	par;

					ga_a_subsume(action_set,offspring1,par);
					if (par!=action_set.end())
					{				
						(**par).numerosity++;
						population_size++;
					} else {
						insert_classifier(offspring1);
					}
				}
			}
	
			if (subsume(**parent1, offspring2))
			{	//! parent1 subsumes offspring2
				(**parent1).numerosity++;
				population_size++;
			}
			else if (subsume(**parent2, offspring2))
			{	//! parent2 subsumes offspring2
				(**parent2).numerosity++;
				population_size++;
			} else {
				//! neither of the parent subsumes offspring1
				if (!flag_gaa_subsumption)
				{
					//! if the usual GA subsumption is used, offspring classifier is inserted
					insert_classifier(offspring2);
				} else {
					//! if Martin's GA subsumption is used, offspring classifier is compared to the classifiers in [A]
					t_set_iterator 	par;

					ga_a_subsume(action_set,offspring2,par);
					if (par!=action_set.end())
					{				
						(**par).numerosity++;
						population_size++;
					} else {
						insert_classifier(offspring2);
					}
				}
			}
			delete_classifier();
			delete_classifier();
		} else {	
			// insert offspring classifiers without subsumption
			insert_classifier(offspring1);
			insert_classifier(offspring2);

			delete_classifier();
			delete_classifier();
		}

	} else {
		// when in condensation
		(**parent1).numerosity++;
		population_size++;
		delete_classifier();

		(**parent2).numerosity++;
		population_size++;
		delete_classifier();
	}
}

void	
xcs_classifier_system::step(const bool exploration_mode, const bool condensationMode)
{
	t_action	action;					//! selected action
	unsigned long	match_set_size;		//! number of microclassifiers in [M]
	//unsigned long	action_set_size;	//! number of microclassifiers in [A]
	double		P;						//! value for prediction update, computed as r + gamma * max P(.) 
	double		max_prediction;

	//! reads the current input
	current_input = environment->state(); 

	//! update the number of learning steps performed so far
	if (exploration_mode)
	{
		total_steps++;
		total_learning_steps++;
	} 

	total_time++;

	/*! 
	 * check if [M] needs covering,
	 * if it does, it apply the selected covering strategy, i.e., standard as defined in Wilson 1995,
	 * or action_based as defined in Butz and Wilson 2001
	 */

	do {
		match_set_size = match(current_input);
	}
   	while (perform_covering(match_set, current_input));

	//! build the prediction array P(.)
	build_prediction_array();

#ifdef __DEBUG__
	cout << "BUILT THE PREDICTION ARRAY" << endl;
	print_prediction_array(cout);
	cout << endl;
#endif
	
	//! select the action to be performed
	if (exploration_mode)
		select_action(action_selection_strategy, action);
	else 
		select_action(ACTION_SELECTION_DETERMINISTIC, action);

	//! build [A]
	build_action_set(action);

#ifdef __DEBUG__
	cout << "ACTION " << action << endl;
#endif
	//! store the current input before performing the selected action	
	previous_input = environment->state();

	environment->perform(action);

	//! if the environment is single step, the system error is collected
	if (environment->single_step())
	{
		double payoff = prediction_array[action.value()].payoff;
		system_error = fabs(payoff-environment->reward());
	}

#ifdef __DEBUG__
	cout << "ACTION " << action << "\t";
	cout << "REWARD " << Environment->reward() << endl;

	if (flag_update_test)
		cerr << "Update During Test" << endl;
	else 
		cerr << "No Update During Test" << endl;
#endif

	total_reward = total_reward + environment->reward();

	//! reinforcement component
	
	//! if [A]-1 is not empty it computes P
	if ((exploration_mode || flag_update_test) && previous_action_set.size())
	{

#ifdef __DEBUG__
		cerr << "Fa l'update" << endl;
#endif

		vector<t_system_prediction>::iterator	pr = prediction_array.begin();
		max_prediction = pr->payoff;

		for(pr = prediction_array.begin(); pr!=prediction_array.end(); pr++)
		{
			if (max_prediction<pr->payoff)
			{
				max_prediction = pr->payoff;
			}
		}

		P = previous_reward + discount_factor * max_prediction;

		//! use P to update the classifiers parameters
		update_set(P, previous_action_set);
	}

	if (environment->stop())
	{
		P = environment->reward();
		if (exploration_mode || flag_update_test)
		{
#ifdef __DEBUG__
			cerr << "Fa l'update" << endl;
#endif
			update_set(P, action_set);
		}
	}

	//! apply the genetic algorithm to [A] if needed
	if (flag_discovery_component && need_ga(action_set, exploration_mode))
	{
		genetic_algorithm(action_set, previous_input, condensationMode);
		stats.no_ga++;
	}
	
	//!	[A]-1 <= [A]
	//!	r-1 <= r
	previous_action_set = action_set;
	action_set.clear();
	previous_reward = environment->reward();
}

void	
xcs_classifier_system::save_population(ostream& output) 
{
	t_set_iterator		pp;
	t_classifier_set 	save;

	for(pp=population.begin(); pp!=population.end(); pp++)
	{
		output << (**pp) << endl;
	}
}

void	
xcs_classifier_system::save_state(ostream& output) 
{
	output << stats << endl;
	output << total_steps << endl;
	t_classifier::save_state(output);
	output << macro_size << endl;

	save_population(output);
}

void
xcs_classifier_system::restore_state(istream& input)
{
	unsigned long size;
	input >> stats;
	input >> total_steps;
	t_classifier::restore_state(input);
	input >> size;
	
	population.clear();
	
    t_classifier in_classifier;
	population_size = 0;
	macro_size = 0;

	for(unsigned long cl=0; cl<size; cl++)
	{
		if (!input.eof() && (input >> in_classifier))
		{
			t_classifier	*classifier = new t_classifier(in_classifier);
			population.push_back(classifier);
			population_size += classifier->numerosity;
			macro_size++;
		}
	};
	assert(macro_size==size);
}

//! defines what has to be done when a new experiment begins
void 
xcs_classifier_system::begin_experiment()
{
	//! reset the overall time step
	total_steps = 0;
	total_time = 0;

	//! reset the number of overall learning steps
	total_learning_steps = 0;

	//! reset the counter of classifiers ids
	t_classifier::reset_id();

	//! init the experiment statistics
	stats.reset();
	
	//! [P] contains 0 macro/micro classifiers
	population_size = 0;
	macro_size = 0;

	//! init [P]
	init_classifier_set();
}


//! defines what has to be done when a new problem begins. 
void
xcs_classifier_system::begin_problem()
{
	//! clear [A]-1
	previous_action_set.clear();
	
	//! clear [A]
	action_set.clear();

	//! set the steps within the problem to 0
	problem_steps = 0;

	//! clear the total reward gained
	total_reward = 0;

	//! added to clear the population
	/*while (population_size>max_population_size)
		delete_classifier();*/
}

//! defines what must be done when the current problem ends
void 
xcs_classifier_system::end_problem() 
{
	match_set.clear();
	action_set.clear();
}



bool	
xcs_classifier_system::perform_nma_covering(t_classifier_set &match_set, const t_state& detectors)
{
	vector<t_system_prediction>::iterator	pr;
	t_action		act;
	unsigned long		total_actions = act.actions();
	unsigned long		covered_actions = total_actions;
	bool			covered_some_actions = false;		//! becomes true when covering classifiers are created

	//! clear the prediction array
	init_prediction_array();

	//! build P(.)
	build_prediction_array();

	/*for(pr=prediction_array.begin(); pr!=prediction_array.end(); pr++ )
	{
		//! 
		if (pr->n==0)
		{	
			covered_actions--;
		}
	}
	*/

	//! the number of actions that are covered is computed as the number of available actions
	//! in the prediction array P(.)
	
	covered_actions = available_actions.size();

	covered_some_actions = false;

	if (covered_actions<tetha_nma)
	{
		for(pr=prediction_array.begin(); pr!=prediction_array.end() && (covered_actions<tetha_nma); pr++ )
		{
			//! 
			if (pr->n==0)
			{
				t_classifier	classifier;
				
				classifier.cover(detectors);
				classifier.action = pr->action;

				init_classifier(classifier, flag_cover_average_init);
				
				insert_classifier(classifier);

				delete_classifier();

				covered_actions++;
			}
		}
		covered_some_actions = true;
	}

	return covered_some_actions;
};

void	
xcs_classifier_system::init_classifier(t_classifier& classifier, bool average)
{
	if (!average || (population_size==0))
	{
		classifier.prediction = init_prediction;
		classifier.error = init_error;
		classifier.fitness = init_fitness;
	
		classifier.actionset_size = init_set_size;
		classifier.experience = 0;
		classifier.time_stamp = total_steps;

		classifier.numerosity = 1;

	} else {
		t_set_iterator	cl;

		double		toterror = 0;
	   	double		totprediction = 0;
	   	double		totfitness = 0;
	   	double		totactionset_size = 0;
		unsigned long	popSize = 0;
		unsigned long	pop_sz = 0;
			
		for(cl=population.begin(); cl!=population.end(); cl++)
		{
			toterror += (**cl).error * (**cl).numerosity;
			totprediction += (**cl).prediction * (**cl).numerosity;
			totfitness += (**cl).fitness;
			totactionset_size += (**cl).actionset_size * (**cl).numerosity;
			popSize += (**cl).numerosity;
			pop_sz++;
		}

		classifier.prediction = totprediction/popSize;
		classifier.error = .25 * toterror/popSize;
		classifier.fitness = 0.1 * totfitness/pop_sz;
		classifier.actionset_size = totactionset_size/popSize;
		classifier.numerosity = 1;
		classifier.time_stamp = total_steps;
		assert(classifier.actionset_size>=0);
		assert(classifier.fitness>=0);

	};
}

//! build [A] from [M] and an action "act"
/*!
 * \param action selected action
 */
void	
xcs_classifier_system::build_action_set(const t_action& action)
{
	//! iterator in [M]
	t_set_iterator 	mp;

	//! clear [A]
	action_set.clear();

	//! fill [A] with the classifiers in [M] that have action "act"
	for( mp=match_set.begin(); mp!=match_set.end(); mp++ )
	{
		//if ((**mp).action==action)
		if ((*mp)->action==action)
		//if (true)
		{
			action_set.push_back(*mp);
		}
	}

	std::shuffle(action_set.begin(), action_set.end(), xcs_random::rng());
}

//! clear [P]
void
xcs_classifier_system::clear_population()
{
	//! iterator in [P]
	t_set_iterator	pp;

	//! delete all the classifiers in [P]
	for(pp=population.begin(); pp!=population.end(); pp++)
	{
		delete *pp; 
	}

	//! delete all the pointers in [P]
	population.clear();

	//! number of macro classifiers is set to 0
	macro_size = 0;

	//! number of micro classifiers is set to 0
	population_size = 0;
}

//! print a set of classifiers
/*!
 * \param set set of classifiers
 * \bug cause a memory leak!
 */
void 
xcs_classifier_system::print_set(t_classifier_set &set, ostream& output) 
{
	t_set_const_iterator	pp;

	output << "================================================================================" << endl;
	for(pp=set.begin(); pp!=set.end(); pp++)
	{
		output << (**pp);
		output << endl;
	}
	output << "================================================================================" << endl;
}

//! check the integrity of the population
/*!
 * \param str comment that is printed before the trace information
 * \param output stream where the check information are printed
 */
void
xcs_classifier_system::check(string str, ostream& output) 
{
	//! iterator in [P]
	t_set_iterator	pp;

	//! check for the number of microclassifiers
	unsigned long check_population_size = 0;
	//! check for the number of macroclassifiers
	unsigned long check_macro_size = 0;

	//! count the number of micro and macro classifiers in [P]
	output << "CHECK <" << str << ">" << endl;
	output << "======================================================================" << endl;
	for(pp=population.begin(); pp!=population.end(); pp++)
	{
		check_population_size += (**pp).numerosity;
		check_macro_size ++;
	}
	output << "counter   = " << population_size << endl;
	output << "check     = " << check_population_size << endl;
	output << "limit     = " << max_population_size << endl;
	output  << "======================================================================" << endl;

	//! compare che check parameters to the current parameters
	assert(check_macro_size==macro_size);
	assert(check_population_size==population_size);
}


//! perform the operations needed at the end of the experiment
void 
xcs_classifier_system::end_experiment() 
{

}

//@{
//! set the strategy to init [P] at the beginning of the experiment
/*! 
 * \param strategy can be either \emph empty or \emph random
 * two strategies are allowed \emph empty set [P] to the empty set
 * \emph random fills [P] with random classifiers
 */
void
xcs_classifier_system::set_init_strategy(string strategy)
{

#ifdef __DEBUG__
	cout << "STRATEGY " << strategy << endl;
	cout << "PREFIX " << strategy.substr(0,5)<< endl;
	cout << "SUFFIX " << strategy.substr(5)<< endl;
#endif

	if ( (strategy!="random") && (strategy!="empty") && (strategy.substr(0,5)!="load:") && (strategy.substr(0,9)!="solution:"))
	{	
		//!	unrecognized deletion strategy
		string	msg = "Unrecognized population init policy";
		xcs_utility::error(class_name(),"set_init_strategy", msg, 1);
	}

	if (strategy=="empty")
		population_init = POPULATION_INIT_EMPTY;
	else if (strategy=="random")
		population_init = POPULATION_INIT_RANDOM;
	else if (strategy.substr(0,5)=="load:")
	{
		population_init = POPULATION_INIT_LOAD;
		population_init_file = strategy.substr(5);
		cout << "LOAD FROM <" << population_init_file << ">" << endl;
	} else if (strategy.substr(0,9)=="solution:")
	{
		population_init = POPULATION_INIT_SOLUTION;
		population_init_file = strategy.substr(9);
		cout << "INIT POPULATION WITH SOLUTION FROM <" << population_init_file << ">" << endl;
	} else {
		//!	unrecognized deletion strategy
		string	msg = "Unrecognized population init policy";
		xcs_utility::error(class_name(),"set_init_strategy", msg, 1);
	}
}
//@}

//! select offspring classifiers from the population
//	20020722 modified the two for cycles so to continue from one to another
void
xcs_classifier_system::select_offspring(t_classifier_set &action_set, t_set_iterator &clp1, t_set_iterator &clp2)
{
	t_set_iterator	as;		//! iterator in [A]
	vector<double>	select;		//! vector used to implement the roulette wheel
	unsigned long	sel;		//! counter

	double		fitness_sum;
	double		random1;
	double		random2;

	select.clear();

	fitness_sum = 0;
	for(as=action_set.begin(); as!=action_set.end(); as++)
	{
		fitness_sum += (**as).fitness;
		select.push_back( fitness_sum );
	}

	random1 = (xcs_random::random())*fitness_sum;
	random2 = (xcs_random::random())*fitness_sum;


	if (random1>random2)
		swap(random1,random2);

	t_set_iterator test1 = select_proportional(action_set, select, random1);
	t_set_iterator test2 = select_proportional(action_set, select, random2);

	for(sel = 0; (sel<select.size()) && (random1>=select[sel]); sel++);
	clp1 = action_set.begin()+sel;	// to be changed if list containers are used
	assert(sel<select.size());

	for(; (sel<select.size()) && (random2>=select[sel]); sel++);
	clp2 = action_set.begin()+sel;	// to be changed if list containers are used
	assert(sel<select.size());

	assert((**clp1).id()==(**test1).id());
	assert((**clp2).id()==(**test2).id());
}

void	
xcs_classifier_system::set_covering_strategy(const string& strategy)
{
	double threshold = 0;

	if (strategy=="single")
	{
		covering_strategy = COVERING_STANDARD;
		fraction_for_covering = threshold;
	} else if (strategy=="standard") {
		t_action	action;

		covering_strategy = COVERING_ACTION_BASED;
		tetha_nma = action.actions();

	} else {
			xcs_utility::error(class_name(),"set_covering_strategy", "covering strategy \'"+strategy+"\' not recognized", 1);
	}
}

void	
xcs_classifier_system::create_prediction_array()
{
	t_action		action;
	t_system_prediction	prediction;

	//! clear the prediction array
	prediction_array.clear();

	//cout << "NUMBER OF ACTIONS " << action.actions() << endl;
	//!	build the prediction array with all the possible actions
	action.reset_action();

	do {
		//clog << "ADD ACTION IN PREDICTION ARRAY <" << action << ">" << endl; 
		prediction.action = action;
		prediction.n = 0;
		prediction.payoff = 0;
		prediction.sum = 0;
		prediction_array.push_back(prediction);
	} while (action.next_action());

}

void	
xcs_classifier_system::init_prediction_array()
{
	vector<t_system_prediction>::iterator	sp;

	for(sp=prediction_array.begin(); sp!=prediction_array.end(); sp++)
	{
		sp->n =0;
		sp->payoff = 0;
		sp->sum = 0;
	}
}

void 
xcs_classifier_system::erase_population()
{
	t_set_iterator			pp;		//! iterator for visiting [P]
	for(pp=population.begin(); pp!=population.end(); pp++)
	{
		delete (*pp);
	}
	population.clear();

	population_size = 0;
}

//! delete a set of classifiers from [P], [M], [A], [A]-1
void	
xcs_classifier_system::as_subsume(t_set_iterator classifier, t_classifier_set &set)
{
	t_set_iterator	sp;		//! iterator for visiting the set of classifier
	t_set_iterator	pp;		//! iterator for visiting [P]

        t_classifier *most_general;	//! keeps track of the most general classifier

	most_general = *classifier;

	for(sp=set.begin(); sp!=set.end(); sp++)
	{
		//! remove cl from [M], [A], and [A]-1
		t_set_iterator	clp;
		clp = find(action_set.begin(),action_set.end(), *sp);
		if (clp!=action_set.end())
		{
			action_set.erase(clp);
		}

		clp = find(match_set.begin(),match_set.end(), *sp);
		if (clp!=match_set.end())
		{
			match_set.erase(clp);
		}

		clp = find(previous_action_set.begin(),previous_action_set.end(), *sp);
		if (clp!=previous_action_set.end())
		{
			previous_action_set.erase(clp);
		}


		pp = lower_bound(population.begin(),population.end(),*sp,compare_cl);
		if ((pp!=population.end()))
		{
			//! found another classifier, something is wrong
			if ( (*pp)!=(*sp) )
			{
				xcs_utility::error(class_name(),"as_subsumption", "classifier not found", 1);
				exit(-1);
			}
		}

		macro_size--;

                most_general->numerosity += (*pp)->numerosity;

		delete *pp;
		population.erase(pp);
	}
	set.clear();
}

//! find the classifiers in set that are subsumed by the classifier 
void	
xcs_classifier_system::find_as_subsumed(
	t_set_iterator classifier, 
	t_classifier_set &set, 
	t_classifier_set &subsumed)
const
{
	// t_set_iterator	sp;	//! pointer to the elements in the classifier set

	subsumed.clear();

	if (classifier!=set.end())
	{
		for(t_set_const_iterator sp=set.begin(); sp!=set.end(); sp++ )
		{
			if (*classifier!=*sp)
			{
				if ((*classifier)->is_more_general_than(**sp))
				{
					subsumed.push_back(*sp);
				}
			}
		}
	}
}

//! perform action set subsumption on the classifier in the set
void	
xcs_classifier_system::do_as_subsumption(t_classifier_set &set)
{
	/*! 
	 * \brief check whether the condition type allow action set subsumption
	 *
	 * the same check is already performed at construction time. 
	 * thus this might be deleted.
	 */
	t_condition	cond;
	if (!cond.allow_as_subsumption())
	{
		xcs_utility::error(class_name(),
			"do_as_subsumption", 
			"condition does not allow action set subsumption", 1);
	}

	//! find the most general classifier
	t_set_iterator most_general;
	
	most_general = find_most_general(set);

	if ((most_general!=set.end()) && !classifier_could_subsume(**most_general, epsilon_zero, theta_sub))
	{
		xcs_utility::error(class_name(),
			"do_as_subsumption", 
			"classifier could not subsume", 1);
	}


	//! if there is a "most general" classifier, it extracts all the subsumed classifiers
	if (most_general!=set.end())
	{
		t_classifier_set subsumed;

		find_as_subsumed(most_general, set, subsumed);

		if (subsumed.size())
		{
			as_subsume(most_general, subsumed);
		}
	}
}

xcs_classifier_system::t_set_iterator
xcs_classifier_system::find_most_general(t_classifier_set &set) const
{
	t_set_iterator 	most_general = set.end();	
	
	t_set_iterator	sp;

	for( sp=set.begin(); sp!=set.end(); sp++ )
	{
		if ( classifier_could_subsume( (**sp), epsilon_zero, theta_as_sub) )
		{
			if (most_general==set.end())
				most_general = sp;
			else {
				if ( (*sp)->subsume(**most_general))
						
					most_general = sp;
			}
		}
	}

	return most_general;
}

void
xcs_classifier_system::init_population_random()
{
	unsigned long	cl;
	timer		check;

	erase_population();

	check.start();
	for(cl=0; cl<max_population_size; cl++)
	{
		t_classifier *classifier = new t_classifier();
		classifier->random();
		init_classifier(*classifier);
		insert_classifier(*classifier);
	}

	check.stop();
	macro_size = population.size();
	population_size = max_population_size;
};

void
xcs_classifier_system::print_prediction_array(ostream& output) const
{
	vector<t_system_prediction>::const_iterator		pr;	

	for(pr=prediction_array.begin(); pr!=prediction_array.end(); pr++)
	{
		output << "(" << pr->action << ", " << pr->payoff << ")";
	}
}

void
xcs_classifier_system::select_offspring_ts(t_classifier_set& set, t_set_iterator& clp)
{
	t_set_iterator	as;				//! iterator in set
	t_set_iterator	winner = set.end();

	while (winner==set.end())
	{
		for(as=set.begin(); as!=set.end(); as++)
		{
			bool selected = false;

			for(unsigned long num=0; (!selected && (num<(**as).numerosity)); num++)
			{
				if (xcs_random::random()<tournament_size)
				{
					if ((winner==set.end()) ||
					    (((**winner).fitness/(**winner).numerosity)<((**as).fitness/(**as).numerosity)))
					{
						winner = as;
						selected = true;
					}
				}
			}
		}
	}

	clp = winner;
}

void
xcs_classifier_system::init_population_load(string filename)
{

	population.clear();		//! clear [P] before loading (20030808)

	ifstream	POPULATION(filename.c_str());

	if (!POPULATION.good())
	{
		xcs_utility::error( class_name(), "init_population_load", "file <"+filename+"> not found", 1);
	}
	t_classifier	in_classifier;
	unsigned long	n = 0;
	macro_size = 0;
	population_size = 0;

	// while(!(POPULATION.eof()) && POPULATION>>in_classifier)
	// {
	// 	t_classifier	*classifier = new t_classifier(in_classifier);
	// 	classifier->time_stamp = total_steps;
	// 	population.push_back(classifier);
	// 	population_size += classifier->numerosity;
	// 	macro_size++;
	// }


	string str_classifier;
	while(!POPULATION.eof())
	{
		std::getline(POPULATION, str_classifier);
		t_classifier classifier;

		str_classifier = xcs_utility::trim(str_classifier);
		cout << "STR  " << str_classifier << endl;

		// classifier.read_from_string(str_classifier);

		if (str_classifier!="")
		{
			stringstream CLASSIFIER(str_classifier);
			CLASSIFIER >> in_classifier;
			cout << "READ " << in_classifier << endl << endl;

			t_classifier	*classifier = new t_classifier(in_classifier);
			classifier->time_stamp = total_steps;
			population.push_back(classifier);
			population_size += classifier->numerosity;
			macro_size++;
		}
	}

	// do {
	// 	POPULATION>>in_classifier;
	// 	t_classifier	*classifier = new t_classifier(in_classifier);
	// 	classifier->time_stamp = total_steps;
	// 	population.push_back(classifier);
	// 	population_size += classifier->numerosity;
	// 	macro_size++;
	// }
	// while(!POPULATION.eof());


	sort(population.begin(),population.end(),compare_cl);
}

void
xcs_classifier_system::init_population_solution(string filename)
{

	population.clear();		//! clear [P] before loading (20030808)

	ifstream	SOLUTION(filename.c_str());
	string		condition;
	string		action;
	double 		prediction;

	if (!SOLUTION.good())
	{
		xcs_utility::error( class_name(), "init_population_solution", "file <"+filename+"> not found", 1);
	}

	t_classifier	*in_classifier;
	unsigned long	n = 0;
	macro_size = 0;
	population_size = 0;

	while(!(SOLUTION.eof()) && SOLUTION>>condition>>action>>prediction)
	{
		t_classifier	*classifier = new t_classifier();

		classifier->condition.set_string_value(condition);
		classifier->action.set_string_value(action);
		classifier->prediction = prediction;
		population.push_back(classifier);
		macro_size++;
	}

	unsigned long numerosity = max_population_size/macro_size;

	population_size = 0;

	for(size_t i=0; i<population.size(); i++)
	{
		population[i]->fitness = 1.0;
		population[i]->error = 0.0;
		population[i]->actionset_size = numerosity;
		population[i]->numerosity = numerosity;
		population[i]->experience = 1;
		population[i]->time_stamp = total_steps;
		population_size = population_size+numerosity;
	}

	for(size_t i=0; i<population.size() && population_size<max_population_size; i++)
	{
		population[i]->numerosity++;
		population_size++;
	}

	for(size_t i=0; i<population.size(); i++)
	{
		cout << *(population[i]) << endl;
	}

	sort(population.begin(),population.end(),compare_cl);
}

//! random deletion 
xcs_classifier_system::t_set_iterator
xcs_classifier_system::select_delete_random(t_classifier_set &set)
{
	t_set_iterator 	pp;
	vector<double>	select;

	unsigned long	random;
	unsigned long	size;

	unsigned long	sel;
	unsigned long	sum;

	select.clear();
	select.reserve(set.size());

	//! check [P]

	size = 0;

	for(pp=set.begin(); pp!=set.end(); pp++)
	{
		size += (**pp).numerosity;
		select.push_back(size);
	}

	random = xcs_random::dice(size);


	pp = set.begin();

	for(sel=0; ((sel<select.size()) && (random>select[sel])); sel++,pp++);
	assert(sel<select.size());

	t_set_iterator test1 = select_proportional(action_set, select, random);
	assert((**pp).id()==(**test1).id());

	
	return pp;
}

//! roulette wheel selection
xcs_classifier_system::t_set_iterator
xcs_classifier_system::select_delete_rw(t_classifier_set &set)
{
	t_set_iterator 	pp;
	vector<double>	select;

	double		average_fitness = 0.;
	double		vote_sum;
	double		vote;
	double		random;
	double		size;

	unsigned long	sel;

	select.clear();
	select.reserve(set.size());

	//! check [P]
	//check("enter delete", clog);

	for(pp=set.begin(); pp!=set.end(); pp++)
	{
		average_fitness += (**pp).fitness;
		size += (**pp).numerosity;
	}

	average_fitness /= ((double) size);

	vote_sum = 0;

	unsigned long	el = 0;		//! set element
	for(pp=set.begin(); pp!=set.end(); pp++)
	{
		//! compute the deletion vote;
		vote = (**pp).actionset_size * (**pp).numerosity;

		if (flag_delete_with_accuracy)
		{
			if (((**pp).experience>theta_del) && ((((**pp).fitness)/double((**pp).numerosity))<delta_del*average_fitness))
			{
				vote = vote * average_fitness/(((**pp).fitness)/double((**pp).numerosity));
			}
		}
		vote_sum += vote;
		select.push_back(vote_sum);
	}

	random = vote_sum*(xcs_random::random());

	vector<double>::iterator pp_test = lower_bound(select.begin(),select.end(),random);

	size_t index = (pp_test - select.begin());

	return (population.begin()+index);
}

//! delete classifier from the population according to the selected strategy
void
xcs_classifier_system::delete_classifier()
{	
	t_set_iterator 	pp;

	double		average_fitness = 0.;
	double		vote_sum;
	double		vote;
	double		random;

	unsigned long	sel;

	if (population_size<=max_population_size)
		return;

	switch(delete_strategy)
	{
 		case XCS_DELETE_RWS_SETBASED:
 		case XCS_DELETE_RWS_FITNESS:
			pp = select_delete_rw(population);
			break;
		case XCS_DELETE_RANDOM:				//! random delete
		case XCS_DELETE_RANDOM_WITH_ACCURACY:		//! random delete
			pp = select_delete_random(population);
			break;
		default:
			xcs_utility::error(class_name(),"delete_classifier", "delete strategy not allowed", 1);
	}

	if ((**pp).numerosity>1)
	{
		(**pp).numerosity--;
		population_size--;
	} else {
		//	remove cl from [M], [A], and [A]-1
		t_set_iterator	clp;
		clp = find(action_set.begin(),action_set.end(), *pp);
		if (clp!=action_set.end())
		{
			action_set.erase(clp);
		}

		clp = find(match_set.begin(),match_set.end(), *pp);
		if (clp!=match_set.end())
		{
			match_set.erase(clp);
		}

		clp = find(previous_action_set.begin(),previous_action_set.end(), *pp);
		if (clp!=previous_action_set.end())
		{
			previous_action_set.erase(clp);
		}

		delete *pp;
		
		population.erase(pp);
		population_size--;
		macro_size--;
	}
}

double	
xcs_classifier_system::specificity(const t_classifier_set &set) const
{
	double specificity = 0;
	double sz = 0;

	t_set_const_iterator	clp;

	for(clp=set.begin(); clp != set.end(); clp++)
	{
// 		specificity += (**clp).condition.specificity()*(**clp).numerosity; 
		sz += (**clp).numerosity;
	}
	return specificity/sz;
}

double	
xcs_classifier_system::average_gradient(const t_classifier_set &set) const
{
	t_set_const_iterator	clp;

	double	fitness_sum = 0;	//! sum of classifier fitness in [A]
	double	avg_gradient = 0;	//! average gradient
	
	//! compute fitness sum
	for(clp=set.begin(); clp != set.end(); clp++)
	{
		fitness_sum += (**clp).fitness;
	}

	//! compute sum of gradients
	avg_gradient = 0;
	for(clp=set.begin(); clp != set.end(); clp++)
	{
		double gr = ((**clp).fitness/fitness_sum);
		assert(gr<=double(1.0));
		avg_gradient += ((**clp).fitness/fitness_sum);
	}

#ifdef __DEBUG__
	cerr << "AVG GR = "  << avg_gradient << "/" << action_set.size();
#endif
	//! average gradient
	avg_gradient = avg_gradient/action_set.size();

	return avg_gradient;
}

//! check whether cl is subsumed by any of the classifiers in the action set
void
xcs_classifier_system::ga_a_subsume(t_classifier_set &action_set, const t_classifier &cl, t_set_iterator &mg)
{
	cout << "SI ";
	t_set_iterator	as;
	
	mg = action_set.end();
	
	//! set the time stamp of classifiers in [A]
	for(as=action_set.begin(); (mg==action_set.end())&&(as!=action_set.end()); as++)
	{
		if (subsume(**as, cl))
		{
			mg = as;	
		}
	}
}

xcs_classifier_system::t_set_iterator 
xcs_classifier_system::select_proportional(t_classifier_set& classifier_set, const vector<double>& select, double value) const
{
	vector<double>::const_iterator selected = lower_bound(select.begin(),select.end(),value);

	size_t index = (selected - select.begin());

	return classifier_set.begin()+index;
}

std::vector<double>
xcs_classifier_system::predict(t_state inputs)
{
	unsigned long match_set_size;
	vector<double> prediction;

	do {
		match_set_size = match(inputs);
	}
   	while (perform_covering(match_set, inputs));

	//! build the prediction array P(.)
	build_prediction_array();

	for(int i=0; i<prediction_array.size(); i++)
	{
		prediction.push_back(0);
	}
	for(int i=0; i<prediction_array.size(); i++)
	{
		prediction[prediction_array[i].action.value()]=prediction_array[i].payoff;
	}

	return prediction;
}

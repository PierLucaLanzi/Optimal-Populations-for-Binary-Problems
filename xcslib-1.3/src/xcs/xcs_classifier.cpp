#include <ostream>
#include <sstream>

#include "xcs_classifier.h"
#include "xcs_utility.h"

using namespace std;

unsigned long		xcs_classifier::id_count = 0;
unsigned int		xcs_classifier::output_precision = 5;	


//! class constructor
xcs_classifier::xcs_classifier()
{
	set_initial_values();	
}

//! copy constructor
xcs_classifier::xcs_classifier(const xcs_classifier& classifier)
{
	set_initial_values();	
	condition = classifier.condition;
	action = classifier.action;
	prediction = classifier.prediction;		//!< prediction 
	error = classifier.error;			//!< prediction error
	fitness = classifier.fitness;			//!< classifier fitness
	actionset_size = classifier.actionset_size;	//!< estimate of the size of the action set [A]
	
	experience = classifier.experience;		//!< classifier experience, i.e., the number of times that the classifier has een updated
	numerosity = classifier.numerosity;		//!< classifier numerosity, i.e., the number of micro classifiers
	time_stamp = classifier.time_stamp;		//!< time of the last genetic algorithm application


}

//! comparison operators 

bool 
operator==(const xcs_classifier& classifier1, const xcs_classifier& classifier2)
{
	return ((classifier1.condition == classifier2.condition) && (classifier1.action == classifier2.action));
}

bool 
operator!=(const xcs_classifier& classifier1, const xcs_classifier& classifier2)
{
	return !(classifier1==classifier2);
}

bool 
operator<(const xcs_classifier& classifier1, const xcs_classifier& classifier2)
{
	return ( (classifier1.condition < classifier2.condition) ||  
		 ((classifier1.condition == classifier2.condition) && (classifier1.action < classifier2.action)) );
}

//! stream operators

ostream&
operator<<(ostream& output,const xcs_classifier& classifier)
{
	output << classifier.id() << '\t';
	output << classifier.condition << "\t";
	output << classifier.action << '\t';
	output.setf(ios::scientific);
	output.precision(xcs_classifier::output_precision);
	output << classifier.prediction << '\t';
  	output.precision(xcs_classifier::output_precision);
  	output << classifier.error << '\t';
  	output.precision(xcs_classifier::output_precision);
  	output << classifier.fitness << '\t';
  	output.precision(xcs_classifier::output_precision);
  	output << classifier.actionset_size << '\t';
  	output.precision(xcs_classifier::output_precision);
  	output << classifier.experience << '\t';
  	output.precision(xcs_classifier::output_precision);
  	output << classifier.numerosity;

	return (output);
}


//! read the classifier from an input stream
istream&
operator>>(istream& is, xcs_classifier& classifier)
{
	char sep;

	long double prediction;
	long double error;
	long double fitness;
	long double actionset_size;


	if (!(is>>classifier.identifier))
	{
		xcs_utility::error(classifier.class_name(), ">>", "identifier failed to read", 1);
	} else {
		clog << "identifier " << classifier.identifier << endl;
	}

	if (!(is>>classifier.condition))
	{
		xcs_utility::error(classifier.class_name(), ">>", "condition failed to read", 1);
	}
		
	if (!(is>>classifier.action))
	{
		xcs_utility::error(classifier.class_name(), ">>", "action failed to read", 1);
	}
		
	if (!(is>>prediction))
	{
		xcs_utility::error(classifier.class_name(), ">>", "prediction failed to read", 1);
	} else {
		classifier.prediction = prediction;
	}
		
	if (!(is >> error))
	{
		xcs_utility::error(classifier.class_name(), ">>", "error failed to read", 1);
	} else {
		classifier.error = error;
	}

	if (!(is >> fitness))
	{
		xcs_utility::error(classifier.class_name(), ">>", "fitness failed to read", 1);
	} else {
		classifier.fitness = fitness;
	}

	if (!(is >> actionset_size))
	{
		xcs_utility::error(classifier.class_name(), ">>", "actionset_size failed to read", 1);
	}else {
		classifier.actionset_size = actionset_size;
	}

	if (!(is >> classifier.experience))
	{
		xcs_utility::error(classifier.class_name(), ">>", "experience failed to read", 1);
	}

	if (!(is >> classifier.numerosity))
	{
		xcs_utility::error(classifier.class_name(), ">>", "numerosity failed to read", 1);
	}

	return (is);
}

bool	
xcs_classifier::match(const t_state& inputs)
{
	return (condition.match(inputs));
}

void	
xcs_classifier::random()
{
	condition.random();
	action.random();
	set_initial_values();
}

void	
xcs_classifier::cover(const t_state& inputs)
{
	condition.cover(inputs);
	action.random();
	set_initial_values();
}

void
xcs_classifier::mutate(const float mutation_probability, const t_state& inputs) 
{
	condition.mutate(mutation_probability,inputs);
	action.mutate(mutation_probability);
}

void
xcs_classifier::recombine(xcs_classifier& classifier)
{
	condition.recombine(classifier.condition);
	swap(action,classifier.action);
}


bool 
xcs_classifier::subsume(const xcs_classifier& classifier)
const
{
	return ((action==classifier.action) && this->condition.is_more_general_than(classifier.condition));
};


inline
void 
xcs_classifier::set_initial_values()
{
	identifier = xcs_classifier::id_count++;
	numerosity = 1;
	time_stamp=0;
	experience=0;
	prediction=0;
	error=0;
	fitness=0;
	actionset_size=0;

}

void 
xcs_classifier::read_from_string(string line)
{
	std::vector<std::string> fields = xcs_utility::split(line, "\t");
	cout << "# fields = " << fields.size() << endl;

	identifier = stoul(fields[0]);
	// condition = fields[1];
	// action = fields[2];
	prediction = stod(fields[3]);
	error = stod(fields[4]);
	fitness = stod(fields[5]);
}

//! assignment operator for a constant value
xcs_classifier& 
xcs_classifier::operator=(const xcs_classifier& classifier)
{
	set_initial_values();	
	condition = classifier.condition;
	action = classifier.action;
	prediction = classifier.prediction;		//!< prediction 
	error = classifier.error;			//!< prediction error
	fitness = classifier.fitness;			//!< classifier fitness
	actionset_size = classifier.actionset_size;	//!< estimate of the size of the action set [A]
	
	experience = classifier.experience;		//!< classifier experience, i.e., the number of times that the classifier has een updated
	numerosity = classifier.numerosity;		//!< classifier numerosity, i.e., the number of micro classifiers

	return *this;
}
/*!
 * \file bf_env.cc
 *
 * \brief implements the Boolean functions parity
 *
 * \author Pier Luca Lanzi
 *
 * \version 0.01
 *
 * \date 2005/08/01
 *
 */

// #define __DEBUG_ENVIRONMENT__

#include <sstream>
#include <cmath>
#include "xcs_utility.h"
#include "xcs_random.h"

#include "bf_env.h"

using namespace std;

//!< set the init flag to false so that the use of the config manager becomes mandatory
bool	bf_env::init=false;	

bf_env::bf_env(xcs_configuration_manager& xcs_config)
{
	ifstream 	config;
	string 		str_input;
	string		str_function; 
	
	if (!bf_env::init)
	{
		bf_env::init = true;

		if (!xcs_config.exist(tag_name()))
		{
			xcs_utility::error(class_name(), "constructor", "section <" + tag_name() + "> not found", 1);	
		}
		
		xcs_config.save(cerr);

		set_parameters(xcs_config)		;

		no_configurations = 1;
		no_configurations <<= state_size;
	}

	bf_env::init = true;
}

void
bf_env::set_parameters(xcs_configuration_manager& xcs_config)
{
	string str_function;

	try {
		str_function = (string) xcs_config.Value(tag_name(), "function");
	} catch (const char *attribute) {
		xcs_utility::error(class_name(), "set_parameters", "attribute \'function\' not found in <" + tag_name() + ">", 1);
	}

	binary_function = get_binary_function(str_function);
	
	switch (binary_function)
	{
		case MULTIPLEXER_FUNCTION:

			#ifdef __DEBUG_ENVIRONMENT__
			cout << "SETTING MULTIPLEXER PARAMETERS" << endl;
			#endif
			set_parameters_mp(xcs_config);
			break;

		case MAJORITY_ON_FUNCTION:
			set_parameters_majority_on(xcs_config);
			break;

		case EQUALITY_FUNCTION:
			set_parameters_eq(xcs_config);
			break;

		default:
			xcs_utility::error(class_name(), "set_parameters","binary function not supported",1);
	}

	no_configurations = 1;
	no_configurations <<= state_size;

}

void
bf_env::set_parameters_eq(xcs_configuration_manager& xcs_config)
{
	try {
		state_size = (unsigned long) xcs_config.Value(tag_name(), "input size");
	} catch (...) {
		xcs_utility::error(class_name(), "constructor", "attribute \'input size\' not found in <" + tag_name() + ">", 1);
	}
	try {
		no_ones = (unsigned long) xcs_config.Value(tag_name(), "number of ones");
	} catch (...) {
		xcs_utility::error(class_name(), "constructor", "attribute \'number of ones\' not found in <" + tag_name() + ">", 1);
	}
}

void
bf_env::set_parameters_mp(xcs_configuration_manager& xcs_config)
{
	try {
		address_size = (unsigned long) xcs_config.Value(tag_name(), "address size");
	} catch (...) {
		xcs_utility::error(class_name(), "constructor", "attribute \'address size\' not found in <" + tag_name() + ">", 1);
	}

	unsigned long BitStringSize = 1;
	BitStringSize <<= address_size;
	state_size = address_size + BitStringSize;
	state_size = address_size + long(pow(double(2),int(address_size)));

	cout << "STATE SIZE = " << state_size << endl;

}
void
bf_env::set_parameters_majority_on(xcs_configuration_manager& xcs_config)
{
	try {
		state_size = (unsigned long) xcs_config.Value(tag_name(), "input size");
	} catch (...) {
		xcs_utility::error(class_name(), "constructor", "attribute \'input size\' not found in <" + tag_name() + ">", 1);
	}

	threshold = state_size/2;
}

bf_env::t_binary_function 
bf_env::get_binary_function(string str_function)
{
	if (str_function=="multiplexer")
		return MULTIPLEXER_FUNCTION;
	if (str_function=="hidden-parity")
		return HIDDEN_PARITY_FUNCTION;
	if (str_function=="count-ones")
		return COUNT_ONES_FUNCTION;
	if (str_function=="equality")
		return EQUALITY_FUNCTION;
	if (str_function=="majority")
		return MAJORITY_ON_FUNCTION;
}

string
bf_env::print_binary_function(t_binary_function binary_function)
{
	switch (binary_function)
	{
		case MULTIPLEXER_FUNCTION:
			return "multiplexer";

		case MAJORITY_ON_FUNCTION:
			return "majority-on";

		case EQUALITY_FUNCTION:
			return "eq";

		case HIDDEN_PARITY_FUNCTION:
			return "hidden-parity";

		case COUNT_ONES_FUNCTION:
			return "count-ones";

		case CARRY_FUNCTION:
			return "carry";

		default:
			xcs_utility::error(class_name(), "set_parameters","binary function not supported",1);
	}
}

/*!
 * \fn void bf_env::begin_problem(const bool explore)
 * \param explore true if the problem is solved in exploration
 *
 * \brief generates a new input configuration for the Boolean parity
 *
 * If the inputs must be visited uniformly, indicated by uniform_start set to true,
 * the variable current_configuration is used to generate the next available input;
 * otherwise, a random input configuration is generated
 */
void	
bf_env::begin_problem(const bool explore)
{
	string	str = "";

	for(string::size_type bit = 0; bit<state_size; bit++)
	{
		str += '0' + xcs_random::dice(2);
	}

	inputs.set_string_value(str);

	current_reward = 0;

	first_problem = false;
}

bool	
bf_env::stop()
const
{
	return(true); 
}

void	
bf_env::perform(const t_action& action)
{
	// cout << "PERFORMING " << print_binary_function(binary_function) << " ON " << state() << endl;

	switch (binary_function)
	{
		case MULTIPLEXER_FUNCTION:
			perform_mp(action);
			break;

		case EQUALITY_FUNCTION:
			perform_eq(action);
			break;

		case MAJORITY_ON_FUNCTION:
			perform_majority_on(action);
			break;

		case CARRY_FUNCTION:
			perform_carry(action);
			break;

		default:
			xcs_utility::error(class_name(),"perform","function not supported",1);
	}
}

//! only the current reward is traced
void
bf_env::trace(ostream& output) const
{
	output << current_reward;
}

void 
bf_env::reset_input()
{
	current_state = 0;
	inputs.set_string_value(xcs_utility::long2binary(current_state,state_size));
}

bool 
bf_env::next_input()
{
	string	binary;
	bool	valid = false;

	current_state++; 
	if (current_state<no_configurations)
	{
		inputs.set_string_value(xcs_utility::long2binary(current_state,state_size));
		valid = true;
	} else {
		current_state = 0;
		inputs.set_string_value(xcs_utility::long2binary(current_state,state_size));
		valid = false;
	}
	return valid;
}

void
bf_env::save_state(ostream& output) const
{
	output << endl;
	output << current_state << endl;
}

void
bf_env::restore_state(istream& input)
{
	input >> current_state; 
	begin_problem(true);
}

bf_env::bf_env()
{
	if (!bf_env::init)
	{
		xcs_utility::error(class_name(),"class constructor", "not inited", 1);
	} else {
		// nothing to init
	}
}

void	
bf_env::perform_eq(const t_action& action)
{
	string				str_inputs;
	unsigned long		in;
	unsigned long		sum;
	unsigned long		result;
	
	str_inputs = inputs.string_value();

	sum = 0;

	for(in=0; in<inputs.size(); in++)
	{
		sum += str_inputs[in]-'0';
	}

	if (sum==no_ones)
		result = 1;
	else 
		result = 0;

	if (result==action.value())
	{
		current_reward = 1000;
	} else {
		current_reward = 0;
	}
}

void	
bf_env::perform_majority_on(const t_action& action)
{
	string				str_inputs;
	unsigned long		in;
	unsigned long		sum;
	unsigned long		result;
	
	str_inputs = inputs.string_value();

	sum = 0;

	for(in=0; in<inputs.size(); in++)
	{
		sum += str_inputs[in]-'0';
	}

	if (sum>int(str_inputs.size()/2))
	{
		result = 1;
	} else {
		result = 0;
	}

	if (result==action.value())
	{
		current_reward = 1000;
	} else {
		current_reward = 0;
	}
}

void	
bf_env::perform_mp(const t_action& action)
{

	#ifdef __DEBUG_ENVIRONMENT__
	cout << "PERFORM_MP MULTIPLEXER PARAMETERS" << endl;
	#endif

	string str_inputs = inputs.string_value();

	unsigned long		address;
	unsigned long		index;		

	index = xcs_utility::binary2long(str_inputs.substr(0,address_size));
	address = address_size + index;

	if (!flag_layered_reward)
	{
		if ((str_inputs[address]-'0')==action.value())
		{
			current_reward = 1000;
		} else {
			current_reward = 0;
		}
	} else {
		if ((str_inputs[address]-'0')==action.value())
		{
			current_reward = 300 + index*200 + double(100*(unsigned long)(str_inputs[address]-'0'));
		} else {
			current_reward = index*200 + double(100*(unsigned long)(str_inputs[address]-'0'));
		}
	}
#ifdef __DEBUG_ENVIRONMENT__
	cout << "INPUT " << inputs << " BIT " << str_inputs[address] << " ACTION " << action.value() << " REWARD " << current_reward << endl;
#endif
}

void	
bf_env::perform_carry(const t_action& action)
{
	string				str_inputs;
	unsigned long		in;
	unsigned long		sum;
	unsigned long		result;
	
	str_inputs = inputs.string_value();

    unsigned int no_bits = str_inputs.size()/2;

    std::string first = str_inputs.substr(0,no_bits);
    std::string second = str_inputs.substr(no_bits,no_bits);

    int carry = 0;
    int digit = 0;

    for(int i=no_bits-1; i>=0; i--)
    {
        digit = (int(first[i]-'0')+int(second[i]-'0')+carry)%2;
        int updated_carry = (int(first[i]-'0')+int(second[i]-'0')+carry)/2;
        std::cout << first[i] << "+" << second[i] << "+" << carry << " = " << digit << " carry " << updated_carry << std::endl;
        carry = updated_carry;
    }

	if (carry==action.value())
	{
		current_reward = 1000;
	} else {
		current_reward = 0;
	}
}



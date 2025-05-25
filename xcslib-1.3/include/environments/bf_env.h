#ifndef __EQENV__
#define __EQENV__

#include <cassert>
#include "rl_definitions.h"
#include "environment_base.h"
#include "xcs_configuration_manager.h"

using namespace std;

using namespace std;

class bf_env : public virtual environment_base
{
 public:

	typedef enum {
		MULTIPLEXER_FUNCTION, 
		EQUALITY_FUNCTION,
		HIDDEN_PARITY_FUNCTION,
		COUNT_ONES_FUNCTION, 
		MAJORITY_ON_FUNCTION,
		CARRY_FUNCTION
	} t_binary_function;

	string class_name() const { return string("bf_env"); };
	string tag_name() const { return string("environment::binary_function"); };
		
	//! Constructor for the parity class that read the class parameters through the configuration manager
	/*!
	 *  This is the first constructor that must be used. Otherwise an error is returned.
	 */
	bf_env(xcs_configuration_manager&);

	//! Default constructor for the Boolean parity class
	/*!
	 *  If the class parameters have not yet initialized through the configuration manager, 
	 *  an error is returned and the method exists to shell. 
	 *  \sa bf_env(xcs_config_mgr&)
	 *  \sa xcs_config_mgr
	 */
	bf_env();
	
	void begin_experiment() {};
	void end_experiment() {};

	void begin_problem(const bool explore);
	void end_problem() {};

	bool stop() const;
	
	void perform(const t_action& action);

	void trace(ostream& output) const;

	void reset_input();
	bool next_input();

	void save_state(ostream& output) const;
	void restore_state(istream& input);
	
	virtual double reward() const {assert(current_reward==bf_env::current_reward); return current_reward;};

	virtual t_state state() const { return inputs; };

 private:
	/*! \var bool init 
	 *  \brief true if the class parameters have been already initialized
	 */
	static bool			init;

	/*! 
	 * \var t_state inputs
	 * \brief inputs current input configuration
	 */
	t_state			inputs;			// input configuration

	/*! 
	 * \var bool first_problem 
	 * \brief true if the first problem is running
	 */
	bool				first_problem;

	/*! \var unsigned long string_size
	 * \brief number of string bits for the Boolean parity
	 */
	unsigned long			string_size;

	//! selected variables
	vector<unsigned long>		selected_inputs;

	//! true if it the system must visit all the available configuration as a sequence
	bool				uniform_start;		

	//! size of the whole parity string, e.g., 6 for the 6-way parity
	unsigned long			state_size;

	//! the reward returned as a consequence of the last performed action
	double				current_reward;

	//! the value that defines the equality function
	unsigned long			k;

	void set_function(string str_function);
	
 private:
	void set_parameters(xcs_configuration_manager&);

	void set_parameters_eq(xcs_configuration_manager&);	
	void set_parameters_mp(xcs_configuration_manager&);
	void set_parameters_majority_on(xcs_configuration_manager&);
	void set_parameters_count_ones(xcs_configuration_manager&);

	//! read selected variables
	void read_selected_variables(char*, vector<unsigned long>&) const;

	void perform_eq(const t_action& action);	
	void perform_mp(const t_action& action);
	void perform_majority_on(const t_action& action);
	void perform_count_ones(const t_action& action);
	void perform_carry(const t_action& action);

	t_binary_function get_binary_function(string);
	string print_binary_function(t_binary_function);

	t_binary_function binary_function;

	// multiplexer parameters
	unsigned long address_size;
	bool flag_layered_reward = false;

	// eq parameters
	unsigned long no_ones;

	// majority on parameters
	unsigned long threshold;

};
#endif

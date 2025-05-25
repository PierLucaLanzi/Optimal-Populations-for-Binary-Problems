#ifndef __WOODS_ENV__
#define __WOODS_ENV__

#include <sstream>

#include "rl_definitions.h"
#include "environment_base.h"
#include "xcs_configuration_manager.h"

/*!
 * \class woods_env woods_env.h
 * \brief implements the methods specific for woods environments
 * \sa environment_base
 */

class woods_env : public virtual environment_base
{
public:
	string class_name() const { return string("woods_env"); };
	string tag_name() const { return string("environment::woods"); };
	
	//! Constructor for the woods environment class. It reads the class parameters through the configuration manager.
	/*!
	 *  This is the only constructor that can be used. 
	 */
	woods_env(xcs_configuration_manager&);

	//! Destructor for the woods environment class.
	~woods_env();
	
	void begin_problem(const bool explore);
	void end_problem() {};

	void begin_experiment() {};
	void end_experiment() {};

	bool stop() const;
	
	void perform(const t_action& action);

	void trace(ostream& output) const;

	bool allow_test() const {return true;};
	void reset_problem();
	bool next_problem();

	void reset_input();
	bool next_input();

	void save_state(ostream& output) const;
	void restore_state(istream& input);

	void set_parameters(xcs_configuration_manager& xcs_config);
	void print_parameters(ostream&) const;

	//! indicates that woods environments are multiple step problems
	virtual bool single_step() const {return false;};

 public:
	virtual double reward() const {assert(current_reward==woods_env::current_reward); return current_reward;};
	virtual t_state state() const { return inputs; };
	virtual void print(ostream& output) const { output << "(" << current_pos_x << "," << current_pos_y << ")\t" << state();};

 private:
	//! computes the current position on an axis given the grid limits
	inline int	cicle(const int op, const int limit) const;

	//! computes the sensory inputs that are returned in position <x,y>
	void		get_input(const unsigned long x, const unsigned long y, t_state& sensors) const;

	//! given the current <x,y> position sets the current input and the current reward \sa current_position_x \sa current_position_y
	inline void	set_state();

	//! return true if the position <x,y> is free (i.e., it contains ".")
	inline bool	is_free(const unsigned long x, const unsigned long y) const;

	//! return true if the position <x,y> contains food (i.e., "F")
	inline bool	is_food(const unsigned long x, const unsigned long y) const;

	//! return true if the position <x,y> contains food (i.e., "F")
	void binary_encode(const string&, string&) const;
	void binary_encode_woods1(const string&, string&) const;
	void binary_encode_woods2(const string &input, string &binary) const;


	//! reads the map
	void read_map(string filename);
	
	static bool		init;			//!< true if the class has been inited through the configuration manager
	t_state			inputs;			//!< current input configuration

        //! true if the start position in the environment are set so to visit all the positions the same number of time
	bool		uniform_start;
	
        //! current reward returned for the last action performed
	double		current_reward;
	//unsigned long	no_configurations;			// #configurazioni possibili

	//! \var current_configuration index of the current agent's input
	/*!
	 * it is used when scanning all the possible environment configurations with 
	 * the functions \fn reset_input and \fn next_input
	 * \sa reset_input
	 * \sa next_input
	 */
	////unsigned long	current_configuration;			// counter for uniform problem start

	string map_filename;

	//! \var current_state current agent's input
	//unsigned long	current_state;				// counter for the scan of states

	//! \var configurations stores all the possible input configurations
	vector<string>	configurations;				// configurations

        //! current x position in the environment
	unsigned long 	current_pos_x;
        //! current y position in the environment
	unsigned long 	current_pos_y;

 	//! \var flag_binary_sensors specifies whether binary inputs (i.e., 00100...) or symbolic inputs (i.e., "TF......T.") should be returned
	bool		flag_binary_sensors;

        //! \var prob_slide specifies the probability that the agent can slip while it moves (Colombetti and Lanzi 1999)
	double		prob_slide; 

        //! \var env_rows number of rows
	unsigned long		env_rows;

        //! \var env_columns number of columns
	unsigned long		env_columns;

        //! \var map environment map 
	vector<string>		map;

        //! \var env_free_pos number of free positions (i.e., ".") in the environment
	unsigned long 		env_free_pos;

        //! x coordinates of the free positions in the environment
	vector<unsigned long>	free_pos_x;
	
        //! y coordinates of the free positions in the environment
	vector<unsigned long>	free_pos_y;

	//! path traces the path the agent followed during the problem
	string			path;

	//!  supported configuration parameters
	const static std::vector<std::string> configuration_parameters;

	//! true if the woods2 environment is used
	bool flag_use_woods2_symbols;
};
#endif

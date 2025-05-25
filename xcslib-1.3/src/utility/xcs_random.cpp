#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <random>
#include "xcs_random.h"

using namespace std;

//! \var unsigned long xcs_random::seed is seed for random number generator.
unsigned long	xcs_random::seed = 0;
std::random_device xcs_random::rd;
std::mt19937_64 xcs_random::generator;
std::uniform_real_distribution<> xcs_random::uniform_distribution{0.0,1.0};
std::normal_distribution<> xcs_random::normal_distribution;
const std::vector<std::string> xcs_random::configuration_parameters = {"seed"};

xcs_random::xcs_random()
{
	generator.seed(rd());
}

/*! 
 * \fn void xcs_random::set_seed(xcs_config_mgr& xcs_config)
 * \param xcs_config represents the configuration manager currently in use.
 *
 * \brief set the seed for random number generation through the configuration manager.
 */
void
xcs_random::set_seed(xcs_configuration_manager& xcs_config)
{

	//! look for the init section in the configuration file
	if (!xcs_config.exist(tag_name()))
	{
		xcs_utility::error(class_name(), "constructor", "section <" + tag_name() + "> not found", 1);	
	}
	
	xcs_config.check_parameters(tag_name(), configuration_parameters);

	xcs_config.save(cerr);
	
	try {
		seed = xcs_config.Value(tag_name(), "seed");
	} catch (const char *attribute) {
		string msg = "attribute \'" + string(attribute) + "\' not found in <" + tag_name() + ">";
		xcs_utility::error(class_name(), "constructor", msg, 1);
	}

	if (xcs_random::seed!=0)
	{
		xcs_random::set_seed(xcs_random::seed);
	} else {
		//! use clock to set the seed
		generator.seed(rd());
	}
}

/*! 
 * \fn float xcs_random::random()
 *
 * \brief generate a real random number between 0 and 1.
 */
double 
xcs_random::random()
{
	return (double) uniform_distribution(generator);
}

void 
xcs_random::set_seed(long new_seed)
{
	xcs_random::seed = new_seed;
 	generator.seed(xcs_random::seed);	
}

/*! 
 * \fn unsigned int xcs_random::dice(const unsigned int limit)
 *
 * \brief generate an integer random number between 0 and limit - 1.
 */
unsigned int 
xcs_random::dice(const unsigned int limit)
{
	return ((unsigned int)((xcs_random::random()*float(limit))));
}

/*!
 * \fn double nrandom()
 *
 * \brief returns a floating-point random number generated according to a normal distribution with mean 0 and standard deviation 1
 */

double
xcs_random::nrandom()
{
	return xcs_random::normal_distribution(generator);
}

//! returns a random sign
int
xcs_random::sign()
{
	if (xcs_random::random()<0.5)
		return -1;
	else
		return 1;
}

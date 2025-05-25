#include "xcs_definitions.h" 
#include "experiment_mgr.h"

t_environment		*environment;
t_classifier_system	*xcs;

experiment_mgr* experiment;
bool flag_verbose = false;		//! if true verbose output is printed
bool flag_print = false;

/*! 
 * \fn int main(int Argc, char *Argv[])
 * \param argc number of arguments
 * \param argv list of arguments
 * 
 * main code to exploit the XCS implementation
 */
int
main(int argc, char *argv[])
{
	string	str_suffix = ""; 		//! configuration file suffix
	int		o;							//! current option
	
	if (argc==1)
	{
		cerr << "USAGE:\t\t" << argv[0] << "\t" << "-f <suffix> [-v] [-s <set>] " << endl;
		cerr << "      \t\t\t\t" << "<suffix>     suffix for the configuration file" << endl;
		cerr << "      \t\t\t\t" << "-v           verbose output" << endl;
		cerr << "      \t\t\t\t" << "-h           print version" << endl;
		return 0;
	}		


	while ( (o = getopt(argc, argv, "hpvf:")) != -1 )
	{
		switch (o)
		{
			case 'v':
				flag_verbose = true;
				break;

			case 'p':
				flag_print = true;
				break;

			case 'f':
				str_suffix = string(optarg);
				break;
			case 'h':
				//! print version
				cerr << "XCSLIB\tVERSION " << __XCSLIB_VERSION__ << endl;
				cerr << "      \tBUILT   " << __DATE__ << endl;
				cerr << "      \tTIME    " << __TIME__ << endl;
				cerr << endl;
				cerr << "      \tSTATE       " << __INPUTS_VERSION__ << endl;
				cerr << "      \tACTION      " << __ACTION_VERSION__ << endl;
				cerr << "      \tCONDITIIONS " << __CONDITION_VERSION__ << endl;
				cerr << endl << endl;
				return 0;
				break;	
			default:
				string msg = "unrecognized option" + string(optarg);
				xcs_utility::error("main","main",msg,0);
		}
	}
	
	if (flag_print)
	{
		cout << "XCS - Configuration" << endl;
		cout << "================================================================================" << endl;
	}

	//! system configuration begins
	string suffix(str_suffix);
	
	//! init the configuration manager
	xcs_configuration_manager	xcs_config(suffix);
	
	if (flag_verbose)
		clog << "Configuration Manager\t\tok." << endl;

	//! init random the number generator
	xcs_random::set_seed(xcs_config);
	if (flag_verbose) 
		clog << "Random numbers         \t\tok." << endl;

	//! init the action class
	t_action		dummy_action(xcs_config);
	if (flag_verbose) 
	{
		clog << "Actions                \t\tok." << endl;
	}

	//! init the environment
	environment = new t_environment(xcs_config);
	if (flag_verbose)
		clog << "Environment            \t\tok." << endl << endl;

	//! init the condition class
	t_condition		condition(xcs_config);
	if (flag_verbose) 
		clog << "Conditions             \t\tok." << endl << endl;

	//! init the XCS classifier system
	xcs = new t_classifier_system(xcs_config, environment);

	if (flag_verbose)
		clog << "Classifier System      \t\tok." << endl;

	if (flag_print) 
	{
		xcs->print_parameters(cout);
		cout << endl << endl;
	}

	//! init the experiment manager
	experiment = new experiment_mgr(xcs_config, xcs, environment);
	if (flag_verbose) 
		clog << "Session Manager        \t\tok." << endl;
	if (flag_print)
	{
		experiment->print_parameters(cout);
		cout << endl << endl;
	}

	// it if asks just for print
	if (flag_print)
	{
		cout << "================================================================================" << endl << endl;
		return 0;
	} else {

		//! the experiment session begins
		if (flag_verbose) 
			cout << "Begin Experiments...\n" << endl;
		experiment->perform_experiments();
		if (flag_verbose) 
			cout << "End Experiments...\n" << endl;
	}

}

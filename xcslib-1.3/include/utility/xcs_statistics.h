/*!
 * \class xcs_statistics
 * \brief collects various XCS statistics
 */

#include <istream>
#include <ostream>

using namespace std;

#ifndef __XCS_STATISTICS__
#define __XCS_STATISTICS__

class xcs_statistics
{
	public:
		double	average_prediction;			//! average prediction in [P]
		double	average_fitness;			//! average fitness in [P]
		double	average_error;				//! average prediction error in [P]
		double	average_actionset_size;		//! average size of [A]
		double	average_experience;			//! average experience
		double	average_numerosity;			//! average numerosity
		double	average_time_stamp;			//! average time stampe
		double	average_no_updates;			//! average number of classifier updates
		double	system_error;				//! system error in [P]

		unsigned long no_macroclassifiers;	//! number of macroclassifiers in [P]
		unsigned long no_ga;				//! number of GA activations
		unsigned long no_cover;				//! number of covering activations 
		unsigned long no_subsumption;		//! number of subsumption activations

		//! class constructor; it invokes the reset method \sa reset
		xcs_statistics();

		//! reset all the collected statistics
		void reset();

		//! read the statistics from an output stream
		friend istream& operator>>(istream&, xcs_statistics&);

		//! write the statistics to an output stream
		friend ostream& operator<<(ostream&, const xcs_statistics&);
};	

#endif
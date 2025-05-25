#include "xcs_statistics.h"

//! reset all the collected statistics
void
xcs_statistics::reset()
{
	average_prediction = 0;
	average_fitness = 0;
	average_error = 0;
	average_actionset_size = 0;
	average_experience = 0;
	average_numerosity = 0;
	average_time_stamp = 0;
	average_no_updates = 0;	
	system_error = 0;

	no_macroclassifiers = 0;
	no_ga = 0;
	no_cover = 0;
	no_subsumption = 0;
}

//! class constructor; it invokes the reset method \sa reset
xcs_statistics::xcs_statistics()
{
	reset();
}

ostream& 
operator<<(ostream& output, const xcs_statistics& stats)
{
	output << stats.average_prediction << "\t";
	output << stats.average_fitness << "\t";
	output << stats.average_error << "\t";
	output << stats.average_actionset_size << "\t";
	output << stats.average_experience << "\t";
	output << stats.average_numerosity << "\t";
	output << stats.average_time_stamp << "\t";
	output << stats.average_no_updates << "\t";
	output << stats.system_error << "\t";

	output << stats.no_macroclassifiers << "\t";
	output << stats.no_ga << "\t";
	output << stats.no_cover << "\t";
	output << stats.no_subsumption << "\t";
	return (output);
}

istream& 
operator>>(istream& input, xcs_statistics& stats)
{
	input >> stats.average_prediction;
	input >> stats.average_fitness;
	input >> stats.average_error;
	input >> stats.average_actionset_size;
	input >> stats.average_experience;
	input >> stats.average_numerosity;
	input >> stats.average_time_stamp;
	input >> stats.average_no_updates;
	input >> stats.system_error;

	input >> stats.no_macroclassifiers;
	input >> stats.no_ga;
	input >> stats.no_cover;
	input >> stats.no_subsumption;
	return (input);
}
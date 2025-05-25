//-------------------------------------------------------------------------
// Filename      : binary_action.hh
//
// Purpose       : class that implements binary actions
//                 
// Special Notes : 
//
// Creator       : Pier Luca Lanzi
//
// Creation Date : 2002/05/27
//
// Last Change   : 
//
// Current Owner : Pier Luca Lanzi
//-------------------------------------------------------------------------

/*! 
 * \file 	binary_action.hpp
 * \brief 	class for binary actions
 * \author 	Pier Luca Lanzi
 * \version 	1.0
 * \date 	2008/08/06
 */
 
#include "action_base.h"
#include "xcs_utility.h"
#include "xcs_configuration_manager.h"

#ifndef __BINARY_ACTION__
#define __BINARY_ACTION__

#define __ACTION_VERSION__ "binary string (class binary_action)"

using namespace std;

class binary_action : public virtual action_base<binary_action>
{
		
private:
	static bool				init;
	static unsigned long	no_actions;
	static unsigned long	no_bits;
	string					bitstring;

public:
	string class_name() const { return string("binary_action"); };
	string tag_name() const { return string("action::binary"); };

	binary_action();
	binary_action(int);
	binary_action(xcs_configuration_manager&);

	~binary_action();
	

	unsigned long actions() const;

	void set_value(unsigned long);
	string string_value() const;
	void set_string_value(string);

	void random();
	void mutate(const double&); 
};

#endif

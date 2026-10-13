// This file is part of MAGE, and is therefore released under the LGPL.  //
// Permission is given for its release under the MAME license, in the    //
// case of MAME and its derivatives.Any fixes for this driver should be  //
// forwarded to MAGENET (www.magesrc.sf.net/index.php)					 //

#ifndef INC_STEPPERS
#define INC_STEPPERS

#define MAX_STEPPERS		  16 // maximum number of steppers

#define STEPPER_48STEP_REEL    0 // STARPOINT RM   reel unit
#define STEPPER_144STEPS_DICE  1 // STARPOINT 1DCU DICE mechanism - tech sheet available from Re-A or EC on request

void Stepper_init(  int id, int type);		// init a stepper motor

void Stepper_reset_position(int id);		// reset a motor to position 0

int  Stepper_optic_state(   int id);		// read a motor's optics

int  Stepper_update(int id, UINT8 pattern);	// update a motor

int  Stepper_get_position(int id);		    // get current position in half steps

#endif

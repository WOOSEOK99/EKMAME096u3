/*This file is part of MAGE, and is therefore released under the LGPL. Permission
is given for its release under the MAME license, in the case of MAME and its derivatives.
Any fixes for the games contained in this driver should be forwarded to MAGENET
(www.magesrc.sf.net/index.php)*/

#ifndef INC_MMTR
#define INC_MMTR

#define MAXMECHMETERS 8

#define METERREACTTIME 30000L	  // number of cycle meter has to be active to tick


void Mechmtr_init(     int number);

int  MechMtr_GetNumberMeters(void);

void MechMtr_Setcount( int id, long count);

long MechMtr_Getcount( int id);

void MechMtr_ReactTime(int id, long cycles);

int  Mechmtr_update(   int id, long cycles, int state); // returns 1 if meter ticked

#endif

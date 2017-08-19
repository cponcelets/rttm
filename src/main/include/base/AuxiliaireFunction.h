//
//  AuxiliaireFunction.h
//  timedtracechecker
//
//  Created by Clement Poncelet on 16/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#ifndef offline_trace_manager_AuxiliaireFunction_h
#define offline_trace_manager_AuxiliaireFunction_h

//a minute into seconds
#define MINUTE 60.0
//buffer length for parsers
#define MAX_SIZE 1024

//translation functions
/*** relative time into seconds:
  double -> double -> double
@arg[in] relative, duration in relative time
@arg[in] tempo, current tempo
@arg[out] duration in seconds
***/
double R_to_S(double relative, double tempo);

/*** seconds into relative time:
  double -> double -> double
@arg[in] absolute, duration in seconds
@arg[in] tempo, current tempo
@arg[out] duration in relative time
***/
double S_to_R(double absolute, double tempo);

#endif

//
//  AuxiliaireFunction.cpp
//  timedtracechecker
//
//  Created by Clement Poncelet on 16/05/2015.
//  Copyright (c) 2015 Clement Poncelet. All rights reserved.
//

#include <base/AuxiliaireFunction.h>

double R_to_S(double relative, double tempo)
{
    double res = relative / (tempo/MINUTE);
    return res;
}

double S_to_R(double absolute, double tempo)
{
    return absolute * (tempo/MINUTE);
}

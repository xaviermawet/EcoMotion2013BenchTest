#ifndef __POLIFITGSL_H__
#define __POLIFITGSL_H__

#include <gsl/gsl_multifit.h> // GSL : GNU Scientific Library
#include <stdbool.h>
#include <math.h>

#if QT_VERSION >= 0x050000 //  0xMMNNPP (MM = major, NN = minor, PP = patch)
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

bool polynomialfit(int obs, int degree, double *dx, double *dy, double *store); /* n, p */

#endif /* __POLIFITGSL_H__ */

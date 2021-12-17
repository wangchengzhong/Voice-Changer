#pragma once
//====================================================== file = autoc.c =====
//=  Program to compute autocorrelation for a series X of size N            =
//===========================================================================
//=  Notes:                                                                 =
//=    1) Input from input file "in.dat" to stdin (see example below)       =
//=        * Comments are bounded by "&" characters at the beginning and    =
//=          end of the comment block                                       =
//=    2) Output is to stdout                                               =
//=    3) Must manually set NUM_LAG                                         =
//=    4) Computes an unbiased ACF, see equation (7) in:                    =
//=         http://www.ltrr.arizona.edu/~dmeko/notes_3.pdf                  =
//=-------------------------------------------------------------------------=
//= Example "in.dat" file:                                                  =
//=                                                                         =
//=    & Sample series of data which can be integers or reals.              =
//=      There are 11 values in this file. &                                =
//=    50                                                                   =
//=    42                                                                   =
//=    48                                                                   =
//=    61                                                                   =
//=    60                                                                   =
//=    53                                                                   =
//=    39                                                                   =
//=    54                                                                   =
//=    42                                                                   =
//=    59                                                                   =
//=    53                                                                   =

#include <stdio.h>                 // Needed for printf() and feof()
#include <math.h>                  // Needed for pow()?
#include <stdlib.h>                // Needed for exit() and atof()
#include <string.h>                // Needed for strcmp()

//----- Defines -------------------------------------------------------------
#define MAX_SIZE 10000000          // Maximum size of time series data array
#define NUM_LAG       100          // Number of lags to compute for

//----- Globals -------------------------------------------------------------
double* X;                      // Time series read from "in.dat"
long int   N;                      // Number of values in "in.dat"
double     Mean;                   // Mean of series X
double     Variance;               // Variance of series X

//----- Function prototypes -------------------------------------------------
void   load_X_array(void);         // Load X array
double compute_mean(void);         // Compute Mean for X
double compute_variance(void);     // Compute Variance for X
double compute_autoc(int lag);     // Compute autocorrelation for X



//===========================================================================
//=  Function to load X array from stdin and determine N                    =
//===========================================================================


//===========================================================================
//=  Function to compute mean for a series X                                =
//===========================================================================
double compute_mean(void)
{
    double   mean;        // Computed mean value to be returned
    int      i;           // Loop counter

    // Loop to compute mean
    mean = 0.0;
    for (i = 0; i < N; i++)
        mean = mean + (X[i] / N);

    return(mean);
}

//===========================================================================
//=  Function to compute variance for a series X                            =
//===========================================================================
double compute_variance(void)
{
    double   var;         // Computed variance value to be returned
    int      i;           // Loop counter

    // Loop to compute variance
    var = 0.0;
    for (i = 0; i < N; i++)
        var = var + (pow((X[i] - Mean), 2.0) / N);

    return(var);
}

//===========================================================================
//=  Function to compute autocorrelation for a series X                     =
//=   - Corrected divide by N to divide (N - lag) from Tobias Mueller       =
//===========================================================================
double compute_autoc(int lag)
{
    double   autocv;      // Autocovariance value
    double   ac_value;    // Computed autocorrelation value to be returned
    int      i;           // Loop counter

    // Loop to compute autovariance
    autocv = 0.0;
    for (i = 0; i < (N - lag); i++)
        autocv = autocv + ((X[i] - Mean) * (X[i + lag] - Mean));
    autocv = (1.0 / (N - lag)) * autocv;

    // Autocorrelation is autocovariance divided by variance
    ac_value = autocv / Variance;

    return(ac_value);
}
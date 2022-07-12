// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#include "MeanAnomalyToTrueAnomaly.h"
// All we use is Pi, so....
#include "OrbitalMechanics.h"

double EccAnom(double ec, double m, int dp);
double TrueAnom(double ec, double E, int dp);

/*
*   Given a Mean Anomaly and conic elements, Compute a True Anomaly
*   There is no analytic solution to this problem, it must be solved
*   by numerical algorithms.
*
*   Inspiration for this implementation:
*   http://www.jgiesen.de/kepler/kepler.html
*/
double MeanAnomalyToTrueAnomaly(double meanAnomaly, double eccentricity, int decimalPlaces)
{
    double E = EccAnom(eccentricity, meanAnomaly, decimalPlaces);
    return TrueAnom(eccentricity, E, decimalPlaces);
}

double EccAnom(double ec, double m, int dp)
{

    // arguments:
    // ec=eccentricity, m=mean anomaly,
    // dp=number of decimal places
    double K = pi<double> / 180.0;

    int maxIter = 30, i = 0;

    double delta = pow(10, -dp);

    double E, F;

    m = m / 360.0;

    m = 2.0 * pi<double> * (m - floor(m));

    if (ec < 0.8) E = m; else E = pi<double>;

    F = E - ec * sin(m) - m;

    while ((abs(F) > delta) && (i < maxIter)) {

        E = E - F / (1.0 - ec * cos(E));
        F = E - ec * sin(E) - m;

        i = i + 1;

    }

    E = E / K;

    return round(E * pow(10, dp)) / pow(10, dp);

}

double TrueAnom(double ec, double E, int dp) {
    double K = pi<double> / 180.0;

    E *= K;

    double S = sin(E);
    double C = cos(E);

    double fak = sqrt(1.0 - ec * ec);

    double phi = atan2(fak * S, C - ec) / K;

    return round(phi * pow(10, dp)) / pow(10, dp);
}

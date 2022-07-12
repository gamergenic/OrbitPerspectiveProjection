// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once

/*
*   Given a Mean Anomaly and conic elements, Compute a True Anomaly
*   There is no analytic solution to this problem, it must be solved
*   by numerical algorithms.
* 
*   Inspiration for this implementation:
*   http://www.jgiesen.de/kepler/kepler.html
*/

ORBITALPHYSICS_API double MeanAnomalyToTrueAnomaly(double meanAnomaly, double eccentricity, int decimalPlaces = 8);

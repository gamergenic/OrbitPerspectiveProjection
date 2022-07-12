// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

//-----------------------------------------------------------------------------
// ClipEllipseToFrustum
// Implmenentation of algoritm that clips an ellipse to a rectangle.
// Although it is defined as a template, it is unlikley to compile for types
// that are not used in this project (exclusively double types).
// The implementation is not designed to be robust, is intended for
// demonstration purposes only - not intended to be used in production.
// This algorithm has not been optimized (performance, memory, readability, etc)
// See ReadMe.txt for more information.
//-----------------------------------------------------------------------------

#pragma once

#include "GTE/Mathematics/Math.h"
#include "GTE/Mathematics/Vector.h"
#include "GTE/Mathematics/Vector3.h"
#include "GTE/Mathematics/Matrix3x3.h"
#include "GTE/Mathematics/Matrix.h"

#include <cmath>
#include <utility>
#include <vector>
#include <algorithm>
using std::pair;
using std::vector;

using namespace gte;


/*
*   Clip a screen-space projected ellipse to the viewable area
*/

// Intercept:
//  a*cos(t) + b*sin(t) + c == d
template<class T>
bool ClipEllipse(T a, T b, T c, T d, bool& interceptsFustumLine, T& theta1, T& dt1, T& theta2, T& dt2)
{
    // https://www.mathcentre.ac.uk/resources/uploaded/mc-ty-rcostheta-alpha-2009-1.pdf
    T R = (T)(sqrt(sqr(a) + sqr(b)));
    T act = (d - c) / R;

    // Is there an intercept?  Welp, can we take the arcos of act?
    if (act <= (T)1. && act >= (T)-1.)
    {
        // Yes, there's an intercept!  Two, in fact!

        // Compute the phase shift
        T alpha = (T)atan2((double)b, (double)a);

        T arcos = (T)acos(act);
        theta1 = alpha + arcos;
        theta2 = alpha - arcos;

        dt1 = -R * sin(arcos);
        dt2 = +R * sin(arcos);
        interceptsFustumLine = true;
        return true;
    }
    else
    {
        interceptsFustumLine = false;
        return d * act > 0;
    }

    return false;
}


// SegmentList - X < Y for non-wrap around segments
//               X > Y if the segment wraps around 0   
template<class T>
bool ClipEllipseToFrustum(
    const FrustumParameters<T>& frustum,
    const ConicProjection<T>& projection,
    vector<ConicSegment<T>>& SegmentList
)
{
    // Inputs, unpack...
    T z = frustum.z;
    T fov = frustum.fov;
    T aspectRatio = frustum.aspectRatio;
    Vector<2, T> k = projection.k;
    Vector<2, T> v1 = projection.u;
    Vector<2, T> v2 = projection.v;
    T a1 = projection.a;
    T a2 = projection.b;
#if defined(REDUCE_FOV)
    fov *= 0.75f;
#endif

    struct ClipChange
    {
        int     Whom;
        bool    HiddenChange;
        T       When;
        int     HiddenState;
        ClipChange(int whom, bool hidden, T when)
        {
            Whom = whom;
            HiddenChange = hidden;
            When = normalizeRadians0toTwoPi(when);
            HiddenState = 0;
        }
    };

    vector<ClipChange> clipPoints;
    SegmentList.clear();

    T maxX = z * tan(fov / 2);
    T maxY = maxX / aspectRatio;
    T a, b, c;
    T theta1, dt1;
    T theta2, dt2;
    bool interceptFound;

    // Right
    a = a1 * v1[0]; b = a2 * v2[0]; c = k[0];
    if (ClipEllipse(a, b, c, (T)+maxX, interceptFound, theta1, dt1, theta2, dt2))
    {
        if (interceptFound)
        {
            //        UE_LOG(LogTemp, Warning, TEXT("X+ Clip at th = %f (%f)/ %f (%f)"), theta1, dt1, theta2, dt2);
            bool hidden = dt1 >= 0;
            clipPoints.push_back(ClipChange(0, hidden, theta1));
            clipPoints.push_back(ClipChange(0, !hidden, theta2));
        }
    }
    else
    {
        // Outside of frustum... early out!
        return false;
    }

    // Bottom
    a = a1 * v1[1]; b = a2 * v2[1]; c = k[1];
    if (ClipEllipse(a, b, c, (T)-maxY, interceptFound, theta2, dt2, theta1, dt1))
    {
        if (interceptFound)
        {
            //        UE_LOG(LogTemp, Warning, TEXT("Y+ Clip at th = %f (%f)/ %f (%f)"), theta1, dt1, theta2, dt2);
            bool hidden = dt2 >= 0;

            clipPoints.push_back(ClipChange(1, hidden, theta1));
            clipPoints.push_back(ClipChange(1, !hidden, theta2));
        }
    }
    else
    {
        // Outside of frustum... early out!
        return false;
    }

    // Left
    a = a1 * v1[0]; b = a2 * v2[0]; c = k[0];
    if (ClipEllipse(a, b, c, (T)-maxX, interceptFound, theta2, dt2, theta1, dt1))
    {
        if (interceptFound)
        {
            //        UE_LOG(LogTemp, Warning, TEXT("X- Clip at th = %f (%f)/ %f (%f)"), theta1, dt1, theta2, dt2);
            bool hidden = dt2 >= 0;

            clipPoints.push_back(ClipChange(2, hidden, theta1));
            clipPoints.push_back(ClipChange(2, !hidden, theta2));
        }
    }
    else
    {
        // Outside of frustum... early out!
        return false;
    }

    // Top
    a = a1 * v1[1]; b = a2 * v2[1]; c = k[1];
    if (ClipEllipse(a, b, c, (T)+maxY, interceptFound, theta1, dt1, theta2, dt2))
    {
        if (interceptFound)
        {
            //        UE_LOG(LogTemp, Warning, TEXT("Y- Clip at th = %f (%f)/ %f (%f)"), theta1, dt1, theta2, dt2);
            bool hidden = dt1 >= 0;

            clipPoints.push_back(ClipChange(3, hidden, theta1));
            clipPoints.push_back(ClipChange(3, !hidden, theta2));
        }
    }
    else
    {
        // Outside of frustum... early out!
        return false;
    }

    // If we survived here, the ellipse is visible in at least one place...
    // No clip points means it never crosses a frustum line/plane, so it must be fully visible
    bool allVisible = clipPoints.empty();

    if (allVisible)
    {
        return true;
    }
    else
    {
        // The ellipse is hidden in at least one place...
        // It will hidden anytime it's crossed 1 or more clip line.
        // So, for each clip line noted, remove all states (via removing associated state
        // changes) where that clip segment becomes hidden.

        // We're effectively ref counting the segment 'hides'.
        // Segments are visible anywhere there's zero hidden segments on the ref count.

        // Start, by sorting the state changes by time...
        std::sort(
            clipPoints.begin(), clipPoints.end(),
            [](const ClipChange& a, const ClipChange& b)
            {
                return a.When < b.When;
            });

        // Loop twice through the list to wrap around the states...
        int state = 0;
        for (int _rawindex = 0; _rawindex < 2 * clipPoints.size(); ++_rawindex)
        {
            int index = _rawindex % clipPoints.size();

            // We can't early out if the state is non-zero, because there may be
            // outstanding ref's to tally...

            // If this is the start of a segment, clear the hidden flag
            bool segmentIsHidden = clipPoints[index].HiddenChange;
            int whom = clipPoints[index].Whom;
            int bitFlag = 1 << whom;

            if (segmentIsHidden)
            {
                state |= bitFlag;
            }
            else
            {
                state &= (-1) ^ bitFlag;
            }

            clipPoints[index].HiddenState = state;
        }

        // loop through twice again
        // It would be moar elegant, logically, if these loops were combined.
        // ... but its still only O(n) and this isn't an inner loop.
        bool wasVisible = false;
        T segmentStart = 0;
        for (int _rawindex = 0; _rawindex < 2 * clipPoints.size(); ++_rawindex)
        {
            int index = _rawindex % clipPoints.size();
            int hiddenState = clipPoints[index].HiddenState;

            // We can early out if this one's already been done...
            if (hiddenState == -1) break;

            bool visible = hiddenState == 0;
            if (!wasVisible && visible)
            {
                segmentStart = clipPoints[index].When;
            }
            else if (wasVisible && !visible)
            {
                T segmentEnd = clipPoints[index].When;
                T segmentLength = segmentEnd - segmentStart;
                if (segmentLength < 0) segmentLength += twopi<T>;
                if (segmentStart < 0) segmentStart += twopi<T>;
                SegmentList.push_back(ConicSegment<T>(normalizeRadians0toTwoPi<T>(segmentStart), segmentLength));
            }

            // Mark the state so we don't reconsider it... If, it was segmentized
            if (visible)
            {
                clipPoints[index].HiddenState = -1;
            }

            wasVisible = visible;
        }

        return clipPoints.size() > 0;
    }
}

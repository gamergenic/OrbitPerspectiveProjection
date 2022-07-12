// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

//-----------------------------------------------------------------------------
// ClipHyperbolaToFrustum
// Implmenentation of algoritm that clips a 2D Hyperbola against a rectangle.
// This is necessary not only for hyperbolic orbits, but for elliptical orbits
// as the perspective projection of an ellipse turns hyperbolic when the
// ellipse intercepts the eye plane.
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
using namespace gte;

#include <cmath>
#include <vector>
#include <algorithm>
using std::vector;

#include "ProjectEllipseToPlane.h"



// Hyperbola:
//      sqr(x)/sqr(a) - sqr(y)/sqr(b) = 1
// 
// Line:
//      A*x + B*y = C
//
// find the intersection... And discard cases where X < 0
template<class T>
int BisectHyperbola(T A, T B, T C, bool positiveOrientation, T a, T b, bool& initiallyHidden, T(&points)[2])
{
    double sign = positiveOrientation ? +1. : -1;

    // The intersection is solved by a simple quadratic equation....
    // (Easily derived by substititing the line equation into the hyperbola eq)
    T A_quad = sqr(B) - sqr(a * A / b);
    T B_quad = 2 * B * C;
    T C_quad = sqr(C) - sqr(a * A);

    T d = sqr(B_quad) - 4. * A_quad * C_quad;

    int n = 0;

    const Vector<2, T> lineNormal({ A, B });

    // First Asymptote
    const Vector<2, T> asymptote({ sign * a, -sign * b });

    // Does the first asymptote cross INTO the hidden side?  If so, it starts off hidden
    // By definition, the lines have a positive cross product in this case...
    // Which means their normal vector points towards the hidden side...
    // Which means the dot product of it and another line will be positive if it's heading off to the hidden side
    initiallyHidden = Dot(lineNormal, asymptote) > 0;

    // if the discriminant is > 0, there's two real roots
    if (d > 0)
    {
        // But only one of them is against the x>0 half of the hyperbola we care about...
        T x, y;

        // (Wash)
        // Solve the quatratic, using the '-' discriminant
        T q = (-B_quad - sqrt(d)) / (2. * A_quad);

        // what does the quadratic yield?  the point intersection point (x(y), y).
        y = q;
        // knowing y, we can resolve z...
        x = (-B * y - C) / A;

        // (Rinse)
        // We only care about the hyperbola in the same side as (x,y)...
        // Any -sign*x < 0 intersections are on the mirrored hyperbola, which is of no interest
        // to us in this use case.
        if (sign * x >= 0)
            // Intersetction at: {x, y}
            points[n++] = atanh(y / x * a / b);

        // (Repeat.)
        // using the '+' discriminant
        q = (-B_quad + sqrt(d)) / (2. * A_quad);
        y = q;
        x = (-B * y - C) / A;

        if (sign * x >= 0)
            points[n++] = atanh(y / x * a / b);
    }
    // vvv -------------------
    // Included for clarity.
    // (The compiler is smart enough to optimize the branches away, ... Except for
    // unoptimized builds, which means you may want to set a breakpoint here anyways
    // since you're likely debugging.)
    else if (d == 0)
    {
        // This case only happens if the intersection is a tangent...
        // In the case of a tanget, a hyperbola never crosses the line...
        // So, we don't care about this case.... as the hyperbola will never
        // need clipped here.
    }
    else if (d < 0)
    {
        // There are no real roots, to the quadratic, which means the line does
        // not intersect the hyperbola at all.
    }
    // ^^^ -------------------

    return n;
}


template<class T>
void ClipHyperbolaToLine(
    const pair<Vector<2, T>, Vector<2, T>>& init,
    const Vector<2, T>& screenDimentions,
    const Matrix<2, 2, T>& R,
    bool positiveOrientation,
    const Vector<2, T>& k,
    T a, T b,
    bool& initialState,
    T (&stateChanges)[2],
    int& nStateChangeCount
)
{
    // Transform the line from screen space into the hyperbola's space...
    const Vector<2, T >& direction = init.first;
    const Vector<2, T >& center = init.second * screenDimentions;

    const Vector<2, T> direction_prime = R * direction;
    const Vector<2, T> center_prime = R * (center - k);

    // Get linear coefficients of the line in the form:
    // Ax + By + C = 0
    double A, B, C;
    A = -direction_prime[1];
    B = direction_prime[0];
    C = -A * center_prime[0] - B * center_prime[1];

    // Now the line is position in they hyperbola's space, so the intersection can be solved
    // via an intersection against a "standard" form hyperbola: x^2/a^2 - y^2/b^2 == 1
    nStateChangeCount = BisectHyperbola(A, B, C, positiveOrientation, a, b, initialState, stateChanges);
}




template<class T>
void ClipHyperbolaToFrustum(
    const FrustumParameters<T>& frustum,
    const ConicProjection<T>& projection,
    bool positiveOrientation,
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
    T a = projection.a;
    T b = projection.b;

#if defined(REDUCE_FOV)
    fov *= 0.75f;
#endif

    T maxX = z * tan(fov / 2);
    T maxY = maxX / aspectRatio;
    const Vector<2, T> screenDimentions({ maxX, maxY });

    // { {direction}, {center} }
    static const pair<Vector<2, T>, Vector<2, T>> clipLines[4] =
    {
        // Top Line
        {{ +1,  0 }, { 0, +1 }},
        // Right Light
        {{  0, -1 }, { +1, 0 }},
        // Bottom
        {{ -1,  0 }, { 0, -1 }},
        // Left
        {{  0, +1 }, { -1, 0 }}
    };

    // rotate line's coordinate system coordinate system so:
    // y ->  v1
    // x -> -v2
    // (we do this for convenience in intercepting the hyperbolic equation, in its standard form)
    Matrix<2, 2, T> R;
    R.SetRow(0, v1);
    R.SetRow(1, v2);

    SegmentList.clear();

    // bit field of hidden/showing states....
    int states = 0;
    vector<pair <int, T>> stateChangeList;

    const bool hidden = true;
    const bool showing = false;

    for (int i = 0; i < 4; ++i)
    {
        const pair<Vector<2, T>, Vector<2, T>>& line = clipLines[i];

        bool initialState;
        T stateChanges[2];
        int n;

        ClipHyperbolaToLine(line, screenDimentions, R, positiveOrientation, k, a, b, initialState, stateChanges, n);

        if (initialState == hidden && n == 0)
        {
            // The hyperbola is never on the visible side of this line...
            /// We can bail right now...
            return;
        }
        
        int stateFlag = initialState ? hidden : showing;

        // Remember if this line initially was hidden or showing
        states |= stateFlag << i;

        // Add all state changes...
        if (n > 0)
        {
            stateChangeList.push_back({ i, stateChanges[0] });
            if (n > 1)
            {
                stateChangeList.push_back({ i, stateChanges[1] });
            }
        }
    }

    if (stateChangeList.size() > 0)
    {
        std::sort(
            stateChangeList.begin(), stateChangeList.end(),
            [](const  pair <int, T>& a, const  pair <int, T>& b)
            {
                return a.second < b.second;
            });

        // Ignore any cases where the line is visible to infinity (initially showing)
        int lastState = states;
        T segmentBegin {};

        // Loop through the state changes looking for transitions from
        // one or more hidden clip line bisections.
        for (int i = 0; i < stateChangeList.size(); ++i)
        {
            const pair <int, T>& change = stateChangeList[i];

            int who = change.first;
            T when = change.second;

            int bitMask = (1 << who);

            // Flip the state...
            states ^= bitMask;

            bool visible = !states;

            if (!states && lastState)
            {
                segmentBegin = when;
            }
            else if (states && !lastState)
            {
                SegmentList.push_back({ segmentBegin, when-segmentBegin });
            }

            lastState = states;
        }
    }
}

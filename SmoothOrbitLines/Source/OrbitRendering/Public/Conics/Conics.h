// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com
//-----------------------------------------------------------------------------
// Public Definitions needed by apis processing conic projection data.
//-----------------------------------------------------------------------------

#pragma once
#include "GTE/Mathematics/Vector.h"
#include "GTE/Mathematics/Vector3.h"
using namespace gte;

// Miscellaneous definitions used by the conic function templates
enum ProjectionType
{
    NotVisible,
    Ellipse,
    PositiveHyperbola,
    NegativeHyperbola
};


template<class T>
struct ConicProjection
{
    Vector<2, T> k;
    Vector<2, T> u;
    Vector<2, T> v;
    T a;
    T b;
};

template<class T>
struct ConicSegment
{
    T SegmentStart;
    T SegmentLength;

    ConicSegment() : SegmentStart(default), SegmentLength(default) {}
    ConicSegment(T start, T length) : SegmentStart(start), SegmentLength(length) {}
};

template<class T>
struct FrustumParameters
{
    T z;
    T fov;
    T aspectRatio;

    FrustumParameters()
    {
        memset(this, 0, sizeof(FrustumParameters));
    }

    FrustumParameters(T _z, T _fov, T _aspectRatio)
    {
        z = _z;
        fov = _fov;
        aspectRatio = _aspectRatio;
    }
};


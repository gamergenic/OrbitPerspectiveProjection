// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

//-----------------------------------------------------------------------------
// ProjectionAngleToTrueAnomaly
// Given an angle on the projection conic, determine the point's true anomaly
// angle on the original orbit plane
//-----------------------------------------------------------------------------

#pragma once

#include "GTE/Mathematics/IntrLine3Plane3.h"
#include "GTE/Mathematics/Matrix3x3.h"

using namespace gte;

#include "Conics.h"
#include "ProjectEllipseToPlane.h"

template<class T>
bool ProjectionAngleToTrueAnomaly(
    const EllipseProjectionInputs<T>& orbitData,
    const EllipseProjectionOutputs<T>& projectionData,
    T theta,
    T& trueAnomaly
)
{
    bool result = true;
    Vector3<double> point;
    Vector<2,double> pointOnConicPlane;
    const Vector3<T>& E = orbitData.E;
    const Vector3<T>& Ce = orbitData.Ce;
    const Vector3<T>& Ne = orbitData.Ne;
    const Vector3<T>& Ue = orbitData.Ue;
    const Vector3<T>& Ve = orbitData.Ve;
    /*
    T A = orbitData.A;
    T B = orbitData.B;
    */
    const Vector3<T>& Cp = orbitData.Cp;
    const Vector3<T>& Np = orbitData.Np;
    const Vector3<T>& Up = orbitData.Up;
    const Vector3<T>& Vp = orbitData.Vp;

    ProjectionType projectionType = projectionData.projectionType;
    Vector<2, T> k = projectionData.projection.k;
    Vector<2, T> u = projectionData.projection.u;
    Vector<2, T> v = projectionData.projection.v;
    T a = projectionData.projection.a;
    T b = projectionData.projection.b;

    if (projectionData.projectionType == ProjectionType::Ellipse)
    {
        const auto cosT = cos(theta);
        const auto sinT = sin(theta);
        pointOnConicPlane = cosT * a * u + sinT * b * v;
    }
    else if (projectionData.projectionType == ProjectionType::PositiveHyperbola)
    {
        const auto coshT = cosh(theta);
        const auto sinhT = sinh(theta);
        pointOnConicPlane = coshT * a * u + sinhT * b * v;
    }
    else if (projectionData.projectionType == ProjectionType::NegativeHyperbola)
    {
        const auto coshT = -cosh(theta);
        const auto sinhT = -sinh(theta);
        pointOnConicPlane = coshT * a * u - sinhT * b * v;
    }
    else
    {
        result = false;
    }

    if (result)
    {
        pointOnConicPlane += k;
        Vector3<double> pointOnProjectionPlane = Cp + pointOnConicPlane[0] * Up + pointOnConicPlane[1] * Vp;

        auto toEyePoint = E - pointOnProjectionPlane;
        Normalize(toEyePoint);

        Line3<T> line(pointOnProjectionPlane, toEyePoint);
        Plane3<T> plane(Ne, Ce);

        FIQuery<T, Line3<T>, Plane3<T>> intersectionSolver;
        auto intr = intersectionSolver(line, plane);
        point = intr.point;

        point -= Ce;

        Matrix3x3<double> Q;
        Q.SetRow(0, Ue);
        Q.SetRow(1, Ve);
        Q.SetRow(2, Ne);

        auto perifocalPoint = Q * point;

        double x = perifocalPoint[0];
        double y = perifocalPoint[1];

        double ae = Length(Ce);

        trueAnomaly = atan2(y, (x - ae));
    }

    return result;
}

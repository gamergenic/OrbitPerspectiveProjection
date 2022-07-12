// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

//-----------------------------------------------------------------------------
// ProjectEllipseToPlane
// Implmenentation of algoritm that projects an ellipse onto a view plane via.
// a *perspective* projection.
// Note - the ellipse is specified by center position.   To use with a perifocal
// orbit coordinate system, the center is computed as a*e, (a = semi-major axis,
// e = eccentricity).
// Although it is defined as a template, it is unlikley to compile for types
// that are not used in this project (exclusively double types).
// The implementation is not designed to be robust.   The variables were named
// to be consistent with an implmentation described as a documented by
// David Eberly's Geometric Tools.  The algorithm has been revised and the
// variable names may no longer make sense.
// This implementation is intended for demonstration purposes only and not
// intended to be used in production.
// This algorithm has not been optimized (performance, memory, readability, etc)
// See ReadMe.txt for more information.
//-----------------------------------------------------------------------------

#pragma once

#include "GTE/Mathematics/Math.h"
#include "GTE/Mathematics/Vector.h"
#include "GTE/Mathematics/Vector3.h"
#include "GTE/Mathematics/Matrix3x3.h"
#include "GTE/Mathematics/Matrix.h"
#include "GTE/Mathematics/IntrLine3Plane3.h"
#include "GTE/Mathematics/SymmetricEigensolver2x2.h"
#include <array>

using namespace gte;

#include "Conics.h"

template<class T>
struct EllipseProjectionInputs
{
    Vector3<T>  E, Ce, Ne, Ue, Ve;
    T           A, B;
    Vector3<T>  Cp, Np, Up, Vp;
    T           TestTheta;

    EllipseProjectionInputs()
    {
        memset(this, 0, sizeof(EllipseProjectionInputs));
    }

    EllipseProjectionInputs(
        Vector3<T>  _E,  Vector3<T>  _Ce, Vector3<T>  _Ne,  Vector3<T>  _Ue,  Vector3<T>  _Ve,
        T           _A,  T           _B,
        Vector3<T>  _Cp, Vector3<T>  _Np, Vector3<T>  _Up,  Vector3<T>  _Vp,
        T           _TestTheta
    )
    {
        E = _E;     Ce = _Ce;   Ne = _Ne;   Ue = _Ue;   Ve = _Ve;
        A = _A;     B = _B;
        Cp = _Cp;   Np = _Np;   Up = _Up;   Vp = _Vp;
        TestTheta = _TestTheta;
    }
};


template<class T>
struct EllipseProjectionOutputs
{
    ConicProjection<T> projection;
    ProjectionType projectionType;
    T ThetaLocation;
};


/*
 * Implementation of:
 * https://www.geometrictools.com/Documentation/PerspectiveProjectionEllipse.pdf
 * (Originally published algorithm - the reference page has undergone a
 * significant rewrite since original publication.)
 */
template<class T>
void ProjectEllipseToPlane(
    const EllipseProjectionInputs<T>& inputs,
    EllipseProjectionOutputs<T>& outputs
)
{
    // Inputs, unpack...
    const Vector3<T>& E = inputs.E;
    const Vector3<T>& Ce = inputs.Ce;
    const Vector3<T>& Ne = inputs.Ne;
    const Vector3<T>& Ue = inputs.Ue;
    const Vector3<T>& Ve = inputs.Ve;
    T d0 = inputs.A;
    T d1 = inputs.B;
    const Vector3<T>& Cp = inputs.Cp;
    const Vector3<T>& Np = inputs.Np;
    const Vector3<T>& Up = inputs.Up;
    const Vector3<T>& Vp = inputs.Vp;
    const T TestTheta = inputs.TestTheta;

    // ------------------------------------------------------------------------
    // Start of Projection Algorithm
    // ------------------------------------------------------------------------

    // 1b
    Matrix3x3<T> Re33;
    Re33.SetCol(0, Ue);
    Re33.SetCol(1, Ve);
    Re33.SetCol(2, Ne);
    Matrix3x3<T> Re33_T = Transpose(Re33);

    Matrix<3, 2, T> Jp32;
    Jp32.SetCol(0, Up);
    Jp32.SetCol(1, Vp);
    Matrix<2, 3, T> Jp32_T = Transpose(Jp32);

    // 2a
    Vector3<T> E_bar;
    Vector3<T> Ce_bar;
    T e0, e1, e2;

    E_bar = Re33_T * (E - Ce);
    Ce_bar = Re33_T * Ce;
    e0 = E_bar[0]; e1 = E_bar[1]; e2 = E_bar[2];

    // 2b
    Matrix3x3<T> A_bar33;
    Vector3<T> B_bar({ (T)0., (T)0., (T)2. / e2 });
    T c_bar = -1.;

    T a00 = (T)1. / sqr(d0);
    T a11 = (T)1. / sqr(d1);
    T a01 = (T)0.;
    T a02 = -e0 / e2 * a00;
    T a12 = -e1 / e2 * a11;
    T a22 = (sqr(e0) * a00 + sqr(e1) * a11 - (T)1) / sqr(e2);

    A_bar33.SetCol(0, Vector3<T>{a00, a01, a02});
    A_bar33.SetCol(1, Vector3<T>{a01, a11, a12});
    A_bar33.SetCol(2, Vector3<T>{a02, a12, a22});

    // 2c
    Matrix3x3<T> A33;
    Vector3<T> B;
    T c;

    A33 = Re33 * A_bar33 * Re33_T;
    B = Re33 * (B_bar - (T)2. * (A_bar33 * Ce_bar));
    c = Dot(Ce_bar, A_bar33 * Ce_bar) - Dot(B_bar, Ce_bar) + c_bar;

    // 2d
    Matrix<2, 2, T> A_hat22;
    Vector<2, T> B_hat;
    T c_hat;

    A_hat22 = Jp32_T * A33 * Jp32;
    B_hat = Jp32_T * (B + (T)2. * (A33 * Cp));
    c_hat = Dot(Cp, A33 * Cp) + Dot(B, Cp) + c;

    // 3b
    Matrix<2, 2, T> A_hat22_I = Inverse(A_hat22);
    Vector<2, T> k = -(A_hat22_I * ((T)0.5 * B_hat));

    T d = (T)0.25 * Dot(B_hat, A_hat22_I * B_hat) - c_hat;
    Matrix<2, 2, T> M = ((T)1. / d) * A_hat22;

    // 3c
    // Factor M2 = R*D*R^T.
    SymmetricEigensolver2x2<T> eigensolver;
    std::array<T, 2> S;
    std::array<std::array<T, 2>, 2> evec;
    eigensolver(M(0, 0), M(0, 1), M(1, 1), -1, S, evec);

    T a1 = 1. / sqrt(S[0]);

    bool isHyperbolic = (S[1] < 0);
    T a2 = 1. / sqrt(abs(S[1]));

    // ------------------------------------------------------------------------
    // End of Projection Algorithm
    // ------------------------------------------------------------------------

    // Whew.!   We have our conic...  But, we need to check whether or not
    // it's visible.
    // ALSO, since we'll need it later, we need to compute exactly where the body
    // currently is... We can do both things at the same time, but projecting
    // the body's current location into the conic's coordinate system...
    // Outputs
    ProjectionType projectionType;
    T ThetaLocation;

    const auto cosT = cos(TestTheta);
    const auto sinT = sin(TestTheta);
    auto point = Ce + d0 * Ue * cosT + d1 * Ve * sinT;
    auto tangent = -d0 * Ue * sinT + d1 * Ve * cosT;

    auto toEyePoint = E - point;
    Normalize(toEyePoint);

    bool forwardOfCamera = Dot(toEyePoint, Np) > 0;

    Line3<T> line(point, toEyePoint);
    Plane3<T> plane(Np, Cp);

    FIQuery<T, Line3<T>, Plane3<T>> intersectionSolver;
    auto result = intersectionSolver(line, plane);

    auto pointOnProjectionPlane = result.point;
    pointOnProjectionPlane -= Cp;

    auto planeSpaceTangentCoords = Jp32_T * tangent;
    auto planeSpaceCoords = Jp32_T * pointOnProjectionPlane;
    planeSpaceCoords -= k;

    Matrix<2, 2, T> R_T;
    R_T.SetRow(0, evec[0]);
    R_T.SetRow(1, evec[1]);

    // And, now we know the location of this point in the conic's coord system!
    auto conicSpaceCoords = R_T * planeSpaceCoords;

    if (isHyperbolic)
    {
        if ((conicSpaceCoords[0] > 0) ^ (!forwardOfCamera))
        {
            projectionType = ProjectionType::PositiveHyperbola;
        }
        else
        {
            projectionType = ProjectionType::NegativeHyperbola;
        }

        // Ensure the hyperbola is defined such that Cross(v1,v2) > 0
        auto conicSpaceTangent = R_T * planeSpaceTangentCoords;
        bool velocityPointsUpwards = conicSpaceTangent[1] > 0;
        if ((projectionType == ProjectionType::PositiveHyperbola) != velocityPointsUpwards)
        {
            a2 *= -1;
        }

        // Avoid dividing by zero, ... which realistically isn't going to happen
        // because then we're not hyperbolic... Which, is why there's a singularity here
        // that blows up the math anyways.
        if (forwardOfCamera)
        {
            ThetaLocation = atanh(conicSpaceCoords[1] * a1 / conicSpaceCoords[0] / a2);
        }
        else
        {
            // The current object is off the projected hyperbola.
            // We can at least find out which side it's closer to...
            ThetaLocation = -std::numeric_limits<T>::infinity();
            if (atanh(conicSpaceCoords[1] * a1 / conicSpaceCoords[0] / a2) > 0)
            {
                ThetaLocation *= -1;
            }
        }
    }
    else
    {
        if (forwardOfCamera)
        {
            projectionType = ProjectionType::Ellipse;
        }
        else
        {
            projectionType = ProjectionType::NotVisible;
        }

        // Ensure the ellipse is defined such that Cross(v1,v2) > 0
        if (planeSpaceCoords[0] * planeSpaceTangentCoords[1] - planeSpaceCoords[1] * planeSpaceTangentCoords[0] < 0)
        {
            a2 *= -1.;

            ThetaLocation = atan2(-conicSpaceCoords[1] * a1, -conicSpaceCoords[0] * a2);
        }
        else
        {
            ThetaLocation = atan2(+conicSpaceCoords[1] * a1, +conicSpaceCoords[0] * a2);
        }
        if (ThetaLocation < 0) ThetaLocation += twopi<T>;
    }

    // Pack up...
    outputs.projection.k = k;
    outputs.projection.u = evec[0];
    outputs.projection.v = evec[1];
    outputs.projection.a = a1;
    outputs.projection.b = a2;
    outputs.projectionType = projectionType;
    outputs.ThetaLocation = ThetaLocation;
}



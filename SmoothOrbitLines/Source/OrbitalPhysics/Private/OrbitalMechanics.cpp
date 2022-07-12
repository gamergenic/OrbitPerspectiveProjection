// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#include "OrbitalMechanics.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "MeanAnomalyToTrueAnomaly.h"

using namespace gte;


void UOrbitalMechanics::ComputePerifocalPosition(const FConicElements& ConicElements, double trueAnom, double& r, FFrameVector& R, ES_ResultCode& ResultCode)
{
    if (ConicElements.ecc >= 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot compute state for eccentricies >= 1"));
        ResultCode = ES_ResultCode::Error;
        return;
    }

    // Semi-Major Axis
    double a = ConicElements.rp / (1 - ConicElements.ecc);
    double ecc = ConicElements.ecc;

    double ta = trueAnom / 360 * twopi<double>;

    // Orbital Mechanics for Engineering Students (Eq. 2.72)
    r = a * (1 - ecc * ecc) / (1 + ecc * cos(ta));

    // State usually means r, v pair (position + velocity)
    // But here, we don't care about v so much... 
    // But v is trivially easy to compute from here, if needed
    R = Vector3<double>{ r * cos(ta), r * sin(ta), 0. };

    ResultCode = ES_ResultCode::Success;
}


void UOrbitalMechanics::ComputePerifocalState(const FConicElements& ConicElements, double et, double& M, double& trueAnom, double& r, FFrameVector& R, ES_ResultCode& ResultCode)
{
    // Semi-Major Axis
    double a = ConicElements.rp / (1 - ConicElements.ecc);

    // Orbital Mechanics for Engineering Students (Eq. 3.8 & 2.83)
    double n = sqrt(a * a * a / ConicElements.mu);

    // Orbital Mechanics for Engineering Students (Eq. 3.8 & 2.83)
    double T = twopi<double> *n;

    // Time that had elapsed since periapsis at epoch
    double t0 = (ConicElements.m0 / 360.) * T;

    // How many seconds have ellapsed since the reference epoch?
    double t = et - ConicElements.et0;

    // at the reference epoch, how many seconds had already elapsed since Periapsis?
    t += t0;

    double tpe = fmod(t, T);

    n = tpe / T;
    M = n * 360;

    trueAnom = ::MeanAnomalyToTrueAnomaly(M, ConicElements.ecc);

    ComputePerifocalPosition(ConicElements, trueAnom, r, R, ResultCode);
}



void UOrbitalMechanics::ComputeState(const FConicElements& ConicElements, double et, FState& State, ES_ResultCode& ResultCode)
{

    FFrameVector R;
    ComputePerifocalState(ConicElements, et, State.Me, State.Theta, State.r, R, ResultCode);

    Matrix3x3<double> Q;
    MakeQ(ConicElements.inc, ConicElements.lnode, ConicElements.argp, Q);

    State.StateVector.r = FFramePosition(Q * (Vector3<double>)R);

    ResultCode = ES_ResultCode::Success;
}

void UOrbitalMechanics::ComputeGeometry(const FConicElements& ConicElements, FOscullatingOrbitGeometry& Geometry, ES_ResultCode& ResultCode)
{
    if (ConicElements.ecc >= 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot compute geometry for eccentricies >= 1"));
        ResultCode = ES_ResultCode::Error;
        return;
    }

    Matrix3x3<double> Q;

    MakeQ(ConicElements.inc, ConicElements.lnode, ConicElements.argp, Q);

    Geometry.a = ConicElements.rp / (1 - ConicElements.ecc);
    // Could just grab the first column...
    Geometry.p_hat = Q.GetCol(0);

    // Semi-Minor Axis
    Geometry.b = Geometry.a * sqrt(1 - ConicElements.ecc * ConicElements.ecc);
    Geometry.q_hat  = Q.GetCol(1);

    // Orbital plane normal
    Geometry.w_hat = Q.GetCol(2);

    Geometry.ae = Geometry.a * ConicElements.ecc;

    ResultCode = ES_ResultCode::Success;
}

void UOrbitalMechanics::MeanAnomalyToTrueAnomaly(double meanAnomaly, double eccentricity, double& trueAnomaly, ES_ResultCode& ResultCode, int decimalPlaces)
{
    if (eccentricity >= 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot compute true anomaly for eccentricies >= 1"));
        ResultCode = ES_ResultCode::Error;
        return;
    }
    
    ResultCode = ES_ResultCode::Success;

    trueAnomaly = ::MeanAnomalyToTrueAnomaly(meanAnomaly, eccentricity, decimalPlaces);
}

// Computes the rotation matrix per the conic elements
// (RHS Coordinate System, Perifocal to (parent) Fixed Equatorial "Geo"-centric Frame)
void UOrbitalMechanics::MakeQ(double inc, double lnode, double argp, RotationMatrix& q)
{
    inc *= pi<double> / 180.;
    lnode *= pi<double> / 180.;
    argp *= pi<double> / 180.;

    // Orbital Mechanics for Engineering Students (Eq. 4.47)
    EulerAngles<double> R313 = EulerAngles<double>(2, 0, 2, argp, inc, lnode);

    Rotation<3, double> r = Rotation<3, double>(R313);
    q = Transpose((RotationMatrix)r);
}

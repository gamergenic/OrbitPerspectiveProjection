// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"

#include "GTE/Mathematics/Math.h"
#include "GTE/Mathematics/Vector.h"
#include "GTE/Mathematics/Vector3.h"
#include "GTE/Mathematics/Matrix3x3.h"
#include "GTE/Mathematics/Matrix.h"
#include "GTE/Mathematics/Rotation.h"
#include "GTE/Mathematics/AxisAngle.h"
#include "GTE/Mathematics/EulerAngles.h"

#include "OrbitalMechanics.generated.h"

template<typename T>
T pi = (T)acos(-1.);
template<typename T>
T twopi = (T)(2. * acos(-1.));

template<class T>
T normalizeRadians0toTwoPi(T value)
{
    if (value >= (T)0)
    {
        return (T)(std::fmod(value, twopi<T>));
    }
    else
    {
        return twopi<T> -normalizeRadians0toTwoPi<T>(-value);
    }
}

typedef gte::Matrix3x3<double> RotationMatrix;
typedef gte::Vector3<double> PositionVector;
typedef gte::Vector3<double> DirectionVector;

UENUM(BlueprintType)
enum class ES_ResultCode : uint8
{
    Success UMETA(DisplayName = "Ok"),
    Error UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FConicElements
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Conics",
        meta = (
            ToolTip = "Perifocal Distance (Kilometers)",
            ClampMin = "0"
            ))
    double rp;

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Conics",
        meta = (
            ToolTip = "Eccentricity (Dimensionless)",
            ClampMin = "0", ClampMax = "0.9"   // Only osclillating orbits (elliptcial)
            ))
    double ecc;

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Conics",
        meta = (
            ToolTip = "Inclination (Degrees)",
            ClampMin = "-90", ClampMax = "90"
            ))
    double inc;

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Conics",
        meta = (
            ToolTip = "Longitude of the Ascending Node (Degrees)",
            ClampMin = "0", ClampMax = "360"
            ))
    double lnode;

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Conics",
        meta = (
            ToolTip = "Argument of Periapsis (Degrees)",
            ClampMin = "0", ClampMax = "360"
            ))
    double argp;

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Conics",
        meta = (
            ToolTip = "Mean Anomaly At Epoch (Degrees)",
            ClampMin = "0", ClampMax = "360"
            ))
    double m0;

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Conics",
        meta = (
            ToolTip = "Epoch (Seconds Past J2000)",
            ClampMin = "0"
            ))
    double et0;

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Orbit | Conics",
        meta = (
            ToolTip = "GM/Gravitational Parameter (km^3 /sec^2, Sun is 1.327e+11)",
            ClampMin = "0"
            ))
    double mu;
};

USTRUCT(BlueprintType)
struct FFrameVector
{
    GENERATED_BODY()

    FFrameVector()
    {
        X = Y = Z = 0.;
    }


    FFrameVector(const gte::Vector3<double>& value)
    {
        X = value[0];
        Y = value[1];
        Z = value[2];
    }

    FFrameVector(double x, double y, double z)
    {
        X = x;
        Y = y;
        Z = z;
    }

    UPROPERTY(EditAnywhere, meta = (ToolTip = "X Value (Dimension Unassigned)"))
    double X;

    UPROPERTY(EditAnywhere, meta = (ToolTip = "Y Value (Dimension Unassigned)"))
    double Y;

    UPROPERTY(EditAnywhere, meta = (ToolTip = "Z Value (Dimension Unassigned)"))
    double Z;

    inline operator gte::Vector3<double>() const
    {
        return gte::Vector3<double>{ X, Y, Z};
    }

    inline FFrameVector operator=(gte::Vector3<double> vec)
    {
        X = vec[0];
        Y = vec[1];
        Z = vec[2];
        return *this;
    }

    inline FFrameVector operator* (double scalar) const
    {
        return FFrameVector(scalar * X, scalar * Y, scalar * Z);
    }

    inline FFrameVector operator+ (const FFrameVector& other) const
    {
        return FFrameVector(other.X + X, other.Y + Y, other.Z + Z);
    }

    inline double Normalize()
    {
        gte::Vector3<double> v = *this;
        double mag = gte::Normalize(v);
        *this = v;
        return mag;
    }

    static inline double Dot(FFrameVector a, FFrameVector b)
    {
        return gte::Dot((gte::Vector3<double>) a, (gte::Vector3<double>)b);
    }

    static inline FFrameVector Cross(FFrameVector a, FFrameVector b)
    {
        auto result = gte::Cross((gte::Vector3<double>) a, (gte::Vector3<double>)b);
        return FFrameVector(result);
    }
};

USTRUCT(BlueprintType)
struct FFramePosition
{
    GENERATED_BODY()

    FFramePosition()
    {
        X = Y = Z = 0.;
    }

    FFramePosition(const gte::Vector3<double>& value)
    {
        X = value[0];
        Y = value[1];
        Z = value[2];
    }

    FFramePosition(double x, double y, double z)
    {
        X = x;
        Y = y;
        Z = z;
    }

    UPROPERTY(EditAnywhere, meta = ( ToolTip = "X position (kilometers)" ))
    double X;

    UPROPERTY(EditAnywhere, meta = (ToolTip = "Y position (kilometers)"))
    double Y;

    UPROPERTY(EditAnywhere, meta = (ToolTip = "Z position (kilometers)"))
    double Z;

    inline operator gte::Vector3<double>() const
    {
        return gte::Vector3<double>{ X, Y, Z};
    }

    inline FFramePosition& operator=(gte::Vector3<double> vec)
    {
        X = vec[0];
        Y = vec[1];
        Z = vec[2];
        return *this;
    }
};

inline FFrameVector operator-(const FFramePosition& first, const FFramePosition& second)
{
    gte::Vector3<double> _this = (gte::Vector3<double>)first;
    gte::Vector3<double> _other = (gte::Vector3<double>)second;
    return FFrameVector(_this - _other);
}

inline FFramePosition operator+(const FFramePosition& first, const FFrameVector& second)
{
    gte::Vector3<double> _this = (gte::Vector3<double>)first;
    gte::Vector3<double> _other = (gte::Vector3<double>)second;
    return FFramePosition(_this + _other);
}

inline FFrameVector operator*(const FFrameVector& vec, double scalar)
{
    return FFrameVector(scalar * vec.X, scalar * vec.Y, scalar * vec.Z);
}

inline FFrameVector operator*(double scalar, const FFrameVector& vec)
{
    return FFrameVector(scalar * vec.X, scalar * vec.Y, scalar * vec.Z);
}


USTRUCT(BlueprintType)
struct FStateVector
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, meta = (ToolTip = "r, the position vector)"))
    FFramePosition r;

    // Normally the position v, too, would be here -- but we don't need it for this
};


USTRUCT(BlueprintType)
struct FState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        meta = (
            ToolTip = "Mean Anomaly (Elliptical)"
            ))
    double Me;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        meta = (
            ToolTip = "True Anomaly"
            ))

    double Theta;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        meta = (
            ToolTip = "Distance"
            ))
    double r;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        meta = (
            ToolTip = "Position in Parent Frame"
            ))

    FStateVector StateVector;
};


USTRUCT(BlueprintType)
struct FOscullatingOrbitGeometry
{
    GENERATED_BODY()

    UPROPERTY(meta = (ToolTip = "Semi-Major Axis (Scene Units)"), BlueprintReadOnly)
    double a;

    UPROPERTY(meta = (ToolTip = "Semi-Minor Axis (Scene Units)"), BlueprintReadOnly)
    double b;

    UPROPERTY(meta = (ToolTip = "Unit Normal of Periapsis Axis"), BlueprintReadOnly)
    FFrameVector p_hat;

    UPROPERTY(meta = (ToolTip = "Unit Normal of Semilatus Rectum (a disorder which afflicts astronauts who don't get enough fiber)"), BlueprintReadOnly)
    FFrameVector q_hat;

    UPROPERTY(meta = (ToolTip = "Unit Normal of perpendicular to plane of orbit... =Cross(p_hat, q_hat)"), BlueprintReadOnly)
    FFrameVector w_hat;

    UPROPERTY(meta = (ToolTip = "Distance from Ellipse Center to foci"), BlueprintReadOnly)
    double ae;

    FFramePosition Position(const FFramePosition& center, double theta)
    {
        return center + cos(theta) * a * p_hat + sin(theta) * b * q_hat;
    }

    FFrameVector Tangent(double theta)
    {
        FFrameVector result = -sin(theta) * a * p_hat + cos(theta) * b * q_hat;
        result.Normalize();
        return result;
    }
};

UCLASS()
class ORBITALPHYSICS_API UOrbitalMechanics : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable,
        Category = "Orbital Mechanics",
        meta = (
            ExpandEnumAsExecs = "ResultCode"
            ))
    static void ComputePerifocalPosition(const FConicElements& ConicElements, double trueAnom, double& r, FFrameVector& _R, ES_ResultCode& ResultCode);

    UFUNCTION(BlueprintCallable,
        Category = "Orbital Mechanics",
        meta = (
            ExpandEnumAsExecs = "ResultCode"
            ))
    static void ComputePerifocalState(const FConicElements& ConicElements, double et, double& M, double& trueAnom, double& r, FFrameVector& _R, ES_ResultCode& ResultCode);

    UFUNCTION(BlueprintCallable,
        Category = "Orbital Mechanics",
        meta = (
            ExpandEnumAsExecs = "ResultCode"
            ))
    static void ComputeState(const FConicElements& ConicElements, double et, FState& State, ES_ResultCode& ResultCode);
    
    UFUNCTION(BlueprintCallable,
        Category = "Orbital Mechanics",
        meta = (
            ExpandEnumAsExecs = "ResultCode"
            ))
    static void ComputeGeometry(const FConicElements& ConicElements, FOscullatingOrbitGeometry& Geometry, ES_ResultCode& ResultCode);

    UFUNCTION(BlueprintCallable,
        Category = "Orbital Mechanics",
        meta = (
            ExpandEnumAsExecs = "ResultCode",
            ToolTip = "Compute a true anomaly from a given mean anomaly"
            ))
    static void MeanAnomalyToTrueAnomaly(double meanAnomaly, double eccentricity, double& trueAnomal, ES_ResultCode& ResultCode, int decimalPlaces = 8);

    static void MakeQ(double inc, double lnode, double argp, RotationMatrix& q);
};


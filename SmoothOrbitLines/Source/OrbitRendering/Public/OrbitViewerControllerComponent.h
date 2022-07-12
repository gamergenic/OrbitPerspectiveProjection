// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "OrbitSystemStateComponent.h"
#include "OrbitingBodyComponent.h"

#include "GTE/Mathematics/Vector3.h"

#include "OrbitViewerControllerComponent.generated.h"



USTRUCT(BlueprintType)
struct FSceneStateVector
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "r, the position vector (Scene Units)"))
    FVector r;

    // Normally the position v, too, would be here -- but we don't need it for this
};

USTRUCT(BlueprintType)
struct FSceneOrbitGeometry
{
    GENERATED_BODY()

    UPROPERTY(meta = (ToolTip = "Semi-Major Axis (Scene Units)"), BlueprintReadOnly, VisibleInstanceOnly)
    float a;

    UPROPERTY(meta = (ToolTip = "Semi-Minor Axis (Scene Units)"), BlueprintReadOnly, VisibleInstanceOnly)
    float b;

    UPROPERTY(meta = (ToolTip = "Unit Normal of Periapsis Axis"), BlueprintReadOnly, VisibleInstanceOnly)
    FVector p_hat;

    UPROPERTY(meta = (ToolTip = "Unit Normal of Semilatus Rectum (a disorder which afflicts astronauts who don't get enough fiber)"), BlueprintReadOnly, VisibleInstanceOnly)
    FVector q_hat;

    UPROPERTY(meta = (ToolTip = "Unit Normal of perpendicular to plane of orbit"), BlueprintReadOnly, VisibleInstanceOnly)
    FVector w_hat;

    UPROPERTY(meta = (ToolTip = "Distance from Ellipse Center to foci (Scene Units)"), BlueprintReadOnly, VisibleInstanceOnly)
    float ae;
};



UCLASS()
class ORBITRENDERING_API UOrbitViewerControllerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    UWorld* World;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadWrite,
        Category = "Orbit Scene View"
    )
    UOrbitingBodyComponent* FocusBody;


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbit Scene View")
    UOrbitSystemStateComponent* GameState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe", meta = (ToolTip = "Celestial Bodies found in the universe"))
    TArray<UOrbitingBodyComponent*> Bodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe", meta = (ToolTip = "Centimeters Per Kilometer"))
    double SceneScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe", meta = (ToolTip = "Centimeters Per Kilometer"))
    class UOrbitViewerControllerComponent* OrbitViewerController;

public:
    UFUNCTION(BlueprintPure,
        Category = "Orbit Scene View",
        meta = (
            ToolTip = "Adds the body to the focusable objects"
            ))
    void AddBody(FString& BodyId, UOrbitingBodyComponent* Body);


    UFUNCTION(BlueprintPure,
        Category = "Orbit Scene View",
        meta = (
            ToolTip = "Establishes the focus object & shifts coordinate system to be centered at its origin"
            ))
    void SetFocus(FString& BodyId);

    /*
        Moving back and forth between the rhs d.p. frames and lhs s.p. frames involves:
        Swizzling x,y
        Scaling (to/from scene scale)
        Reposititioning (to/from scene/solar system origins)
        (in some variety depending on what's being moved)
    */
    UFUNCTION(BlueprintPure,
        Category = "Orbit Scene View",
        meta = (
            ToolTip = "Given a position in Scene-Space, convert it to Universe-Space"
            ))
    void GetFramePosition(const FVector& ScenePosition, FFramePosition& FramePosition);

    UFUNCTION(BlueprintPure,
        Category = "Orbit Scene View",
        meta = (
            ToolTip = "Given a direction in Scene-Space, convert it to Universe-Space"
            ))
    void GetFrameVector(const FVector& SceneVector, FFrameVector& FrameVector) const;

    UFUNCTION(BlueprintPure,
        Category = "Orbit Scene View",
        meta = (
            ToolTip = "Given a distance in Scene-Space, convert it to Universe-Space"
            ))
    void GetFrameDistance(double SceneDistance, double& FrameDistance);

    UFUNCTION(BlueprintCallable,
        Category = "Orbital Mechanics",
        meta = (
            ExpandEnumAsExecs = "ResultCode",
            ShortToolTip = "SPK, constant velocity target state",
            ToolTip = "Return the state of a specified target relative to an 'observer', where the observer has constant velocity in a specified reference frame.The observer's state is provided by the calling program rather than by loaded SPK files"
            ))
    void ComputeState(const FConicElements& ConicElements, double et, FSceneStateVector& State, ES_ResultCode& ResultCode);

    UFUNCTION(BlueprintCallable,
        Category = "Orbital Mechanics",
        meta = (
            ExpandEnumAsExecs = "ResultCode",
            ShortToolTip = "SPK, constant velocity target state",
            ToolTip = "Return the state of a specified target relative to an 'observer', where the observer has constant velocity in a specified reference frame.The observer's state is provided by the calling program rather than by loaded SPK files"
            ))
    void ComputeGeometry(const FConicElements& ConicElements, FSceneOrbitGeometry& Geometry, ES_ResultCode& ResultCode);

    UFUNCTION(BlueprintPure,
        Category = "Orbit Scene View",
        meta = (
            ToolTip = "Given a state vector in Universe-Space, convert it to Scene-Space"
            ))
    void GetSceneStateVector(const FStateVector& StateVector, FSceneStateVector& SceneStateVector);

    UFUNCTION(BlueprintPure,
        Category = "Orbit Scene View",
        meta = (
            ToolTip = "Given a position in Universe-Space, convert it to Scene-Space"
            ))
    void GetScenePosition(const FFramePosition& FramePosition, FVector& ScenePosition);

    UFUNCTION(BlueprintPure,
        Category = "Orbit Scene View",
        meta = (
            ToolTip = "Given a position in Universe-Space, convert it to Scene-Space"
            ))
    void GetSceneDistance(double FrameDistance, double& SceneDistance);

    UFUNCTION(BlueprintPure,
        Category = "Orbit Scene View",
        meta = (
            ToolTip = "Given a direction in Universe-Space, convert it to Scene-Space"
            ))
    void GetSceneVector(const FFrameVector& FrameVector, FVector& SceneVector) const;

    UFUNCTION(BlueprintPure,
        Category = "Orbit Scene View",
        meta = (
            ToolTip = "Given Scene Geometry in Universe-Space, convert it to Scene-Space"
            ))
    void GetSceneGeometry(const FOscullatingOrbitGeometry& OrbitGeometry, FSceneOrbitGeometry& SceneGeometry) const;

    void GetActiveBodies(TArray<class UOrbitingBodyComponent*>& ActiveBodies);

    // To RHS coordinate system (used for engineering/science/math)
    static gte::Vector3<double> Swizzle(const FVector& value);
    // To LHS coordinate system (used for UE graphis/physics/etc)
    static FVector Swizzle(const gte::Vector3<double>& value);

    UFUNCTION(BlueprintCallable,
        Category = "Orbit Scene View | Debug",
        meta = (
            ToolTip = "Draw the orbit with debug lines"
            ))
    void DrawDebugOrbit(const FSceneOrbitGeometry& Geometry, FColor lineColor, float thickness);

    static void DrawDebugOrbit(UWorld* World, const FSceneOrbitGeometry& Geometry, const FVector& SunLocation, double SceneScale, FColor lineColor, float thickness);

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

private:
    TMap<FString, class UOrbitingBodyComponent*> BodyMap;
    FStateVector __internal_ScenegraphOriginState;
    double __internal_ScenegraphOriginState_timestamp;

private:
    FFramePosition GetScenegraphOrigin();
    void UpdateScenegraphOrigin();
};


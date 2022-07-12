// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#include "OrbitViewerControllerComponent.h"
#include "OrbitalMechanics.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "OrbitSystemStateComponent.h"

using namespace gte;

void UOrbitViewerControllerComponent::BeginPlay()
{
    for (auto Component : GetWorld()->GetGameState()->GetComponents())
    {
        if (!GameState) GameState = Cast<UOrbitSystemStateComponent>(Component);
    }
    
    Super::BeginPlay();
}

void UOrbitViewerControllerComponent::AddBody(FString& BodyId, UOrbitingBodyComponent* Body)
{
    Bodies.Add(Body);
    BodyMap.Add(BodyId, Body);
}

void UOrbitViewerControllerComponent::SetFocus(FString& BodyId)
{

    if (BodyMap.Contains(BodyId))
    {
        FocusBody = BodyMap[BodyId];

        if (FocusBody)
        {
            // Everything else that's aleady been positioned is now using a stale position.
            // TODO: Fix this.
            UpdateScenegraphOrigin();
        }
    }
}


void UOrbitViewerControllerComponent::ComputeState(const FConicElements& ConicElements, double et, FSceneStateVector& StateVector, ES_ResultCode& ResultCode)
{
    FState State;
    UOrbitalMechanics::ComputeState(ConicElements, et, State, ResultCode);
    GetSceneStateVector(State.StateVector, StateVector);
}

void UOrbitViewerControllerComponent::ComputeGeometry(const FConicElements& ConicElements, FSceneOrbitGeometry& Geometry, ES_ResultCode& ResultCode)
{
    FOscullatingOrbitGeometry OscillatingGeometry;
    UOrbitalMechanics::ComputeGeometry(ConicElements, OscillatingGeometry, ResultCode);
    GetSceneGeometry(OscillatingGeometry, Geometry);
}

void UOrbitViewerControllerComponent::GetFramePosition(const FVector& ScenePosition, FFramePosition& FramePosition)
{
    FramePosition = GetScenegraphOrigin() + FFrameVector((1. / SceneScale) * Swizzle(ScenePosition));
}

void UOrbitViewerControllerComponent::GetFrameVector(const FVector& SceneVector, FFrameVector& FrameVector) const
{
    FrameVector = FFrameVector((1. / SceneScale) * Swizzle(SceneVector));
}

void UOrbitViewerControllerComponent::GetFrameDistance(double SceneDistance, double& FrameDistance)
{
    FrameDistance = (1. / SceneScale) * SceneDistance;
}

void UOrbitViewerControllerComponent::GetSceneStateVector(const FStateVector& StateVector, FSceneStateVector& SceneStateVector)
{
    GetScenePosition(StateVector.r, SceneStateVector.r);
}

void UOrbitViewerControllerComponent::GetScenePosition(const FFramePosition& Position, FVector& ScenePosition)
{
    ScenePosition = SceneScale * Swizzle(Position - GetScenegraphOrigin());
}

void UOrbitViewerControllerComponent::GetSceneDistance(double FrameDistance, double& SceneDistance)
{
    SceneDistance = SceneScale * FrameDistance;
}

void UOrbitViewerControllerComponent::GetSceneVector(const FFrameVector& Vector, FVector& SceneVector) const
{
    // Vectors represent directions etc and are not translated, as positions are above
    SceneVector = SceneScale * Swizzle(Vector);
}

void UOrbitViewerControllerComponent::GetSceneGeometry(const FOscullatingOrbitGeometry& OrbitGeometry, FSceneOrbitGeometry& SceneGeometry) const
{
    SceneGeometry.a = (float)OrbitGeometry.a * SceneScale;
    SceneGeometry.b = (float)OrbitGeometry.b * SceneScale;
    SceneGeometry.p_hat = Swizzle(OrbitGeometry.p_hat);
    SceneGeometry.q_hat = Swizzle(OrbitGeometry.q_hat);
    SceneGeometry.w_hat = Swizzle(OrbitGeometry.w_hat);
    SceneGeometry.ae = (float)OrbitGeometry.ae * SceneScale;
}

Vector3<double> UOrbitViewerControllerComponent::Swizzle(const FVector& value)
{
    return Vector3<double>{value.Y, value.X, value.Z};
}

FVector UOrbitViewerControllerComponent::Swizzle(const Vector3<double>& value)
{
    return FVector(value[1], value[0], value[2]);
}

FFramePosition UOrbitViewerControllerComponent::GetScenegraphOrigin()
{
    UpdateScenegraphOrigin();
    return __internal_ScenegraphOriginState.r;
}

void UOrbitViewerControllerComponent::UpdateScenegraphOrigin()
{
    double et = GameState->et;

    if (et != __internal_ScenegraphOriginState_timestamp)
    {
        memset(&__internal_ScenegraphOriginState, 0, sizeof(__internal_ScenegraphOriginState));

        FState State;
        ES_ResultCode ResultCode;
        if (!FocusBody->IsInertial)
        {
            UOrbitalMechanics::ComputeState(FocusBody->ConicElements, et, State, ResultCode);
            if (ResultCode == ES_ResultCode::Success)
            {
                __internal_ScenegraphOriginState = State.StateVector;
            }
        }

        __internal_ScenegraphOriginState_timestamp = et;
    }
}

void UOrbitViewerControllerComponent::GetActiveBodies(TArray<UOrbitingBodyComponent*>& ActiveBodies)
{
    BodyMap.GenerateValueArray(ActiveBodies);
}

void UOrbitViewerControllerComponent::DrawDebugOrbit(const FSceneOrbitGeometry& Geometry, FColor lineColor, float thickness)
{
#if defined(CONTROLLER_DRAW_DEBUG_ORBIT) && CONTROLLER_DRAW_DEBUG_ORBIT==1
    FVector center;
    FFramePosition origin = FFramePosition(Vector3<double>{0., 0., 0.});
    GetScenePosition(origin, center);
    DrawDebugOrbit(World, Geometry, center, SceneScale, lineColor, thickness);
#endif
}

void UOrbitViewerControllerComponent::DrawDebugOrbit(UWorld* World, const FSceneOrbitGeometry& Geometry, const FVector& SunLocation, double SceneScale, FColor lineColor, float thickness)
{
#if defined(CONTROLLER_DRAW_DEBUG_ORBIT) && CONTROLLER_DRAW_DEBUG_ORBIT==1
    APlayerCameraManager* cameraManager = UGameplayStatics::GetPlayerCameraManager(World, 0);
    if (cameraManager && 0)
    {
        FVector location = cameraManager->GetCameraLocation();

        FVector eyeToOrigin = SunLocation - location;

        float distanceToOrbitalPlane = FVector::DotProduct(eyeToOrigin, Geometry.w_hat);

        thickness *= (distanceToOrbitalPlane + 200) / 500;
    }

    FVector elipseCenter = -Geometry.ae * Geometry.p_hat;
    elipseCenter += SunLocation;

    for (float f = 0; f <= 360.f; f += 1.f)
    {
        float f0 = f * 3.14159f / 180;
        float f1 = (f + 1) * 3.14159f / 180;
        DrawDebugLine(
            World,
            elipseCenter + (Geometry.a * Geometry.p_hat * cos(f0) + Geometry.b * Geometry.q_hat * sin(f0)),
            elipseCenter + (Geometry.a * Geometry.p_hat * cos(f1) + Geometry.b * Geometry.q_hat * sin(f1)),
            lineColor,
            false, -1.f, 0, thickness
        );
    }
#endif
}

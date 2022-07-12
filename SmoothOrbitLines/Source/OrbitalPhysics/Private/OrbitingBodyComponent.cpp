// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com
// ----------------------------------------------------------------------------
// OrbitingBody.cpp
// Most of whatever an orbiting body does is implemented in its blueprint.
// ----------------------------------------------------------------------------


#include "OrbitingBodyComponent.h"
#include "GameFramework/GameStateBase.h"
#include "OrbitSystemStateComponent.h"
#include "DrawDebugHelpers.h"


// Sets default values
UOrbitingBodyComponent::UOrbitingBodyComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    BodyId = TEXT("EARTH");
    LineColor = FColor(255, 255, 0);
    DrawDebug = false;

    ConicElements.rp = 1.47095000e+08;
    ConicElements.ecc = 0.0167086;
    ConicElements.inc = 0.00005;
    ConicElements.lnode = -11.2604;
    ConicElements.argp = 114.20783;
    ConicElements.m0 = 358.617;
    ConicElements.mu = 1.3271244004193938e+11;
    ConicElements.et0 = 0;
}


// Called when the game starts or when spawned
void UOrbitingBodyComponent::BeginPlay()
{
    for (auto Component : GetWorld()->GetGameState()->GetComponents())
    {
        if(!GameState) GameState = Cast<UOrbitSystemStateComponent>(Component);
    }

    Super::BeginPlay();
}


// Called every frame
void UOrbitingBodyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if defined(BODY_DRAW_DEBUG) && BODY_DRAW_DEBUG==1
    if (DrawDebug)
    {
        AOrbitViewerController* controller = nullptr;

        for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
        {
            controller = Cast<AOrbitViewerController>(Iterator->Get());

            if (controller != nullptr) break;
        }

        if (controller != nullptr)
        {
            auto World = GetWorld();

            FSceneOrbitGeometry SceneGeometry;
            ES_ResultCode result;
            controller->ComputeGeometry(ConicElements, SceneGeometry, result);

            if (result == ES_ResultCode::Success)
            {
                FVector center = -SceneGeometry.ae * SceneGeometry.p_hat;

                FColor debugColor = LineColor;
                debugColor.R = (uint8)((float)debugColor.R * 0.75f);
                debugColor.G = (uint8)((float)debugColor.G * 0.75f);
                debugColor.B = (uint8)((float)debugColor.B * 0.75f);
                debugColor.A = 128;

                for (float th = 0; th <= 360; th += 1.f)
                {
                    float th1 = th / 180.f * PI;
                    float th2 = th1 + PI/180.f;

                    DrawDebugLine(
                        World,
                        center + SceneGeometry.a * cos(th1) * SceneGeometry.p_hat + SceneGeometry.b * sin(th1) * SceneGeometry.q_hat,
                        center + SceneGeometry.a * cos(th2) * SceneGeometry.p_hat + SceneGeometry.b * sin(th2) * SceneGeometry.q_hat,
                        debugColor,
                        false, -1.f, 0, 100.f
                    );
                }

                DrawDebugLine(
                    World,
                    center,
                    center + SceneGeometry.a * 1 * SceneGeometry.p_hat + SceneGeometry.b * 0 * SceneGeometry.q_hat,
                    debugColor,
                    false, -1.f, 0, 1000.f
                );

                DrawDebugLine(
                    World,
                    center,
                    center + SceneGeometry.a * 0 * SceneGeometry.p_hat + SceneGeometry.b * 1 * SceneGeometry.q_hat,
                    debugColor,
                    false, -1.f, 0, 1000.f
                );
            }
        }
    }
#endif
}





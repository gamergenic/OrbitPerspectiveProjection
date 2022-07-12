// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com
// ----------------------------------------------------------------------------
// OrbitingBody.cpp
// Most of whatever an orbiting body does is implemented in its blueprint.
// ----------------------------------------------------------------------------


#include "OrbitingBody.h"
#include "OrbitingBodyComponent.h"
#include "OrbitGameState.h"
#include "DrawDebugHelpers.h"


// Sets default values
AOrbitingBody::AOrbitingBody()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Planet Mesh");
    SetRootComponent(Mesh);

    OrbitingBody = CreateDefaultSubobject<UOrbitingBodyComponent>("Orbiting Body");

    radii = FVector(6378.1366f, 6378.1366f, 6356.7519f);
}


// Called when the game starts or when spawned
void AOrbitingBody::BeginPlay()
{
    GameState = Cast<AOrbitGameState>(GetWorld()->GetGameState());

    Super::BeginPlay();
}


// Called every frame
void AOrbitingBody::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

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





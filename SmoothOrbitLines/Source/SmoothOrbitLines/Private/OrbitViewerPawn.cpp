// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#include "OrbitViewerPawn.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "OrbitalMechanics.h"
#include "GameFramework/Pawn.h"
#include "OrbitGameState.h"
#include "ConicRendererComponent.h"
#include "OrbitingBodyComponent.h"
#include "OrbitingBody.h"

using namespace gte;

AOrbitViewerPawn::AOrbitViewerPawn()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Create Root
    USceneComponent* root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(root);

    // Create Camera Boom (pulls towards the player if there's a collision)
    CameraBoom = CreateDefaultSubobject<USceneComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());

    // Create Follow Camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom);
    // Attach the camera to the end of the boom and let the boom adjust to match
    // the controller orientation
    FollowCamera->bUsePawnControlRotation = false;

    OrbitProjector = CreateDefaultSubobject<UOrbitProjectorComponent>(TEXT("OrbitProjector"));
    ConicRenderer = CreateDefaultSubobject<UConicRendererComponent>(TEXT("ConicRenderer"));
    ConicRenderer->SetupAttachment(FollowCamera);

    CameraZoomSpeed = 0.1f;
}

// Called when the game starts or when spawned
void AOrbitViewerPawn::BeginPlay()
{
    Super::BeginPlay();

    // "Universe Model" sorta stuff.
    for (int i = 0; i < Bodies.Num(); i++)
    {
        BodyMap.Emplace(Bodies[i]->OrbitingBody->BodyId, Bodies[i]);
    }

    CameraTargetDistance = 0.f;

    CameraTargetPitch = -20;
    CameraTargetYaw = 0;
    bCameraMove = bCameraZoom = false;

    if (!FocusObject.IsEmpty())
    {
        SetFocus(FocusObject, SceneScale);
    }

    UpdateCameraValues();
}

// Called every frame
void AOrbitViewerPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector TargetPosition(0.f, 0.f, 0.f);
    SetActorLocation(TargetPosition);

    UpdateCameraValues();
}

// Called to bind functionality to input
void AOrbitViewerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    check(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("CameraYaw"), this, &AOrbitViewerPawn::CameraYawAxisInput);
    PlayerInputComponent->BindAxis(TEXT("CameraPitch"), this, &AOrbitViewerPawn::CameraPitchAxisInput);
    PlayerInputComponent->BindAxis(TEXT("CameraZoom"), this, &AOrbitViewerPawn::CameraZoomAxisInput);
    PlayerInputComponent->BindAction(TEXT("CameraMoveLock"), EInputEvent::IE_Pressed, this, &AOrbitViewerPawn::CameraMoveLockBegin);
    PlayerInputComponent->BindAction(TEXT("CameraMoveLock"), EInputEvent::IE_Released, this, &AOrbitViewerPawn::CameraMoveLockEnd);
    PlayerInputComponent->BindAction(TEXT("CameraZoomLock"), EInputEvent::IE_Pressed, this, &AOrbitViewerPawn::CameraZoomLockBegin);
    PlayerInputComponent->BindAction(TEXT("CameraZoomLock"), EInputEvent::IE_Released, this, &AOrbitViewerPawn::CameraZoomLockEnd);
}

void AOrbitViewerPawn::UpdateCameraValues()
{
    bCameraDirty = true;
    ValidateAndUpdateCamera();
}

void AOrbitViewerPawn::ValidateAndUpdateCamera()
{
    if (bCameraDirty)
    {
        CameraTargetDistance = FMath::Clamp(CameraTargetDistance, CameraZoomMinimum, CameraZoomMaximum);
        FVector currentArmLocation = FVector(-CameraTargetDistance, 0, 0);
        CameraBoom->SetRelativeLocation(currentArmLocation);

        CameraTargetPitch = FMath::Clamp(CameraTargetPitch, -85.f, 85.f);
        CameraTargetPitch = FMath::Clamp(CameraTargetPitch, -85.f, 85.f);
        FVector eulers = FVector(0, CameraTargetPitch, CameraTargetYaw);
        eulers.UnwindEuler();
        CameraTargetYaw = eulers.Z;
        SetActorRotation(FRotator::MakeFromEuler(eulers));
    }

    bCameraDirty = false;
}

void AOrbitViewerPawn::CameraZoomAxisInput(float Value)
{
    if (Value != 0)
    {
        CameraTargetDistance -= Value * CameraZoomSpeed * CameraTargetDistance;

        UpdateCameraValues();
    }
}


void AOrbitViewerPawn::CameraYawAxisInput(float Value)
{
    if (Value != 0 && bCameraMove)
    {
        CameraTargetYaw += Value;

        UpdateCameraValues();
    }
}


void AOrbitViewerPawn::CameraPitchAxisInput(float Value)
{
    if (Value != 0)
    {
        if (bCameraMove)
        {
            CameraTargetPitch += Value;

            UpdateCameraValues();
        }
        else if (bCameraZoom)
        {
            CameraZoomAxisInput(-Value);
        }
    }
}


void AOrbitViewerPawn::CameraMoveLockBegin()
{
    // If zooming, don't also move
    if (!bCameraZoom)
    {
        bCameraMove = true;
    }
}


void AOrbitViewerPawn::CameraMoveLockEnd()
{
    bCameraMove = false;
}

void AOrbitViewerPawn::CameraZoomLockBegin()
{
    // If moving, don't also zoomm
    if (!bCameraMove)
    {
        bCameraZoom = true;
    }
}

void AOrbitViewerPawn::CameraZoomLockEnd()
{
    bCameraZoom = false;
}


void AOrbitViewerPawn::SetFocus(FString& NewFocusId, double NewSceneScale)
{
    SceneScale = NewSceneScale;
    
    SaveCamera(FocusObject);

    FocusObject = NewFocusId;

    RestoreCamera(FocusObject, SceneScale);
    ValidateAndUpdateCamera();
}


void AOrbitViewerPawn::SaveCamera(FString& ObjectId)
{
    if (CameraTargetDistance > 0.f && BodyMap.Contains(ObjectId))
    {
        AOrbitingBody* Body = BodyMap[ObjectId];

        FVector Radii = Body->GetActorScale();
        float& Zoom = CameraZooms.FindOrAdd(Body->OrbitingBody->BodyId);
        Zoom = CameraTargetDistance / CameraZoomMinimum;
    }
}

void AOrbitViewerPawn::RestoreCamera(FString& ObjectId, double NewSceneScale)
{
    const float ZPlaneMagicNumber = 12.f;
    const float CameraZoomMaximumMagicNumber = 5570736.00;

    if (BodyMap.Contains(ObjectId))
    {
        AOrbitingBody* Body = BodyMap[ObjectId];
        FVector Radii = Body->GetActorScale();
        
        // Get no nearer than one diameter away from the planet.
        CameraZoomMinimum = 2.f * (float)NewSceneScale * Body->radii.GetMax() + ZPlaneMagicNumber;
        CameraZoomMaximum = CameraZoomMaximumMagicNumber;

        if (CameraZooms.Contains(Body->OrbitingBody->BodyId))
        {
            CameraTargetDistance = CameraZooms[Body->OrbitingBody->BodyId] * CameraZoomMinimum;
        }
    }
}


// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#include "OrbitViewerController.h"
#include "OrbitViewerControllerComponent.h"
#include "OrbitViewerPawn.h"
#include "OrbitalMechanics.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

using namespace gte;

AOrbitViewerController::AOrbitViewerController()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    OrbitViewerController = CreateDefaultSubobject<UOrbitViewerControllerComponent>("OrbitViewerController");
}

// Called when the game starts or when spawned
void AOrbitViewerController::BeginPlay()
{
    World = GetWorld();
    GameState = Cast<AOrbitGameState>(World->GetGameState());

    Super::BeginPlay();

    // "Universe Model" sorta stuff.
    for (int i = 0; i < Bodies.Num(); i++)
    {
        BodyMap.Emplace(Bodies[i]->OrbitingBody->BodyId, Bodies[i]);
        OrbitViewerController->AddBody(Bodies[i]->OrbitingBody->BodyId, Bodies[i]->OrbitingBody);
    }

    FBodyControllerDescription BodyDescription;
    GetFocus(FocusObject, 0, BodyDescription);
    SetFocus(BodyDescription);
}


void AOrbitViewerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    auto pawn = Cast<AOrbitViewerPawn>(InPawn);
    if (pawn)
    {
        pawn->SetFocus(FocusObject, OrbitViewerController->SceneScale);
    }
}

void AOrbitViewerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAction(TEXT("FocusNext"), EInputEvent::IE_Pressed, this, &AOrbitViewerController::FocusNext);
    InputComponent->BindAction(TEXT("FocusPrevious"), EInputEvent::IE_Pressed, this, &AOrbitViewerController::FocusPrevious);
    InputComponent->BindAction(TEXT("IncreaseTimeRate"), EInputEvent::IE_Pressed, this, &AOrbitViewerController::IncreaseTimeRate);
    InputComponent->BindAction(TEXT("DecreaseTimeRate"), EInputEvent::IE_Pressed, this, &AOrbitViewerController::DecreaseTimeRate);
    InputComponent->BindAction(TEXT("Quit"), EInputEvent::IE_Released, this, &AOrbitViewerController::QuitGame);
}


// Called every frame
void AOrbitViewerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}



void AOrbitViewerController::FocusNext()
{
    FBodyControllerDescription NewFocus;
    GetFocus(FocusObject, +1, NewFocus);
    SetFocus(NewFocus);
}

void AOrbitViewerController::FocusPrevious()
{
    FBodyControllerDescription NewFocus;
    GetFocus(FocusObject, -1, NewFocus);
    SetFocus(NewFocus);
}

void AOrbitViewerController::IncreaseTimeRate()
{
    GameState->TimeScalePowerValue += 10.f;
}

void AOrbitViewerController::DecreaseTimeRate()
{
    GameState->TimeScalePowerValue -= 10.f;
}

void AOrbitViewerController::SetFocus(FBodyControllerDescription& NewFocus)
{
    FocusObject = NewFocus.BodyId;
    FocusObjectDescription = NewFocus.BodyDescription;

    if (BodyMap.Contains(FocusObject))
    {
        FocusBody = BodyMap[FocusObject];

        if (FocusBody)
        {
            // Everything else that's aleady been positioned is now using a stale position.
            // TODO: Fix this.
            OrbitViewerController->SetFocus(FocusObject);
        }

        auto pawn = Cast<AOrbitViewerPawn>(this->GetPawn());
        if (pawn)
        {
            pawn->SetFocus(FocusObject, OrbitViewerController->SceneScale);
        }
    }
}

void AOrbitViewerController::GetFocus(const FString& InitialTarget, int offset, FBodyControllerDescription& NewTarget)
{
    // The target list may change at runtime, so search by name, don't cache an index
    int index;
    for (index = FocusList.Num() - 1; index >= 0; --index)
    {
        if (FocusList[index].BodyId == InitialTarget) break;
    }

    index += offset;

    // Unfortunately, wrapunders generate negative indices in c++
    if (index < 0) index += FocusList.Num();

    NewTarget = FocusList[index % FocusList.Num()];
}

void AOrbitViewerController::QuitGame()
{
    UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
}


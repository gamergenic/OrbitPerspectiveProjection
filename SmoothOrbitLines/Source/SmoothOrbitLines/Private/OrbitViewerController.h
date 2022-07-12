// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "OrbitGameState.h"
#include "OrbitingBody.h"

#include "GTE/Mathematics/Vector3.h"

#include "OrbitViewerController.generated.h"


USTRUCT(BlueprintType)
struct FBodyControllerDescription
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString BodyId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString BodyDescription;

    FBodyControllerDescription()
    {
        BodyId = FString();
        BodyDescription = FString();
    }

    FBodyControllerDescription(
        const FString& bodyId,
        const FString& bodyDescription
    )
    {
        BodyId = FString(bodyId);
        BodyDescription = FString(bodyDescription);
    }
};



UCLASS()
class AOrbitViewerController : public APlayerController
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    UWorld* World;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Orbit Scene View",
        Meta = (
            Tooltip = "FocusObject [\"EARTH BARYCENTER\", \"EARTH\", \"BODY399\", etc]"
            ))
    FString FocusObject;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadWrite,
        Category = "Orbit Scene View"
    )
    AOrbitingBody* FocusBody;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Orbit Scene View",
        Meta = (
            Tooltip = "Description Of FocusObject [\"Earth Barycenter\", \"Earth\", etc]"
            ))
    FString FocusObjectDescription;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit Scene View")
    TArray<FBodyControllerDescription> FocusList;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Orbit Scene View")
    AOrbitGameState* GameState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe", meta = (ToolTip = "Celestial Bodies found in the universe"))
    TArray<AOrbitingBody*> Bodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe", meta = (ToolTip = "Centimeters Per Kilometer"))
    class UOrbitViewerControllerComponent* OrbitViewerController;

public:
    // Sets default values for this pawn's properties
    AOrbitViewerController();

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    virtual void SetupInputComponent() override;

    virtual void OnPossess(APawn* InPawn) override;

private:
    TMap<FString, AOrbitingBody*> BodyMap;
    FStateVector __internal_ScenegraphOriginState;
    double __internal_ScenegraphOriginState_timestamp;

private:
    void FocusNext();
    void FocusPrevious();
    void IncreaseTimeRate();
    void DecreaseTimeRate();
    void GetFocus(const FString& initialTarget, int offset, FBodyControllerDescription& newTarget);
    void SetFocus(FBodyControllerDescription& NewFocus);
    void QuitGame();
};


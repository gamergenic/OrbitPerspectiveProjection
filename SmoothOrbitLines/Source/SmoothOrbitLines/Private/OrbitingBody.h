// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "OrbitGameState.h"
#include "OrbitingBody.generated.h"

UCLASS()
class AOrbitingBody : public AActor
{
    GENERATED_BODY()
    
public:	
    // Sets default values for this actor's properties
    AOrbitingBody();

    /*
    *   Assigned by OrbitingBody.cpp
    */
    UPROPERTY(BlueprintReadOnly, EditInstanceOnly)
    UStaticMeshComponent* Mesh;

    UPROPERTY(BlueprintReadOnly, EditInstanceOnly)
    AOrbitGameState* GameState;

    UPROPERTY(BlueprintReadOnly, EditInstanceOnly)
    class UOrbitingBodyComponent* OrbitingBody;

    /*
    *   Editable Values
    */
    UPROPERTY(EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Orbiting Body|Body",
        meta = (
            ToolTip = "Friendly Name"
            ))
    FString BodyName;

    UPROPERTY(EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Orbiting Body|Body",
        meta = (
            ToolTip = "Re/Re/Rp, Equatorial/Equatorial/Polar Body radius (Kilomenters, Earth is 6378/6378/6356)",
            ClampMin = "0"
            ))
    FVector radii;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;
};

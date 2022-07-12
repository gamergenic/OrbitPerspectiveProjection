// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once

// Workaround for:
// fatal error LNK1169: one or more multiply defined symbols found
// Caused by:
// The Epic GeometryProcessing->GeometryAlgorithms module also incorporates
// GTE, and some of the log methods are not inlined.
#include "GTE/Mathematics/Logger.h"
#undef LogError
#define LogError(message) \
    throw std::runtime_error(message)

#include <vector>
#include <utility>
#include "CoreMinimal.h"
#include "OrbitalMechanics.h"
#include "Conics/Conics.h"
#include "OrbitProjectorComponent.generated.h"

UENUM(BlueprintType)
enum class ES_ConicType : uint8
{
    None UMETA(DisplayName = "Ok"),
    Circle UMETA(DisplayName = "Circle"),
    Ellipse UMETA(DisplayName = "Ellipse"),
    Parabola UMETA(DisplayName = "Parabola"),
    PositiveHyperbola UMETA(DisplayName = "Positive Hyperbola"),
    NegativeHyperbola UMETA(DisplayName = "Negative Hyperbola")
};


USTRUCT(BlueprintType)
struct FConicSection
{
    GENERATED_BODY()
        
    ES_ConicType ConicType;

    FColor Color;
    FVector Center;
    FVector Axis1;
    FVector Axis2;
    FVector OrbitalPlaneCenter;
    FVector OrbitalPlaneNormal;
    float AdvancementState; // <- the 'theta' point along conic(theta) where the body should be visualized.

    TArray<FVector2D> Segments;
    TArray<FVector2D> TrueAnomalies;
};


USTRUCT(BlueprintType)
struct FOrbitItem
{
    GENERATED_BODY()

    ES_ConicType ConicType;
    FColor Color;
    
    FFramePosition Center;   // <- Center of ellipse, which is NOT the ellipse focus
    FFrameVector Axis1;
    FFrameVector Axis2;
    FFramePosition Focus;    // <- What is being orbited (Sun, etc)
    FFrameVector Normal;     // <- Orbital Plane Normal
    FState FrameState;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ORBITRENDERING_API UOrbitProjectorComponent : public UActorComponent
{
    GENERATED_BODY()


public:	
    // Sets default values for this component's properties
    UOrbitProjectorComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
    TArray<FOrbitItem> OrbitArray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View")
    float ProjectionPlaneDistance = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
    class UOrbitSystemStateComponent* OrbitSystemState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View")
    class UCameraComponent* OrbitCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View")
    class UOrbitViewerControllerComponent* OrbitViewerController;


public:

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void GetConics(TArray<FConicSection>& Conics);



protected:
    // Called when the game starts
    virtual void BeginPlay() override;


        
public:
//    UFUNCTION(BlueprintCallable)
    void ProjectToPlane(
        const FrustumParameters<double>& frustum,
        const FFramePosition& ellipseCenterPosition,
        const FFrameVector& ellipseAxis1Vector,
        const FFrameVector& ellipseAxis2Vector,
        const FFramePosition& eyePointPosition,
        const FFrameVector& eyeDirection,
        ProjectionType& projectionType,
        FFramePosition& projectedCenterPosition,
        FFrameVector& projectedAxis1Vector,
        FFrameVector& projectedAxis2Vector,
        std::vector<ConicSegment<double>>& segmentList,
        std::vector<ConicSegment<double>>& trueAnomalies,
        double& Advancement,
        double TestTheta
    );

private:
    bool CollectOrbit(class UOrbitingBodyComponent* ActiveBody, FOrbitItem& orbit);
    bool TransformOrbit(const FOrbitItem& orbit, FConicSection& conic);
};

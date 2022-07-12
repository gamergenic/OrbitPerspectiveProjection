// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once

#include "CoreMinimal.h"

#include "GTE/Mathematics/Vector3.h"

#include "OrbitViewerPawn.generated.h"


UCLASS()
class AOrbitViewerPawn : public APawn
{
    GENERATED_BODY()

public:
    /** Camera boom positioning behind the player */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USceneComponent* CameraBoom;

    /** Follow Camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UOrbitProjectorComponent* OrbitProjector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UConicRendererComponent* ConicRenderer;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View")
    float CameraZoomMinimum;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View")
    float CameraZoomMaximum;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "View")
    float CameraZoomSpeed;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe", meta = (ToolTip = "Celestial Bodies found in the universe"))
    TArray<class AOrbitingBody*> Bodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe", meta = (ToolTip = "Multiples of Minimum Zoom (~1 diameter)"))
    TMap<FString, float> CameraZooms;


public:
    // Sets default values for this pawn's properties
    AOrbitViewerPawn();

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


    UFUNCTION()
    void UpdateCameraValues();

    UFUNCTION()
    void SetFocus(FString& NewFocusId, double SceneScale);

private:
    bool bCameraMove;
    bool bCameraZoom;
    bool bCameraDirty;
    float CameraTargetDistance;
    float CameraTargetPitch;
    float CameraTargetYaw;
    double SceneScale;
    TMap<FString, class AOrbitingBody*> BodyMap;

    FString FocusObject;

private:
    void ValidateAndUpdateCamera();
    void CameraZoomAxisInput(float Value);
    void CameraYawAxisInput(float Value);
    void CameraPitchAxisInput(float Value);
    void CameraMoveLockBegin();
    void CameraMoveLockEnd();
    void CameraZoomLockBegin();
    void CameraZoomLockEnd();

    void SaveCamera(FString& ObjectId);
    void RestoreCamera(FString& ObjectId, double SceneScale);
};


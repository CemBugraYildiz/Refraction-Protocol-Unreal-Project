// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RefractionCharacter.generated.h"

class UCameraComponent;
class USphereComponent;
class UParticleSystem;
class UUserWidget;

UCLASS()
class REFRACTIONPROTOCOL_API ARefractionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARefractionCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// --- Movement ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Sensivity = 1.4f;

	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	float Forward = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	float Right = 0.f;

	void Movement();

	void OnMoveForwardPressed();
	void OnMoveForwardReleased();
	void OnMoveBackwardPressed();
	void OnMoveBackwardReleased();
	void OnMoveRightPressed();
	void OnMoveRightReleased();
	void OnMoveLeftPressed();
	void OnMoveLeftReleased();

	void OnMouseX(float AxisValue);
	void OnMouseY(float AxisValue);

	void OnJumpPressed();

	// --- Pause ---

	UPROPERTY(EditDefaultsOnly, Category = "Pause")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Pause")
	FName MainMenuLevelName = "MainMenu";

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PauseMenuWidgetInstance;

	void OnPausePressed();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void ResumeGame();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void ReturnToMainMenu();

	// --- Weapon / Laser ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 ShootType = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser")
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser")
	bool TraceComplex = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser", meta = (DisplayName = "Ignore This"))
	TArray<AActor*> IgnoreThis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser")
	TEnumAsByte<EDrawDebugTrace::Type> DebugDraw = EDrawDebugTrace::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser", meta = (DisplayName = "Mirror Materials"))
	TArray<UMaterialInterface*> MirrorMaterials;

	UPROPERTY(BlueprintReadWrite, Category = "Laser")
	FVector StartVec = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Laser")
	FVector EndVec = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser")
	float MaxNonReflectDistantion = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser")
	int32 MaxReflectCount = 7;

	UPROPERTY(BlueprintReadWrite, Category = "Laser")
	int32 CurrentReflectCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser")
	float LaserBoldness = 3.f;

	UPROPERTY(BlueprintReadWrite, Category = "Laser")
	bool LaserShootEnable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser")
	FVector LaserStartOffset = FVector(0.f, 0.f, 30.f);

	// Preserved from the original Blueprint variables (currently unused by any trace call,
	// kept for compatibility with anything that may still reference them by name).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser", meta = (DisplayName = "Trace Radius"))
	float TraceRadius = 16.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laser")
	TArray<AActor*> LaserIgnore;

	UPROPERTY(EditDefaultsOnly, Category = "Laser")
	TSubclassOf<AActor> LaserLineClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Laser")
	TObjectPtr<UParticleSystem> LaserHitEffect;

	// Class used to detect laser-redirector cubes (e.g. BP_LaserCube). Their custom
	// exit ray is computed in the CH_Actor Blueprint via OnLaserCubeHit, since that
	// actor's getReflectionVector function is Blueprint-only.
	UPROPERTY(EditDefaultsOnly, Category = "Laser")
	TSubclassOf<AActor> LaserCubeClass;

	void OnFirePressed();
	void OnFireReleased();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StandartShoot();

	UFUNCTION(BlueprintCallable, Category = "Laser")
	void LaserTick();

	UFUNCTION(BlueprintCallable, Category = "Laser")
	void Reflection();

	void SetLaserVisual(FVector Start, FVector End) const;

	// Hooks implemented in the CH_Actor Blueprint so the existing sound setup
	// (SoundControl custom event, FL_PuzzleLib data table lookups) stays untouched.
	UFUNCTION(BlueprintImplementableEvent, Category = "Audio")
	void OnLaserSoundControl(bool bPlay);

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio")
	void OnStandartShootSound();

	// Implemented in the CH_Actor Blueprint: casts HitActor to BP_LaserCube, calls its
	// getReflectionVector function, and writes the result into StartVec/EndVec.
	UFUNCTION(BlueprintImplementableEvent, Category = "Laser")
	void OnLaserCubeHit(AActor* HitActor);

private:
	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> CachedCamera;

	UPROPERTY(Transient)
	TObjectPtr<USphereComponent> CachedSphere;
};

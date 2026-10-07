// Copyright Epic Games, Inc. All Rights Reserved.

#include "RefractionCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

ARefractionCharacter::ARefractionCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
}

void ARefractionCharacter::BeginPlay()
{
	Super::BeginPlay();

	CachedCamera = FindComponentByClass<UCameraComponent>();
	CachedSphere = FindComponentByClass<USphereComponent>();
}

void ARefractionCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Movement();

	if (LaserShootEnable)
	{
		LaserTick();
	}
}

void ARefractionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::W, IE_Pressed, this, &ARefractionCharacter::OnMoveForwardPressed);
	PlayerInputComponent->BindKey(EKeys::W, IE_Released, this, &ARefractionCharacter::OnMoveForwardReleased);
	PlayerInputComponent->BindKey(EKeys::S, IE_Pressed, this, &ARefractionCharacter::OnMoveBackwardPressed);
	PlayerInputComponent->BindKey(EKeys::S, IE_Released, this, &ARefractionCharacter::OnMoveBackwardReleased);
	PlayerInputComponent->BindKey(EKeys::D, IE_Pressed, this, &ARefractionCharacter::OnMoveRightPressed);
	PlayerInputComponent->BindKey(EKeys::D, IE_Released, this, &ARefractionCharacter::OnMoveRightReleased);
	PlayerInputComponent->BindKey(EKeys::A, IE_Pressed, this, &ARefractionCharacter::OnMoveLeftPressed);
	PlayerInputComponent->BindKey(EKeys::A, IE_Released, this, &ARefractionCharacter::OnMoveLeftReleased);

	PlayerInputComponent->BindAxisKey(EKeys::MouseX, this, &ARefractionCharacter::OnMouseX);
	PlayerInputComponent->BindAxisKey(EKeys::MouseY, this, &ARefractionCharacter::OnMouseY);

	PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ARefractionCharacter::OnJumpPressed);

	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ARefractionCharacter::OnFirePressed);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ARefractionCharacter::OnFireReleased);

	// bExecuteWhenPaused: SetGamePaused(true) is active while the pause menu is open,
	// and UPlayerInput::ProcessInputStack drops key bindings during pause unless this is set,
	// so without it a second Escape/P press could never reach OnPausePressed to close the menu.
	PlayerInputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ARefractionCharacter::OnPausePressed).bExecuteWhenPaused = true;
	PlayerInputComponent->BindKey(EKeys::P, IE_Pressed, this, &ARefractionCharacter::OnPausePressed).bExecuteWhenPaused = true;
}

void ARefractionCharacter::OnPausePressed()
{
	if (IsValid(PauseMenuWidgetInstance) && PauseMenuWidgetInstance->IsInViewport())
	{
		ResumeGame();
		return;
	}

	if (!PauseMenuWidgetClass)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	if (!IsValid(PauseMenuWidgetInstance))
	{
		PauseMenuWidgetInstance = CreateWidget<UUserWidget>(PC, PauseMenuWidgetClass);
	}

	if (!PauseMenuWidgetInstance)
	{
		return;
	}

	PauseMenuWidgetInstance->AddToViewport();

	// GameAndUI (not UIOnly) so the P/Escape BindKey toggle in this class keeps
	// receiving key events while the pause menu is open, alongside UI button clicks.
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(PauseMenuWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);

	UGameplayStatics::SetGamePaused(this, true);
}

void ARefractionCharacter::ResumeGame()
{
	if (IsValid(PauseMenuWidgetInstance))
	{
		PauseMenuWidgetInstance->RemoveFromParent();
	}

	UGameplayStatics::SetGamePaused(this, false);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
}

void ARefractionCharacter::ReturnToMainMenu()
{
	if (IsValid(PauseMenuWidgetInstance))
	{
		PauseMenuWidgetInstance->RemoveFromParent();
	}

	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}

void ARefractionCharacter::Movement()
{
	if (!CachedCamera)
	{
		return;
	}

	AddMovementInput(CachedCamera->GetForwardVector(), Forward);
	AddMovementInput(CachedCamera->GetRightVector(), Right);
}

void ARefractionCharacter::OnMoveForwardPressed() { Forward += 1.f; }
void ARefractionCharacter::OnMoveForwardReleased() { Forward -= 1.f; }
void ARefractionCharacter::OnMoveBackwardPressed() { Forward -= 1.f; }
void ARefractionCharacter::OnMoveBackwardReleased() { Forward += 1.f; }
void ARefractionCharacter::OnMoveRightPressed() { Right += 1.f; }
void ARefractionCharacter::OnMoveRightReleased() { Right -= 1.f; }
void ARefractionCharacter::OnMoveLeftPressed() { Right -= 1.f; }
void ARefractionCharacter::OnMoveLeftReleased() { Right += 1.f; }

void ARefractionCharacter::OnMouseX(float AxisValue)
{
	if (!CachedCamera || FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	CachedCamera->AddRelativeRotation(FRotator(0.f, AxisValue * Sensivity, 0.f));
}

void ARefractionCharacter::OnMouseY(float AxisValue)
{
	if (!CachedCamera || FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	const FRotator CurrentRotation = CachedCamera->GetRelativeRotation();
	const float NewPitch = FMath::Clamp(CurrentRotation.Pitch + AxisValue * Sensivity, -70.f, 70.f);
	CachedCamera->SetRelativeRotation(FRotator(NewPitch, CurrentRotation.Yaw, 0.f));
}

void ARefractionCharacter::OnJumpPressed()
{
	Jump();
}

void ARefractionCharacter::OnFirePressed()
{
	switch (ShootType)
	{
	case 1: // Shot
		StandartShoot();
		LaserShootEnable = false;
		OnLaserSoundControl(false);
		break;
	case 2: // Laser
		LaserShootEnable = true;
		OnLaserSoundControl(true);
		break;
	default: // None
		LaserShootEnable = false;
		OnLaserSoundControl(false);
		break;
	}
}

void ARefractionCharacter::OnFireReleased()
{
	LaserShootEnable = false;
	OnLaserSoundControl(false);
}

void ARefractionCharacter::StandartShoot()
{
	if (!ProjectileClass || !CachedSphere || !CachedCamera || !GetWorld())
	{
		return;
	}

	FTransform SpawnTransform(CachedCamera->GetComponentRotation(), CachedSphere->GetComponentLocation(), FVector(0.25f));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTransform, SpawnParams);

	OnStandartShootSound();
}

void ARefractionCharacter::LaserTick()
{
	if (!CachedCamera || !GetWorld())
	{
		return;
	}

	const FVector CameraLocation = CachedCamera->GetComponentLocation();
	const FVector Muzzle = CameraLocation - LaserStartOffset;

	// Trace from the crosshair itself to find what it is actually aiming at, then
	// point the offset muzzle at that same spot. A plain parallel offset would only
	// converge with the crosshair at very long range, and visibly miss up close.
	FVector AimTarget = CameraLocation + CachedCamera->GetForwardVector() * MaxNonReflectDistantion;
	FHitResult CrosshairHit;
	const bool bCrosshairHit = UKismetSystemLibrary::LineTraceSingle(
		this, CameraLocation, AimTarget, TraceChannel, TraceComplex, IgnoreThis,
		EDrawDebugTrace::None, CrosshairHit, true);
	if (bCrosshairHit)
	{
		AimTarget = CrosshairHit.Location;
	}

	const FVector Direction = (AimTarget - Muzzle).GetSafeNormal();

	CurrentReflectCount = 1;
	StartVec = Muzzle;
	EndVec = Muzzle + Direction * MaxNonReflectDistantion;

	Reflection();
}

void ARefractionCharacter::Reflection()
{
	if (!GetWorld())
	{
		return;
	}

	FHitResult Hit;
	const bool bHit = UKismetSystemLibrary::LineTraceSingle(
		this, StartVec, EndVec, TraceChannel, TraceComplex, IgnoreThis,
		DebugDraw, Hit, true);

	SetLaserVisual(Hit.TraceStart, bHit ? Hit.Location : EndVec);

	if (!bHit)
	{
		return;
	}

	int32 SectionIndex = 0;
	UMaterialInterface* HitMaterial = Hit.Component.IsValid()
		? Hit.Component->GetMaterialFromCollisionFaceIndex(Hit.FaceIndex, SectionIndex)
		: nullptr;

	if (HitMaterial && MirrorMaterials.Contains(HitMaterial))
	{
		StartVec = Hit.Location;
		const FVector Reflected = UKismetMathLibrary::GetReflectionVector(Hit.Location - Hit.TraceStart, Hit.Normal);
		EndVec = Hit.Location + Reflected.GetSafeNormal() * MaxNonReflectDistantion;
	}
	else if (LaserCubeClass && Hit.GetActor() && Hit.GetActor()->IsA(LaserCubeClass))
	{
		OnLaserCubeHit(Hit.GetActor());
	}
	else
	{
		if (LaserHitEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, LaserHitEffect, Hit.Location);
		}
		return;
	}

	if (CurrentReflectCount < MaxReflectCount)
	{
		++CurrentReflectCount;
		Reflection();
	}
}

void ARefractionCharacter::SetLaserVisual(FVector Start, FVector End) const
{
	if (!LaserLineClass || !GetWorld())
	{
		return;
	}

	const FVector Direction = (End - Start).GetSafeNormal();
	const FRotator Rotation = UKismetMathLibrary::MakeRotFromX(Direction);
	const float Length = FVector::Dist(Start, End);

	const FVector LineScale(Length, LaserBoldness, LaserBoldness);
	FTransform SpawnTransform(Rotation, Start, LineScale);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	AActor* LaserLine = GetWorld()->SpawnActor<AActor>(LaserLineClass, SpawnTransform, SpawnParams);

	// Explicitly re-apply the scale after spawn so the visible length is correct
	// regardless of whatever the spawned actor's construction script or the
	// spawn-time scale method may have done to its root component's scale.
	if (LaserLine)
	{
		LaserLine->SetActorScale3D(LineScale);
	}
}

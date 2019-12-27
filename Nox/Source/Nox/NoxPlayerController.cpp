// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "NoxPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "NoxCharacter.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

#define ECC_CursorMovement ECC_GameTraceChannel1

ANoxPlayerController::ANoxPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;

	DistanceToChangeWalkingSpeed = 120.0f;
	WalkingSpeedPercentage = 0.25f;

	
}

void ANoxPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	// Find what is under cursor 
	GetHitResultUnderCursor(ECollisionChannel::ECC_CursorMovement, true, OUT HitUnderCursor);

	// Check if any montage is playing on controlled character and turn off character rotation during play time
	const bool bCanCharacterRotate = !GetCharacter()->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying();

	if (bCanCharacterRotate)
	{
		// Rotate front of the pawn to point at cursor
		RotatePawnToCursor();
	}
	
	
	
	
}

void ANoxPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();
		
	InputComponent->BindAxis("MoveForward", this, &ANoxPlayerController::MoveForward);
		
}

void ANoxPlayerController::RotatePawnToCursor()
{
	// Find new pawn rotation based on start location and target location.
	const FRotator NewPawnRotation = UKismetMathLibrary::FindLookAtRotation(GetPawn()->GetActorLocation(), HitUnderCursor.Location);

	// Set Yaw rotation to point at cursor
	GetPawn()->SetActorRotation(FRotator(0.0f, NewPawnRotation.Yaw, 0.0f));	
}

void ANoxPlayerController::MoveForward(float Value)
{	
	if (Value != 0.0f)
	{
		// Find front of a pawn  
		const FVector Direction = GetPawn()->GetActorForwardVector();		
		
		// Calculate distance between a pawn and a cursor
		const float DistanceToCursor = FVector::Dist(GetPawn()->GetActorLocation(), HitUnderCursor.Location);		

		if (DistanceToCursor < DistanceToChangeWalkingSpeed)
		{
			Value = WalkingSpeedPercentage;					
		}		
			
		GetPawn()->AddMovementInput(Direction, Value);		
	}	
}

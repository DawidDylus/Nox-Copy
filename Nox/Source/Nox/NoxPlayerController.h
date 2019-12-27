// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NoxPlayerController.generated.h"

UCLASS()
class ANoxPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANoxPlayerController();

protected:
	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

public:	

	void RotatePawnToCursor();

	void MoveForward(float Value);

	// Length of a line betwen pawn and a cursor to change walking speed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float DistanceToChangeWalkingSpeed;

	// Walking speed percentage based of Maximum speed - maximum 1 (100%)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float WalkingSpeedPercentage;

private:	

FHitResult HitUnderCursor;
	
	
};



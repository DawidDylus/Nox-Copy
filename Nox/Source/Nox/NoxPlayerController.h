// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "NoxPlayerController.generated.h"

UCLASS()
class ANoxPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ANoxPlayerController();

protected:
	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	
	virtual void SetupInputComponent() override;
	// End PlayerController interface

private:
	FGenericTeamId TeamId;

public:
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

public:	

	void RotatePawnToCursor();

	void MoveForward(float Value);

	bool CanCharacterRotate();

	// Length of a line betwen pawn and a cursor to change walking speed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float DistanceToChangeWalkingSpeed;

	// Walking speed percentage based of Maximum speed - maximum 1 (100%)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float WalkingSpeedPercentage;

private:
	bool bCanCharacterRotate;	

	FHitResult HitUnderCursor;
	
	
};



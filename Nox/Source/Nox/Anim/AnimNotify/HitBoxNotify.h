// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "HitBoxNotify.generated.h"


UENUM(BlueprintType)
enum class ECollisionPart : uint8
{
	CP_None			 UMETA(DisplayName = "None"),
	CP_RightHand	 UMETA(DisplayName = "Right Hand"),
	CP_LeftHand		 UMETA(DisplayName = "Left Hand")
};

//////////////////////////////////////////////////////////////////////////
//  UHitBoxNotify
//////////////////////////////////////////////////////////////////////////

UCLASS()
class NOX_API UHitBoxNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UHitBoxNotify(const FObjectInitializer& ObjectInitializer);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
		ECollisionPart CollisionPart;
	
	virtual void BranchingPointNotify(FBranchingPointNotifyPayload& BranchingPointPayload) override;

};

//////////////////////////////////////////////////////////////////////////
// UHitBoxNotifyWindow
//////////////////////////////////////////////////////////////////////////
/**
 * 
 */
UCLASS()
class NOX_API UHitBoxNotifyWindow : public UAnimNotifyState
{
	GENERATED_BODY()


public:
	UHitBoxNotifyWindow(const FObjectInitializer& ObjectInitializer);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
		ECollisionPart CollisionPart;

	virtual void BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	virtual void BranchingPointNotifyTick(FBranchingPointNotifyPayload& BranchingPointPayload, float FrameDeltaTime) override;
	virtual void BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	
	
};

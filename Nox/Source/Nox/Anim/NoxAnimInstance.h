// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NoxAnimInstance.generated.h"


/** Delegate called by 'HitBoxNotify'(not implemented yet) and 'HitBoxNotifyWindow' **/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHitBoxAnimNotifyDelegate, const ECollisionPart&, CollisionPart, const FBranchingPointNotifyPayload&, BranchingPointPayload);

/**
 * 
 */
UCLASS(Blueprintable)
class NOX_API UNoxAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public: 

	/** Called when a montage hits a 'HitBoxNotify' or 'HitBoxNotifyWindow' begin */	
	FHitBoxAnimNotifyDelegate OnHitBoxNotifyBegin;

	/** Called when a montage hits a 'HitBoxNotifyWindow' end */
	FHitBoxAnimNotifyDelegate OnHitBoxNotifyEnd;

	/** Called during whole notify state duration */
	FHitBoxAnimNotifyDelegate OnHitBoxNotifyTick;

};

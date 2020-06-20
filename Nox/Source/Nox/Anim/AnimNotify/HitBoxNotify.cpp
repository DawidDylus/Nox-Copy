// Fill out your copyright notice in the Description page of Project Settings.


#include "HitBoxNotify.h"
#include "Nox/Anim/NoxAnimInstance.h"


//////////////////////////////////////////////////////////////////////////
//  UHitBoxNotify
//////////////////////////////////////////////////////////////////////////

UHitBoxNotify::UHitBoxNotify(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsNativeBranchingPoint = true;
}

void UHitBoxNotify::BranchingPointNotify(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotify(BranchingPointPayload);

	if (USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent)
	{
		if (UNoxAnimInstance* NoxAnimInstance = Cast<UNoxAnimInstance>(MeshComp->GetAnimInstance()))
		{
			NoxAnimInstance->OnHitBoxNotifyBegin.Broadcast(CollisionPart, BranchingPointPayload);			
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// UHitBoxNotifyWindow
//////////////////////////////////////////////////////////////////////////

UHitBoxNotifyWindow::UHitBoxNotifyWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsNativeBranchingPoint = true;
}

void UHitBoxNotifyWindow::BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyBegin(BranchingPointPayload);	

	if (USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent)
	{
		if (UNoxAnimInstance* NoxAnimInstance = Cast<UNoxAnimInstance>(MeshComp->GetAnimInstance()))
		{			
			NoxAnimInstance->OnHitBoxNotifyBegin.Broadcast(CollisionPart, BranchingPointPayload);
		}
	}
}

void UHitBoxNotifyWindow::BranchingPointNotifyTick(FBranchingPointNotifyPayload& BranchingPointPayload, float FrameDeltaTime)
{
	Super::BranchingPointNotifyTick(BranchingPointPayload, FrameDeltaTime);

	if (USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent)
	{
		if (UNoxAnimInstance* NoxAnimInstance = Cast<UNoxAnimInstance>(MeshComp->GetAnimInstance()))
		{
			NoxAnimInstance->OnHitBoxNotifyTick.Broadcast(CollisionPart, BranchingPointPayload);
		}
	}
}

void UHitBoxNotifyWindow::BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyEnd(BranchingPointPayload);

	if (USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent)
	{
		if (UNoxAnimInstance* NoxAnimInstance = Cast<UNoxAnimInstance>(MeshComp->GetAnimInstance()))
		{
			NoxAnimInstance->OnHitBoxNotifyEnd.Broadcast(CollisionPart, BranchingPointPayload);
		}
	}
}

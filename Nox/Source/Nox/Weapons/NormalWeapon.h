// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "NormalWeapon.generated.h"


UCLASS()
class NOX_API ANormalWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	ANormalWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;	
	//

private:
	// Use AnimNotifyState to change CanDealDamage variable. Variable is changet to true at the begining and to false at the end of NotifyState.
	//*@warning To correctly use this function AnimNotifyState (PlayMontageNotifyWindow) have to be set in montage you are using it for. 
	void CanDealDamageByAnimNotifyState();
	

public:
	virtual void WeaponAttack() override;

	/** called when weapon hits something */
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalIMpulse, const FHitResult& Hit) override;

	/** called when weapon starts overlaps with something */	
	virtual void OnOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:



	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "MeleeWeapon.generated.h"

UCLASS()
class NOX_API AMeleeWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	AMeleeWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;	
	//		

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//

	virtual void OnWeaponAttackBegin() override;

	virtual void OnWeaponAttackEnd() override;
	
	/** called when weapon starts overlaps with something */	
	virtual void OnOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	
};

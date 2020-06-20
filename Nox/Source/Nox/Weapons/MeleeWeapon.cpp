// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"

AMeleeWeapon::AMeleeWeapon()
{

}

void AMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();

	// Its here insted of in the constructor, because this class dont know anything about children BP classes 
	GetWeaponMesh()->OnComponentBeginOverlap.AddDynamic(this, &AMeleeWeapon::OnOverlap); // set up a notification for when this component statrs overlaping with something 

	
	
}

void AMeleeWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

void AMeleeWeapon::OnWeaponAttackBegin()
{		
	// Activate function MeleeAttackBegins (function is used in with tick)
	bIsMeleeAttackActive = true;
}

void AMeleeWeapon::OnWeaponAttackEnd()
{
	// Activate function MeleeAttackEnd (function is used in with tick)
	bIsMeleeAttackActive = false;
}

void AMeleeWeapon::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != GetInstigator() && bCanDealDamage)
	{		
		float FinalDamage = CalculateFinalDamage(WeaponDamage, AttackDamageParams);		

		FDamageEvent DamageEvent;
	
		// check if actor that overlaps was not attacked
		if (!AttackedActorsWithWeapon.Contains(OtherActor) && OtherActor->CanBeDamaged())
		{
			OtherActor->TakeDamage(FinalDamage, DamageEvent, GetInstigatorController(), this);

			// Ignore an actor that tookk damage during this attack
			AttackedActorsWithWeapon.AddUnique(OtherActor);			
		}
				
	}
}

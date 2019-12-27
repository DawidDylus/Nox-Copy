// Fill out your copyright notice in the Description page of Project Settings.


#include "NormalWeapon.h"
#include "Animation/AnimInstance.h"

#include "Engine/Engine.h"

ANormalWeapon::ANormalWeapon()
{	
	
}

void ANormalWeapon::BeginPlay()
{
	Super::BeginPlay();

	
	
}

void ANormalWeapon::CanDealDamageByAnimNotifyState()
{
	FScriptDelegate OnNotifyBegin;
	OnNotifyBegin.BindUFunction(this, FName("OnStartDealingDamage"));
	OwnerAnimInstance->OnPlayMontageNotifyBegin.AddUnique(OnNotifyBegin);

	FScriptDelegate OnNotifyEnd;
	OnNotifyEnd.BindUFunction(this, FName("OnEndDealingDamage"));
	OwnerAnimInstance->OnPlayMontageNotifyEnd.AddUnique(OnNotifyEnd);
}

void ANormalWeapon::WeaponAttack()
{		
	if (OwnerAnimInstance != NULL)
	{		
		// Check if animation can be played without other animations overlaping 
		if (!OwnerAnimInstance->IsAnyMontagePlaying())
		{						
			// Play animation for attacking without weapon 
			OwnerAnimInstance->Montage_Play(WeaponAttackAnimMontage);			
			
			// Define when damage can be dealt during attack montage (AnimNotifyState for montage set in WeaponAttackAnimMontage).
			CanDealDamageByAnimNotifyState();


			FOnMontageEnded OnWeaponAttackMontageEnded;

			OnWeaponAttackMontageEnded.BindUFunction(this, FName("OnAttackEnds"), false);
			OwnerAnimInstance->Montage_SetEndDelegate(OnWeaponAttackMontageEnded, WeaponAttackAnimMontage);
			
		}
	}	
}


void ANormalWeapon::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalIMpulse, const FHitResult& Hit)
{
	//UE_LOG(LogTemp, Warning, TEXT("Test Hit Normal Weapon"));
	

	FString message = OtherActor->GetName();
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, message);
	//UE_LOG(LogTemp, Warning, TEXT("Instigator Normal Weapon %s"), GetInstigatorPropertyName());
	

	//InflictDamage(WeaponDamage, GetInstigatorController(), Hit);

}

void ANormalWeapon::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != GetInstigator() && bCanDealDamage && OtherActor->CanBeDamaged())
	{		
		FDamageEvent DamageEvent;
		if (AttackedActorsID.Num() == NULL)
		{
			// Add unique id of an actor to table when he was attacked during time that DealDamage is allowed (CanDealDamage variable).
			AttackedActorsID.AddUnique(OtherActor->GetUniqueID());

			OtherActor->TakeDamage(WeaponDamage, DamageEvent, GetInstigatorController(), this);			
		}
		else
		{
			// Deal Damage to actor only once during attack
			for (int i = 0; i < AttackedActorsID.Num(); i++)
			{
				if (OtherActor->GetUniqueID() == AttackedActorsID[i])
				{
					// Terminate loop
					break;
				}
				else if (i == AttackedActorsID.Num() - 1)
				{
					AttackedActorsID.AddUnique(OtherActor->GetUniqueID());					

					OtherActor->TakeDamage(WeaponDamage, DamageEvent, GetInstigatorController(), this);						
				}
			}
		}
		
		
		
		
		//InflictDamage(WeaponDamage, GetInstigatorController(), SweepResult);

		

			
		
	}
	



}

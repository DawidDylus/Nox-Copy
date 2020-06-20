// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;		

	// Can be damaged should be true only for characters and things that can be destroyed.
	SetCanBeDamaged(false);		

	// Create Weapon mesh component
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // Change default collision profile for OverlapAllDynamic to avoiding collision with Owner 		
	WeaponMesh->SetGenerateOverlapEvents(true);		
		
	// Set default value for variable, weapon can't deal damage only when attacking 
	bCanDealDamage = false;

	// Set default value for weapon damage
	WeaponDamage = 10.0f;

	// Set default value indicating if function for melee attack is active
	bIsMeleeAttackActive = false;
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	// Its here insted of in the constructor, because this class dont know anything about children BP classes made in Engine.
	//      WeaponMesh->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnOverlap); // set up a notification for when this component statrs overlaping with something 	
	//      WeaponMesh->OnComponentHit.AddDynamic(this, &ABaseWeapon::OnHit); // set up a notification for when this component hits something blocking		
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMeleeAttackActive)
	{
		MeleeAttackBegin();
	}	
	else
	{
		MeleeAttackEnd();
	}
}

float ABaseWeapon::CalculateFinalDamage(const float BaseDamage, const FAttackDamageParams& AttackDamageParams)
{			
	float FinalDamage;

	FinalDamage = BaseDamage + AttackDamageParams.AdditionalAttackDamage;

	if (AttackDamageParams.bOverrideAttackDamage)
	{
		FinalDamage = AttackDamageParams.OverridenAttackDamage;
	}		
	
	return FinalDamage;
}

template <typename UObjectTemplate>
void ABaseWeapon::CreateCollisionByPointLocation(UObjectTemplate* InEventInstigator, TArray<FHitResult>& OutHits, const FMeleeCollisionParams& InCollisionParams, const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToCollideWith, const TArray<AActor*> ActorsToIgnore, const TArray<FVector> InPointLocations)
{
	FVector StartLocation;
	FVector EndLocation;

	TArray<FHitResult> HitResults;	

	// Check if there is enought points to make a line
	if (InPointLocations.Num() > 1)
	{
		for (int i = 0; i + 1 < InPointLocations.Num(); i++)
		{		
			StartLocation = InPointLocations[i];
			
			EndLocation = InPointLocations[i + 1];			

			if (InCollisionParams.bUseForwardCollision)
			{
				StartLocation = InEventInstigator->GetActorLocation();
				EndLocation = StartLocation + (InEventInstigator->GetActorForwardVector() * InCollisionParams.AttackRange);
			}
			else
			{
				// Check if it's the last run through the loop
				if (i + 2 == InPointLocations.Num())
				{
					FVector ForwardVector = UKismetMathLibrary::GetDirectionUnitVector(StartLocation, EndLocation);

					// Change location of a last point in array to extend range  
					EndLocation += ForwardVector * InCollisionParams.AdditionalAttackRange;						
				}

				if (InCollisionParams.bUseUniversalHight)
				{
					
					StartLocation.Z = InEventInstigator->GetActorLocation().Z + InCollisionParams.ZValue;
					EndLocation.Z = InEventInstigator->GetActorLocation().Z + InCollisionParams.ZValue;
				}
			}			

			UKismetSystemLibrary::SphereTraceMultiForObjects(
				InEventInstigator,
				StartLocation,
				EndLocation,
				InCollisionParams.ColisionLineWidth,
				ObjectTypesToCollideWith,
				false,
				ActorsToIgnore,
				InCollisionParams.DrawDebugTrace,
				OUT HitResults,
				true
			);			
			
			OutHits += HitResults;					
		}		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Add more locations. Number of points(locations) must be more that one to create a line!"))
	}
}

void ABaseWeapon::MeleeAttackBegin()
{
	if (MeleeWeaponCollision.MeleeCollisionType == EMeleeCollisionType::MCT_CollisionByObject)
	{
		bCanDealDamage = true;
	}
	else if (MeleeWeaponCollision.MeleeCollisionType == EMeleeCollisionType::MCT_CollisionBySocketsLocations)
	{
		// Create array that contain sockets location. If attack does't use hands socket for collision HandCollisionSocketsLocation will be empty;
		TArray<FVector> CollisionSocketsLocations;		
		
		for (const auto& HandCollisionSocket : MeleeCollisionParams.CollisionSockets)
		{
			if (MeleeCollisionParams.OwnerMesh->DoesSocketExist(HandCollisionSocket))
			{
				FVector SocketLocation = MeleeCollisionParams.OwnerMesh->GetSocketLocation(HandCollisionSocket);
				CollisionSocketsLocations.AddUnique(SocketLocation);
			}
		}			

		// Add to array locations of a weapon sockets
		for (const auto& WeaponCollisionSocket : WeaponCollisionSockets)
		{
			if (GetWeaponMesh()->DoesSocketExist(WeaponCollisionSocket))
			{
				FVector SocketLocation = GetWeaponMesh()->GetSocketLocation(WeaponCollisionSocket);
				CollisionSocketsLocations.AddUnique(SocketLocation);
			}
		}

		// Include additional weapon range in attack collision params.	
		MeleeCollisionParams.AdditionalAttackRange += MeleeWeaponCollision.AdditionalWeaponRange;

		TArray<FHitResult> HitResults;

		CreateCollisionByPointLocation<AActor>(GetInstigator(), OUT HitResults, MeleeCollisionParams, ObjectTypesToCollideWithWeapon, AttackedActorsWithWeapon, CollisionSocketsLocations);

		float FinalDamage = CalculateFinalDamage(WeaponDamage, AttackDamageParams);

		FDamageEvent DamageEvent;
		
		for (const auto& Hit : HitResults)
		{
			if (Hit.GetActor()->CanBeDamaged() && !AttackedActorsWithWeapon.Contains(Hit.GetActor()))
			{
				AttackedActorsWithWeapon.AddUnique(Hit.GetActor());

				Hit.GetActor()->TakeDamage(FinalDamage, DamageEvent, GetInstigatorController(), this);				
			}			
		}
	}
}

void ABaseWeapon::MeleeAttackEnd()
{
	AttackedActorsWithWeapon.Empty();
	bCanDealDamage = false;
	MeleeCollisionParams.CollisionSockets.Empty();
}

void ABaseWeapon::OnWeaponAttackBegin()
{
}

void ABaseWeapon::OnWeaponAttackEnd()
{
}

void ABaseWeapon::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalIMpulse, const FHitResult& Hit)
{
}

void ABaseWeapon::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

#if WITH_EDITOR
bool ABaseWeapon::CanEditChange(const UProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);	

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(FMeleeWeaponCollision, AdditionalWeaponRange))
	{
		return ParentVal && MeleeWeaponCollision.MeleeCollisionType == EMeleeCollisionType::MCT_CollisionBySocketsLocations;
	}

	return ParentVal;
}
#endif




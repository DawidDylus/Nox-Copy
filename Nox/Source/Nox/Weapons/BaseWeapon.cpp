// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "Components/StaticMeshComponent.h"

#include "Engine/Engine.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetCanBeDamaged(false);	

	// Create Weapon mesh component
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // Change collision profile to OverlapAllDynamic to avoiding collision with Owner 		
	WeaponMesh->SetGenerateOverlapEvents(true);		
		
	// Set default value for variable, weapon cant deal damage when not attacking 
	bCanDealDamage = false;

	// Set default value for weapon damage
	WeaponDamage = 10.0f;

}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	// Its here insted of in the constructor, because this class dont know anything about children BP classes made in Engine.
	WeaponMesh->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnOverlap); // set up a notification for when this component statrs overlaping with something 	
	//WeaponMesh->OnComponentHit.AddDynamic(this, &ABaseWeapon::OnHit); // set up a notification for when this component hits something blocking		
}

void ABaseWeapon::OnAttackEnds()
{
	// Set bCanDealDamage to false in case that animation was interupted and OnPlayMontageNotifyEnd never triggered 	
	bCanDealDamage = false;	
	
	AttackedActorsID.Empty();
		
}

void ABaseWeapon::OnStartDealingDamage()
{
	bCanDealDamage = true;
}

void ABaseWeapon::OnEndDealingDamage()
{
	bCanDealDamage = false;

	AttackedActorsID.Empty();
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseWeapon::WeaponAttack()
{
}

void ABaseWeapon::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalIMpulse, const FHitResult& Hit)
{
	
}

void ABaseWeapon::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Test Overlap Base Weapon"));
}





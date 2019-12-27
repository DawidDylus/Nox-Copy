// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

UCLASS()
class NOX_API ABaseWeapon : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* WeaponMesh;
	
public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//

	// Animation montage to play while attacking
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Animation")
		class UAnimMontage* WeaponAttackAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
		float WeaponDamage;

	bool bCanDealDamage;

	// Array with Actors(ID) that had been damaged during time in one attack when damage could be dealt (CanDealDamage)
	TArray<uint32> AttackedActorsID;

public:
	class UAnimInstance* OwnerAnimInstance;	

	// Casted OwnerController
	class AController* OwnerController;	

	// UFUNCTION is required for delegate, Changing name will cause error for delegate (NormalWeapon.cpp Attack())
	UFUNCTION()
		void OnAttackEnds();

	// Function without parameters needed for delegate
	UFUNCTION()
		void OnStartDealingDamage();

	// Function without parameters needed for delegate
	UFUNCTION()
		void OnEndDealingDamage();
		
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;	

	virtual void WeaponAttack();	

public:

	/** called when weapon hits something */
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalIMpulse, const FHitResult& Hit);

	/** called when weapon starts overlaps with something */
	UFUNCTION()
	virtual void OnOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	FORCEINLINE class UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
};

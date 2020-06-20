// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Actor.h"
#include "Nox/Anim/AnimNotify/HitBoxNotify.h"
#include "Kismet/KismetSystemLibrary.h"
#include "BaseWeapon.generated.h"


UENUM(BlueprintType)
enum class EMeleeCollisionType : uint8
{	
	MCT_None						UMETA(DisplayName = "None"),
	// Collide with everything that overlaps this weapon
	MCT_CollisionByObject			UMETA(DisplayName = "Collision By Object"),
	MCT_CollisionBySocketsLocations	UMETA(DisplayName = "Collision By Sockets Locations")
};

USTRUCT(BlueprintType)
struct FAttackDamageParams
{
	GENERATED_BODY()

		// Add value to Base Attack Damage 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (EditCondition = "!bOverrideAttackDamage"))
		float AdditionalAttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayAfter = "AdditionalAttackDamage", InlineEditConditionToggle))
		uint8 bOverrideAttackDamage : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (EditCondition = "bOverrideAttackDamage"))
		float OverridenAttackDamage;
};

USTRUCT(BlueprintType)
struct FMeleeCollisionParams
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bUseTraceSocketsCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayAfter = "bUseTraceSocketsCollision", EditCondition = "!bUseForwardCollision"))
		float AdditionalAttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayAfter = "AdditionalAttackRange", EditCondition = "!bUseForwardCollision", InlineEditConditionToggle))
		uint8 bUseUniversalHight : 1;

	// 0 corresponds to center of an actor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayName = "Universal Hight", DisplayAfter = "bUseUniversalHight", EditCondition = "bUseUniversalHight"))
		float ZValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayAfter = "ZValue"))
		bool bUseForwardCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayAfter = "bUseForwardCollision", EditCondition = "bUseForwardCollision"))
		float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayAfter = "AttackRange"))
		TEnumAsByte<EDrawDebugTrace::Type> DrawDebugTrace;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayAfter = "DrawDebugTrace"))
		float ColisionLineWidth = 0.05f;

	// Mesh that contain those sockets
	USkeletalMeshComponent* OwnerMesh;

	TArray<FName> CollisionSockets;
};

USTRUCT(BlueprintType)
struct FMeleeWeaponCollision
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attributes")
		EMeleeCollisionType MeleeCollisionType;		

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attributes")
		float AdditionalWeaponRange;
};

///////////////////////////////////////////////////////////////////////////////////
/// ABaseWeapon CLASS
///////////////////////////////////////////////////////////////////////////////////

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

	bool bCanDealDamage;

	bool bIsMeleeAttackActive;

	// Array with Actors that had been damaged during time in one attack when damage could be dealt (CanDealDamage)
	TArray<AActor*> AttackedActorsWithWeapon;

public:

	// Tag used to determine which animation should be played for this weapon.
	//	e.g. Weapon have tag Sword.HeavyAttack. Montage with that tag will be played when character attack with a weapon.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Attributes")
		FGameplayTag WeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attributes")
		float WeaponDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attributes")
		FMeleeWeaponCollision MeleeWeaponCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Collision Sockets", Meta = (DisplayAfter = "bUseTraceSocketsCollision", EditCondition = "bUseTraceSocketsCollision"))
		TArray<FName> WeaponCollisionSockets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Collision Sockets", AdvancedDisplay)
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToCollideWithWeapon;	

public:	
	FAttackDamageParams AttackDamageParams;
	FMeleeCollisionParams MeleeCollisionParams;	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	static float CalculateFinalDamage(const float BaseDamage, const FAttackDamageParams& AttackDamageParams);	
	
	// TODO Fix Function
	// Use sockets locations to create line trace collision (SphereTraceMultiForObjects)
	
	template <typename UObjectTemplate>
	static void CreateCollisionByPointLocation(UObjectTemplate* InEventInstigator, TArray<FHitResult>& OutHits, const FMeleeCollisionParams& InCollisionParams, const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToCollideWith, const TArray<AActor*> ActorsToIgnore, const TArray<FVector> InPointLocations);

	UFUNCTION()
	virtual void MeleeAttackBegin();

	UFUNCTION()
	virtual void MeleeAttackEnd();

	UFUNCTION()
	virtual void OnWeaponAttackBegin();

	UFUNCTION()
	virtual void OnWeaponAttackEnd();

	/** called when weapon hits something */
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalIMpulse, const FHitResult& Hit);

	/** called when weapon starts overlaps with something */
	UFUNCTION()
	virtual void OnOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
		
	// Note CanEditChange is only available when compiling with the editor. You must add this or your builds might not work!
#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif

public:
	FORCEINLINE class UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

};

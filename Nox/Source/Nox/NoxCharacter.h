// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Character.h"
#include "Weapons/BaseWeapon.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NoxCharacter.generated.h"

USTRUCT(BlueprintType)
struct FUnarmedAttack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimMontage* Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FAttackDamageParams AttackDamageParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FMeleeCollisionParams MeleeCollisionParams;
};

USTRUCT(BlueprintType)
struct FWeaponAttack
{
	GENERATED_BODY()

	// Montage tag used to determine for which weapon this animation should be played.
	//	e.g. Weapon have tag Sword.HeavyAttack. Montage with that tag will be played when character attack with a weapon.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FGameplayTag Tag;	 

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimMontage* Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FAttackDamageParams AttackDamageParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta =(InlineEditConditionToggle))
		bool bOverrideMeleeCollisionParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayName = "Override Melee Collision Params", EditCondition = "bOverrideMeleeCollisionParams"))
		FMeleeCollisionParams MeleeCollisionParams;

	// Turn on collision for hands when attacking with weapon (only usable with weapon that use Trace Socket Collision)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayAfter = "MeleeCollisionParams", EditCondition = "bOverrideMeleeCollisionParams"))
		bool bUseHandCollisionWithWeaponAttack;
};


///////////////////////////////////////////////////////////////////////////////////
/// ANoxCharacter CLASS
///////////////////////////////////////////////////////////////////////////////////


UCLASS(Blueprintable)
class ANoxCharacter : public ACharacter
{
	GENERATED_BODY()

		/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UDecalComponent* CursorToWorld;

	/** Widget that contain information about character. Bar is floating above character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Floating Bar", meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* InformationBar;

	/** Perception stimuli for AI */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception Stimuli", meta = (AllowPrivateAccess = "true"))
		class UAIPerceptionStimuliSourceComponent* AIPerceptionStimuliSource;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception Senses", meta = (AllowPrivateAccess = "true"))
		class UAISense_Sight* AISight;

public:
	ANoxCharacter();

protected:
	// APawn interface	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	/**Material is used to change opaque material of static meshes blocking view on pawn.
	*@note - Set material reference in BP. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Visibility")
		UMaterialInterface* TranslucentMaterial;

	// Current Health and Mana values
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	float Health;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	float Mana;

	// Maximum Health and Mana values
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
		float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
		float MaxMana;

	// Percentage values of Health and Mana
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
		float HealthPercentage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
		float ManaPercentage;	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Collision Sockets")
		TArray<FName> RightHandCollisionSockets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Collision Sockets")
		TArray<FName> LeftHandCollisionSockets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Collision Sockets", AdvancedDisplay)
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToCollideWithHands;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unarmed Attack")
		float UnarmedDamage;

	// Chose iterator of array as a new default. If not specified first animation in array will be used (iterator 0)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unarmed Attack", meta = (ArrayClamp = "UnarmedAttacks", DisplayAfter = "UnarmedDamage"))
		int UnarmedAttackToUse;

	// Animation montages to play when attack without a weapon is done 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unarmed Attack")
		TArray<FUnarmedAttack> UnarmedAttacks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attack")
		TArray<FWeaponAttack> WeaponAttacks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equip Weapon")
		TSubclassOf<class ABaseWeapon> WeaponClassToEquip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equip Weapon")
		bool bCanWieldWeapon;

	 // Socket name to attach weapon to
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equip Weapon", Meta = (EditCondition = "bCanWieldWeapon"))
		FName WeaponGripPointSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equip Weapon", Meta = (EditCondition = "bCanWieldWeapon"))
		class UAnimMontage* EquipWeaponAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equip Weapon", Meta = (EditCondition = "bCanWieldWeapon"))
		float DelayTimeToEquipWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equip Weapon", Meta = (EditCondition = "bCanWieldWeapon"))
		class UAnimMontage* UnequipWeaponAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equip Weapon", Meta = (EditCondition = "bCanWieldWeapon"))
		float DelayTimeToUnequipWeapon;

	// Chose iterator of array as a new default. If not specified first animation in array will be used (iterator 0)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death", meta = (ArrayClamp = "DeathAnimMontages"))
		int DeathMontageToUse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death")
		TArray<class UAnimMontage*> DeathAnimMontages;				

	UPROPERTY(BlueprintReadOnly)
		bool bIsAttacking;

	UPROPERTY(BlueprintReadOnly)
		bool bIsAttackingWithHands;

	UPROPERTY(BlueprintReadOnly)
		bool bIsWeaponEquiped;

	UPROPERTY(BlueprintReadOnly)
		bool bCanAttack;

	UPROPERTY(BlueprintReadOnly)
		bool bIsAlive;

private:
	ABaseWeapon* EquippedWeapon;	

public:



private:
	// Return objects that cover camera view on pawn 
	void GetCameraViewPointCollisions(TArray<FHitResult>& OutHits);

	/** define new structure that stores object reference and its original material.
   *@note We need pointer to original material, because HitResult uses r-value. Using only HitResult to store original material, would lead to change of material reference from original to NewMaterial whenever material is changed, which is pointes for our needs.*/
	struct FObjectAndOriginalMaterial
	{
		const FHitResult Object;
		UMaterialInterface* OriginalMaterial; // pointer to original material		
	};
	// Internal array for ChangeMaterialOfCollidingObjects function. Array of colliding objects with changed material and pointer to original material 
	TArray<FObjectAndOriginalMaterial> ObjectsWithChangedMaterial;

	/**
	*@param CollidingObjects - Array of colliding objects that new material should be set to
	*@param NewMaterial - Pointer to material that colliding object should be changed to
	*@param bRestoreOriginalMaterial - Should restore original material if collision with object stop
	*/
	void ChangeMaterialOfCollidingObjects(const TArray<FHitResult>& CollidingObjects, UMaterialInterface* NewMaterial, bool bRestoreOriginalMaterial = false);
		
	void GetCollisionUnderCharacter(FHitResult& OutHit);

	// Variables needed in function ChangeChannelCollisionResponseWhileColliding()
	FHitResult LastHit;
	ECollisionResponse LastHitOriginalCollisionResponse;
	void ChangeChannelCollisionResponseWhileColliding(const FHitResult& InHit, const ECollisionChannel&& InChannel, const ECollisionResponse&& NewResponseDuringCollision);

	/** Calculate percentage based on current value and max value
   *@return - Value between 0 and 1
   */
	float CalculatePercentage(const float CurrentValue, const float MaxValue);

	UFUNCTION()
	void Ragdoll();

	float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;
		
	// Line trace single collision in front of this character in specified range (Raycast by channel) 
	void GetActorSingleFrontCollision(FHitResult& OutHitResult, const float RaycastRange, const ECollisionChannel CollisionChannel, const bool InbDrawRange = false);
	
	/** Play anim montage and call function delegate as effect in specified moment of the animation.
	*@param UObjectTemplate* InUserObject 
	*@param AnimMontageToPlay - Montage to play
	*@param DelayTimeForDealingDamage - Time in animation to call function.  (default - 0 sec)
	*@param InNameFunction - have to be defined with UFUNCTION macro
	*@param Vars... - variables that are passed to earlier specified function
	*@note function is protected from overlapping animation by IsAnyMontagePlaying()
	*@warning Function that is specified by name in InFunctionName, have to be public and defined with UFUNCTION macro for function to work as intended
	*/
	template <typename UObjectTemplate, typename... VarTypes>
	void PlayMontageWithSpecificEffect(UObjectTemplate* InUserObject, UAnimMontage* AnimMontageToPlay, const float DelayTimeToTriggerFunction, const FName& InFunctionName, VarTypes... Vars);	
	
	TArray<AActor*> AttackedActors;		
	FUnarmedAttack CurrentUnarmedAttack;
	FWeaponAttack CurrentWeaponAttack;
	/*
	* @return TArray<FVector> - Array of socket locations (sockets are in "Melee Collision Sockets" category)
	*/
	UFUNCTION()
	TArray<FName> GetSocketsByECollisionPart(const ECollisionPart& CollisionPart);	

	TArray<FName> CurrentHandCollisionSockets;
	UFUNCTION(BlueprintCallable)
		void Attack();

	void UnarmedAttack();

	// Function bind to HitBoxNotify
	UFUNCTION()
	void OnDealDamageBegin(const ECollisionPart& CollisionPart = ECollisionPart::CP_None);
	
	// Function bind to HitBoxNotify
	UFUNCTION()
		void OnDealDamageEnd(const ECollisionPart& CollisionPart = ECollisionPart::CP_None);

	// Internal function for equiping weapon
	UFUNCTION()
		void CreateWeapon();

	UFUNCTION()
		void DestroyWeapon();

	UFUNCTION(BlueprintCallable)
		void EquipWeapon();

public:
	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }
	/** Returns InformationBar subobject **/
	FORCEINLINE class UWidgetComponent* GetInformationBar() const { return InformationBar; }
	/** Returns AIPerceptionStimuliSource subobject **/
	FORCEINLINE class UAIPerceptionStimuliSourceComponent* GetAIPerceptionStimuliSource() const { return AIPerceptionStimuliSource; }
	/** Returns AISight subobject **/
	FORCEINLINE class UAISense_Sight* GetAISight() const { return AISight; }


};


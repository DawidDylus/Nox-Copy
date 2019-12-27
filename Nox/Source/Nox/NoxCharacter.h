// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NoxCharacter.generated.h"

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
		float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
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

	// Range of attacks 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
		float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
		float AttackDamage;

	// Animation montage to play when attack without a weapon is done 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Animation")
		class UAnimMontage* UnarmedStrikeAnimMontage;

	// Time in seconds that corresponds to moment the damage should be done
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Animation")
		float DelayTimeForUnarmedStrike;	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
		 TSubclassOf<class ABaseWeapon> WeaponClassToEquip;

private:
	bool bIsWeaponEquiped;

	ABaseWeapon* EquippedWeapon;		
	
public:

private:		
	// Return objects that cover camera view on pawn 
	 void GetCameraViewPointCollisions(TArray<FHitResult>& OutHits);	 

	 /** define new structure that stores object reference and its original material.
	*@note We need pointer to original material, because HitResult uses r-value. Using only HitResult to store original material, would lead to change of material reference from original to NewMaterial whenever material is changed, which is pointes for our needs.*/
	 struct ObjectAndOriginalMaterial
	 {
		 const FHitResult Object;
		 UMaterialInterface* OriginalMaterial; // pointer to original material		
	 };
	 // Internal array for ChangeMaterialOfCollidingObjects function. Array of colliding objects with changed material and pointer to original material 
	 TArray<ObjectAndOriginalMaterial> ObjectsWithChangedMaterial;

	 /** 
	 *@param CollidingObjects - Array of colliding objects that new material should be set to
	 *@param NewMaterial - Pointer to material that colliding object should be changed to
	 *@param bRestoreOriginalMaterial - Should restore original material if collision with object stop
	 */
	 void ChangeMaterialOfCollidingObjects(const TArray<FHitResult>& CollidingObjects, UMaterialInterface* NewMaterial, bool bRestoreOriginalMaterial = false);
	
	 /** Calculate percentage based on current value and max value
	*@return - Value between 0 and 1
	*/
	 float CalculatePercentage(const float CurrentValue, const float MaxValue);

	 float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	 // Line trace single collision in front of this character in specified range (Raycast by channel) 
	 void GetActorSingleFrontCollision(FHitResult& OutHitResult, const float RaycastRange, const ECollisionChannel CollisionChannel);	

	 /** Unanimated attack to front of this character
	 *@note Name of this function is used in the delegate in UnarmedStrike. This delegate reference will not be change automatically
	 */
	 UFUNCTION()
	 void DealDamageForward(const float Damage, const float Range);

	 /** Play attack animation and deal basic damage to actors in front of this character
	 *@param AnimMontageToPlay - Montage to play 
	 *@param DelayTimeForDealingDamage - Delay time for raycast to fire. Use to match animation and dealing damage.  (default - 0 sec)
	 */
	 void UnarmedAttack(UAnimMontage* AnimMontageToPlay, const float DelayTimeForDealingDamage, float InAttackDamage, float InAttackRange);

	 void Attack();

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
};


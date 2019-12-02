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

public:
	ANoxCharacter();		

protected:
	// APawn interface	
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	/**Material is used to change opaque material of static meshes blocking view on pawn.
	*@note - Set material reference in BP. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Visibility")
	UMaterialInterface* TranslucentMaterial;	

private:		
	// Return objects that cover camera view on pawn 
	 void CameraViewPointCollisions(TArray<FHitResult>& OutHits);	 

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
	
public:
	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }
};


// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "NoxCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

#define ECC_CameraView ECC_GameTraceChannel2

ANoxCharacter::ANoxCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->RelativeRotation = FRotator(-60.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm	

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;		
	
}

void ANoxCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);	

	
	TArray<FHitResult> CameraViewHits;
	CameraViewPointCollisions(OUT CameraViewHits);	
		
	ChangeMaterialOfCollidingObjects(CameraViewHits, TranslucentMaterial, true);
	
}

void ANoxCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

}

void ANoxCharacter::CameraViewPointCollisions(TArray<FHitResult>& OutHits)
{	
	// Find Character's CameraComponent StartLocation
	const FVector StartLocation = TopDownCameraComponent->GetComponentLocation();
	// Find Character's CameraComponent EndLocation where lenght of ray corresponds to CameraBoom arm length
	const FVector EndLocation = (StartLocation + (TopDownCameraComponent->GetForwardVector() * CameraBoom->TargetArmLength));	

	// Collisions for subcalsses of APawn are ignored by default in DefaultEngine.ini for ECC_CameraView channel. Default ECC_CameraView response is overlap  
	GetWorld()->LineTraceMultiByChannel(OutHits, StartLocation, EndLocation, ECollisionChannel::ECC_CameraView);	
}

void ANoxCharacter::ChangeMaterialOfCollidingObjects(const TArray<FHitResult>& CollidingObjects, UMaterialInterface* NewMaterial, bool bRestoreOriginalMaterial)
{
/// Set new material, add elements to array

	// For loop works only, if there are collisions.   
	for (int i = 0; i < CollidingObjects.Num(); i++)
	{	
		if (NewMaterial != NULL)
		{			
			// Change material only once
			if (CollidingObjects[i].GetComponent()->GetMaterial(0)->GetName() != NewMaterial->GetName())
			{				
				// Add elements to array only if Origin material should be restored.
				if(bRestoreOriginalMaterial)
				{
					// Object with changed material and pointer to its original material 
					ObjectAndOriginalMaterial TempObjectAndMaterial{ CollidingObjects[i], CollidingObjects[i].GetComponent()->GetMaterial(0) };
					// Add element to array
					ObjectsWithChangedMaterial.Push(TempObjectAndMaterial);
				}							

				// Set new translucent material
				CollidingObjects[i].GetComponent()->SetMaterial(0, NewMaterial);
			}
		}
		else
		{
			// error code
			UE_LOG(LogTemp, Warning, TEXT("(ChangeMaterialWhileColliding) Pointer to NewMaterial is not found!"));
			return;
		}
	}	
/// Restore original material, remove elements from array

	// Check if any object stopped colliding. If bRestoreOriginalMaterial is false, this condition is also false (ObjectsWithChangedMaterial always 0).
	if (CollidingObjects.Num() < ObjectsWithChangedMaterial.Num())
	{
		if (CollidingObjects.Num() == NULL)
		{
			for (int k = 0; k < ObjectsWithChangedMaterial.Num(); k++)
			{
				ObjectsWithChangedMaterial[k].Object.GetComponent()->SetMaterial(0, ObjectsWithChangedMaterial[k].OriginalMaterial);
			}
			ObjectsWithChangedMaterial.Empty();
		}
		else
		{
			for (int i = 0; i < ObjectsWithChangedMaterial.Num(); i++)
			{
				for (int j = 0; j < CollidingObjects.Num(); j++)
				{
					//Check if camera view is still blocked and object should stay translucent
					if (ObjectsWithChangedMaterial[i].Object.GetActor()->GetName() == CollidingObjects[j].GetActor()->GetName())
					{
						// Terminate the loop [j] 
						break;
					}
					// If camera view is not blocked and all posibilities were checked, change object material from translucent back to original
					else if (j == CollidingObjects.Num() - 1)
					{
						ObjectsWithChangedMaterial[i].Object.GetComponent()->SetMaterial(0, ObjectsWithChangedMaterial[i].OriginalMaterial);				

						// remove element from array 
						ObjectsWithChangedMaterial.RemoveAt(i);							
					}
				}
			}
		}
	}
}



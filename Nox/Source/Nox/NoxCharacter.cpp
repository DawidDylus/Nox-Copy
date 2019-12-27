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
#include "Components/WidgetComponent.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Weapons/BaseWeapon.h"

#include "Engine/Engine.h"

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
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
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

	// Create widget above character
	InformationBar = CreateDefaultSubobject<UWidgetComponent>("Information Bar");
	InformationBar->SetupAttachment(RootComponent);
	InformationBar->SetRelativeLocation(FVector(0.0f, 0.0f, 160.0f));
	InformationBar->SetDrawSize(FVector2D(120.0f, 500.0f));
	InformationBar->SetWidgetSpace(EWidgetSpace::Screen);
	
	// Set default attributes
	MaxHealth = 100.f;
	Health = MaxHealth;
	MaxMana = 100.0f;
	Mana = MaxMana;

	// Set health percentage	
	HealthPercentage = CalculatePercentage(Health, MaxHealth);
	ManaPercentage = CalculatePercentage(Mana, MaxMana);	

	// Set default attack parameters
	AttackDamage = 25.f;
	AttackRange = 125.f;	
	DelayTimeForUnarmedStrike = 0.f;
	SetCanBeDamaged(true);
	bIsWeaponEquiped = false;	

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;		
		
}


void ANoxCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	

}

void ANoxCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);		
	
	TArray<FHitResult> CameraViewHits;
	GetCameraViewPointCollisions(OUT CameraViewHits);	
		
	ChangeMaterialOfCollidingObjects(CameraViewHits, TranslucentMaterial, true);	


}

void ANoxCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ANoxCharacter::Attack);
	PlayerInputComponent->BindAction("EquipWeapon", IE_Pressed, this, &ANoxCharacter::EquipWeapon);

}

void ANoxCharacter::GetCameraViewPointCollisions(TArray<FHitResult>& OutHits)
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
		
		TSubclassOf<APawn> APawnSubclass;
		TSubclassOf<APawn> CollidingObjectOwnerClass = TSubclassOf<APawn>(CollidingObjects[i].GetActor()->GetOwner()->GetClass());

		// Ignore actors that have Owner of APawn subclass 
		if (CollidingObjectOwnerClass != APawnSubclass)
		{
			if (NewMaterial != NULL)
			{
				// Change material only once
				if (CollidingObjects[i].GetComponent()->GetMaterial(0)->GetName() != NewMaterial->GetName())
				{
					// Add elements to array only if Origin material should be restored.
					if (bRestoreOriginalMaterial)
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
				// Error code
				UE_LOG(LogTemp, Warning, TEXT("(Function: ChangeMaterialWhileColliding) - Pointer to NewMaterial is not found!"));
				return;
			}
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

float ANoxCharacter::CalculatePercentage(const float CurrentValue, const float MaxValue)
{
	const float Percentage = CurrentValue / MaxValue;
	
	return Percentage;
}

float ANoxCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Call the base class - this will tell us how much damage to apply  
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f && Health > 0.f)
	{
		// Specify variable for health and apply damage.
		Health -= ActualDamage;

		// Update health procentage for correct operation of widgets.
		HealthPercentage = CalculatePercentage(Health, MaxHealth);		
	
		// If the damage depletes our health set our lifespan to zero - which will destroy the actor  
		//if (Health <= 0.f)
		//{
			// TODO if actor died play dead animation, else if static mesh destroyed change lifespawn and spawn debris of 
			//SetLifeSpan(0.001f);
		//}		
		
	}

	return ActualDamage;
}

void ANoxCharacter::GetActorSingleFrontCollision(FHitResult& OutHitResult, const float RaycastRange, const ECollisionChannel CollisionChannel)
{
	// Find Character's Character StartLocation
	const FVector StartLocation = GetActorLocation();
	// Find Character's EndLocation 
	const FVector EndLocation = (StartLocation + (GetActorForwardVector() * RaycastRange));

	// Ignore actor instigating the event
	FCollisionQueryParams QueryParams;	
	QueryParams.AddIgnoredActor(GetUniqueID());

	// Line trace for collision  
	GetWorld()->LineTraceSingleByChannel(OutHitResult, StartLocation, EndLocation, CollisionChannel, QueryParams);

	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 200, 1, 3);
}

void ANoxCharacter::DealDamageForward(const float Damage, const float InRange)
{
	FHitResult Hit;
	GetActorSingleFrontCollision(OUT Hit, InRange, ECollisionChannel::ECC_Camera);

	// Check if attack hit something and it can be damaged
	if (Hit.bBlockingHit && Hit.GetActor()->CanBeDamaged())
	{
		FDamageEvent DamageEvent;

		// Call function TakeDamage on Target
		Hit.GetActor()->TakeDamage(Damage, DamageEvent, GetController(), this);
	}
}

void ANoxCharacter::UnarmedAttack(UAnimMontage* AnimMontageToPlay, const float DelayTimeForDealingDamage, float InAttackDamage, float InAttackRange)
{
	if (AnimMontageToPlay != NULL)
	{
		// Create Anim Instance used to play animations
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance != NULL)
		{
			// Check if any montage is currently playing (To prevent overlaping animation)
			if (!AnimInstance->IsAnyMontagePlaying())
			{
				// Play animation for attacking without weapon 
				AnimInstance->Montage_Play(AnimMontageToPlay);

				// Create Timer Handle for animation montage
				FTimerHandle InflictDamageTimerHandle;
				
				// if delay time for calling unarmed strike is 0 timer would not call function at all
				if (DelayTimeForDealingDamage > 0)
				{					
					FTimerDelegate StrikeForwardDelegate;
					StrikeForwardDelegate.BindUFunction(this, FName("DealDamageForward"), InAttackDamage, InAttackRange);

					GetWorldTimerManager().SetTimer(InflictDamageTimerHandle, StrikeForwardDelegate, DelayTimeForDealingDamage, false);					
				}
				else
				{
					DealDamageForward(InAttackDamage, InAttackRange);
				}
			}
		}
	}
	else
	{
		// Error code
		UE_LOG(LogTemp, Warning, TEXT("Pointer to UnarmedStrikeAnimMontage is not found!"));
	}
}

void ANoxCharacter::Attack()
{		
	if (bIsWeaponEquiped && EquippedWeapon != NULL)
	{		
		// Attack with equipped weapon		
		if (EquippedWeapon != NULL)
		{
			EquippedWeapon->WeaponAttack();
		}
	}
	else 
	{			
		// Unarmed Attack 
		UnarmedAttack(UnarmedStrikeAnimMontage, DelayTimeForUnarmedStrike, AttackDamage, AttackRange);		
	}	
}

void ANoxCharacter::EquipWeapon()
{	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance !=NULL)
	{
		// Check if any montage is currently playing (To prevent overlaping animation)
		if (!AnimInstance->IsAnyMontagePlaying())
		{
			if (!bIsWeaponEquiped)
			{
				UWorld* World = GetWorld();
				if (World != NULL)
				{
					// Set SpawnParams
					FActorSpawnParameters SpawnParams;
					SpawnParams.Instigator = GetInstigator();
					SpawnParams.Owner = GetOwner();

					EquippedWeapon = World->SpawnActor<ABaseWeapon>(WeaponClassToEquip, SpawnParams);
					EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "RightHandGripPoint");

					// Pass anim instance to the weapon, to keep required animation in weapon calling it.
					EquippedWeapon->OwnerAnimInstance = GetMesh()->GetAnimInstance();										

					// Ignore physical collision while moving?
					MoveIgnoreActorAdd(EquippedWeapon);
					
					

					bIsWeaponEquiped = true;
				}
			}
			else
			{
				if (EquippedWeapon != NULL)
				{
					EquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);

					EquippedWeapon->Destroy();
					bIsWeaponEquiped = false;
					
				}
			}
		}
	}
}




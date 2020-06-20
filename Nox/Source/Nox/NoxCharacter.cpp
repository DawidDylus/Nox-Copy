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
#include "Nox/Anim/NoxAnimInstance.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "GameplayTagsManager.h"

#include "Engine/Engine.h"

#define ECC_CursorMovement ECC_GameTraceChannel1 
#define ECC_CameraView ECC_GameTraceChannel2
#define ECC_Ground ECC_GameTraceChannel3

ANoxCharacter::ANoxCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	// GetCharacterMovement()->bOrientRotationToMovement = false; // Rotate character to moving direction
	GetCharacterMovement()->bUseControllerDesiredRotation = true; // Smothly rotate character towards the controller's desired location
	GetCharacterMovement()->RotationRate = FRotator(0.f, 470.f, 0.f);
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
	InformationBar->SetWindowVisibility(EWindowVisibility::Visible);
	
	// Create AI stimuli source
	AIPerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("AIPerceptionStimuliSource");	
	AISight = CreateDefaultSubobject<UAISense_Sight>("AISight");
	AIPerceptionStimuliSource->RegisterForSense(AISight->GetClass());
	AIPerceptionStimuliSource->bAutoRegister = true;
	
	// Set default attributes
	MaxMana = 100.0f;
	MaxHealth = 100.f;	

	// Set default attack parameters
	UnarmedDamage = 25.f;

	DeathMontageToUse = 0;
	
	bIsAlive = true;
	SetCanBeDamaged(true);
	bIsWeaponEquiped = false;	
	bCanAttack = true;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;				
}

void ANoxCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Default values that have to be calculated by BP children of which counctructor does not know anything. Thats why those variables have to be here.
	Health = MaxHealth;
	Mana = MaxMana;	
	// Set health percentage
	HealthPercentage = CalculatePercentage(Health, MaxHealth);
	ManaPercentage = CalculatePercentage(Mana, MaxMana);

	// Bind function to Hitbox notify delegates
	UNoxAnimInstance* NoxAnimInstance = Cast<UNoxAnimInstance>(GetMesh()->GetAnimInstance());
	if (NoxAnimInstance != NULL)
	{
		FScriptDelegate OnNotifyBegin;
		OnNotifyBegin.BindUFunction(this, FName("OnDealDamageBegin"));
		NoxAnimInstance->OnHitBoxNotifyBegin.AddUnique(OnNotifyBegin);

		FScriptDelegate OnNotifyEnd;
		OnNotifyEnd.BindUFunction(this, FName("OnDealDamageEnd"));
		NoxAnimInstance->OnHitBoxNotifyEnd.AddUnique(OnNotifyEnd);		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("(Binding Delegate for HitBoxNotify Failed! Cast To UNoxAnimInstance is NULL!"));
	}
	
}

void ANoxCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);		
	
	if (GetController()->IsPlayerController())
	{
		TArray<FHitResult> CameraViewHits;
		GetCameraViewPointCollisions(OUT CameraViewHits);
		ChangeMaterialOfCollidingObjects(CameraViewHits, TranslucentMaterial, true);

		FHitResult HitUnderCharacter;
		GetCollisionUnderCharacter(OUT HitUnderCharacter);
		ChangeChannelCollisionResponseWhileColliding(HitUnderCharacter, ECollisionChannel::ECC_CursorMovement, ECollisionResponse::ECR_Block);
	}

	if (bIsAttackingWithHands)
	{
		UnarmedAttack();
	}	
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
	
	FCollisionQueryParams QueryParams;	

	// Ignore all objects with non static/stationary mobility type (ignore movable mobility type) 
	QueryParams.MobilityType = EQueryMobilityType::Static;

	// Collisions for subcalsses of APawn are ignored by default in DefaultEngine.ini for ECC_CameraView channel. Default ECC_CameraView response is overlap  
	GetWorld()->LineTraceMultiByChannel(OutHits, StartLocation, EndLocation, ECollisionChannel::ECC_CameraView, QueryParams);
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
				if (bRestoreOriginalMaterial)
				{
					// Object with changed material and pointer to its original material 
					FObjectAndOriginalMaterial TempObjectAndMaterial{ CollidingObjects[i], CollidingObjects[i].GetComponent()->GetMaterial(0) };
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

void ANoxCharacter::GetCollisionUnderCharacter(FHitResult& OutHit)
{
	// Find Character Location
	const FVector StartLocation = GetActorLocation();
	// Find LineTrace EndLocation, object under character
	const FVector EndLocation = (StartLocation + ((GetActorForwardVector() + FVector(0.f, 0.f, -90.f)) * 1.25f));

	// Collisions for subcalsses of APawn are ignored by default in DefaultEngine.ini for ECC_Ground channel. Default ECC_Ground response is block  
	GetWorld()->LineTraceSingleByChannel(OutHit, StartLocation, EndLocation, ECollisionChannel::ECC_Ground);	
}

void ANoxCharacter::ChangeChannelCollisionResponseWhileColliding(const FHitResult& InHit, const ECollisionChannel&& InChannel, const ECollisionResponse&& NewResponseDuringCollision)
{	
	if (LastHit.GetActor() != NULL)
	{
		if (InHit.GetActor() != LastHit.GetActor())
		{				
				LastHit.GetComponent()->SetCollisionResponseToChannel(InChannel, LastHitOriginalCollisionResponse);
		}
	}		

	if (InHit.bBlockingHit)
	{
		if (InHit.GetComponent()->GetCollisionResponseToChannel(InChannel) != NewResponseDuringCollision)
		{
			LastHit = InHit;			
			LastHitOriginalCollisionResponse = InHit.GetComponent()->GetCollisionResponseToChannel(InChannel);
			InHit.GetComponent()->SetCollisionResponseToChannel(InChannel, NewResponseDuringCollision);			
		}
	}		
}

float ANoxCharacter::CalculatePercentage(const float CurrentValue, const float MaxValue)
{
	const float Percentage = CurrentValue / MaxValue;
	
	return Percentage;
}

void ANoxCharacter::Ragdoll()
{
	GetMesh()->SetAllBodiesBelowSimulatePhysics(GetMesh()->GetBoneName(1), true, true);
	GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(GetMesh()->GetBoneName(1), 1);
}	

float ANoxCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Call the base class - this will tell us how much damage to apply  
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	
	if (CanBeDamaged())
	{
		if (ActualDamage > 0.f && Health > 0.f)
		{
			// Specify variable for health and apply damage.
			Health -= ActualDamage;

			// Update health procentage for correct operation of widgets.
			HealthPercentage = CalculatePercentage(Health, MaxHealth);			

			// If the damage depletes our health set our lifespan to zero - which will destroy the actor  
			if (Health <= 0.f)
			{
				bIsAlive = false;

				// Change team to neutral when dead to stop attacking 
				auto igtaiController = Cast<IGenericTeamAgentInterface>(GetController()); // ActorBot
				igtaiController->SetGenericTeamId(255);

				// Turn off attack ability for dead character
				bCanAttack = false;

				GetInformationBar()->bHiddenInGame = true;				
								
				GetCursorToWorld()->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
				GetCursorToWorld()->SetLifeSpan(0.1f);

				GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

				GetMovementComponent()->Deactivate();	
				
				GetAIPerceptionStimuliSource()->ToggleActive();

				

				// Check if array is not empty
				if (DeathAnimMontages.Num() != NULL)
				{
					// Check if specified slot for montage is not null
					if (DeathAnimMontages[DeathMontageToUse] != NULL)
					{
						float MontageLength = DeathAnimMontages[DeathMontageToUse]->GetPlayLength();											

						// Call ragdoll at the end of animation						
						PlayMontageWithSpecificEffect(this, DeathAnimMontages[DeathMontageToUse], MontageLength, TEXT("Ragdoll"));
						
					}
					else // If montages are empty call Ragdoll
					{
						Ragdoll();						
					}
				}
				else  // If there are no montages call Ragdoll
				{
					Ragdoll();
				}
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Take Damage"));

		return ActualDamage;
	}
	else
	{
		// TODO Check if this cousing errors
		UE_LOG(LogTemp, Warning, TEXT("this Object cannot be damaged %s"), *GetName());

		return 0;
	}	
}

void ANoxCharacter::GetActorSingleFrontCollision(FHitResult& OutHitResult, const float RaycastRange, const ECollisionChannel CollisionChannel, const bool InbDrawRange)
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

	if (InbDrawRange)
	{
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 200, 1, 3);
	}	
}

template <typename UObjectTemplate, typename... VarTypes>
void ANoxCharacter::PlayMontageWithSpecificEffect(UObjectTemplate* InUserObject, UAnimMontage* AnimMontageToPlay, const float DelayTimeToTriggerFunction, const FName& InFunctionName, VarTypes... Vars)
{	
	if (AnimMontageToPlay != NULL)
	{
		// Create Anim Instance used to play animations
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance != NULL)
		{			
			// Play animation for attacking without weapon 
			AnimInstance->Montage_Play(AnimMontageToPlay);

			// Create Timer Handle for animation montage
			FTimerHandle InflictDamageTimerHandle;

			// Create and bind delegate for function that will be called 
			FTimerDelegate StrikeForwardDelegate;
			StrikeForwardDelegate.BindUFunction(InUserObject, InFunctionName, Vars...);

			// if delay time for calling unarmed strike is 0 timer would not call function at all
			if (DelayTimeToTriggerFunction > 0)
			{
				GetWorldTimerManager().SetTimer(InflictDamageTimerHandle, StrikeForwardDelegate, DelayTimeToTriggerFunction, false);
			}
			else
			{
				StrikeForwardDelegate.ExecuteIfBound();
			}			
		} 
	}
	else
	{
		// Error code
		UE_LOG(LogTemp, Warning, TEXT("Pointer to animation montage is not found!"));
	}
}

TArray<FName> ANoxCharacter::GetSocketsByECollisionPart(const ECollisionPart& CollisionPart)
{
	TArray<FName> HandCollisionSockets;	

	// Choose an array on whitch we will be working on 
	switch (CollisionPart)
	{
	case ECollisionPart::CP_RightHand:
	{
		HandCollisionSockets = RightHandCollisionSockets;
		break;
	}
	case ECollisionPart::CP_LeftHand:
	{
		HandCollisionSockets = LeftHandCollisionSockets;
		break;
	}
	default:
		UE_LOG(LogTemp, Warning, TEXT("HitBoxNotify Collision Part is set to 'None'"));
		break;
	}
	
	return HandCollisionSockets;
}
		

void ANoxCharacter::Attack()
{
	if (bCanAttack)
	{
		if (!bIsWeaponEquiped)
		{
			// Unarmed strike
			UNoxAnimInstance* NoxAnimInstance = Cast<UNoxAnimInstance>(GetMesh()->GetAnimInstance());
			if (NoxAnimInstance != NULL)
			{
				if (!NoxAnimInstance->IsAnyMontagePlaying())
				{
					CurrentUnarmedAttack = UnarmedAttacks[UnarmedAttackToUse];

					PlayAnimMontage(UnarmedAttacks[UnarmedAttackToUse].Montage);
				}
			}
		}
		else if (EquippedWeapon != NULL)
		{
			// Attack with weapon
			// Get Tag of equipped weapon
			FGameplayTag EquippedWeaponTag = EquippedWeapon->WeaponTag;
			if (EquippedWeaponTag.IsValid())
			{
				// Find animation by tag for this type of weapon 		
				for (const auto& WeaponAttack : WeaponAttacks)
				{
					if (EquippedWeaponTag.MatchesTagExact(WeaponAttack.Tag))
					{
						if (WeaponAttack.Montage != NULL)
						{
							UNoxAnimInstance* NoxAnimInstance = Cast<UNoxAnimInstance>(GetMesh()->GetAnimInstance());
							if (NoxAnimInstance != NULL)
							{
								if (!NoxAnimInstance->IsAnyMontagePlaying())
								{
									CurrentWeaponAttack = WeaponAttack;

									// Pass params of current attack to weapon class
									EquippedWeapon->MeleeCollisionParams = WeaponAttack.MeleeCollisionParams;
									EquippedWeapon->AttackDamageParams = WeaponAttack.AttackDamageParams;

									PlayAnimMontage(WeaponAttack.Montage);
								}
							}
						}
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid Tag in Equipped Weapon"))
			}
		}
	}	
}

void ANoxCharacter::UnarmedAttack()
{
	TArray<FHitResult> HitResults;

	// Create array that contain sockets locations
	TArray<FVector> HandCollisionSocketsLocations;
	for (const auto& HandCollisionSocket : CurrentHandCollisionSockets)
	{
		if (GetMesh()->DoesSocketExist(HandCollisionSocket))
		{
			FVector SocketLocation = GetMesh()->GetSocketLocation(HandCollisionSocket);
			HandCollisionSocketsLocations.AddUnique(SocketLocation);
		}
	}

	ABaseWeapon::CreateCollisionByPointLocation<AActor>(this, OUT HitResults, CurrentUnarmedAttack.MeleeCollisionParams, ObjectTypesToCollideWithHands, AttackedActors, HandCollisionSocketsLocations);

	float FinalDamage = ABaseWeapon::CalculateFinalDamage(UnarmedDamage, CurrentUnarmedAttack.AttackDamageParams);

	FDamageEvent DamageEvent;
	for (const auto& Hit : HitResults)
	{
		if (Hit.GetActor()->CanBeDamaged() && !AttackedActors.Contains(Hit.GetActor()))
		{
			AttackedActors.AddUnique(Hit.GetActor());

			Hit.GetActor()->TakeDamage(FinalDamage, DamageEvent, GetController(), this);		
		}
	}
}

void ANoxCharacter::OnDealDamageBegin(const ECollisionPart& CollisionPart)
{
	// Set Flag to true at the start of an attack
	bIsAttacking = true;

	// Get locations used to create collision line 	
	CurrentHandCollisionSockets = GetSocketsByECollisionPart(CollisionPart);

	if (!bIsWeaponEquiped)
	{
		// Enable Function UnarmedAttack() in tick;
		bIsAttackingWithHands = true;		
	}	
	else if (EquippedWeapon != NULL)
	{
		if (CurrentWeaponAttack.bOverrideMeleeCollisionParams)
		{
			// Empty array with hand sockets locations
			if (!CurrentWeaponAttack.bUseHandCollisionWithWeaponAttack)
			{
				CurrentHandCollisionSockets.Empty();
			}

			if (EquippedWeapon->MeleeWeaponCollision.MeleeCollisionType == EMeleeCollisionType::MCT_CollisionByObject)
			{
				// If attack, change weapon collision type, then weapon additional range is always 0
				EquippedWeapon->MeleeWeaponCollision.AdditionalWeaponRange = 0.f;

				// Change method of geting collisions
				EquippedWeapon->MeleeWeaponCollision.MeleeCollisionType = EMeleeCollisionType::MCT_CollisionBySocketsLocations;
			}			
		}
		else if(CurrentWeaponAttack.MeleeCollisionParams.DrawDebugTrace != EDrawDebugTrace::None)
		{
			// if weapon attack does not override params then change DrawDebugTrace to None so that unusable(greyed out) varibable will not work on function.
			CurrentWeaponAttack.MeleeCollisionParams.DrawDebugTrace = EDrawDebugTrace::None;
		}

		// Pass hands collision sockets to weapon for future use  
		EquippedWeapon->MeleeCollisionParams.OwnerMesh = GetMesh();		
		EquippedWeapon->MeleeCollisionParams.CollisionSockets = CurrentHandCollisionSockets;

		EquippedWeapon->OnWeaponAttackBegin();
	}	
}

void ANoxCharacter::OnDealDamageEnd(const ECollisionPart& CollisionPart)
{	
	if (!bIsWeaponEquiped)
	{
		AttackedActors.Empty();
	}
	else if(EquippedWeapon != NULL)
	{		
		EquippedWeapon->OnWeaponAttackEnd();
	}

	bIsAttacking = false;
	bIsAttackingWithHands = false;
}

void ANoxCharacter::CreateWeapon()
{
	// Set SpawnParams
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.Owner = GetOwner();

	EquippedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(WeaponClassToEquip, SpawnParams);
	EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponGripPointSocket);

	// Ignore physical collision while moving?
	MoveIgnoreActorAdd(EquippedWeapon);

	bIsWeaponEquiped = true;
}

void ANoxCharacter::DestroyWeapon()
{	
	EquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
		
	EquippedWeapon->Destroy();	

	bIsWeaponEquiped = false;
}

void ANoxCharacter::EquipWeapon()
{	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// Check if any montage is currently playing (To prevent overlaping animation)
	if (!AnimInstance->IsAnyMontagePlaying())
	{
		if (!bIsWeaponEquiped)
		{
			PlayMontageWithSpecificEffect(this, EquipWeaponAnimMontage, DelayTimeToEquipWeapon, FName("CreateWeapon"));
		}
		else if (EquippedWeapon != NULL)
		{
			PlayMontageWithSpecificEffect(this, UnequipWeaponAnimMontage, DelayTimeToUnequipWeapon, FName("DestroyWeapon"));
		}
	}
}



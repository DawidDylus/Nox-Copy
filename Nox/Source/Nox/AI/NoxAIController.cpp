// Fill out your copyright notice in the Description page of Project Settings.


#include "NoxAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Nox/NoxCharacter.h"

#include "Engine/Engine.h"


ANoxAIController::ANoxAIController()
{
	// Create AI perception and sight sense
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>("AIPerception");
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));		

	// Set default for sight sense	
	SightConfig->SightRadius = 300.0f;
	SightConfig->LoseSightRadius = 350.0f;
	SightConfig->PeripheralVisionAngleDegrees = 360.0f;

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception->ConfigureSense(*SightConfig);	
	

	TeamId = FGenericTeamId(255);

}

void ANoxAIController::BeginPlay()
{
	Super::BeginPlay();	
	
}

ETeamAttitude::Type ANoxAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	return ETeamAttitude::Type();

	// Check if Actor is a pawn
	auto OtherPawn = Cast<APawn>(&Other);
	if (OtherPawn == nullptr)
	{
		return ETeamAttitude::Neutral;
	}

	// Check if Actori (Bot or Player) implements IGenericTeamAgentInterface.
	auto igtaiActorBot = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController()); // ActorBot
	auto igtaiActorPlayer = Cast<IGenericTeamAgentInterface>(&Other); // ActorPlayer
	if (igtaiActorBot == nullptr && igtaiActorPlayer == nullptr)
	{
		return ETeamAttitude::Neutral;
	}

	if (igtaiActorBot != nullptr) // Print ActorBot TeamID
	{
		FGenericTeamId fgtiActorBotTeamId = igtaiActorBot->GetGenericTeamId();
		int iActorBotTeamId = (int)fgtiActorBotTeamId;
		FString fstrActorBotTeamId = FString::FromInt(iActorBotTeamId);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, fstrActorBotTeamId);
	}

	if (igtaiActorPlayer != nullptr) // Print ActorPlayer TeamID
	{
		FGenericTeamId fgtiActorPlayerTeamId = igtaiActorPlayer->GetGenericTeamId();
		int iActorPlayerTeamId = (int)fgtiActorPlayerTeamId;
		FString fstrActorPlayerTeamId = FString::FromInt(iActorPlayerTeamId);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, fstrActorPlayerTeamId);
	}

	// Get the TeamId of the Actor (Bot or Player)
	FGenericTeamId fgtiOtherActorTeamId = NULL;
	if (igtaiActorBot != nullptr)
	{
		fgtiOtherActorTeamId = igtaiActorBot->GetGenericTeamId();
	}
	else if (igtaiActorPlayer != nullptr)
	{
		fgtiOtherActorTeamId = igtaiActorPlayer->GetGenericTeamId();
	}

	// Determines ThisBot attitude towards OtherActor (Bot or Player) as either Neutral, Friendly, or Hostile.

	FGenericTeamId fgtiThisBotTeamId = this->GetGenericTeamId();
	if (fgtiOtherActorTeamId == 255) // The are not on the team
	{			
		return ETeamAttitude::Neutral;	
	}
	else if (fgtiThisBotTeamId == fgtiOtherActorTeamId)	 // They are on the same team
	{
		return ETeamAttitude::Friendly;
	}
	else // they are on different teams
	{
		return ETeamAttitude::Hostile;
	}
}

ANoxCharacter* ANoxAIController::GetControlledNoxCharacter() const
{
	return GetPawn<ANoxCharacter>();
}

void ANoxAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	
}


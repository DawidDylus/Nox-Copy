// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NoxAIController.generated.h"

/**
 * 
 */
UCLASS()
class NOX_API ANoxAIController : public AAIController
{
	GENERATED_BODY()

	/** Perception for AI **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception", meta = (AllowPrivateAccess = "true"))
	class UAIPerceptionComponent* AIPerception;

	/** Sight config for AI **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception Senses", meta = (AllowPrivateAccess = "true"))
		class UAISenseConfig_Sight* SightConfig;

public:
	ANoxAIController();	

protected:
	virtual void BeginPlay() override;	

private:
	FGenericTeamId TeamId;

public:
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

public:
	// Get cast to controlled nox character
	class ANoxCharacter* GetControlledNoxCharacter() const;	

protected:	
	virtual void OnPossess(APawn* InPawn) override;	
	
public:
	/** Returns AIPerception subobject **/
	FORCEINLINE class UAIPerceptionComponent* GetAIPerception() const { return AIPerception; }
	/** Returns SightConfig subobject **/
	FORCEINLINE class UAISenseConfig_Sight* GetSightConfig() const { return SightConfig; }

	
};

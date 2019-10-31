// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "NoxGameMode.h"
#include "NoxPlayerController.h"
#include "NoxCharacter.h"
#include "UObject/ConstructorHelpers.h"

ANoxGameMode::ANoxGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ANoxPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
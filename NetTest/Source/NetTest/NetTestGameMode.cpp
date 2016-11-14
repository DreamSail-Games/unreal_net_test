// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NetTest.h"
#include "NetTestGameMode.h"
#include "NetTestCharacter.h"

ANetTestGameMode::ANetTestGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "WCTestGameMode.h"
#include "WCTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWCTestGameMode::AWCTestGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Character/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

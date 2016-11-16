// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "NetTestGameMode.generated.h"

UCLASS(minimalapi)
class ANetTestGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	FHttpModule* HttpModule;

	ANetTestGameMode();


	void SendServerHeartbeat();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void OnHttpRequestReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

};




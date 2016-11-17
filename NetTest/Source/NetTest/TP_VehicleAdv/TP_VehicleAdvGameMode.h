// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "TP_VehicleAdvGameMode.generated.h"

UCLASS(minimalapi)
class ATP_VehicleAdvGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	FHttpModule* HttpModule;

	ATP_VehicleAdvGameMode();

	void SendServerHeartbeat();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void OnHttpRequestReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};




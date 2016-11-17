// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NetTest.h"
#include "TP_VehicleAdvGameMode.h"
#include "TP_VehicleAdvPawn.h"
#include "TP_VehicleAdvHud.h"
#include "Engine.h"

ATP_VehicleAdvGameMode::ATP_VehicleAdvGameMode()
{
	HttpModule = &FHttpModule::Get();

	DefaultPawnClass = ATP_VehicleAdvPawn::StaticClass();
	HUDClass = ATP_VehicleAdvHud::StaticClass();
}

void ATP_VehicleAdvGameMode::BeginPlay()
{
	if (IsRunningDedicatedServer())
	{
		SendServerHeartbeat();
	}

	Super::BeginPlay();
}

void ATP_VehicleAdvGameMode::SendServerHeartbeat()
{
	if (Role == ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server, send heartbeat!"));

		const FString BASE_URL = TEXT("https://unreal-net-test-master-server.herokuapp.com");
		const FString GAME_SERVER_HEARTBEAT = TEXT("/game-server-heartbeat");
		const FString LIST_SERVER = TEXT("/list-server");

		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString(TEXT("creating http request")));

		TSharedRef<IHttpRequest> Request = HttpModule->CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ATP_VehicleAdvGameMode::OnHttpRequestReceived);
		Request->SetURL(BASE_URL + GAME_SERVER_HEARTBEAT);
		Request->SetVerb("POST");
		Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

		Request->SetContentAsString(TEXT("{\"ip\":\"0.0.0.0\",\"port\":7777}"));

		Request->ProcessRequest();

		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString(TEXT("HTTP response sent!")));
	}
}

void ATP_VehicleAdvGameMode::OnHttpRequestReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString(TEXT("HTTP Resonse got!")));
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, Response->GetContentAsString());
	UE_LOG(LogTemp, Warning, TEXT("Got http response"));


	//Create a pointer to hold the json serialized data
	//TSharedPtr<FJsonObject> JsonObject;

	//Create a reader pointer to read the json data
	//TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	//Deserialize the json data given Reader and the actual object to deserialize
	//if (FJsonSerializer::Deserialize(Reader, JsonObject))
	//{
	//Output it to the engine
	//	GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Green, FString::FromInt(recievedInt));
	//}
}
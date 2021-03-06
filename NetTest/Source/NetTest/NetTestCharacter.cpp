// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NetTest.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "NetTestCharacter.h"
#include "Engine.h"
#include "Json.h"

//////////////////////////////////////////////////////////////////////////
// ANetTestCharacter

ANetTestCharacter::ANetTestCharacter()
{
	HttpModule = &FHttpModule::Get();

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void ANetTestCharacter::BeginPlay()
{
	
	ListServers();

	Super::BeginPlay();
}


void ANetTestCharacter::SendServerHeartbeat()
{
	if (Role == ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server, send heartbeat!"));

		const FString BASE_URL = TEXT("https://unreal-net-test-master-server.herokuapp.com");
		const FString GAME_SERVER_HEARTBEAT = TEXT("/game-server-heartbeat");
		const FString LIST_SERVER = TEXT("/list-server");

		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString(TEXT("creating http request")));

		TSharedRef<IHttpRequest> Request = HttpModule->CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ANetTestCharacter::OnHttpRequestReceived);
		Request->SetURL(BASE_URL + GAME_SERVER_HEARTBEAT);
		Request->SetVerb("POST");
		Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

		Request->SetContentAsString(TEXT("{\"ip\":\"0.0.0.0\",\"port\":7777}"));

		Request->ProcessRequest();

		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString(TEXT("HTTP response sent!")));
	}
}

void ANetTestCharacter::ListServers()
{
	if (Role == ROLE_Authority)
	{

		const FString BASE_URL = TEXT("https://unreal-net-test-master-server.herokuapp.com");
		const FString GAME_SERVER_HEARTBEAT = TEXT("/game-server-heartbeat");
		const FString LIST_SERVER = TEXT("/list-server");

		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString(TEXT("creating http request")));

		TSharedRef<IHttpRequest> Request = HttpModule->CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ANetTestCharacter::OnHttpRequestReceived);
		Request->SetURL(BASE_URL + LIST_SERVER);
		Request->SetVerb("GET");
		Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

		Request->ProcessRequest();

		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString(TEXT("HTTP response sent!")));
	}
}


void ANetTestCharacter::OnHttpRequestReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString(TEXT("HTTP Resonse got!")));
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, Response->GetContentAsString() );
	UE_LOG(LogTemp, Warning, TEXT("Got http response"));

	//Create a pointer to hold the json serialized data
	TSharedPtr<FJsonObject> JsonObject;

	//Create a reader pointer to read the json data
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	//Deserialize the json data given Reader and the actual object to deserialize
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		//Output it to the engine
		TArray<TSharedPtr<FJsonValue>> serverList = JsonObject->GetArrayField(TEXT("serverList"));

		FString ipAddrToUse = "";
		for (int i = 0; i < serverList.Num(); i++)
		{
			TSharedPtr<FJsonObject> serverObj = serverList[i]->AsObject();
			FString ipAddr					  = serverObj->GetStringField(TEXT("ip"));
			if (ipAddr.Len() > 0)
			{
				ipAddrToUse = ipAddr;
			}
		}

		if (!ipAddrToUse.Equals(""))
		{
			ConnectToServerWithIP(ipAddrToUse);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANetTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Spawn", IE_Pressed, this, &ANetTestCharacter::OnSpawnInput);


	PlayerInputComponent->BindAxis("MoveForward", this, &ANetTestCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANetTestCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANetTestCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ANetTestCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ANetTestCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ANetTestCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ANetTestCharacter::OnResetVR);
}

void ANetTestCharacter::OnSpawnInput()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString(TEXT("Spawn a thing!")));
	SpawnThing();
}

void ANetTestCharacter::ConnectToServerWithIP(FString ipAddr)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString(TEXT("Try to connect!")));

	FString openCmd = "open " + ipAddr;

	if ( !ipAddr.Equals("") )
	{
		GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Green, openCmd);

		//HACK
		//Find out how to do this WITHOUT a hacky console command. 
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController != NULL)
		{
			PlayerController->ConsoleCommand(openCmd, true);
		}
		//END HACK
	}
}

void ANetTestCharacter::SpawnThing_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString(TEXT("spawn thing implementation hit, but velociiity")));

	if ( Role == ROLE_Authority )
	{
		UE_LOG(LogTemp, Warning, TEXT("Server, spawn a thing!"));
		//How to get this to happen from the master over net?
		//Spawn Code
		FVector Location = this->GetActorLocation();
		FRotator Rotation = FRotator::ZeroRotator;

		UWorld* const World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Instigator = this;


			AActor* actor = World->SpawnActor<AActor>(WhatToSpawn, Location, Rotation, SpawnParams);
			this->GetCharacterMovement()->Velocity += FVector::UpVector * 250.0f;
		}
	}
}

bool ANetTestCharacter::SpawnThing_Validate()
{
	return true;
}



void ANetTestCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ANetTestCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// jump, but only on the first touch
	if (FingerIndex == ETouchIndex::Touch1)
	{
		Jump();
	}
}

void ANetTestCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (FingerIndex == ETouchIndex::Touch1)
	{
		StopJumping();
	}
}

void ANetTestCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ANetTestCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ANetTestCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ANetTestCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

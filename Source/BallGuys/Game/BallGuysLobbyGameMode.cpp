// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BallGuysLobbyGameMode.h"
#include "Game/BallGuysGameInstance.h"
#include "Game/BallGuysGameState.h"
#include "Player/BallGuysPlayerState.h"
#include "Player/BallGuysPlayerController.h"
#include "Player/BallPawn.h"
#include "UI/BallGuysHUD.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ABallGuysLobbyGameMode::ABallGuysLobbyGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	GameStateClass = ABallGuysGameState::StaticClass();
	PlayerStateClass = ABallGuysPlayerState::StaticClass();
	PlayerControllerClass = ABallGuysPlayerController::StaticClass();
	DefaultPawnClass = ABallPawn::StaticClass();
	HUDClass = ABallGuysHUD::StaticClass();
}

void ABallGuysLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	BallGuysGameState = GetGameState<ABallGuysGameState>();
	if (BallGuysGameState)
	{
		BallGuysGameState->SetGamePhase(EBallGuysGamePhase::WaitingForPlayers);
	}

	UpdateSpawnPoints();
}

void ABallGuysLobbyGameMode::PreLogin(
	const FString& Options,
	const FString& Address,
	const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (GetNumPlayers() >= UBallGuysGameInstance::MAX_LOBBY_PLAYERS)
	{
		ErrorMessage = TEXT("Lobby is full (max 4 players).");
	}
}

void ABallGuysLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ABallGuysPlayerState* PS = NewPlayer ? NewPlayer->GetPlayerState<ABallGuysPlayerState>() : nullptr)
	{
		PS->bIsReady = false;
	}

	RespawnLobbyPlayer(NewPlayer);
	CheckReadyStatus();
}

void ABallGuysLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	CheckReadyStatus();
}

void ABallGuysLobbyGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!BallGuysGameState || bTravelInitiated)
	{
		return;
	}

	CheckReadyStatus();

	if (BallGuysGameState->CurrentGamePhase == EBallGuysGamePhase::Countdown && bCountdownActive)
	{
		CountdownTimer = FMath::Max(0.0f, CountdownTimer - DeltaSeconds);
		BallGuysGameState->TimeRemaining = CountdownTimer;

		if (CountdownTimer <= 0.0f)
		{
			TravelToGameplayMap();
		}
	}
}

void ABallGuysLobbyGameMode::CheckReadyStatus()
{
	if (!BallGuysGameState || bTravelInitiated)
	{
		return;
	}

	int32 ReadyCount = 0;
	int32 TotalPlayers = 0;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			TotalPlayers++;
			if (const ABallGuysPlayerState* PS = PC->GetPlayerState<ABallGuysPlayerState>())
			{
				if (PS->bIsReady)
				{
					ReadyCount++;
				}
			}
		}
	}

	BallGuysGameState->ConnectedPlayerCount = TotalPlayers;
	BallGuysGameState->ReadyPlayerCount = ReadyCount;

	// Check if enough players are connected and ready for the auto-countdown
	if (TotalPlayers >= MinPlayersToStart && ReadyCount >= MinPlayersToStart)
	{
		const bool bAllReady = (ReadyCount == TotalPlayers);
		const float TargetDuration = bAllReady ? AllReadyCountdownDuration : PartialReadyCountdownDuration;

		if (!bCountdownActive)
		{
			bCountdownActive = true;
			CountdownTimer = TargetDuration;
			BallGuysGameState->TimeRemaining = CountdownTimer;
			BallGuysGameState->SetGamePhase(EBallGuysGamePhase::Countdown);
		}
		else if (bAllReady && CountdownTimer > AllReadyCountdownDuration)
		{
			// Shorten an existing 20s timer down to 10s if the remaining players ready up
			CountdownTimer = AllReadyCountdownDuration;
			BallGuysGameState->TimeRemaining = CountdownTimer;
		}
	}
	else if (bCountdownActive)
	{
		// Cancel countdown if players unready or drop below MinPlayersToStart
		bCountdownActive = false;
		CountdownTimer = 0.0f;
		BallGuysGameState->TimeRemaining = 0.0f;
		BallGuysGameState->SetGamePhase(EBallGuysGamePhase::WaitingForPlayers);
	}
}

void ABallGuysLobbyGameMode::HostStartGame()
{
	if (bTravelInitiated)
	{
		return;
	}

	const int32 TotalPlayers = GetNumPlayers();
	if (TotalPlayers >= MinPlayersToStart || (bAllowSoloHostStart && TotalPlayers >= 1))
	{
		TravelToGameplayMap();
	}
}

void ABallGuysLobbyGameMode::TravelToGameplayMap()
{
	if (bTravelInitiated || GameplayMapName.IsEmpty())
	{
		return;
	}

	bTravelInitiated = true;
	bCountdownActive = false;

	if (UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		GI->ExpectedMatchPlayers = FMath::Clamp(GetNumPlayers(), 1, UBallGuysGameInstance::MAX_LOBBY_PLAYERS);
	}

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(FString::Printf(TEXT("%s?listen"), *GameplayMapName));
	}
}

void ABallGuysLobbyGameMode::RespawnLobbyPlayer(AController* Controller)
{
	if (!Controller)
	{
		return;
	}

	if (APawn* OldPawn = Controller->GetPawn())
	{
		OldPawn->Destroy();
	}

	UpdateSpawnPoints();

	AActor* ChosenStart = (SpawnPoints.Num() > 0)
		? SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)]
		: nullptr;

	RestartPlayerAtPlayerStart(Controller, ChosenStart);
}

void ABallGuysLobbyGameMode::UpdateSpawnPoints()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPoints);
}

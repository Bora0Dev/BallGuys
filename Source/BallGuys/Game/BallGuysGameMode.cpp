#include "Game/BallGuysGameMode.h"
#include "Game/BallGuysGameInstance.h"
#include "Game/BallGuysGameState.h"
#include "Player/BallGuysPlayerState.h"
#include "Player/BallGuysPlayerController.h"
#include "Player/BallPawn.h"
#include "UI/BallGuysHUD.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ABallGuysGameMode::ABallGuysGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	GameStateClass = ABallGuysGameState::StaticClass();
	PlayerStateClass = ABallGuysPlayerState::StaticClass();
	PlayerControllerClass = ABallGuysPlayerController::StaticClass();
	DefaultPawnClass = ABallPawn::StaticClass();
	HUDClass = ABallGuysHUD::StaticClass();

	GameTimer = GameDuration;
	LoadWaitTimer = 0.0f;
	bGameStarted = false;
	ExpectedPlayers = 1;
}

void ABallGuysGameMode::BeginPlay()
{
	Super::BeginPlay();

	BallGuysGameState = GetGameState<ABallGuysGameState>();

	if (const UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		ExpectedPlayers = FMath::Clamp(GI->ExpectedMatchPlayers, 1, UBallGuysGameInstance::MAX_LOBBY_PLAYERS);
	}

	if (BallGuysGameState)
	{
		BallGuysGameState->SetGamePhase(EBallGuysGamePhase::WaitingForPlayers);
	}

	UpdateSpawnPoints();
	CheckAllPlayersLoaded();
}

void ABallGuysGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	HandleGameLoop(DeltaSeconds);
}

void ABallGuysGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	RespawnPlayer(NewPlayer);
	CheckAllPlayersLoaded();
}

void ABallGuysGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (BallGuysGameState)
	{
		BallGuysGameState->ConnectedPlayerCount = GetNumPlayers();
	}
}

void ABallGuysGameMode::CheckAllPlayersLoaded()
{
	if (bGameStarted || !BallGuysGameState)
	{
		return;
	}

	const int32 CurrentPlayerCount = GetNumPlayers();
	BallGuysGameState->ConnectedPlayerCount = CurrentPlayerCount;

	if (CurrentPlayerCount >= ExpectedPlayers)
	{
		StartGame();
	}
}

void ABallGuysGameMode::HandleGameLoop(float DeltaSeconds)
{
	if (!BallGuysGameState)
	{
		return;
	}

	if (BallGuysGameState->CurrentGamePhase == EBallGuysGamePhase::WaitingForPlayers)
	{
		LoadWaitTimer += DeltaSeconds;
		CheckAllPlayersLoaded();

		// Safety fallback if a player disconnected during ServerTravel so the match doesn't hang forever
		if (!bGameStarted && LoadWaitTimer >= MaxPlayerLoadWaitTime && GetNumPlayers() >= 1)
		{
			StartGame();
		}
	}
	else if (BallGuysGameState->CurrentGamePhase == EBallGuysGamePhase::Playing)
	{
		GameTimer = FMath::Max(0.0f, GameTimer - DeltaSeconds);
		BallGuysGameState->TimeRemaining = GameTimer;

		if (GameTimer <= 0.0f)
		{
			EndGame();
		}
	}
}

void ABallGuysGameMode::StartGame()
{
	if (bGameStarted || !BallGuysGameState)
	{
		return;
	}

	bGameStarted = true;
	GameTimer = GameDuration;
	BallGuysGameState->TimeRemaining = GameTimer;
	BallGuysGameState->SetGamePhase(EBallGuysGamePhase::Playing);

	// Reset lives and respawn all connected players at the start of the match
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (ABallGuysPlayerState* PS = PC->GetPlayerState<ABallGuysPlayerState>())
			{
				PS->ResetLives();
			}
			RespawnPlayer(PC);
		}
	}
}

void ABallGuysGameMode::EndGame()
{
	bGameStarted = false;
	if (BallGuysGameState)
	{
		BallGuysGameState->SetGamePhase(EBallGuysGamePhase::GameOver);
	}
}

void ABallGuysGameMode::PlayerDied(AController* Controller)
{
	if (!Controller)
	{
		return;
	}

	if (ABallGuysPlayerState* PS = Controller->GetPlayerState<ABallGuysPlayerState>())
	{
		PS->LoseLife();

		if (PS->CurrentLives > 0)
		{
			RespawnPlayer(Controller);
		}
		else if (APawn* Pawn = Controller->GetPawn())
		{
			// Player eliminated
			Pawn->Destroy();
		}
	}
}

void ABallGuysGameMode::RespawnPlayer(AController* Controller)
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

void ABallGuysGameMode::UpdateSpawnPoints()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPoints);
}

#include "BallGuysGameMode.h"
#include "BallGuysGameState.h"
#include "BallGuysPlayerState.h"
#include "BallGuysPlayerController.h"
#include "BallPawn.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ABallGuysGameMode::ABallGuysGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Set default classes
	GameStateClass = ABallGuysGameState::StaticClass();
	PlayerStateClass = ABallGuysPlayerState::StaticClass();
	PlayerControllerClass = ABallGuysPlayerController::StaticClass();
	// DefaultPawnClass should be set in BP or here if BallPawn is the only pawn
	DefaultPawnClass = ABallPawn::StaticClass();

	CountdownTimer = 0.0f;
	GameTimer = GAME_DURATION;
	bGameStarted = false;
	bCountdownActive = false;
}

void ABallGuysGameMode::BeginPlay()
{
	Super::BeginPlay();

	BallGuysGameState = GetGameState<ABallGuysGameState>();
	UpdateSpawnPoints();
}

void ABallGuysGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	HandleGameLoop();
}

void ABallGuysGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Respawn player at a random spawn point on join
	RespawnPlayer(NewPlayer);
}

void ABallGuysGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void ABallGuysGameMode::HandleGameLoop()
{
	if (!BallGuysGameState) return;

	int32 CurrentPlayerCount = GetNumPlayers();

	// Waiting for players / Ready Logic
	if (BallGuysGameState->CurrentGamePhase == EBallGuysGamePhase::WaitingForPlayers)
	{
		if (CurrentPlayerCount >= MIN_PLAYERS_TO_START)
		{
			CheckReadyStatus();
		}
		else
		{
			// Reset countdown if players drop below min
			bCountdownActive = false;
			CountdownTimer = 0.0f;
		}
	}
	else if (BallGuysGameState->CurrentGamePhase == EBallGuysGamePhase::Countdown)
	{
		CountdownTimer -= GetWorld()->GetDeltaSeconds();
		BallGuysGameState->TimeRemaining = CountdownTimer;

		if (CountdownTimer <= 0.0f)
		{
			StartGame();
		}
	}
	else if (BallGuysGameState->CurrentGamePhase == EBallGuysGamePhase::Playing)
	{
		GameTimer -= GetWorld()->GetDeltaSeconds();
		BallGuysGameState->TimeRemaining = GameTimer;

		if (GameTimer <= 0.0f)
		{
			EndGame();
		}
	}
}

void ABallGuysGameMode::CheckReadyStatus()
{
	int32 ReadyCount = 0;
	int32 TotalPlayers = 0;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			TotalPlayers++;
			ABallGuysPlayerState* PS = PC->GetPlayerState<ABallGuysPlayerState>();
			if (PS && PS->bIsReady)
			{
				ReadyCount++;
			}
		}
	}

	// Logic: If 2 players ready -> 10s countdown.
	// If > 2 players and 2 are ready -> 20s countdown (simplified from prompt: "If 2 players are ready and there is more than 2 players in 20 seconds game will start regardless")
	// Let's interpret: 
	// 1. If 2 players are ready -> Start 10s timer? Or is it "Wait 10s then start"?
	// 2. "If 2 players are ready and there is more than 2 players in 20 seconds game will start regardless" -> This implies a timeout.
	
	// Let's implement a simple logic:
	// If All players ready -> Start immediately (or short countdown)
	// If >= 2 players ready -> Start countdown if not already started.
	
	if (ReadyCount >= 2 && !bCountdownActive)
	{
		bCountdownActive = true;
		BallGuysGameState->SetGamePhase(EBallGuysGamePhase::Countdown);
		
		if (TotalPlayers > 2)
		{
			CountdownTimer = COUNTDOWN_DURATION_LONG;
		}
		else
		{
			CountdownTimer = COUNTDOWN_DURATION_SHORT;
		}
	}
}

void ABallGuysGameMode::StartGame()
{
	bGameStarted = true;
	bCountdownActive = false;
	BallGuysGameState->SetGamePhase(EBallGuysGamePhase::Playing);
	
	// Reset Lives and Respawn everyone?
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			ABallGuysPlayerState* PS = PC->GetPlayerState<ABallGuysPlayerState>();
			if (PS)
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
	BallGuysGameState->SetGamePhase(EBallGuysGamePhase::GameOver);
	// Show scoreboard logic handled by UI observing the state
}

void ABallGuysGameMode::PlayerDied(AController* Controller)
{
	if (!Controller) return;

	ABallGuysPlayerState* PS = Controller->GetPlayerState<ABallGuysPlayerState>();
	if (PS)
	{
		PS->LoseLife();
		
		if (PS->CurrentLives > 0)
		{
			RespawnPlayer(Controller);
		}
		else
		{
			// Player Eliminated
			// Maybe move to spectator mode or just leave them dead
			if (APawn* Pawn = Controller->GetPawn())
			{
				Pawn->Destroy();
			}
		}
	}
}

void ABallGuysGameMode::RespawnPlayer(AController* Controller)
{
	if (!Controller) return;

	// Destroy old pawn if exists
	if (APawn* OldPawn = Controller->GetPawn())
	{
		OldPawn->Destroy();
	}

	UpdateSpawnPoints();

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (SpawnPoints.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
		AActor* SpawnPoint = SpawnPoints[RandomIndex];
		if (SpawnPoint)
		{
			SpawnLocation = SpawnPoint->GetActorLocation();
			SpawnRotation = SpawnPoint->GetActorRotation();
		}
	}

	RestartPlayerAtPlayerStart(Controller, SpawnPoints.Num() > 0 ? SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)] : nullptr);
}

void ABallGuysGameMode::UpdateSpawnPoints()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPoints);
}

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BallGuysGameState.h"
#include "BallGuysGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BALLGUYS_API ABallGuysGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABallGuysGameMode();

	virtual void Tick(float DeltaSeconds) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// Game Loop Logic
	void CheckReadyStatus();
	void StartGame();
	void EndGame();
	void HandleGameLoop();

	// Player Management
	void PlayerDied(AController* Controller);
	void RespawnPlayer(AController* Controller);

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	ABallGuysGameState* BallGuysGameState;

	// Timers
	float CountdownTimer;
	float GameTimer;
	bool bGameStarted;
	bool bCountdownActive;

	// Config
	const float COUNTDOWN_DURATION_SHORT = 10.0f;
	const float COUNTDOWN_DURATION_LONG = 20.0f;
	const float GAME_DURATION = 300.0f; // 5 minutes
	const int32 MIN_PLAYERS_TO_START = 2;

	// Spawn Points
	TArray<AActor*> SpawnPoints;
	void UpdateSpawnPoints();
};

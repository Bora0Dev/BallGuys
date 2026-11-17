#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Game/BallGuysGameState.h"
#include "BallGuysGameMode.generated.h"

/**
 * Gameplay Match GameMode.
 * Waits until all expected players traveling from the Lobby have finished loading into the map,
 * then starts the match timer, manages player lives/respawns, and ends the match.
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

	// Match Loop Logic
	void StartGame();
	void EndGame();
	void HandleGameLoop(float DeltaSeconds);

	// Player Management
	void PlayerDied(AController* Controller);
	void RespawnPlayer(AController* Controller);

protected:
	virtual void BeginPlay() override;

	/** Checks whether all expected players from the Lobby have loaded into the Gameplay Map */
	void CheckAllPlayersLoaded();

	UPROPERTY()
	TObjectPtr<ABallGuysGameState> BallGuysGameState;

	// Match State & Timers
	float GameTimer = 300.0f;
	float LoadWaitTimer = 0.0f;
	bool bGameStarted = false;
	int32 ExpectedPlayers = 1;

	/** Match duration in seconds (default 5 minutes) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Match|Config")
	float GameDuration = 300.0f;

	/** Safety timeout (seconds) to start the match even if a client drops while loading the map */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Match|Config")
	float MaxPlayerLoadWaitTime = 15.0f;

	// Spawn Points
	UPROPERTY()
	TArray<AActor*> SpawnPoints;

	void UpdateSpawnPoints();
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BallGuysLobbyGameMode.generated.h"

class ABallGuysGameState;

/**
 * GameMode for LobbyMap.
 * Spawns players as ABallPawn in a playable waiting room (hardcapped at 4 players),
 * tracks Ready status and countdown timers, allows the Host to manually start the match,
 * and ServerTravels to the Gameplay Map when the countdown completes.
 */
UCLASS()
class BALLGUYS_API ABallGuysLobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABallGuysLobbyGameMode();

	virtual void Tick(float DeltaSeconds) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/**
	 * Evaluates connected players and ready states to start, shorten, or cancel the lobby countdown.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void CheckReadyStatus();

	/**
	 * Called by the Host (e.g. via UI 'Start Game' button) to start the match immediately.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void HostStartGame();

	/**
	 * Respawns a player's BallPawn at a random PlayerStart in the lobby waiting room.
	 */
	void RespawnLobbyPlayer(AController* Controller);

protected:
	virtual void BeginPlay() override;

	/** Executes ServerTravel to GameplayMapName and records ExpectedMatchPlayers on the GameInstance */
	void TravelToGameplayMap();

	void UpdateSpawnPoints();

	UPROPERTY()
	TObjectPtr<ABallGuysGameState> BallGuysGameState;

	/** Map name to ServerTravel to when the lobby countdown finishes or Host starts the game */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lobby|Config")
	FString GameplayMapName = TEXT("GameMap");

	/** Minimum players required for auto-countdown (and Host start unless bAllowSoloHostStart is true) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lobby|Config")
	int32 MinPlayersToStart = 2;

	/** If true, the Host can click 'Start Game' even when testing solo (1 player) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lobby|Config")
	bool bAllowSoloHostStart = false;

	/** Countdown duration (seconds) when ALL connected players are Ready */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lobby|Config")
	float AllReadyCountdownDuration = 10.0f;

	/** Countdown duration (seconds) when >= 2 players are Ready out of 3-4 players */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lobby|Config")
	float PartialReadyCountdownDuration = 20.0f;

	float CountdownTimer = 0.0f;
	bool bCountdownActive = false;
	bool bTravelInitiated = false;

	UPROPERTY()
	TArray<AActor*> SpawnPoints;
};

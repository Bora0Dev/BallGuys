#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BallGuysPlayerController.generated.h"

/**
 * PlayerController handling Ready state toggles and Host lobby start controls.
 */
UCLASS()
class BALLGUYS_API ABallGuysPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** Toggles this player's Ready state in the Lobby */
	UFUNCTION(BlueprintCallable, Category = "BallGuys Lobby")
	void ToggleReadyState();

	/** Called by the Host's 'Start Game' UI button in the Lobby to immediately start the match */
	UFUNCTION(BlueprintCallable, Category = "BallGuys Lobby")
	void HostStartLobbyMatch();

	/** Returns true if this PlayerController belongs to the listen-server Host */
	UFUNCTION(BlueprintPure, Category = "BallGuys Lobby")
	bool IsHostPlayer() const;
};

#include "Player/BallGuysPlayerController.h"
#include "Player/BallGuysPlayerState.h"
#include "Game/BallGuysLobbyGameMode.h"
#include "Engine/World.h"

void ABallGuysPlayerController::ToggleReadyState()
{
	if (ABallGuysPlayerState* PS = GetPlayerState<ABallGuysPlayerState>())
	{
		PS->Server_SetIsReady(!PS->bIsReady);
	}
}

void ABallGuysPlayerController::HostStartLobbyMatch()
{
	if (!HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (ABallGuysLobbyGameMode* LobbyGM = World->GetAuthGameMode<ABallGuysLobbyGameMode>())
		{
			LobbyGM->HostStartGame();
		}
	}
}

bool ABallGuysPlayerController::IsHostPlayer() const
{
	return HasAuthority() && IsLocalController();
}

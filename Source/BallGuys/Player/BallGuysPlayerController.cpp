#include "Player/BallGuysPlayerController.h"
#include "Player/BallGuysPlayerState.h"

void ABallGuysPlayerController::ToggleReadyState()
{
	ABallGuysPlayerState* PS = GetPlayerState<ABallGuysPlayerState>();
	if (PS)
	{
		PS->Server_SetIsReady(!PS->bIsReady);
	}
}

#include "BallGuysPlayerController.h"
#include "BallGuysPlayerState.h"

void ABallGuysPlayerController::ToggleReadyState()
{
	ABallGuysPlayerState* PS = GetPlayerState<ABallGuysPlayerState>();
	if (PS)
	{
		PS->Server_SetIsReady(!PS->bIsReady);
	}
}

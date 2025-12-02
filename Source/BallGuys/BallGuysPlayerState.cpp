#include "BallGuysPlayerState.h"
#include "Net/UnrealNetwork.h"

ABallGuysPlayerState::ABallGuysPlayerState()
{
	CurrentLives = 9;
	bIsReady = false;
	bReplicates = true;
}

void ABallGuysPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABallGuysPlayerState, CurrentLives);
	DOREPLIFETIME(ABallGuysPlayerState, bIsReady);
}

void ABallGuysPlayerState::LoseLife()
{
	if (HasAuthority())
	{
		CurrentLives--;
		if (CurrentLives < 0)
		{
			CurrentLives = 0;
		}
		// GameMode will handle logic for elimination if lives == 0
	}
}

void ABallGuysPlayerState::ResetLives()
{
	if (HasAuthority())
	{
		CurrentLives = 9;
	}
}

void ABallGuysPlayerState::Server_SetIsReady_Implementation(bool bReady)
{
	bIsReady = bReady;
}

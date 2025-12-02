#include "BallGuysGameState.h"
#include "Net/UnrealNetwork.h"

ABallGuysGameState::ABallGuysGameState()
{
	TimeRemaining = 0.0f;
	CurrentGamePhase = EBallGuysGamePhase::WaitingForPlayers;
}

void ABallGuysGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABallGuysGameState, TimeRemaining);
	DOREPLIFETIME(ABallGuysGameState, CurrentGamePhase);
}

void ABallGuysGameState::SetGamePhase(EBallGuysGamePhase NewPhase)
{
	if (HasAuthority())
	{
		CurrentGamePhase = NewPhase;
	}
}

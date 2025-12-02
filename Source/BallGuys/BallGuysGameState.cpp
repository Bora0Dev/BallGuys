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

FText ABallGuysGameState::GetFormattedTimeRemaining() const
{
	int32 Minutes = FMath::FloorToInt(TimeRemaining / 60.0f);
	int32 Seconds = FMath::FloorToInt(TimeRemaining) % 60;
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
}

FText ABallGuysGameState::GetGamePhaseName() const
{
	switch (CurrentGamePhase)
	{
	case EBallGuysGamePhase::WaitingForPlayers:
		return FText::FromString("Waiting for Players");
	case EBallGuysGamePhase::Countdown:
		return FText::FromString("Starting in...");
	case EBallGuysGamePhase::Playing:
		return FText::FromString("Playing");
	case EBallGuysGamePhase::GameOver:
		return FText::FromString("Game Over");
	default:
		return FText::FromString("Unknown");
	}
}

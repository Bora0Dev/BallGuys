#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BallGuysGameState.generated.h"

UENUM(BlueprintType)
enum class EBallGuysGamePhase : uint8
{
	WaitingForPlayers,
	Countdown,
	Playing,
	GameOver
};

/**
 * 
 */
UCLASS()
class BALLGUYS_API ABallGuysGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	ABallGuysGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "BallGuys Gameplay")
	float TimeRemaining;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "BallGuys Gameplay")
	EBallGuysGamePhase CurrentGamePhase;

	UFUNCTION(BlueprintCallable, Category = "BallGuys Gameplay")
	void SetGamePhase(EBallGuysGamePhase NewPhase);
};

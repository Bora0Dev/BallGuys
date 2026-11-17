#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BallGuysPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BALLGUYS_API ABallGuysPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ABallGuysPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Lives system
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "BallGuys Gameplay")
	int32 CurrentLives;

	UFUNCTION(BlueprintCallable, Category = "BallGuys Gameplay")
	void LoseLife();

	UFUNCTION(BlueprintCallable, Category = "BallGuys Gameplay")
	void ResetLives();

	// Ready system
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "BallGuys Gameplay")
	bool bIsReady;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BallGuys Gameplay")
	void Server_SetIsReady(bool bReady);

protected:
	virtual void BeginPlay() override;
	
};

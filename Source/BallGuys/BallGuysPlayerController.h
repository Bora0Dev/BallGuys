#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BallGuysPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BALLGUYS_API ABallGuysPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "BallGuys Gameplay")
	void ToggleReadyState();
};

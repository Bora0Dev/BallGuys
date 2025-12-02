#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BallGuysKillVolume.generated.h"

class UBoxComponent;

UCLASS()
class BALLGUYS_API ABallGuysKillVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	ABallGuysKillVolume();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* KillBox;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};

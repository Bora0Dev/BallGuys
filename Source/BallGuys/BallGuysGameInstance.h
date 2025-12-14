// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BallGuysGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BALLGUYS_API UBallGuysGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Input")
	bool bInvertXPref = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Input")
	bool bInvertYPref = false;
	
};

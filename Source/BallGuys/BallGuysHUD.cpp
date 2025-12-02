#include "BallGuysHUD.h"
#include "Blueprint/UserWidget.h"

ABallGuysHUD::ABallGuysHUD()
{
}

void ABallGuysHUD::BeginPlay()
{
	Super::BeginPlay();

	if (MainHUDWidgetClass)
	{
		MainHUDWidget = CreateWidget<UUserWidget>(GetWorld(), MainHUDWidgetClass);
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport();
		}
	}
}

// (c) 2025 MyLoyalFans. All rights reserved.


#include "WUT_MenuHUD.h"
#include "WUT_MenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

void AWUT_MenuHUD::BeginPlay()
{
    Super::BeginPlay();

    if (!MenuWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("MenuWidgetClass is not set on WUT_MenuHUD"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("WUT_MenuHUD::BeginPlay - World is null"));
        return;
    }

    MenuWidget = CreateWidget<UWUT_MenuWidget>(World, MenuWidgetClass);
    if (MenuWidget)
    {
        MenuWidget->AddToViewport();
        UE_LOG(LogTemp, Log, TEXT("WUT_MenuHUD: Menu widget created and added to viewport."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WUT_MenuHUD: Failed to create menu widget instance."));
    }
}


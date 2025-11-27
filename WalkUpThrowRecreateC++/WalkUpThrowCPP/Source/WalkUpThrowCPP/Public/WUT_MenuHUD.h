// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WUT_MenuHUD.generated.h"

class UWUT_MenuWidget;

/**
 * HUD responsible for creating and owning the main menu widget.
 */
UCLASS()
class WALKUPTHROWCPP_API AWUT_MenuHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    /** Getter so GameMode / BP can access the widget */
    UFUNCTION(BlueprintCallable, Category = "Menu")
    UWUT_MenuWidget* GetMenuWidget() const { return MenuWidget; }

protected:
    /** Widget class to spawn (set this to your WB_Menu in the BP subclass) */
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UWUT_MenuWidget> MenuWidgetClass;

    /** The actual instance added to the viewport */
    UPROPERTY()
    TObjectPtr<UWUT_MenuWidget> MenuWidget;
};

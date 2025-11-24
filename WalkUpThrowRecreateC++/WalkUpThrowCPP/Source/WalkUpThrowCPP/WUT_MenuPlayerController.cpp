// Fill out your copyright notice in the Description page of Project Settings.


#include "WUT_MenuPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "AWUT_MenuGameMode.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"

AWUT_MenuPlayerController::AWUT_MenuPlayerController()
{
    bShowMouseCursor = false;
}

void AWUT_MenuPlayerController::BeginPlay()
{
    Super::BeginPlay();

    AddMenuInputMapping();
}

void AWUT_MenuPlayerController::AddMenuInputMapping()
{
    if (!MenuInputMappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("MenuInputMappingContext is not set on %s"), *GetName());
        return;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("No LocalPlayer found for %s"), *GetName());
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

    if (!Subsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnhancedInputLocalPlayerSubsystem not found for %s"), *GetName());
        return;
    }

    // Priority 0 is fine for menu
    Subsystem->AddMappingContext(MenuInputMappingContext, 0);

    UE_LOG(LogTemp, Log, TEXT("MenuInputMappingContext added for %s"), *GetName());
}

void AWUT_MenuPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInputComp =
        Cast<UEnhancedInputComponent>(InputComponent);

    if (!EnhancedInputComp)
    {
        UE_LOG(LogTemp, Error, TEXT("InputComponent is not an EnhancedInputComponent on %s"), *GetName());
        return;
    }

    if (!MenuStartAction)
    {
        UE_LOG(LogTemp, Warning, TEXT("MenuStartAction is not set on %s"), *GetName());
        return;
    }

    EnhancedInputComp->BindAction(
        MenuStartAction,
        ETriggerEvent::Started,
        this,
        &AWUT_MenuPlayerController::OnMenuStartTriggered
    );
}

void AWUT_MenuPlayerController::OnMenuStartTriggered(const FInputActionValue& Value)
{
    // We don't care about the Value itself for a button.
    UE_LOG(LogTemp, Log, TEXT("Menu Start pressed by controller: %s"), *GetName());

    AWUT_MenuGameMode* MenuGM = GetWorld()->GetAuthGameMode<AWUT_MenuGameMode>();
    if (MenuGM)
    {
        MenuGM->HandleMenuStartPressed(this);
    }
}
// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkMassPedestrianCharacter.h"

#include "Blueprint/UserWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "UObject/ConstructorHelpers.h"

AParkMassPedestrianCharacter::AParkMassPedestrianCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AlertTagComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("AlertTag"));
	AlertTagComponent->SetupAttachment(GetRootComponent());
	AlertTagComponent->SetRelativeLocation(FVector(0.0, 0.0, 210.0));
	AlertTagComponent->SetWidgetSpace(EWidgetSpace::Screen);
	AlertTagComponent->SetDrawSize(FVector2D(160.0, 44.0));
	AlertTagComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AlertTagComponent->SetHiddenInGame(true);
	AlertTagComponent->SetVisibility(false);

	static ConstructorHelpers::FClassFinder<UUserWidget> AlertWidgetFinder(TEXT("/ParkMassAI/Pedestrian/UI/WBP_PedestrianAlertTag"));
	if (AlertWidgetFinder.Succeeded())
	{
		AlertTagWidgetClass = AlertWidgetFinder.Class;
		AlertTagComponent->SetWidgetClass(AlertTagWidgetClass);
	}
}

void AParkMassPedestrianCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!AlertTagWidgetClass)
	{
		AlertTagWidgetClass = LoadClass<UUserWidget>(
			nullptr,
			TEXT("/ParkMassAI/Pedestrian/UI/WBP_PedestrianAlertTag.WBP_PedestrianAlertTag_C"));
	}

	if (AlertTagWidgetClass && AlertTagComponent && AlertTagComponent->GetWidgetClass() == nullptr)
	{
		AlertTagComponent->SetWidgetClass(AlertTagWidgetClass);
	}

	ApplyAlertVisuals();
	UpdateAlertWidgetText();
}

void AParkMassPedestrianCharacter::SetAlertState(const bool bNewAlert)
{
	bIsAlert = bNewAlert;
	ApplyAlertVisuals();
}

void AParkMassPedestrianCharacter::SetAlertText(const FText& InAlertText)
{
	AlertText = InAlertText;
	UpdateAlertWidgetText();
}

void AParkMassPedestrianCharacter::ApplyAlertVisuals()
{
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetRenderCustomDepth(bIsAlert);
		MeshComponent->SetCustomDepthStencilValue(1);
	}

	if (AlertTagComponent)
	{
		AlertTagComponent->SetHiddenInGame(!bIsAlert);
		AlertTagComponent->SetVisibility(bIsAlert, true);
		UpdateAlertWidgetText();
	}
}

void AParkMassPedestrianCharacter::UpdateAlertWidgetText()
{
	if (!AlertTagComponent)
	{
		return;
	}

	UUserWidget* AlertWidget = AlertTagComponent->GetUserWidgetObject();
	if (!AlertWidget)
	{
		AlertTagComponent->InitWidget();
		AlertWidget = AlertTagComponent->GetUserWidgetObject();
	}

	if (!AlertWidget)
	{
		return;
	}

	if (UTextBlock* AlertTextBlock = Cast<UTextBlock>(AlertWidget->GetWidgetFromName(TEXT("AlertTextBlock"))))
	{
		AlertTextBlock->SetText(AlertText);
	}
}

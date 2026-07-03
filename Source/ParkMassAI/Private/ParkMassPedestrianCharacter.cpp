// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkMassPedestrianCharacter.h"

#include "Blueprint/UserWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "ParkMassPedestrianAlertActionSet.h"
#include "ParkMassPedestrianAlertSettings.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogParkMassPedestrianCharacter, Log, All);

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

	static ConstructorHelpers::FClassFinder<UUserWidget> AlertWidgetFinder(TEXT("/ParkMassAI/MassAI/Pedestrian/UI/WBP_PedestrianAlertTag"));
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
			TEXT("/ParkMassAI/MassAI/Pedestrian/UI/WBP_PedestrianAlertTag.WBP_PedestrianAlertTag_C"));
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
	if (!bIsAlert)
	{
		StopAlertAction();
	}
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

bool AParkMassPedestrianCharacter::PlayAlertAction(const FName ActionId)
{
	UE_LOG(
		LogParkMassPedestrianCharacter,
		Log,
		TEXT("PlayAlertAction begin: Actor=%s ActionId=%s CurrentActionId=%s"),
		*GetNameSafe(this),
		*ActionId.ToString(),
		*CurrentAlertActionId.ToString());

	if (ActionId.IsNone())
	{
		UE_LOG(LogParkMassPedestrianCharacter, Log, TEXT("PlayAlertAction no-op: ActionId is None. Actor=%s"), *GetNameSafe(this));
		return true;
	}

	UParkMassPedestrianAlertActionSet* ActionSet = GetEffectiveAlertActionSet();
	UE_LOG(
		LogParkMassPedestrianCharacter,
		Log,
		TEXT("PlayAlertAction state: Actor=%s ActionId=%s AlertActionSetValid=%s AlertActionSet=%s"),
		*GetNameSafe(this),
		*ActionId.ToString(),
		ActionSet ? TEXT("true") : TEXT("false"),
		*GetNameSafe(ActionSet));
	if (!ActionSet)
	{
		UE_LOG(
			LogParkMassPedestrianCharacter,
			Warning,
			TEXT("PlayAlertAction failed: AlertActionSet is invalid. Actor=%s ActionId=%s"),
			*GetNameSafe(this),
			*ActionId.ToString());
		return false;
	}

	const FParkMassPedestrianAlertActionDefinition* Action = ActionSet->FindActionDefinitionById(ActionId);
	UE_LOG(
		LogParkMassPedestrianCharacter,
		Log,
		TEXT("PlayAlertAction state: Actor=%s ActionId=%s ActionDefinitionFound=%s"),
		*GetNameSafe(this),
		*ActionId.ToString(),
		Action ? TEXT("true") : TEXT("false"));
	if (!Action)
	{
		UE_LOG(
			LogParkMassPedestrianCharacter,
			Warning,
			TEXT("PlayAlertAction failed: ActionId not found. Actor=%s ActionId=%s ActionSet=%s"),
			*GetNameSafe(this),
			*ActionId.ToString(),
			*GetNameSafe(ActionSet));
		return false;
	}

	UE_LOG(
		LogParkMassPedestrianCharacter,
		Log,
		TEXT("PlayAlertAction state: Actor=%s ActionId=%s MontageValid=%s Montage=%s"),
		*GetNameSafe(this),
		*ActionId.ToString(),
		Action->Montage ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Action->Montage));
	if (!Action->Montage)
	{
		UE_LOG(
			LogParkMassPedestrianCharacter,
			Warning,
			TEXT("PlayAlertAction failed: Montage is null. Actor=%s ActionId=%s"),
			*GetNameSafe(this),
			*ActionId.ToString());
		return false;
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	UE_LOG(
		LogParkMassPedestrianCharacter,
		Log,
		TEXT("PlayAlertAction state: Actor=%s ActionId=%s MeshValid=%s AnimInstanceValid=%s AnimInstance=%s"),
		*GetNameSafe(this),
		*ActionId.ToString(),
		GetMesh() ? TEXT("true") : TEXT("false"),
		AnimInstance ? TEXT("true") : TEXT("false"),
		*GetNameSafe(AnimInstance));
	if (!AnimInstance)
	{
		UE_LOG(
			LogParkMassPedestrianCharacter,
			Warning,
			TEXT("PlayAlertAction failed: AnimInstance is invalid. Actor=%s ActionId=%s"),
			*GetNameSafe(this),
			*ActionId.ToString());
		return false;
	}

	StopAlertAction();

	const float PlayedLength = AnimInstance->Montage_Play(Action->Montage);
	UE_LOG(
		LogParkMassPedestrianCharacter,
		Log,
		TEXT("PlayAlertAction state: Actor=%s ActionId=%s Montage_Play=%.3f Montage=%s"),
		*GetNameSafe(this),
		*ActionId.ToString(),
		PlayedLength,
		*GetNameSafe(Action->Montage));
	if (PlayedLength <= 0.0f)
	{
		UE_LOG(
			LogParkMassPedestrianCharacter,
			Warning,
			TEXT("PlayAlertAction failed: Montage_Play returned 0. Actor=%s ActionId=%s Montage=%s"),
			*GetNameSafe(this),
			*ActionId.ToString(),
			*GetNameSafe(Action->Montage));
		return false;
	}

	CurrentAlertActionId = ActionId;
	CurrentAlertMontage = Action->Montage;
	UE_LOG(
		LogParkMassPedestrianCharacter,
		Log,
		TEXT("PlayAlertAction: Actor=%s ActionId=%s Montage=%s"),
		*GetNameSafe(this),
		*ActionId.ToString(),
		*GetNameSafe(CurrentAlertMontage));
	return true;
}

void AParkMassPedestrianCharacter::StopAlertAction()
{
	if (!CurrentAlertMontage)
	{
		CurrentAlertActionId = NAME_None;
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->Montage_Stop(0.2f, CurrentAlertMontage);
	}

	CurrentAlertActionId = NAME_None;
	CurrentAlertMontage = nullptr;
}

UParkMassPedestrianAlertActionSet* AParkMassPedestrianCharacter::GetEffectiveAlertActionSet()
{
	if (AlertActionSet)
	{
		UE_LOG(
			LogParkMassPedestrianCharacter,
			Log,
			TEXT("GetEffectiveAlertActionSet: Using character override AlertActionSet. Actor=%s ActionSet=%s"),
			*GetNameSafe(this),
			*GetNameSafe(AlertActionSet));
		return AlertActionSet;
	}

	const UParkMassPedestrianAlertSettings* Settings = GetDefault<UParkMassPedestrianAlertSettings>();
	UE_LOG(
		LogParkMassPedestrianCharacter,
		Log,
		TEXT("GetEffectiveAlertActionSet: Character override empty. SettingsValid=%s Actor=%s"),
		Settings ? TEXT("true") : TEXT("false"),
		*GetNameSafe(this));

	if (!Settings)
	{
		UE_LOG(LogParkMassPedestrianCharacter, Warning, TEXT("GetEffectiveAlertActionSet failed: settings object is invalid. Actor=%s"), *GetNameSafe(this));
		return nullptr;
	}

	const TSoftObjectPtr<UParkMassPedestrianAlertActionSet>& ConfiguredActionSet = Settings->DefaultAlertActionSet;
	UE_LOG(
		LogParkMassPedestrianCharacter,
		Log,
		TEXT("GetEffectiveAlertActionSet: Using settings DefaultAlertActionSet. Configured=%s Path=%s Actor=%s"),
		ConfiguredActionSet.IsNull() ? TEXT("false") : TEXT("true"),
		*ConfiguredActionSet.ToSoftObjectPath().ToString(),
		*GetNameSafe(this));

	if (ConfiguredActionSet.IsNull())
	{
		UE_LOG(LogParkMassPedestrianCharacter, Warning, TEXT("GetEffectiveAlertActionSet failed: Settings.DefaultAlertActionSet is not configured. Actor=%s"), *GetNameSafe(this));
		return nullptr;
	}

	UParkMassPedestrianAlertActionSet* LoadedActionSet = ConfiguredActionSet.LoadSynchronous();
	UE_LOG(
		LogParkMassPedestrianCharacter,
		Log,
		TEXT("GetEffectiveAlertActionSet: Settings LoadSucceeded=%s ActionSet=%s Actor=%s"),
		LoadedActionSet ? TEXT("true") : TEXT("false"),
		*GetNameSafe(LoadedActionSet),
		*GetNameSafe(this));

	if (!LoadedActionSet)
	{
		UE_LOG(
			LogParkMassPedestrianCharacter,
			Warning,
			TEXT("GetEffectiveAlertActionSet failed: could not load Settings.DefaultAlertActionSet Path=%s Actor=%s"),
			*ConfiguredActionSet.ToSoftObjectPath().ToString(),
			*GetNameSafe(this));
	}

	return LoadedActionSet;
}

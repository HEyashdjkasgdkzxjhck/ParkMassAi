// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkMassPedestrianEventDirector.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "ParkMassPedestrianAlertSubsystem.h"
#include "ParkMassPedestrianCharacter.h"
#include "ParkMassPedestrianEventPoint.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogParkMassPedestrianEventDirector, Log, All);

AParkMassPedestrianEventDirector::AParkMassPedestrianEventDirector()
{
	PrimaryActorTick.bCanEverTick = false;
	CandidateClass = AParkMassPedestrianCharacter::StaticClass();
}

void AParkMassPedestrianEventDirector::BeginPlay()
{
	Super::BeginPlay();
	RefreshRandomAlertTimer();
}

void AParkMassPedestrianEventDirector::TriggerRandomAlert()
{
	UParkMassPedestrianAlertSubsystem* AlertSubsystem =
		GetWorld() ? GetWorld()->GetSubsystem<UParkMassPedestrianAlertSubsystem>() : nullptr;
	if (!AlertSubsystem)
	{
		UE_LOG(LogParkMassPedestrianEventDirector, Warning, TEXT("TriggerRandomAlert skipped: alert subsystem is invalid"));
		return;
	}

	if (!AlertSubsystem->IsRandomAlertAllowed())
	{
		UE_LOG(
			LogParkMassPedestrianEventDirector,
			Log,
			TEXT("TriggerRandomAlert skipped by subsystem mode. AlertSourceMode=%s"),
			*UParkMassPedestrianAlertSubsystem::AlertSourceModeToString(AlertSubsystem->GetAlertSourceMode()));
		return;
	}

	switch (RandomMode)
	{
	case EParkMassPedestrianRandomMode::ExistingPedestrian:
		AlertSubsystem->TriggerRandomPedestrianAlert(NSLOCTEXT("ParkMassAI", "DefaultRandomPedestrianAlertText", "异常行为"), RandomAlertDuration);
		break;
	case EParkMassPedestrianRandomMode::EventPoint:
		TrySpawnFromEventPoint();
		break;
	case EParkMassPedestrianRandomMode::Hybrid:
		if (!AlertSubsystem->TriggerRandomPedestrianAlert(NSLOCTEXT("ParkMassAI", "DefaultRandomPedestrianAlertText", "异常行为"), RandomAlertDuration))
		{
			TrySpawnFromEventPoint();
		}
		break;
	default:
		break;
	}
}

void AParkMassPedestrianEventDirector::SetExternalAlertMode(const bool bEnabled)
{
	if (UParkMassPedestrianAlertSubsystem* AlertSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UParkMassPedestrianAlertSubsystem>() : nullptr)
	{
		AlertSubsystem->SetExternalAlertMode(bEnabled);
	}
}

void AParkMassPedestrianEventDirector::SetAlertSourceMode(const EPedestrianAlertSourceMode NewMode)
{
	if (UParkMassPedestrianAlertSubsystem* AlertSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UParkMassPedestrianAlertSubsystem>() : nullptr)
	{
		AlertSubsystem->SetAlertSourceMode(NewMode);
	}
}

bool AParkMassPedestrianEventDirector::TriggerPointAlert(
	const FName PointName,
	const FText AlertText,
	const float Duration,
	const int32 SpawnCount,
	const bool bDestroyAfterAlert)
{
	if (UParkMassPedestrianAlertSubsystem* AlertSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UParkMassPedestrianAlertSubsystem>() : nullptr)
	{
		return AlertSubsystem->TriggerPointAlert(PointName, AlertText, Duration, SpawnCount, bDestroyAfterAlert);
	}

	return false;
}

bool AParkMassPedestrianEventDirector::TriggerRandomPedestrianAlert(const FText AlertText, const float Duration)
{
	if (UParkMassPedestrianAlertSubsystem* AlertSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UParkMassPedestrianAlertSubsystem>() : nullptr)
	{
		return AlertSubsystem->TriggerRandomPedestrianAlert(AlertText, Duration);
	}

	return false;
}

bool AParkMassPedestrianEventDirector::TriggerActorAlert(AActor* TargetActor, const FText AlertText, const float Duration)
{
	if (UParkMassPedestrianAlertSubsystem* AlertSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UParkMassPedestrianAlertSubsystem>() : nullptr)
	{
		return AlertSubsystem->TriggerActorAlert(TargetActor, AlertText, Duration);
	}

	return false;
}

int32 AParkMassPedestrianEventDirector::ClearAllPedestrianAlerts()
{
	if (UParkMassPedestrianAlertSubsystem* AlertSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UParkMassPedestrianAlertSubsystem>() : nullptr)
	{
		return AlertSubsystem->ClearAllPedestrianAlerts();
	}

	return 0;
}

void AParkMassPedestrianEventDirector::RefreshRandomAlertTimer()
{
	if (RandomAlertTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(RandomAlertTimerHandle);
	}

	if (!bEnableRandomAlert)
	{
		UE_LOG(LogParkMassPedestrianEventDirector, Log, TEXT("Random alert timer stopped: bEnableRandomAlert=false"));
		return;
	}

	GetWorldTimerManager().SetTimer(
		RandomAlertTimerHandle,
		this,
		&AParkMassPedestrianEventDirector::TriggerRandomAlert,
		RandomAlertInterval,
		true,
		RandomAlertInterval);

	UE_LOG(LogParkMassPedestrianEventDirector, Log, TEXT("Random alert timer started. Interval=%.2f"), RandomAlertInterval);
}

bool AParkMassPedestrianEventDirector::IsRandomAlertAllowed() const
{
	const UParkMassPedestrianAlertSubsystem* AlertSubsystem =
		GetWorld() ? GetWorld()->GetSubsystem<UParkMassPedestrianAlertSubsystem>() : nullptr;
	return AlertSubsystem && AlertSubsystem->IsRandomAlertAllowed();
}

bool AParkMassPedestrianEventDirector::TrySpawnFromEventPoint()
{
	UWorld* World = GetWorld();
	if (!World || !IsRandomAlertAllowed())
	{
		return false;
	}

	TArray<AActor*> EventPointActors;
	UGameplayStatics::GetAllActorsOfClass(World, AParkMassPedestrianEventPoint::StaticClass(), EventPointActors);
	if (EventPointActors.IsEmpty())
	{
		return false;
	}

	AParkMassPedestrianEventPoint* EventPoint = Cast<AParkMassPedestrianEventPoint>(
		EventPointActors[FMath::RandRange(0, EventPointActors.Num() - 1)]);
	if (!IsValid(EventPoint))
	{
		return false;
	}

	EventPoint->SpawnAlertPedestrianWithParams(
		NSLOCTEXT("ParkMassAI", "DefaultEventPointRandomAlertText", "异常行为"),
		RandomAlertDuration,
		RandomAlertCount,
		false);
	return true;
}


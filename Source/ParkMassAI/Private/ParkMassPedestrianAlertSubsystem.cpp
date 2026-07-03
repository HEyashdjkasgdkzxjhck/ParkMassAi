// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkMassPedestrianAlertSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "ParkMassPedestrianCharacter.h"
#include "ParkMassPedestrianEventPoint.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogParkMassPedestrianAlertSubsystem, Log, All);

void UParkMassPedestrianAlertSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(
		LogParkMassPedestrianAlertSubsystem,
		Log,
		TEXT("ParkMass pedestrian alert subsystem initialized. AlertSourceMode=%s"),
		*AlertSourceModeToString(AlertSourceMode));
}

void UParkMassPedestrianAlertSubsystem::SetExternalAlertMode(const bool bEnabled)
{
	SetAlertSourceMode(bEnabled ? EPedestrianAlertSourceMode::ExternalOnly : EPedestrianAlertSourceMode::DemoRandom);
	UE_LOG(
		LogParkMassPedestrianAlertSubsystem,
		Log,
		TEXT("%s"),
		bEnabled
			? TEXT("External alert mode enabled, demo random disabled")
			: TEXT("External alert mode disabled, demo random enabled"));
}

void UParkMassPedestrianAlertSubsystem::SetAlertSourceMode(const EPedestrianAlertSourceMode NewMode)
{
	AlertSourceMode = NewMode;
	UE_LOG(
		LogParkMassPedestrianAlertSubsystem,
		Log,
		TEXT("SetAlertSourceMode: AlertSourceMode=%s"),
		*AlertSourceModeToString(AlertSourceMode));
}

EPedestrianAlertSourceMode UParkMassPedestrianAlertSubsystem::GetAlertSourceMode() const
{
	return AlertSourceMode;
}

bool UParkMassPedestrianAlertSubsystem::IsRandomAlertAllowed() const
{
	return AlertSourceMode == EPedestrianAlertSourceMode::DemoRandom
		|| AlertSourceMode == EPedestrianAlertSourceMode::Hybrid;
}

bool UParkMassPedestrianAlertSubsystem::IsExternalAlertAllowed() const
{
	return AlertSourceMode != EPedestrianAlertSourceMode::Disabled;
}

bool UParkMassPedestrianAlertSubsystem::TriggerPointAlert(
	const FName PointName,
	const FText& AlertText,
	const float Duration,
	const int32 SpawnCount,
	const bool bDestroyAfterAlert)
{
	UE_LOG(
		LogParkMassPedestrianAlertSubsystem,
		Log,
		TEXT("TriggerPointAlert Input: Mode=%s PointName=%s AlertText=%s Duration=%.2f SpawnCount=%d bDestroyAfterAlert=%s"),
		*AlertSourceModeToString(AlertSourceMode),
		*PointName.ToString(),
		*AlertText.ToString(),
		Duration,
		SpawnCount,
		bDestroyAfterAlert ? TEXT("true") : TEXT("false"));

	if (!IsExternalAlertAllowed())
	{
		UE_LOG(
			LogParkMassPedestrianAlertSubsystem,
			Warning,
			TEXT("TriggerPointAlert rejected. AlertSourceMode=%s"),
			*AlertSourceModeToString(AlertSourceMode));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogParkMassPedestrianAlertSubsystem, Error, TEXT("TriggerPointAlert failed: no world"));
		return false;
	}

	TArray<AActor*> EventPointActors;
	UGameplayStatics::GetAllActorsOfClass(World, AParkMassPedestrianEventPoint::StaticClass(), EventPointActors);

	TArray<AParkMassPedestrianEventPoint*> Matches;
	for (AActor* Actor : EventPointActors)
	{
		AParkMassPedestrianEventPoint* EventPoint = Cast<AParkMassPedestrianEventPoint>(Actor);
		if (IsValid(EventPoint) && EventPoint->PointName == PointName)
		{
			Matches.Add(EventPoint);
		}
	}

	if (Matches.IsEmpty())
	{
		UE_LOG(
			LogParkMassPedestrianAlertSubsystem,
			Error,
			TEXT("TriggerPointAlert failed: PointName not found. PointName=%s"),
			*PointName.ToString());
		return false;
	}

	if (Matches.Num() > 1)
	{
		UE_LOG(
			LogParkMassPedestrianAlertSubsystem,
			Error,
			TEXT("TriggerPointAlert failed: PointName duplicated. PointName=%s Count=%d"),
			*PointName.ToString(),
			Matches.Num());
		for (const AParkMassPedestrianEventPoint* EventPoint : Matches)
		{
			UE_LOG(
				LogParkMassPedestrianAlertSubsystem,
				Error,
				TEXT("Duplicate EventPoint: Actor=%s Location=%s"),
				*GetNameSafe(EventPoint),
				*EventPoint->GetActorLocation().ToString());
		}
		return false;
	}

	AParkMassPedestrianEventPoint* EventPoint = Matches[0];
	EventPoint->SpawnAlertPedestrianWithParams(
		AlertText,
		FMath::Max(0.0f, Duration),
		FMath::Max(1, SpawnCount),
		bDestroyAfterAlert);

	UE_LOG(
		LogParkMassPedestrianAlertSubsystem,
		Log,
		TEXT("TriggerPointAlert Success: PointName=%s EventPoint=%s Location=%s AlertText=%s Duration=%.2f SpawnCount=%d bDestroyAfterAlert=%s"),
		*PointName.ToString(),
		*GetNameSafe(EventPoint),
		*EventPoint->GetActorLocation().ToString(),
		*AlertText.ToString(),
		Duration,
		SpawnCount,
		bDestroyAfterAlert ? TEXT("true") : TEXT("false"));
	return true;
}

bool UParkMassPedestrianAlertSubsystem::TriggerActorAlert(AActor* TargetActor, const FText& AlertText, const float Duration)
{
	if (!IsExternalAlertAllowed())
	{
		UE_LOG(
			LogParkMassPedestrianAlertSubsystem,
			Warning,
			TEXT("TriggerActorAlert rejected. AlertSourceMode=%s"),
			*AlertSourceModeToString(AlertSourceMode));
		return false;
	}

	AParkMassPedestrianCharacter* Pedestrian = Cast<AParkMassPedestrianCharacter>(TargetActor);
	if (!IsValid(Pedestrian))
	{
		UE_LOG(
			LogParkMassPedestrianAlertSubsystem,
			Warning,
			TEXT("TriggerActorAlert failed: TargetActor is not AParkMassPedestrianCharacter. TargetActor=%s"),
			*GetNameSafe(TargetActor));
		return false;
	}

	Pedestrian->SetAlertText(AlertText);
	Pedestrian->SetAlertState(true);

	if (Duration > 0.0f)
	{
		FTimerDelegate ClearDelegate = FTimerDelegate::CreateWeakLambda(this, [this, Pedestrian]()
		{
			ClearAlert(Pedestrian);
		});
		FTimerHandle ClearHandle;
		GetWorld()->GetTimerManager().SetTimer(ClearHandle, ClearDelegate, Duration, false);
	}

	UE_LOG(
		LogParkMassPedestrianAlertSubsystem,
		Log,
		TEXT("TriggerActorAlert Success: Actor=%s AlertText=%s Duration=%.2f"),
		*GetNameSafe(Pedestrian),
		*AlertText.ToString(),
		Duration);
	return true;
}

bool UParkMassPedestrianAlertSubsystem::TriggerRandomPedestrianAlert(const FText& AlertText, const float Duration)
{
	if (!IsRandomAlertAllowed())
	{
		UE_LOG(
			LogParkMassPedestrianAlertSubsystem,
			Warning,
			TEXT("TriggerRandomPedestrianAlert rejected. Random alert disabled in AlertSourceMode=%s"),
			*AlertSourceModeToString(AlertSourceMode));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	TArray<AActor*> PedestrianActors;
	UGameplayStatics::GetAllActorsOfClass(World, AParkMassPedestrianCharacter::StaticClass(), PedestrianActors);

	TArray<AParkMassPedestrianCharacter*> AvailablePedestrians;
	for (AActor* Actor : PedestrianActors)
	{
		AParkMassPedestrianCharacter* Pedestrian = Cast<AParkMassPedestrianCharacter>(Actor);
		if (IsValid(Pedestrian) && !Pedestrian->IsAlert())
		{
			AvailablePedestrians.Add(Pedestrian);
		}
	}

	if (AvailablePedestrians.IsEmpty())
	{
		UE_LOG(LogParkMassPedestrianAlertSubsystem, Warning, TEXT("TriggerRandomPedestrianAlert failed: no available pedestrians"));
		return false;
	}

	AParkMassPedestrianCharacter* Pedestrian = AvailablePedestrians[FMath::RandRange(0, AvailablePedestrians.Num() - 1)];
	Pedestrian->SetAlertText(AlertText);
	Pedestrian->SetAlertState(true);

	if (Duration > 0.0f)
	{
		FTimerDelegate ClearDelegate = FTimerDelegate::CreateWeakLambda(this, [this, Pedestrian]()
		{
			ClearAlert(Pedestrian);
		});
		FTimerHandle ClearHandle;
		World->GetTimerManager().SetTimer(ClearHandle, ClearDelegate, Duration, false);
	}

	UE_LOG(
		LogParkMassPedestrianAlertSubsystem,
		Log,
		TEXT("TriggerRandomPedestrianAlert Success: Actor=%s AlertText=%s Duration=%.2f"),
		*GetNameSafe(Pedestrian),
		*AlertText.ToString(),
		Duration);
	return true;
}

int32 UParkMassPedestrianAlertSubsystem::ClearAllPedestrianAlerts()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	TArray<AActor*> PedestrianActors;
	UGameplayStatics::GetAllActorsOfClass(World, AParkMassPedestrianCharacter::StaticClass(), PedestrianActors);

	int32 ClearedCount = 0;
	for (AActor* Actor : PedestrianActors)
	{
		AParkMassPedestrianCharacter* Pedestrian = Cast<AParkMassPedestrianCharacter>(Actor);
		if (IsValid(Pedestrian) && Pedestrian->IsAlert())
		{
			Pedestrian->SetAlertState(false);
			++ClearedCount;
		}
	}

	UE_LOG(LogParkMassPedestrianAlertSubsystem, Log, TEXT("ClearAllPedestrianAlerts: ClearedCount=%d"), ClearedCount);
	return ClearedCount;
}

FString UParkMassPedestrianAlertSubsystem::AlertSourceModeToString(const EPedestrianAlertSourceMode Mode)
{
	switch (Mode)
	{
	case EPedestrianAlertSourceMode::DemoRandom:
		return TEXT("DemoRandom");
	case EPedestrianAlertSourceMode::ExternalOnly:
		return TEXT("ExternalOnly");
	case EPedestrianAlertSourceMode::Hybrid:
		return TEXT("Hybrid");
	case EPedestrianAlertSourceMode::Disabled:
		return TEXT("Disabled");
	default:
		return TEXT("Unknown");
	}
}

void UParkMassPedestrianAlertSubsystem::ClearAlert(AParkMassPedestrianCharacter* Pedestrian)
{
	if (IsValid(Pedestrian))
	{
		Pedestrian->SetAlertState(false);
	}
}


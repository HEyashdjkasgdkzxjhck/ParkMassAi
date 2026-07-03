// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkMassPedestrianAlertBlueprintLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "ParkMassPedestrianAlertSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogParkMassPedestrianAlertBlueprintLibrary, Log, All);

namespace
{
	UParkMassPedestrianAlertSubsystem* GetAlertSubsystem(const UObject* WorldContextObject)
	{
		if (!GEngine)
		{
			UE_LOG(LogParkMassPedestrianAlertBlueprintLibrary, Warning, TEXT("ParkMass alert call failed: GEngine is invalid"));
			return nullptr;
		}

		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
		if (!World)
		{
			UE_LOG(LogParkMassPedestrianAlertBlueprintLibrary, Warning, TEXT("ParkMass alert call failed: WorldContextObject has no valid world"));
			return nullptr;
		}

		UParkMassPedestrianAlertSubsystem* Subsystem = World->GetSubsystem<UParkMassPedestrianAlertSubsystem>();
		if (!Subsystem)
		{
			UE_LOG(LogParkMassPedestrianAlertBlueprintLibrary, Warning, TEXT("ParkMass alert call failed: UParkMassPedestrianAlertSubsystem is invalid"));
		}
		return Subsystem;
	}
}

void UParkMassPedestrianAlertBlueprintLibrary::ParkMass_SetExternalAlertMode(
	const UObject* WorldContextObject,
	const bool bEnabled)
{
	if (UParkMassPedestrianAlertSubsystem* Subsystem = GetAlertSubsystem(WorldContextObject))
	{
		Subsystem->SetExternalAlertMode(bEnabled);
	}
}

void UParkMassPedestrianAlertBlueprintLibrary::ParkMass_SetPedestrianAlertSourceMode(
	const UObject* WorldContextObject,
	const EPedestrianAlertSourceMode NewMode)
{
	if (UParkMassPedestrianAlertSubsystem* Subsystem = GetAlertSubsystem(WorldContextObject))
	{
		Subsystem->SetAlertSourceMode(NewMode);
	}
}

EPedestrianAlertSourceMode UParkMassPedestrianAlertBlueprintLibrary::ParkMass_GetPedestrianAlertSourceMode(
	const UObject* WorldContextObject)
{
	if (const UParkMassPedestrianAlertSubsystem* Subsystem = GetAlertSubsystem(WorldContextObject))
	{
		return Subsystem->GetAlertSourceMode();
	}

	return EPedestrianAlertSourceMode::DemoRandom;
}

bool UParkMassPedestrianAlertBlueprintLibrary::ParkMass_TriggerPointAlert(
	const UObject* WorldContextObject,
	const FName PointName,
	const FText AlertText,
	const float Duration,
	const int32 SpawnCount,
	const bool bDestroyAfterAlert)
{
	UE_LOG(
		LogParkMassPedestrianAlertBlueprintLibrary,
		Log,
		TEXT("ParkMass_TriggerPointAlert: PointName=%s AlertText=%s Duration=%.2f SpawnCount=%d bDestroyAfterAlert=%s"),
		*PointName.ToString(),
		*AlertText.ToString(),
		Duration,
		SpawnCount,
		bDestroyAfterAlert ? TEXT("true") : TEXT("false"));

	if (UParkMassPedestrianAlertSubsystem* Subsystem = GetAlertSubsystem(WorldContextObject))
	{
		return Subsystem->TriggerPointAlert(PointName, AlertText, Duration, SpawnCount, bDestroyAfterAlert);
	}

	return false;
}

bool UParkMassPedestrianAlertBlueprintLibrary::ParkMass_TriggerPointAlertWithAction(
	const UObject* WorldContextObject,
	const FName PointName,
	const FText AlertText,
	const float Duration,
	const int32 SpawnCount,
	const bool bDestroyAfterAlert,
	const FName ActionId)
{
	UE_LOG(
		LogParkMassPedestrianAlertBlueprintLibrary,
		Log,
		TEXT("ParkMass_TriggerPointAlertWithAction: PointName=%s AlertText=%s Duration=%.2f SpawnCount=%d bDestroyAfterAlert=%s ActionId=%s"),
		*PointName.ToString(),
		*AlertText.ToString(),
		Duration,
		SpawnCount,
		bDestroyAfterAlert ? TEXT("true") : TEXT("false"),
		*ActionId.ToString());

	if (UParkMassPedestrianAlertSubsystem* Subsystem = GetAlertSubsystem(WorldContextObject))
	{
		return Subsystem->TriggerPointAlertWithAction(PointName, AlertText, Duration, SpawnCount, bDestroyAfterAlert, ActionId);
	}

	return false;
}

bool UParkMassPedestrianAlertBlueprintLibrary::ParkMass_TriggerActorAlert(
	const UObject* WorldContextObject,
	AActor* TargetActor,
	const FText AlertText,
	const float Duration)
{
	if (UParkMassPedestrianAlertSubsystem* Subsystem = GetAlertSubsystem(WorldContextObject))
	{
		return Subsystem->TriggerActorAlert(TargetActor, AlertText, Duration);
	}

	return false;
}

bool UParkMassPedestrianAlertBlueprintLibrary::ParkMass_TriggerRandomPedestrianAlert(
	const UObject* WorldContextObject,
	const FText AlertText,
	const float Duration)
{
	if (UParkMassPedestrianAlertSubsystem* Subsystem = GetAlertSubsystem(WorldContextObject))
	{
		return Subsystem->TriggerRandomPedestrianAlert(AlertText, Duration);
	}

	return false;
}

int32 UParkMassPedestrianAlertBlueprintLibrary::ParkMass_ClearAllPedestrianAlerts(const UObject* WorldContextObject)
{
	if (UParkMassPedestrianAlertSubsystem* Subsystem = GetAlertSubsystem(WorldContextObject))
	{
		return Subsystem->ClearAllPedestrianAlerts();
	}

	return 0;
}

TArray<FParkMassPedestrianAlertActionCatalogItem> UParkMassPedestrianAlertBlueprintLibrary::ParkMass_GetPedestrianAlertActionCatalog(
	const UObject* WorldContextObject)
{
	if (UParkMassPedestrianAlertSubsystem* Subsystem = GetAlertSubsystem(WorldContextObject))
	{
		return Subsystem->GetPedestrianAlertActionCatalog();
	}

	return {};
}

TArray<FName> UParkMassPedestrianAlertBlueprintLibrary::ParkMass_GetPedestrianAlertActionIds(
	const UObject* WorldContextObject)
{
	if (UParkMassPedestrianAlertSubsystem* Subsystem = GetAlertSubsystem(WorldContextObject))
	{
		return Subsystem->GetPedestrianAlertActionIds();
	}

	return {};
}

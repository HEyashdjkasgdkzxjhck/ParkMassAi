// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ParkMassPedestrianAlertActionSet.h"
#include "ParkMassPedestrianAlertTypes.h"
#include "ParkMassPedestrianAlertBlueprintLibrary.generated.h"

UCLASS()
class PARKMASSAI_API UParkMassPedestrianAlertBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts", meta = (WorldContext = "WorldContextObject"))
	static void ParkMass_SetExternalAlertMode(const UObject* WorldContextObject, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts", meta = (WorldContext = "WorldContextObject"))
	static void ParkMass_SetPedestrianAlertSourceMode(const UObject* WorldContextObject, EPedestrianAlertSourceMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts", meta = (WorldContext = "WorldContextObject"))
	static EPedestrianAlertSourceMode ParkMass_GetPedestrianAlertSourceMode(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts", meta = (WorldContext = "WorldContextObject"))
	static bool ParkMass_TriggerPointAlert(const UObject* WorldContextObject, FName PointName, FText AlertText, float Duration, int32 SpawnCount, bool bDestroyAfterAlert);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts", meta = (WorldContext = "WorldContextObject"))
	static bool ParkMass_TriggerPointAlertWithAction(const UObject* WorldContextObject, FName PointName, FText AlertText, float Duration, int32 SpawnCount, bool bDestroyAfterAlert, FName ActionId);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts", meta = (WorldContext = "WorldContextObject"))
	static bool ParkMass_TriggerActorAlert(const UObject* WorldContextObject, AActor* TargetActor, FText AlertText, float Duration);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts", meta = (WorldContext = "WorldContextObject"))
	static bool ParkMass_TriggerRandomPedestrianAlert(const UObject* WorldContextObject, FText AlertText, float Duration);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts", meta = (WorldContext = "WorldContextObject"))
	static int32 ParkMass_ClearAllPedestrianAlerts(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alert Actions", meta = (WorldContext = "WorldContextObject"))
	static TArray<FParkMassPedestrianAlertActionCatalogItem> ParkMass_GetPedestrianAlertActionCatalog(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alert Actions", meta = (WorldContext = "WorldContextObject"))
	static TArray<FName> ParkMass_GetPedestrianAlertActionIds(const UObject* WorldContextObject);
};

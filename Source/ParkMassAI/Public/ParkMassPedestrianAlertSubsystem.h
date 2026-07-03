// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ParkMassPedestrianAlertTypes.h"
#include "ParkMassPedestrianAlertSubsystem.generated.h"

class AParkMassPedestrianCharacter;

UCLASS()
class PARKMASSAI_API UParkMassPedestrianAlertSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	void SetExternalAlertMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	void SetAlertSourceMode(EPedestrianAlertSourceMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	EPedestrianAlertSourceMode GetAlertSourceMode() const;

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	bool IsRandomAlertAllowed() const;

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	bool IsExternalAlertAllowed() const;

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	bool TriggerPointAlert(FName PointName, const FText& AlertText, float Duration, int32 SpawnCount, bool bDestroyAfterAlert);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	bool TriggerActorAlert(AActor* TargetActor, const FText& AlertText, float Duration);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	bool TriggerRandomPedestrianAlert(const FText& AlertText, float Duration);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	int32 ClearAllPedestrianAlerts();

	static FString AlertSourceModeToString(EPedestrianAlertSourceMode Mode);

private:
	UPROPERTY()
	EPedestrianAlertSourceMode AlertSourceMode = EPedestrianAlertSourceMode::DemoRandom;

	void ClearAlert(AParkMassPedestrianCharacter* Pedestrian);
};


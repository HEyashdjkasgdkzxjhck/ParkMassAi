// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ParkMassPedestrianAlertTypes.h"
#include "ParkMassPedestrianEventDirector.generated.h"

class AParkMassPedestrianCharacter;
class AParkMassPedestrianEventPoint;

UENUM(BlueprintType)
enum class EParkMassPedestrianRandomMode : uint8
{
	ExistingPedestrian UMETA(DisplayName = "Existing Pedestrian"),
	EventPoint UMETA(DisplayName = "Event Point"),
	Hybrid UMETA(DisplayName = "Hybrid")
};

UCLASS(Blueprintable)
class PARKMASSAI_API AParkMassPedestrianEventDirector : public AActor
{
	GENERATED_BODY()

public:
	AParkMassPedestrianEventDirector();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Events")
	void TriggerRandomAlert();

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	void SetExternalAlertMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	void SetAlertSourceMode(EPedestrianAlertSourceMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	bool TriggerPointAlert(FName PointName, FText AlertText, float Duration, int32 SpawnCount, bool bDestroyAfterAlert);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	bool TriggerRandomPedestrianAlert(FText AlertText, float Duration);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	bool TriggerActorAlert(AActor* TargetActor, FText AlertText, float Duration);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Alerts")
	int32 ClearAllPedestrianAlerts();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events")
	bool bEnableRandomAlert = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Alerts")
	bool bDisableRandomWhenExternalMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events", meta = (ClampMin = "0.1"))
	float RandomAlertInterval = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events", meta = (ClampMin = "0.0"))
	float RandomAlertDuration = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events", meta = (ClampMin = "1"))
	int32 RandomAlertCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events")
	TSubclassOf<AParkMassPedestrianCharacter> CandidateClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events")
	EParkMassPedestrianRandomMode RandomMode = EParkMassPedestrianRandomMode::ExistingPedestrian;

private:
	FTimerHandle RandomAlertTimerHandle;

	void RefreshRandomAlertTimer();
	bool IsRandomAlertAllowed() const;
	bool TrySpawnFromEventPoint();
};

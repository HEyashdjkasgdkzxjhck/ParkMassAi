// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityManager.h"
#include "ParkMassPedestrianEventPoint.generated.h"

class AParkMassPedestrianCharacter;
class UMassEntityConfigAsset;
class USphereComponent;
class UTextRenderComponent;

UCLASS(Blueprintable)
class PARKMASSAI_API AParkMassPedestrianEventPoint : public AActor
{
	GENERATED_BODY()

public:
	AParkMassPedestrianEventPoint();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Events")
	void SpawnPedestrians();

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Events")
	void SpawnAlertPedestrian();

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Events")
	void SpawnAlertPedestrianWithParams(FText InAlertText, float InDuration, int32 InSpawnCount, bool bInDestroyAfterAlert);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Events")
	void SetOwnedPedestriansAlert(bool bAlert);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian Events")
	void ClearOwnedPedestrians();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events")
	FName PointName = TEXT("PedestrianEventPoint");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events")
	TObjectPtr<UMassEntityConfigAsset> EntityConfig = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events", meta = (ClampMin = "1"))
	int32 SpawnCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events", meta = (ClampMin = "0.0"))
	float SpawnRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events", meta = (ClampMin = "0.0"))
	float ClaimRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events", meta = (ClampMin = "0"))
	int32 MaxClaimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events")
	bool bAutoSpawnOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events")
	bool bSpawnAsAlert = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events")
	FText AlertText = NSLOCTEXT("ParkMassAI", "DefaultEventPointAlertText", "异常行为");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events", meta = (ClampMin = "0.0"))
	float AlertDuration = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events")
	bool bDestroyAfterAlert = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events", meta = (ClampMin = "0.0"))
	float DestroyDelayAfterAlertEnd = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events")
	bool bLoop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian Events", meta = (ClampMin = "0.1"))
	float LoopInterval = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Events")
	TArray<TObjectPtr<AParkMassPedestrianCharacter>> OwnedPedestrians;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Events")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Events")
	TObjectPtr<USphereComponent> SpawnRadiusComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Events")
	TObjectPtr<UTextRenderComponent> PointNameComponent = nullptr;

private:
	TArray<FMassEntityHandle> OwnedEntities;
	TArray<FVector> PendingSpawnLocations;
	TArray<TWeakObjectPtr<AParkMassPedestrianCharacter>> PedestriansBeforeSpawn;
	FText PendingAlertText;
	float PendingAlertDuration = 0.0f;
	bool bPendingDestroyAfterAlert = false;
	float PendingDestroyDelayAfterAlertEnd = 0.2f;
	int32 PendingExpectedSpawnCount = 0;
	int32 PendingOwnedStartCount = 0;
	int32 PendingResolveAttempts = 0;
	FTimerHandle LoopTimerHandle;
	FTimerHandle ResolveTimerHandle;
	FTimerHandle ClearAlertTimerHandle;
	FTimerHandle DestroyAfterAlertTimerHandle;

	void SpawnPedestriansInternal(bool bForceAlert);
	void ResolveRecentlySpawnedPedestrians(bool bSetAlert);
	void ScheduleResolveRecentlySpawnedPedestrians(bool bSetAlert);
	void EndAlertForOwnedPedestrians();
	void DestroyOwnedPedestrians();
	int32 GetEffectiveMaxClaimCount() const;
	FVector MakeRandomSpawnLocation() const;
	UMassEntityConfigAsset* GetEffectiveEntityConfig() const;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ParkMassPedestrianAlertActionSet.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct PARKMASSAI_API FParkMassPedestrianAlertActionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action")
	FName ActionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action", meta = (ClampMin = "0.0"))
	float DefaultDuration = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action")
	bool bLoop = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action")
	bool bStopMovementDuringAction = false;
};

USTRUCT(BlueprintType)
struct PARKMASSAI_API FParkMassPedestrianAlertActionCatalogItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action")
	FName ActionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action", meta = (ClampMin = "0.0"))
	float DefaultDuration = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action")
	bool bLoop = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Action")
	bool bStopMovementDuringAction = false;
};

UCLASS(BlueprintType)
class PARKMASSAI_API UParkMassPedestrianAlertActionSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Actions")
	TArray<FParkMassPedestrianAlertActionDefinition> Actions;

	UFUNCTION(BlueprintPure, Category = "ParkMassAI|Pedestrian Alert Actions")
	bool FindActionById(FName ActionId, FParkMassPedestrianAlertActionDefinition& OutAction) const;

	const FParkMassPedestrianAlertActionDefinition* FindActionDefinitionById(FName ActionId) const;

	UFUNCTION(BlueprintPure, Category = "ParkMassAI|Pedestrian Alert Actions")
	TArray<FName> GetAvailableActionIds() const;

	UFUNCTION(BlueprintPure, Category = "ParkMassAI|Pedestrian Alert Actions")
	TArray<FParkMassPedestrianAlertActionCatalogItem> GetActionCatalog() const;

	UFUNCTION(BlueprintPure, Category = "ParkMassAI|Pedestrian Alert Actions")
	bool ValidateActions(TArray<FText>& OutErrors) const;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ParkMassPedestrianAlertSettings.generated.h"

class UParkMassPedestrianAlertActionSet;
class UMassEntityConfigAsset;

UCLASS(config = ParkMassAI, defaultconfig, meta = (DisplayName = "ParkMassAI Pedestrian Alert"))
class PARKMASSAI_API UParkMassPedestrianAlertSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;

	UPROPERTY(EditAnywhere, Config, Category = "Pedestrian Alert Actions", meta = (AllowedClasses = "/Script/ParkMassAI.ParkMassPedestrianAlertActionSet"))
	TSoftObjectPtr<UParkMassPedestrianAlertActionSet> DefaultAlertActionSet;

	UPROPERTY(EditAnywhere, Config, Category = "Pedestrian Mass", meta = (AllowedClasses = "/Script/MassSpawner.MassEntityConfigAsset"))
	TSoftObjectPtr<UMassEntityConfigAsset> DefaultPedestrianEntityConfig;
};

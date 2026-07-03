// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ParkMassPedestrianAlertTypes.generated.h"

UENUM(BlueprintType)
enum class EPedestrianAlertSourceMode : uint8
{
	DemoRandom UMETA(DisplayName = "Demo Random"),
	ExternalOnly UMETA(DisplayName = "External Only"),
	Hybrid UMETA(DisplayName = "Hybrid"),
	Disabled UMETA(DisplayName = "Disabled")
};


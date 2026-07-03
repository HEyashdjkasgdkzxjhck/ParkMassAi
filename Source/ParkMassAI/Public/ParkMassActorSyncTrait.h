// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "ParkMassActorSyncTrait.generated.h"

USTRUCT()
struct PARKMASSAI_API FParkMassActorSyncTag : public FMassTag
{
	GENERATED_BODY()
};

UCLASS(meta = (DisplayName = "Park Mass Actor Sync"))
class PARKMASSAI_API UParkMassActorSyncTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "ParkMassActorSyncProcessor.generated.h"

UCLASS()
class PARKMASSAI_API UParkMassActorSyncProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UParkMassActorSyncProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};

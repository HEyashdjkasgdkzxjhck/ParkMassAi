// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkMassActorSyncTrait.h"

#include "MassEntityTemplateRegistry.h"

void UParkMassActorSyncTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FParkMassActorSyncTag>();
}

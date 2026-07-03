// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkMassPedestrianAlertActionSet.h"

bool UParkMassPedestrianAlertActionSet::FindActionById(
	const FName ActionId,
	FParkMassPedestrianAlertActionDefinition& OutAction) const
{
	if (const FParkMassPedestrianAlertActionDefinition* Action = FindActionDefinitionById(ActionId))
	{
		OutAction = *Action;
		return true;
	}

	return false;
}

const FParkMassPedestrianAlertActionDefinition* UParkMassPedestrianAlertActionSet::FindActionDefinitionById(
	const FName ActionId) const
{
	for (const FParkMassPedestrianAlertActionDefinition& Action : Actions)
	{
		if (Action.ActionId == ActionId)
		{
			return &Action;
		}
	}

	return nullptr;
}

TArray<FName> UParkMassPedestrianAlertActionSet::GetAvailableActionIds() const
{
	TArray<FName> ActionIds;
	ActionIds.Reserve(Actions.Num());

	for (const FParkMassPedestrianAlertActionDefinition& Action : Actions)
	{
		ActionIds.Add(Action.ActionId);
	}

	return ActionIds;
}

TArray<FParkMassPedestrianAlertActionCatalogItem> UParkMassPedestrianAlertActionSet::GetActionCatalog() const
{
	TArray<FParkMassPedestrianAlertActionCatalogItem> Catalog;
	Catalog.Reserve(Actions.Num());

	for (const FParkMassPedestrianAlertActionDefinition& Action : Actions)
	{
		FParkMassPedestrianAlertActionCatalogItem Item;
		Item.ActionId = Action.ActionId;
		Item.DisplayName = Action.DisplayName;
		Item.Description = Action.Description;
		Item.DefaultDuration = Action.DefaultDuration;
		Item.bLoop = Action.bLoop;
		Item.bStopMovementDuringAction = Action.bStopMovementDuringAction;
		Catalog.Add(Item);
	}

	return Catalog;
}

bool UParkMassPedestrianAlertActionSet::ValidateActions(TArray<FText>& OutErrors) const
{
	OutErrors.Reset();

	TSet<FName> SeenIds;
	for (int32 Index = 0; Index < Actions.Num(); ++Index)
	{
		const FParkMassPedestrianAlertActionDefinition& Action = Actions[Index];
		if (Action.ActionId.IsNone() && Index != 0)
		{
			OutErrors.Add(FText::Format(
				NSLOCTEXT("ParkMassAI", "AlertActionNoneOnlyFirst", "Action at index {0} uses None. Keep None as the first no-op action only."),
				FText::AsNumber(Index)));
		}

		if (SeenIds.Contains(Action.ActionId))
		{
			OutErrors.Add(FText::Format(
				NSLOCTEXT("ParkMassAI", "AlertActionDuplicateId", "Duplicate ActionId: {0}"),
				FText::FromName(Action.ActionId)));
		}
		SeenIds.Add(Action.ActionId);
	}

	return OutErrors.IsEmpty();
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkMassPedestrianEventPoint.h"

#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityTemplate.h"
#include "MassSpawnLocationProcessor.h"
#include "MassSpawnerSubsystem.h"
#include "MassSpawnerTypes.h"
#include "ParkMassPedestrianAlertSettings.h"
#include "ParkMassPedestrianCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "StructUtils/StructView.h"
#include "TimerManager.h"
#include "UObject/ObjectKey.h"

DEFINE_LOG_CATEGORY_STATIC(LogParkMassPedestrianEventPoint, Log, All);

namespace
{
	TMap<FObjectKey, TWeakObjectPtr<AParkMassPedestrianEventPoint>> GPedestrianEventPointClaims;

	bool IsClaimedByOtherEventPoint(const AParkMassPedestrianCharacter* Pedestrian, const AParkMassPedestrianEventPoint* EventPoint)
	{
		if (!Pedestrian)
		{
			return false;
		}

		const TWeakObjectPtr<AParkMassPedestrianEventPoint>* Claim = GPedestrianEventPointClaims.Find(FObjectKey(Pedestrian));
		return Claim && Claim->IsValid() && Claim->Get() != EventPoint;
	}

	void ClaimPedestrianForEventPoint(AParkMassPedestrianCharacter* Pedestrian, AParkMassPedestrianEventPoint* EventPoint)
	{
		if (Pedestrian && EventPoint)
		{
			GPedestrianEventPointClaims.FindOrAdd(FObjectKey(Pedestrian)) = EventPoint;
		}
	}

	void ReleasePedestrianClaim(AParkMassPedestrianCharacter* Pedestrian, const AParkMassPedestrianEventPoint* EventPoint)
	{
		if (!Pedestrian)
		{
			return;
		}

		const FObjectKey PedestrianKey(Pedestrian);
		if (const TWeakObjectPtr<AParkMassPedestrianEventPoint>* Claim = GPedestrianEventPointClaims.Find(PedestrianKey))
		{
			if (!Claim->IsValid() || Claim->Get() == EventPoint)
			{
				GPedestrianEventPointClaims.Remove(PedestrianKey);
			}
		}
	}
}

AParkMassPedestrianEventPoint::AParkMassPedestrianEventPoint()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	SpawnRadiusComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SpawnRadius"));
	SpawnRadiusComponent->SetupAttachment(SceneRoot);
	SpawnRadiusComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnRadiusComponent->SetSphereRadius(SpawnRadius);
	SpawnRadiusComponent->SetHiddenInGame(true);

	PointNameComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PointName"));
	PointNameComponent->SetupAttachment(SceneRoot);
	PointNameComponent->SetRelativeLocation(FVector(0.0, 0.0, 120.0));
	PointNameComponent->SetHorizontalAlignment(EHTA_Center);
	PointNameComponent->SetVerticalAlignment(EVRTA_TextCenter);
	PointNameComponent->SetTextRenderColor(FColor(255, 220, 64));
	PointNameComponent->SetWorldSize(32.0f);
	PointNameComponent->SetHiddenInGame(true);
}

void AParkMassPedestrianEventPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (SpawnRadiusComponent)
	{
		SpawnRadiusComponent->SetSphereRadius(SpawnRadius);
	}

	if (PointNameComponent)
	{
		PointNameComponent->SetText(FText::FromName(PointName));
	}
}

void AParkMassPedestrianEventPoint::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoSpawnOnBeginPlay)
	{
		SpawnPedestrians();
	}

	if (bLoop)
	{
		GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			SpawnPedestrians();
			GetWorldTimerManager().SetTimer(
				LoopTimerHandle,
				this,
				&AParkMassPedestrianEventPoint::SpawnPedestrians,
				LoopInterval,
				true);
		}));
	}
}

void AParkMassPedestrianEventPoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (LoopTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(LoopTimerHandle);
	}

	if (ResolveTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(ResolveTimerHandle);
	}

	if (ClearAlertTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(ClearAlertTimerHandle);
	}

	if (DestroyAfterAlertTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(DestroyAfterAlertTimerHandle);
	}

	if (!OwnedEntities.IsEmpty())
	{
		if (UMassSpawnerSubsystem* SpawnerSubsystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld()))
		{
			SpawnerSubsystem->DestroyEntities(OwnedEntities);
		}
		OwnedEntities.Reset();
	}

	for (AParkMassPedestrianCharacter* Pedestrian : OwnedPedestrians)
	{
		ReleasePedestrianClaim(Pedestrian, this);
	}
	OwnedPedestrians.Reset();

	Super::EndPlay(EndPlayReason);
}

void AParkMassPedestrianEventPoint::SpawnPedestrians()
{
	SpawnPedestriansInternal(bSpawnAsAlert);
}

void AParkMassPedestrianEventPoint::SpawnAlertPedestrian()
{
	PendingActionId = NAME_None;
	SpawnPedestriansInternal(true);
}

void AParkMassPedestrianEventPoint::SpawnAlertPedestrianWithParams(
	const FText InAlertText,
	const float InDuration,
	const int32 InSpawnCount,
	const bool bInDestroyAfterAlert)
{
	SpawnAlertPedestrianWithActionParams(
		InAlertText,
		InDuration,
		InSpawnCount,
		bInDestroyAfterAlert,
		NAME_None);
}

void AParkMassPedestrianEventPoint::SpawnAlertPedestrianWithActionParams(
	const FText InAlertText,
	const float InDuration,
	const int32 InSpawnCount,
	const bool bInDestroyAfterAlert,
	const FName ActionId)
{
	const FText PreviousAlertText = AlertText;
	const float PreviousAlertDuration = AlertDuration;
	const int32 PreviousSpawnCount = SpawnCount;
	const bool bPreviousDestroyAfterAlert = bDestroyAfterAlert;

	AlertText = InAlertText;
	AlertDuration = FMath::Max(0.0f, InDuration);
	SpawnCount = FMath::Max(1, InSpawnCount);
	bDestroyAfterAlert = bInDestroyAfterAlert;
	PendingActionId = ActionId;

	SpawnPedestriansInternal(true);

	AlertText = PreviousAlertText;
	AlertDuration = PreviousAlertDuration;
	SpawnCount = PreviousSpawnCount;
	bDestroyAfterAlert = bPreviousDestroyAfterAlert;
}

void AParkMassPedestrianEventPoint::SetOwnedPedestriansAlert(const bool bAlert)
{
	for (int32 Index = OwnedPedestrians.Num() - 1; Index >= 0; --Index)
	{
		AParkMassPedestrianCharacter* Pedestrian = OwnedPedestrians[Index];
		if (!IsValid(Pedestrian))
		{
			OwnedPedestrians.RemoveAtSwap(Index);
			continue;
		}

		Pedestrian->SetAlertText(AlertText);
		Pedestrian->SetAlertState(bAlert);
	}
}

void AParkMassPedestrianEventPoint::ClearOwnedPedestrians()
{
	SetOwnedPedestriansAlert(false);

	if (UMassSpawnerSubsystem* SpawnerSubsystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld()))
	{
		SpawnerSubsystem->DestroyEntities(OwnedEntities);
	}

	for (AParkMassPedestrianCharacter* Pedestrian : OwnedPedestrians)
	{
		ReleasePedestrianClaim(Pedestrian, this);
	}

	OwnedEntities.Reset();
	OwnedPedestrians.Reset();
}

void AParkMassPedestrianEventPoint::SpawnPedestriansInternal(const bool bForceAlert)
{
	UWorld* World = GetWorld();
	UMassEntityConfigAsset* EffectiveConfig = GetEffectiveEntityConfig();
	UMassSpawnerSubsystem* SpawnerSubsystem = World ? UWorld::GetSubsystem<UMassSpawnerSubsystem>(World) : nullptr;
	if (!World || !EffectiveConfig || !SpawnerSubsystem || SpawnCount <= 0)
	{
		return;
	}

	FMassTransformsSpawnData SpawnData;
	SpawnData.bRandomize = false;
	SpawnData.Transforms.Reserve(SpawnCount);
	PendingSpawnLocations.Reset(SpawnCount);
	PedestriansBeforeSpawn.Reset();
	PendingAlertText = AlertText;
	PendingAlertDuration = AlertDuration;
	bPendingDestroyAfterAlert = bDestroyAfterAlert;
	PendingDestroyDelayAfterAlertEnd = DestroyDelayAfterAlertEnd;
	PendingExpectedSpawnCount = SpawnCount;
	PendingOwnedStartCount = OwnedPedestrians.Num();
	PendingResolveAttempts = 0;

	TArray<AActor*> ExistingPedestrians;
	UGameplayStatics::GetAllActorsOfClass(World, AParkMassPedestrianCharacter::StaticClass(), ExistingPedestrians);
	PedestriansBeforeSpawn.Reserve(ExistingPedestrians.Num());
	for (AActor* ExistingActor : ExistingPedestrians)
	{
		if (AParkMassPedestrianCharacter* ExistingPedestrian = Cast<AParkMassPedestrianCharacter>(ExistingActor))
		{
			PedestriansBeforeSpawn.Add(ExistingPedestrian);
		}
	}

	if (bForceAlert)
	{
		UE_LOG(
			LogParkMassPedestrianEventPoint,
			Log,
			TEXT("SpawnAlertPedestrian: Point=%s SpawnCount=%d ExistingActors=%d ClaimRadius=%.1f MaxClaimCount=%d"),
			*GetName(),
			SpawnCount,
			ExistingPedestrians.Num(),
			ClaimRadius,
			GetEffectiveMaxClaimCount());
	}

	for (int32 Index = 0; Index < SpawnCount; ++Index)
	{
		const FVector SpawnLocation = MakeRandomSpawnLocation();
		PendingSpawnLocations.Add(SpawnLocation);
		SpawnData.Transforms.Add(FTransform(GetActorRotation(), SpawnLocation));
	}

	const FMassEntityTemplate& EntityTemplate = EffectiveConfig->GetOrCreateEntityTemplate(*World);
	TArray<FMassEntityHandle> SpawnedEntities;
	SpawnerSubsystem->SpawnEntities(
		EntityTemplate.GetTemplateID(),
		static_cast<uint32>(SpawnCount),
		FConstStructView::Make(SpawnData),
		UMassSpawnLocationProcessor::StaticClass(),
		SpawnedEntities);

	OwnedEntities.Append(SpawnedEntities);
	ScheduleResolveRecentlySpawnedPedestrians(bForceAlert);
}

void AParkMassPedestrianEventPoint::ResolveRecentlySpawnedPedestrians(const bool bSetAlert)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AActor*> PedestrianActors;
	UGameplayStatics::GetAllActorsOfClass(World, AParkMassPedestrianCharacter::StaticClass(), PedestrianActors);

	const int32 EffectiveMaxClaimCount = GetEffectiveMaxClaimCount();
	const float ClaimRadiusSq = FMath::Square(ClaimRadius);
	int32 ResolvedThisAttempt = 0;
	int32 CurrentBatchClaimCount = FMath::Max(0, OwnedPedestrians.Num() - PendingOwnedStartCount);
	int32 NewActorCount = 0;
	for (AActor* Actor : PedestrianActors)
	{
		AParkMassPedestrianCharacter* Pedestrian = Cast<AParkMassPedestrianCharacter>(Actor);
		if (!IsValid(Pedestrian))
		{
			UE_LOG(LogParkMassPedestrianEventPoint, Verbose, TEXT("Reject claim: invalid actor"));
			continue;
		}

		const bool bWasAlreadyInWorld = PedestriansBeforeSpawn.Contains(Pedestrian);
		if (bWasAlreadyInWorld)
		{
			UE_LOG(LogParkMassPedestrianEventPoint, Verbose, TEXT("Reject claim: %s existed before spawn"), *GetNameSafe(Pedestrian));
			continue;
		}

		++NewActorCount;
		if (OwnedPedestrians.Contains(Pedestrian))
		{
			UE_LOG(LogParkMassPedestrianEventPoint, Verbose, TEXT("Reject claim: %s already owned by this EventPoint"), *GetNameSafe(Pedestrian));
			continue;
		}

		if (IsClaimedByOtherEventPoint(Pedestrian, this))
		{
			UE_LOG(LogParkMassPedestrianEventPoint, Log, TEXT("Reject claim: %s already claimed by another EventPoint"), *GetNameSafe(Pedestrian));
			continue;
		}

		if (Pedestrian->IsAlert())
		{
			UE_LOG(LogParkMassPedestrianEventPoint, Log, TEXT("Reject claim: %s is already alert"), *GetNameSafe(Pedestrian));
			continue;
		}

		const float DistanceToPoint = FVector::Dist2D(Pedestrian->GetActorLocation(), GetActorLocation());
		if (DistanceToPoint > ClaimRadius)
		{
			UE_LOG(
				LogParkMassPedestrianEventPoint,
				Log,
				TEXT("Reject claim: %s distance too far %.1f > %.1f"),
				*GetNameSafe(Pedestrian),
				DistanceToPoint,
				ClaimRadius);
			continue;
		}

		if (CurrentBatchClaimCount >= EffectiveMaxClaimCount || CurrentBatchClaimCount >= SpawnCount)
		{
			UE_LOG(LogParkMassPedestrianEventPoint, Log, TEXT("Reject claim: %s exceeds claim count limit"), *GetNameSafe(Pedestrian));
			continue;
		}

		OwnedPedestrians.Add(Pedestrian);
		ClaimPedestrianForEventPoint(Pedestrian, this);
		++CurrentBatchClaimCount;
		++ResolvedThisAttempt;
		Pedestrian->SetAlertText(PendingAlertText);
		if (bSetAlert)
		{
			Pedestrian->SetAlertState(true);
			if (!PendingActionId.IsNone())
			{
				Pedestrian->PlayAlertAction(PendingActionId);
			}
		}

		UE_LOG(
			LogParkMassPedestrianEventPoint,
			Log,
			TEXT("Claimed pedestrian: %s OwnedPedestrians=%d"),
			*GetNameSafe(Pedestrian),
			OwnedPedestrians.Num());
	}

	++PendingResolveAttempts;
	UE_LOG(
		LogParkMassPedestrianEventPoint,
		Log,
		TEXT("Resolve spawned pedestrians: Point=%s Attempt=%d NewActors=%d ClaimedThisAttempt=%d CurrentBatchClaims=%d OwnedPedestrians=%d"),
		*GetName(),
		PendingResolveAttempts,
		NewActorCount,
		ResolvedThisAttempt,
		CurrentBatchClaimCount,
		OwnedPedestrians.Num());

	if (CurrentBatchClaimCount < FMath::Min(PendingExpectedSpawnCount, EffectiveMaxClaimCount) && PendingResolveAttempts < 20)
	{
		ScheduleResolveRecentlySpawnedPedestrians(bSetAlert);
		return;
	}

	if (bSetAlert && PendingAlertDuration > 0.0f)
	{
		FTimerDelegate ClearDelegate = FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			EndAlertForOwnedPedestrians();
		});
		World->GetTimerManager().SetTimer(ClearAlertTimerHandle, ClearDelegate, PendingAlertDuration, false);
	}
}

void AParkMassPedestrianEventPoint::ScheduleResolveRecentlySpawnedPedestrians(const bool bSetAlert)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float ResolveDelay = 0.25f;
	FTimerDelegate ResolveDelegate = FTimerDelegate::CreateWeakLambda(this, [this, bSetAlert]()
	{
		ResolveRecentlySpawnedPedestrians(bSetAlert);
	});
	World->GetTimerManager().SetTimer(ResolveTimerHandle, ResolveDelegate, ResolveDelay, false);
}

void AParkMassPedestrianEventPoint::EndAlertForOwnedPedestrians()
{
	SetOwnedPedestriansAlert(false);

	if (!bPendingDestroyAfterAlert)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerDelegate DestroyDelegate = FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		DestroyOwnedPedestrians();
	});
	World->GetTimerManager().SetTimer(DestroyAfterAlertTimerHandle, DestroyDelegate, PendingDestroyDelayAfterAlertEnd, false);
}

void AParkMassPedestrianEventPoint::DestroyOwnedPedestrians()
{
	if (!OwnedEntities.IsEmpty())
	{
		if (UMassSpawnerSubsystem* SpawnerSubsystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld()))
		{
			UE_LOG(LogParkMassPedestrianEventPoint, Log, TEXT("DestroyOwnedPedestrians: destroying %d Mass entities"), OwnedEntities.Num());
			SpawnerSubsystem->DestroyEntities(OwnedEntities);
			OwnedEntities.Reset();
		}
	}

	if (!OwnedPedestrians.IsEmpty())
	{
		UE_LOG(
			LogParkMassPedestrianEventPoint,
			Log,
			TEXT("DestroyOwnedPedestrians: destroying %d owned actors as fallback/cleanup"),
			OwnedPedestrians.Num());
	}

	for (int32 Index = OwnedPedestrians.Num() - 1; Index >= 0; --Index)
	{
		AParkMassPedestrianCharacter* Pedestrian = OwnedPedestrians[Index];
		ReleasePedestrianClaim(Pedestrian, this);
		if (IsValid(Pedestrian))
		{
			Pedestrian->Destroy();
		}
	}
	OwnedPedestrians.Reset();
}

int32 AParkMassPedestrianEventPoint::GetEffectiveMaxClaimCount() const
{
	return MaxClaimCount > 0 ? FMath::Min(MaxClaimCount, SpawnCount) : SpawnCount;
}

FVector AParkMassPedestrianEventPoint::MakeRandomSpawnLocation() const
{
	const float Angle = FMath::FRandRange(0.0f, UE_TWO_PI);
	const float Radius = FMath::Sqrt(FMath::FRand()) * SpawnRadius;
	const FVector Offset(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
	return GetActorLocation() + Offset;
}

UMassEntityConfigAsset* AParkMassPedestrianEventPoint::GetEffectiveEntityConfig() const
{
	if (EntityConfig)
	{
		UE_LOG(
			LogParkMassPedestrianEventPoint,
			Log,
			TEXT("GetEffectiveEntityConfig: Using EventPoint override EntityConfig. Point=%s EntityConfig=%s"),
			*GetNameSafe(this),
			*GetNameSafe(EntityConfig));
		return EntityConfig;
	}

	const UParkMassPedestrianAlertSettings* Settings = GetDefault<UParkMassPedestrianAlertSettings>();
	UE_LOG(
		LogParkMassPedestrianEventPoint,
		Log,
		TEXT("GetEffectiveEntityConfig: EventPoint override empty. SettingsValid=%s Point=%s"),
		Settings ? TEXT("true") : TEXT("false"),
		*GetNameSafe(this));

	if (!Settings)
	{
		UE_LOG(LogParkMassPedestrianEventPoint, Warning, TEXT("GetEffectiveEntityConfig failed: settings object is invalid. Point=%s"), *GetNameSafe(this));
		return nullptr;
	}

	const TSoftObjectPtr<UMassEntityConfigAsset>& ConfiguredEntityConfig = Settings->DefaultPedestrianEntityConfig;
	UE_LOG(
		LogParkMassPedestrianEventPoint,
		Log,
		TEXT("GetEffectiveEntityConfig: Using settings DefaultPedestrianEntityConfig. Configured=%s Path=%s Point=%s"),
		ConfiguredEntityConfig.IsNull() ? TEXT("false") : TEXT("true"),
		*ConfiguredEntityConfig.ToSoftObjectPath().ToString(),
		*GetNameSafe(this));

	if (ConfiguredEntityConfig.IsNull())
	{
		UE_LOG(LogParkMassPedestrianEventPoint, Warning, TEXT("GetEffectiveEntityConfig failed: Settings.DefaultPedestrianEntityConfig is not configured. Point=%s"), *GetNameSafe(this));
		return nullptr;
	}

	UMassEntityConfigAsset* LoadedEntityConfig = ConfiguredEntityConfig.LoadSynchronous();
	UE_LOG(
		LogParkMassPedestrianEventPoint,
		Log,
		TEXT("GetEffectiveEntityConfig: Settings LoadSucceeded=%s EntityConfig=%s Point=%s"),
		LoadedEntityConfig ? TEXT("true") : TEXT("false"),
		*GetNameSafe(LoadedEntityConfig),
		*GetNameSafe(this));

	if (!LoadedEntityConfig)
	{
		UE_LOG(
			LogParkMassPedestrianEventPoint,
			Warning,
			TEXT("GetEffectiveEntityConfig failed: could not load Settings.DefaultPedestrianEntityConfig Path=%s Point=%s"),
			*ConfiguredEntityConfig.ToSoftObjectPath().ToString(),
			*GetNameSafe(this));
	}

	return LoadedEntityConfig;
}

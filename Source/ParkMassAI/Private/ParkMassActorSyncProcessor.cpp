// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkMassActorSyncProcessor.h"

#include "ParkMassActorSyncTrait.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"

namespace UE::ParkMassAI
{
	static bool bActorSyncLog = false;
	static FAutoConsoleVariableRef CVarActorSyncLog(
		TEXT("ParkMassAI.ActorSync.Log"),
		bActorSyncLog,
		TEXT("Logs ParkMassAI Mass actor fallback synchronization when true."),
		ECVF_Default);

	static float DefaultActorZOffset = 88.0f;
	static FAutoConsoleVariableRef CVarDefaultActorZOffset(
		TEXT("parkmass.ActorSync.DefaultZOffset"),
		DefaultActorZOffset,
		TEXT("Fallback Z offset used by ParkMassAI Mass actor synchronization when no Character capsule is available."),
		ECVF_Default);

	static float MinFacingSpeed = 5.0f;
	static FAutoConsoleVariableRef CVarMinFacingSpeed(
		TEXT("parkmass.ActorSync.MinFacingSpeed"),
		MinFacingSpeed,
		TEXT("Minimum 2D Mass velocity required before ParkMassAI actor synchronization updates actor facing."),
		ECVF_Default);

	static float RotationInterpSpeed = 8.0f;
	static FAutoConsoleVariableRef CVarRotationInterpSpeed(
		TEXT("parkmass.ActorSync.RotationInterpSpeed"),
		RotationInterpSpeed,
		TEXT("Interpolation speed used by ParkMassAI actor synchronization when rotating actors toward Mass velocity."),
		ECVF_Default);

	static bool bWriteCharacterMovementVelocity = true;
	static FAutoConsoleVariableRef CVarWriteCharacterMovementVelocity(
		TEXT("parkmass.ActorSync.WriteCharacterMovementVelocity"),
		bWriteCharacterMovementVelocity,
		TEXT("When true, ParkMassAI actor synchronization writes Mass velocity into CharacterMovement velocity for animation blueprints."),
		ECVF_Default);
}

UParkMassActorSyncProcessor::UParkMassActorSyncProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::UpdateWorldFromMass;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	bRequiresGameThreadExecution = true;
	bAutoRegisterWithProcessingPhases = true;
}

void UParkMassActorSyncProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FParkMassActorSyncTag>(EMassFragmentPresence::All);
	EntityQuery.RequireMutatingWorldAccess();
}

void UParkMassActorSyncProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassActorFragment> ActorList = Context.GetFragmentView<FMassActorFragment>();
		const TConstArrayView<FMassVelocityFragment> VelocityList = Context.GetFragmentView<FMassVelocityFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			AActor* Actor = const_cast<AActor*>(ActorList[EntityIt].Get(FMassActorFragment::EActorAccess::IncludePendingKill));
			if (!IsValid(Actor))
			{
				continue;
			}

			const FTransform& EntityTransform = TransformList[EntityIt].GetTransform();
			float ZOffset = UE::ParkMassAI::DefaultActorZOffset;
			if (const ACharacter* Character = Cast<ACharacter>(Actor))
			{
				if (const UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
				{
					ZOffset = CapsuleComponent->GetScaledCapsuleHalfHeight();
				}
			}

			const FVector ActorLocation = EntityTransform.GetLocation() + FVector(0.0, 0.0, ZOffset);
			const FVector Velocity2D(VelocityList[EntityIt].Value.X, VelocityList[EntityIt].Value.Y, 0.0);
			const FRotator CurrentRotation = Actor->GetActorRotation();
			FRotator NewRotation = CurrentRotation;
			if (Velocity2D.SizeSquared() > FMath::Square(UE::ParkMassAI::MinFacingSpeed))
			{
				const FRotator DesiredRotation = Velocity2D.ToOrientationRotator();
				NewRotation = FMath::RInterpTo(
					CurrentRotation,
					DesiredRotation,
					Context.GetDeltaTimeSeconds(),
					UE::ParkMassAI::RotationInterpSpeed);
			}

			Actor->SetActorLocationAndRotation(ActorLocation, NewRotation, false, nullptr, ETeleportType::None);

			if (UE::ParkMassAI::bWriteCharacterMovementVelocity)
			{
				if (ACharacter* Character = Cast<ACharacter>(Actor))
				{
					if (UCharacterMovementComponent* MoveComponent = Character->GetCharacterMovement())
					{
						MoveComponent->Velocity = Velocity2D;
						MoveComponent->UpdateComponentVelocity();
					}
				}
			}

			if (UE::ParkMassAI::bActorSyncLog)
			{
				UE_LOG(LogTemp, Log, TEXT("ParkMassActorSync Entity=%s Actor=%s Location=%s Rotation=%s Velocity=%s ZOffset=%.2f"),
					*Context.GetEntity(EntityIt).DebugGetDescription(),
					*Actor->GetName(),
					*ActorLocation.ToString(),
					*NewRotation.ToString(),
					*Velocity2D.ToString(),
					ZOffset);
			}
		}
	});
}

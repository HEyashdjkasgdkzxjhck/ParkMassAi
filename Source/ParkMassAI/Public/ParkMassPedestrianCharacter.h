// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ParkMassPedestrianCharacter.generated.h"

class UTextBlock;
class UUserWidget;
class UWidgetComponent;
class UAnimMontage;
class UParkMassPedestrianAlertActionSet;

UCLASS(Blueprintable)
class PARKMASSAI_API AParkMassPedestrianCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AParkMassPedestrianCharacter(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian")
	void SetAlertState(bool bNewAlert);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian")
	void SetAlertText(const FText& InAlertText);

	UFUNCTION(BlueprintPure, Category = "ParkMassAI|Pedestrian")
	bool IsAlert() const { return bIsAlert; }

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian")
	bool PlayAlertAction(FName ActionId);

	UFUNCTION(BlueprintCallable, Category = "ParkMassAI|Pedestrian")
	void StopAlertAction();

	UFUNCTION(BlueprintPure, Category = "ParkMassAI|Pedestrian")
	FName GetCurrentAlertActionId() const { return CurrentAlertActionId; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian")
	bool bIsAlert = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParkMassAI|Pedestrian")
	FText AlertText = NSLOCTEXT("ParkMassAI", "DefaultPedestrianAlertText", "异常行为");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian")
	TObjectPtr<UWidgetComponent> AlertTagComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian")
	TSubclassOf<UUserWidget> AlertTagWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Actions")
	TObjectPtr<UParkMassPedestrianAlertActionSet> AlertActionSet = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Actions")
	FName CurrentAlertActionId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ParkMassAI|Pedestrian Alert Actions")
	TObjectPtr<UAnimMontage> CurrentAlertMontage = nullptr;

	void ApplyAlertVisuals();
	void UpdateAlertWidgetText();
	UParkMassPedestrianAlertActionSet* GetEffectiveAlertActionSet();
};

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "PHSInputConfig.generated.h"

class UInputAction;

/**
 FPHSInputAction

 Associates a UInputAction with a gameplay tag so the
 AbilityInputBindingComponent can map raw Enhanced Input events to GAS
 ability activation via AbilityLocalInputPressed / Released.
 
 The InputTag must match exactly the InputTag on the corresponding
 FPHSAbilitySet_GameplayAbility entry — both sides derive InputID via
 GetTypeHash(InputTag).
 */
USTRUCT(BlueprintType)
struct FPHSInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<const UInputAction> InputAction = nullptr;

	/** Must match the InputTag used in the ability set for this action. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 UPHSInputConfig
 
 Data asset that pairs an Input Mapping Context with a list of
 InputAction-to-GameplayTag bindings.
 
 Referenced by UAbilityInputBindingComponent at runtime. Set this on the
 Blueprint subclass (BP_AbilityInputBindingComponent) Class Defaults before
 the GameFeature injects it.
 
 Marked Const to prevent instance-level edits — all configuration belongs
 in the asset Class Defaults, not on individual component instances.
 If Blueprint subclassing of this asset is needed in the future, remove Const
 from the UCLASS specifier.
 */
UCLASS(BlueprintType, Const)
class PHS_API UPHSInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPHSInputConfig(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 Returns the input action mapped to the given tag, or nullptr if not found.
	 @param bLogNotFound  If true, logs a warning when the tag has no matching action.
	 */
	UFUNCTION(BlueprintCallable, Category = "PHS|Input")
	const UInputAction* FindInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	/** Mapping context to add when this config is bound, and remove on cleanup. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<const UInputMappingContext> InputMappingContext = nullptr;

	/** All input action / gameplay tag pairs this config manages. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (TitleProperty = "InputAction"))
	TArray<FPHSInputAction> InputActions;
};
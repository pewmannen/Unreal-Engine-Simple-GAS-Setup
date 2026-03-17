#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_ActivationPolicy.generated.h"

/**
 EAbilityActivationPolicy
 
 Controls when an ability activates relative to its lifetime.
 */
UENUM(BlueprintType)
enum class EAbilityActivationPolicy : uint8
{
	/** Activated explicitly via player input or code (default). */
	OnInputTriggered UMETA(DisplayName = "On Input Triggered"),

	/**
	 Activates automatically when the ability is granted and re-activates
	 if it ends while still granted (passive ability behaviour).
	 
	 Note: UPHSExtensionComponent calls ShouldActivateOnGrant() after granting
	 and triggers TryActivateAbility if it returns true. Subclasses using this
	 policy should call EndAbility when their passive effect is no longer needed,
	 or the ASC will attempt to re-activate on the next opportunity.
	 */
	WhileActive UMETA(DisplayName = "While Active"),
};

/**
 UGameplayAbility_ActivationPolicy
 
 Base class for all PHS gameplay abilities.
 Sets sensible defaults (InstancedPerActor) and adds an ActivationPolicy
 property that the extension component and ability system query at grant time.
 
 All PHS abilities should inherit from this class or one of its subclasses
 rather than from UGameplayAbility directly.
 */
UCLASS(Abstract, Blueprintable)
class PHS_API UGameplayAbility_ActivationPolicy : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_ActivationPolicy();

	/** Controls when and how this ability activates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PHS|Activation")
	EAbilityActivationPolicy ActivationPolicy = EAbilityActivationPolicy::OnInputTriggered;

	/**
	 Returns true if this ability should auto-activate when granted.
	 Called by UPHSExtensionComponent after GiveToAbilitySystem.
	 */
	UFUNCTION(BlueprintCallable, Category = "PHS|Activation")
	bool ShouldActivateOnGrant() const;
};
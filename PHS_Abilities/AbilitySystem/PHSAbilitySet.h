#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "PHSAbilitySet.generated.h"

class UGameplayAbility;
class UAttributeSet;
class UGameplayEffect;
class UAbilitySystemComponent;

/**
 FPHSAbilitySet_GameplayAbility
 
 Describes a single ability to grant. The InputTag is used to:
 1. Store a queryable tag on the FGameplayAbilitySpec so other systems can
 	identify what input drives this ability.
 2. Derive a stable InputID via GetTypeHash(InputTag) that both this set
 	and UAbilityInputBindingComponent use independently — they must match.
 */
USTRUCT(BlueprintType)
struct FPHSAbilitySet_GameplayAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UGameplayAbility> Ability = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (ClampMin = "1"))
	int32 AbilityLevel = 1;

	/**
	 Input tag that maps this ability to a player input action.
	 Must match the InputTag on the corresponding FPHSInputAction entry
	 in the UPHSInputConfig used by UAbilityInputBindingComponent.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FPHSAbilitySet_GameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Effect", meta = (ClampMin = "1.0"))
	float EffectLevel = 1.0f;
};

USTRUCT(BlueprintType)
struct FPHSAbilitySet_AttributeSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	TSubclassOf<UAttributeSet> AttributeSet = nullptr;
};

/**
 FPHSAbilitySet_GrantedHandles
 
 Stores handles returned when granting an ability set. Pass this to
 UPHSAbilitySet::GiveToAbilitySystem and hold onto it. Call
 TakeFromAbilitySystem to cleanly revoke everything (e.g. in EndPlay).
 
 Never share one instance across multiple GiveToAbilitySystem calls —
 each call should have its own FPHSAbilitySet_GrantedHandles.
 */
USTRUCT(BlueprintType)
struct FPHSAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);

	/**
	 Removes all granted abilities, effects, and attribute sets from the ASC.
	 Safe to call on an empty handle set. Only runs on authority.
	 Resets all internal arrays after revoking.
	 */
	void TakeFromAbilitySystem(UAbilitySystemComponent* ASC);

	/** Returns true if this handle set has any entries (i.e. a grant succeeded). */
	bool IsValid() const;

private:
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

/**
 UPHSAbilitySet
 
 Data asset that defines a bundle of gameplay abilities, gameplay effects,
 and attribute sets to grant together to an AbilitySystemComponent.
 
 Usage:
 	FPHSAbilitySet_GrantedHandles Handles;
 	AbilitySet->GiveToAbilitySystem(ASC, &Handles, this);
 	// ... later, on cleanup:
 	Handles.TakeFromAbilitySystem(ASC);

To port to a new project: rename the PHS_API macro and
update any log categories. The grant/revoke logic is self-contained.
 */
UCLASS(BlueprintType)
class PHS_API UPHSAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPHSAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 Grants everything in this set to the given ASC. Server-only.
	 
	 @param ASC                -The ability system component to grant into. Must not be null.
	 @param OutGrantedHandles  -Optional. Populated with handles for later revocation via TakeFromAbilitySystem.
	 @param SourceObject       -Optional. Stored on each ability spec — useful for debugging who granted what.
	 */
	void GiveToAbilitySystem(UAbilitySystemComponent* ASC,
	                         FPHSAbilitySet_GrantedHandles* OutGrantedHandles = nullptr,
	                         UObject* SourceObject = nullptr) const;

	UPROPERTY(EditDefaultsOnly, Category = "PHS|Abilities")
	TArray<FPHSAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "PHS|Effects")
	TArray<FPHSAbilitySet_GameplayEffect> GrantedGameplayEffects;

	UPROPERTY(EditDefaultsOnly, Category = "PHS|Attributes")
	TArray<FPHSAbilitySet_AttributeSet> GrantedAttributes;
};
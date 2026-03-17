#include "PHSExtensionComponent.h"
#include "PHSLog.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayAbility_ActivationPolicy.h"

UPHSExtensionComponent::UPHSExtensionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPHSExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogPHSAbility, Warning, TEXT("UPHSExtensionComponent::BeginPlay — Owner is null."));
		return;
	}

	if (!Owner->HasAuthority())
	{
		// Ability granting is server-authoritative. Replication handles the client side.
		UE_LOG(LogPHSAbility, Verbose,
			TEXT("UPHSExtensionComponent on '%s': Skipping grant — not authoritative."),
			*Owner->GetName());
		return;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		UE_LOG(LogPHSAbility, Error,
			TEXT("UPHSExtensionComponent on '%s': No ASC found. Ability sets will NOT be granted. "
			     "Ensure the owner implements IAbilitySystemInterface or has an ASC registered with UAbilitySystemGlobals."),
			*Owner->GetName());
		return;
	}

	GrantAbilitySets(ASC);
}

void UPHSExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Revoke all granted abilities, effects, and attribute sets.
	// GetASC() is called once here and passed down — avoids repeated resolution.
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		for (FPHSAbilitySet_GrantedHandles& Handle : GrantedHandles)
		{
			Handle.TakeFromAbilitySystem(ASC);
		}
	}
	else if (GrantedHandles.Num() > 0)
	{
		UE_LOG(LogPHSAbility, Warning,
			TEXT("UPHSExtensionComponent on '%s': Could not resolve ASC during EndPlay — "
			     "%d granted handle set(s) may not have been revoked."),
			*GetNameSafe(GetOwner()),
			GrantedHandles.Num());
	}

	GrantedHandles.Reset();
	Super::EndPlay(EndPlayReason);
}

void UPHSExtensionComponent::GrantAbilitySets(UAbilitySystemComponent* ASC)
{
	const AActor* Owner = GetOwner();

	for (UPHSAbilitySet* AbilitySet : AbilitySetsToGrant)
	{
		if (!AbilitySet)
		{
			UE_LOG(LogPHSAbility, Warning,
				TEXT("UPHSExtensionComponent on '%s': Null entry in AbilitySetsToGrant — skipping."),
				*GetNameSafe(Owner));
			continue;
		}

		FPHSAbilitySet_GrantedHandles& Handles = GrantedHandles.AddDefaulted_GetRef();
		AbilitySet->GiveToAbilitySystem(ASC, &Handles, this);

		UE_LOG(LogPHSAbility, Log,
			TEXT("UPHSExtensionComponent: Granted AbilitySet '%s' to '%s'."),
			*AbilitySet->GetName(),
			*GetNameSafe(Owner));

		// Auto-activate any WhileActive abilities in this set.
		for (const FPHSAbilitySet_GameplayAbility& Entry : AbilitySet->GrantedGameplayAbilities)
		{
			if (!Entry.Ability)
			{
				continue;
			}

			const UGameplayAbility_ActivationPolicy* CDO =
				Entry.Ability->GetDefaultObject<UGameplayAbility_ActivationPolicy>();

			if (CDO && CDO->ShouldActivateOnGrant())
			{
				ASC->TryActivateAbilitiesByTag(
					FGameplayTagContainer(Entry.InputTag),
					/*bAllowRemoteActivation=*/ false);
			}
		}
	}
}

UAbilitySystemComponent* UPHSExtensionComponent::GetASC() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Preferred path: owner explicitly exposes its ASC via the interface.
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		return ASI->GetAbilitySystemComponent();
	}

	// Fallback: global scan. Handles third-party plugins (e.g. Motion marketplace)
	// that have an ASC on the actor but don't implement IAbilitySystemInterface
	// in a way that's visible from our module.
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
}
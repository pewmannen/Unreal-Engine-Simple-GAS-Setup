#include "PHSAbilitySet.h"
#include "PHSLog.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

void FPHSAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FPHSAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FPHSAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	if (Set)
	{
		GrantedAttributeSets.Add(Set);
	}
}

bool FPHSAbilitySet_GrantedHandles::IsValid() const
{
	return AbilitySpecHandles.Num() > 0
		|| GameplayEffectHandles.Num() > 0
		|| GrantedAttributeSets.Num() > 0;
}

void FPHSAbilitySet_GrantedHandles::TakeFromAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ensureMsgf(ASC, TEXT("FPHSAbilitySet_GrantedHandles::TakeFromAbilitySystem called with null ASC.")))
	{
		return;
	}

	// Revocation is server-authoritative. On clients the ASC replication handles
	// removing abilities from the predicted state.
	if (!ASC->IsOwnerActorAuthoritative())
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			ASC->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		ASC->RemoveSpawnedAttribute(Set);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

UPHSAbilitySet::UPHSAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPHSAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC,
                                          FPHSAbilitySet_GrantedHandles* OutHandles,
                                          UObject* SourceObject) const
{
	if (!ensureMsgf(ASC, TEXT("UPHSAbilitySet::GiveToAbilitySystem called with null ASC.")))
	{
		return;
	}

	if (!ASC->IsOwnerActorAuthoritative())
	{
		UE_LOG(LogPHSAbility, Warning,
			TEXT("UPHSAbilitySet '%s': GiveToAbilitySystem skipped — ASC owner is not authoritative."),
			*GetName());
		return;
	}

	// Attribute Sets
	for (const FPHSAbilitySet_AttributeSet& Entry : GrantedAttributes)
	{
		if (!IsValid(Entry.AttributeSet))
		{
			UE_LOG(LogPHSAbility, Warning,
				TEXT("UPHSAbilitySet '%s': Skipping null AttributeSet entry."), *GetName());
			continue;
		}
		
		UAttributeSet* NewSet = NewObject<UAttributeSet>(ASC->GetOwner(), Entry.AttributeSet);
		ASC->AddAttributeSetSubobject(NewSet);

		if (OutHandles)
		{
			OutHandles->AddAttributeSet(NewSet);
		}

		UE_LOG(LogPHSAbility, Log, TEXT("UPHSAbilitySet '%s': Granted AttributeSet '%s'."),
			*GetName(), *GetNameSafe(NewSet));
	}

	// Gameplay Abilities
	for (const FPHSAbilitySet_GameplayAbility& Entry : GrantedGameplayAbilities)
	{
		if (!IsValid(Entry.Ability))
		{
			UE_LOG(LogPHSAbility, Warning,
				TEXT("UPHSAbilitySet '%s': Skipping null Ability entry."), *GetName());
			continue;
		}

		FGameplayAbilitySpec Spec(Entry.Ability, Entry.AbilityLevel, INDEX_NONE, SourceObject);

		if (Entry.InputTag.IsValid())
		{
			// Store the input tag on the spec's dynamic source tags so other systems
			// (e.g. UI, debug tools) can query which input drives this ability by tag.
			// The InputID is derived from the same tag via GetTypeHash — UAbilityInputBindingComponent
			// uses the same hash on its side, so the two values are guaranteed to match.
			Spec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);
			Spec.InputID = static_cast<int32>(GetTypeHash(Entry.InputTag));
		}

		const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);

		if (OutHandles)
		{
			OutHandles->AddAbilitySpecHandle(Handle);
		}

		UE_LOG(LogPHSAbility, Log,
			TEXT("UPHSAbilitySet '%s': Granted '%s' | Level %d | InputTag '%s' | InputID %d."),
			*GetName(),
			*GetNameSafe(Entry.Ability),
			Entry.AbilityLevel,
			*Entry.InputTag.ToString(),
			Spec.InputID);
	}

	// Gameplay Effects
	for (const FPHSAbilitySet_GameplayEffect& Entry : GrantedGameplayEffects)
	{
		if (!IsValid(Entry.GameplayEffect))
		{
			UE_LOG(LogPHSAbility, Warning,
				TEXT("UPHSAbilitySet '%s': Skipping null GameplayEffect entry."), *GetName());
			continue;
		}

		const UGameplayEffect* EffectCDO = Entry.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		const FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectToSelf(EffectCDO, Entry.EffectLevel, Context);

		if (OutHandles)
		{
			OutHandles->AddGameplayEffectHandle(Handle);
		}

		UE_LOG(LogPHSAbility, Log,
			TEXT("UPHSAbilitySet '%s': Applied GameplayEffect '%s' | Level %.1f."),
			*GetName(),
			*GetNameSafe(Entry.GameplayEffect),
			Entry.EffectLevel);
	}
}

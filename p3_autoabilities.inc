SUB p3_autoabilities_init {
	/declare autoAbilitiesCheckTimer timer outer 0

	/if (${iniSectionExists[AutoAbilities]}==FALSE) /call autoabilitiesSetupINI

	/RETURN
}

SUB autoabilitiesSetupINI() {
	/invoke ${getBotIniOption[AutoAbilities.Sense Heading,CastableOnlyResting]}
	/RETURN
}

SUB autoabilitiesMaintenance() {
	/if (${autoAbilitiesCheckTimer}>0) /return

	/varset autoAbilitiesCheckTimer 2
	/call checkAutoAbilities

	/RETURN
}

SUB checkAutoAbilities() {
	/declare i int local
	/declare abilityName string local

	/for i 1 to ${iniAutoAbilities.Size}
		/varset abilityName ${iniAutoAbilities[${i},1]}

		/if (${abilityName.Equal[-]}) /break

		/if (${Me.AbilityReady[${abilityName}]}==FALSE) {
			/continue
		}

		/if (${iniAutoAbilities[${i},${bCastableOnlyResting}].NotEqual[-]}) {
			/if (${assistSpawnID}!=0) /continue
		}

		/if (${iniAutoAbilities[${i},${bSelfUnderHP}].NotEqual[-]}) {
			/if (${Me.PctHPs} >= ${iniAutoAbilities[${i},${bSelfUnderHP}]}) /continue
		}

		/if (${iniAutoAbilities[${i},${bRequireAssistTarget}].NotEqual[-]}) {
			/if (${assistTarget}==0) /continue
		}

		/if (${iniAutoAbilities[${i},${bFreeze}].NotEqual[-]}) {
			/delay ${iniAutoAbilities[${i},${bFreeze}].NotEqual[-]}
		}

		/if (${iniAutoAbilities[${i},${bDoCommand}].NotEqual[-]}) {
			/call p3command "${iniAutoAbilities[${i},${bDoCommand}]}"
		}

		/doability "${abilityName}"
	/next i
	

	/RETURN
}
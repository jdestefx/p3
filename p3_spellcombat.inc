SUB p3_spellcombat_init {
	/call p3RegisterCommand TAGSCHANGED spellcombatTagsChanged 10

	/call p3RegisterCommand KILL spellcombatKill 50
	/call p3RegisterCommand STOP spellcombatStop 50

	/declare combatSpellsCheckTimer timer outer 0
	/declare spellKillID int outer 0

	/call validateTagsFromArray iniCombatSpells

	/RETURN
}


SUB spellcombatTagsChanged() {
	/call validateTagsFromArray iniCombatSpells
	/RETURN
}

SUB spellcombatKill(string cmd, string opts) {
	/varset spellKillID ${getStackCommandOpt[${opts},SPAWNID]}

	/RETURN
}

SUB spellcombatStop(string cmd, string opts) {
	/varset spellKillID 0

	/RETURN
}

SUB spellcombatMaintenance() {
	/declare killSpawn spawn local

	/if (${spellKillID}==0) /RETURN

	/if (${Spawn[id ${spellKillID}].ID}==NULL || ${Spawn[id ${spellKillID}].Dead}==TRUE) {
		/call spellcombatStop
		/RETURN
	}

	/vardata killSpawn Spawn[id ${spellKillID}]

	/if (${killSpawn.ID}==NULL || ${killSpawn.Dead}==TRUE) {
		/call spellcombatStop
		/RETURN
	}

	/if (${assistMustHaveName.NotEqual[FALSE]}) {
		/if (${killSpawn.CleanName.Upper.Find[${assistMustHaveName.Upper}]}==NULL) {
			/call spellcombatStop
			/RETURN
		}
	}
	/if (${assistMustNotHaveName.NotEqual[FALSE]}) {
		/if (${killSpawn.CleanName.Upper.Find[${assistMustNotHaveName.Upper}]}!=NULL) {
			/call spellcombatStop
			/RETURN
		}
	}

	/if (${spellKillID}!=0) {
		/call checkCombatSpells
	}

	/RETURN
}

SUB checkCombatSpells {
	/if (${combatSpellsCheckTimer}>0) /return
	/if (${Me.Casting.ID}!=NULL) /return
	
	/varset combatSpellsCheckTimer 5

	/declare i int local
	/declare cSpell spell local
	/declare cSpellName string local
	/declare maxResists int local 5
	/declare currentResistCount int local 0
	/declare cancelConditions string local

	/declare lifeDeficit int local
	/declare lifeRatio int local

	/for i 1 to ${iniCombatSpells.Size}
		/varset cSpellName ${iniCombatSpells[${i},1]}
		/varset currentResistCount 0
		/varset maxResists 5
		/varset cancelConditions TARGETDEAD|

		/if (${cSpellName.Equal[-]}) /break

		/if (${iniCombatSpells[${i},${bEnabled}].Equal[NO]}) /continue
		/if (${iniCombatSpells[${i},${bTagsInvalidated}].Equal[YES]}) /continue

		/if (${iniCombatSpells[${i},${bNotIfMounted}].NotEqual[-]}) {
			/if (${Me.Mount.ID}!=NULL) /CONTINUE
		}
		
		/if (${iniCombatSpells[${i},${bAutoDebuffOnly}].NotEqual[-]}) {
			/continue
		}

		/if (${iniCombatSpells[${i},${bRequireCastIdle}].NotEqual[-]}) {
			/if (${idleCastingTimer}>0) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bGiftOfMana}].NotEqual[-]}) {
			/if (${Me.Song[Gift Of Mana].ID}==NULL) /continue
		}

		|/if (${iniCombatSpells[${i},${bNotIfAssistTag}].NotEqual[-]}) {
		|	/if (${assistTags.Count}>0 || ${assistTags.Count}>0) {
		|		/call validateAssistTags ${iniCombatSpells[${i},${bNotIfAssistTag}]} TRUE
		|		/if (${Macro.Return}==TRUE) /CONTINUE
		|	}
		|}

		|/if (${iniCombatSpells[${i},${bRequireAssistTag}].NotEqual[-]}) {
		|	/if (${assistTags.Count}==0 && ${assistTags.Count}==0) /CONTINUE
		|	/call validateAssistTags ${iniCombatSpells[${i},${bRequireAssistTag}]}
		|	/if (${Macro.Return}==FALSE) /CONTINUE
		|}

		/if (${iniCombatSpells[${i},${bMaxResists}].NotEqual[-]}) {
			/varset maxResists ${iniCombatSpells[${i},${bMaxResists}]}
		}

		/if (${iniCombatSpells[${i},${bSelfOverMana}].NotEqual[-]}) {
			/if (${Me.PctMana} <= ${iniCombatSpells[${i},${bSelfOverMana}]}) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bSelfUnderHP}].NotEqual[-]}) {
			/if (${Me.PctHPs} >= ${iniCombatSpells[${i},${bSelfUnderHP}]}) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bSelfOverHP}].NotEqual[-]}) {
			/if (${Me.PctHPs} < ${iniCombatSpells[${i},${bSelfOverHP}]}) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bSelfUnderMana}].NotEqual[-]}) {
			/if (${Me.PctMana} >= ${iniCombatSpells[${i},${bSelfUnderMana}]}) /CONTINUE
		}
		/if (${iniCombatSpells[${i},${bSelfOverMana}].NotEqual[-]}) {
			/if (${Me.PctMana} < ${iniCombatSpells[${i},${bSelfOverMana}]}) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bSelfUnderEnd}].NotEqual[-]}) {
			/if (${Me.PctEndurance} >= ${iniCombatSpells[${i},${bSelfUnderEnd}]} ) /CONTINUE
		}
		/if (${iniCombatSpells[${i},${bSelfOverEnd}].NotEqual[-]}) {
			/if (${Me.PctEndurance} < ${iniCombatSpells[${i},${bSelfOverEnd}]} ) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bTargetOverLevel}].NotEqual[-]}) {
			/if (${Spawn[id ${spellKillID}].Level} < ${iniCombatSpells[${i},${bTargetOverLevel}]}) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bTargetUnderLevel}].NotEqual[-]}) {
			/if (${Spawn[id ${spellKillID}].Level} >= ${iniCombatSpells[${i},${bTargetUnderLevel}]}) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bRequireBodyType}].NotEqual[-]}) {
			/if (${Select[${Spawn[id ${spellKillID}].Body.Name},${iniCombatSpells[${i},${bRequireBodyType}]}]}==0) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bTargetRace}].NotEqual[-]}) {
			/if (${Select[${Spawn[id ${spellKillID}].Race},${iniCombatSpells[${i},${bTargetRace}]}]}==0) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bRequireMobsInProximity}].NotEqual[-]}) {
			/if (${SpawnCount[npc loc ${Me.X} ${Me.Y} radius 25]} < ${iniCombatSpells[${i},${bRequireMobsInProximity}]}) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bRequireMaxMobsInProximity}].NotEqual[-]}) {
			/if (${SpawnCount[npc loc ${Me.X} ${Me.Y} radius 45]} > ${iniCombatSpells[${i},${bRequireMaxMobsInProximity}]}) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bRequireTargetClass}].NotEqual[-]}) {
			/if (${Select[${Spawn[id ${spellKillID}].Class.ShortName},${iniCombatSpells[${i},${bRequireTargetClass}]}]}==0) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bTargetOverHP}].NotEqual[-]}) {
			/call fullTarget ${spellKillID}
			/if (${Macro.Return}==FALSE) /CONTINUE
			/if (${Target.PctHPs} < ${iniCombatSpells[${i},${bTargetOverHP}]}) /CONTINUE
			/varset cancelConditions ${cancelConditions}TARGETUNDERHP:${iniCombatSpells[${i},${bTargetOverHP}]}|
		}

		/if (${iniCombatSpells[${i},${bTargetUnderHP}].NotEqual[-]}) {
			/call fullTarget ${spellKillID}
			/if (${Macro.Return}==FALSE) /CONTINUE
			/if (${Target.PctHPs} >= ${iniCombatSpells[${i},${bTargetUnderHP}]}) /CONTINUE
		}

		/if (${iniCombatSpells[${i},${bTargetLifeManaRatio}].NotEqual[-]}) {
			/call fullTarget ${spellKillID}
			/if (${Macro.Return}==FALSE) /CONTINUE

			/varset lifeDeficit ${Math.Calc[100-${Target.PctHPs}]}
			/varset lifeRatio ${iniCombatSpells[${i},${bTargetLifeManaRatio}]}
			/if (${Me.PctMana} < ${Math.Calc[100-(${lifeDeficit}/${lifeRatio})]}) {
				/CONTINUE
			}
		}

		/if (${iniCombatSpells[${i},${bMemInGem}].NotEqual[-]} && ${Me.Gem[${cSpellName}]}==NULL) {
			/call memorizeSpellInGem "${cSpellName}" ${iniCombatSpells[${i},${bMemInGem}]}
		}

		/if (${isCastReady[${cSpellName}]}==FALSE) {
			/CONTINUE
		}

		/if (${iniCombatSpells[${i},${bNoCancelChecks}].NotEqual[-]}) {
			/varset cancelConditions TARGETDEAD|
		}

		/call getSpellFromCastRequest "${cSpellName}"
		/if (${Macro.Return.Equal[UNKNOWN]}) {
			/bc Warning: checkCombatSpells() getSpellFromCastRequest() returned UNKNOWN. Not sure what to do.
		} else {
			/vardata cSpell ${Macro.Return}
			/if (${isLockoutExpired[detriTimer_${cSpell.ID}]}==FALSE) /CONTINUE
			/if (${isLockoutExpired[detriTimer_${spellKillID}_${cSpell.ID}]}==FALSE) /CONTINUE
		
			/if (${cSpell.MyCastTime}>0 && ${movingTimer}>0) /CONTINUE
		}

		/invoke (${cq.Append[CAST;SPELL=${cSpellName};TARGETID=${spellKillID};CANCELCONDITIONS=${cancelConditions};SRC=combatSpells;SRCIDX=${i};ONRESULT=onCombatCastResult]})
		/BREAK

	/next i

	/RETURN
}

SUB onCombatCastResult(string result, string opts) {
	/RETURN
}



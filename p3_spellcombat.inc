SUB p3_spellcombat_init {
	/call p3RegisterCommand KILL spellcombatKill 50
	/call p3RegisterCommand STOP spellcombatStop 50

	/declare combatSpellsCheckTimer timer outer 0
	/declare spellKillID int outer 0

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
		/return
	}

	/if (${spellKillID}!=0) {
		/call checkCombatSpells
	}

	/RETURN
}

SUB checkCombatSpells {
	/if (${movingTimer}>0) /return
	/if (${combatSpellsCheckTimer}>0) /return
	/if (${Me.Casting.ID}!=NULL) /return
	
	/varset combatSpellsCheckTimer 5

	/declare i int local
	/declare castResult string local
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

		/if (${iniCombatSpells[${i},${bNotIfMounted}].NotEqual[-]}) {
			/if (${Me.Mount.ID}!=NULL) /continue
		}

		/if (${iniCombatSpells[${i},${bRequireCastIdle}].NotEqual[-]}) {
			/if (${idleCastingTimer}>0) /continue
		}

		/if (${burnKey.NotEqual[-]}) {
			/if (${iniCombatSpells[${i},${bBurnKey}].NotEqual[${burnKey}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bNotIfAssistTag}].NotEqual[-]}) {
			/if (${assistTags.Count}>0 || ${assistTags.Count}>0) {
				/call validateAssistTags ${iniCombatSpells[${i},${bNotIfAssistTag}]} TRUE
				/if (${Macro.Return}==TRUE) /continue
			}
		}

		/if (${iniCombatSpells[${i},${bRequireAssistTag}].NotEqual[-]}) {
			/if (${assistTags.Count}==0 && ${assistTags.Count}==0) /continue
			/call validateAssistTags ${iniCombatSpells[${i},${bRequireAssistTag}]}
			/if (${Macro.Return}==FALSE) /continue
		}

		/if (${iniCombatSpells[${i},${bMaxResists}].NotEqual[-]}) {
			/varset maxResists ${iniCombatSpells[${i},${bMaxResists}]}
		}

		/if (${iniCombatSpells[${i},${bSelfOverMana}].NotEqual[-]}) {
			/if (${Me.PctMana} <= ${iniCombatSpells[${i},${bSelfOverMana}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bSelfUnderMana}].NotEqual[-]}) {
			/if (${Me.PctMana} >= ${iniCombatSpells[${i},${bSelfUnderMana}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bSelfUnderEnd}].NotEqual[-]}) {
			/if (${Me.PctEndurance} >= ${iniCombatSpells[${i},${bSelfUnderEnd}]} ) /continue
		}
		/if (${iniCombatSpells[${i},${bSelfOverEnd}].NotEqual[-]}) {
			/if (${Me.PctEndurance} < ${iniCombatSpells[${i},${bSelfOverEnd}]} ) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetOverLevel}].NotEqual[-]}) {
			/if (${Spawn[id ${spellKillID}].Level} < ${iniCombatSpells[${i},${bTargetOverLevel}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetUnderLevel}].NotEqual[-]}) {
			/if (${Spawn[id ${spellKillID}].Level} >= ${iniCombatSpells[${i},${bTargetUnderLevel}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bRequireBodyType}].NotEqual[-]}) {
			/if (${Select[${Spawn[id ${spellKillID}].Body.Name},${iniCombatSpells[${i},${bRequireBodyType}]}]}==0) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetRace}].NotEqual[-]}) {
			/if (${Select[${Spawn[id ${spellKillID}].Race},${iniCombatSpells[${i},${bTargetRace}]}]}==0) /continue
		}

		/if (${iniCombatSpells[${i},${bRequireMobsInProximity}].NotEqual[-]}) {
			/if (${SpawnCount[npc loc ${Me.X} ${Me.Y} radius 25]} < ${iniCombatSpells[${i},${bRequireMobsInProximity}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bRequireMaxMobsInProximity}].NotEqual[-]}) {
			/if (${SpawnCount[npc loc ${Me.X} ${Me.Y} radius 45]} > ${iniCombatSpells[${i},${bRequireMaxMobsInProximity}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bRequireTargetClass}].NotEqual[-]}) {
			/if (${Select[${Spawn[id ${spellKillID}].Class.ShortName},${iniCombatSpells[${i},${bRequireTargetClass}]}]}==0) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetOverHP}].NotEqual[-]}) {
			/echo check TargetOverHP... 
			/call fullTarget ${spellKillID}
			/if (${Macro.Return}==FALSE) /continue
			/if (${Target.PctHPs} < ${iniCombatSpells[${i},${bTargetOverHP}]}) /continue
			/varset cancelConditions ${cancelConditions}TARGETUNDERHP:${iniCombatSpells[${i},${bTargetOverHP}]}|
		}

		/if (${iniCombatSpells[${i},${bTargetUnderHP}].NotEqual[-]}) {
			/call fullTarget ${spellKillID}
			/if (${Macro.Return}==FALSE) /continue
			/if (${Target.PctHPs} >= ${iniCombatSpells[${i},${bTargetUnderHP}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetLifeManaRatio}].NotEqual[-]}) {
			/call fullTarget ${spellKillID}
			/if (${Macro.Return}==FALSE) /continue

			/varset lifeDeficit ${Math.Calc[100-${Target.PctHPs}]}
			/varset lifeRatio ${iniCombatSpells[${i},${bTargetLifeManaRatio}]}
			/if (${Me.PctMana} < ${Math.Calc[100-(${lifeDeficit}/${lifeRatio})]}) {
				/continue
			}
		}

		/if (${iniCombatSpells[${i},${bMemInGem}].NotEqual[-]} && ${Me.Gem[${cSpellName}]}==NULL) {
			/call memorizeSpellInGem "${cSpellName}" ${iniCombatSpells[${i},${bMemInGem}]}
		}

		/if (${isCastReady[${cSpellName}]}==FALSE) {
			/continue
		}

		/if (${iniCombatSpells[${i},${bNoCancelChecks}].NotEqual[-]}) {
			/varset cancelConditions TARGETDEAD|
		}

		/call getSpellFromCastRequest "${cSpellName}"
		/vardata cSpell ${Macro.Return}

		/if (${isLockoutExpired[detriTimer_${cSpell.ID}]}==FALSE) /continue
		/if (${isLockoutExpired[detriTimer_${spellKillID}_${cSpell.ID}]}==FALSE) /continue


		/invoke (${cq.Append[CAST;SPELL=${cSpellName};TARGETID=${spellKillID};CANCELCONDITIONS=${cancelConditions};SRC=combatSpells;SRCIDX=${i};ONRESULT=onCombatCastResult]})
		/break

	/next i

	/RETURN
}

SUB onCombatCastResult(string result, string opts) {
	/declare i int local ${getStackCommandOpt[${opts},SRCIDX]}
	/declare cSpellName string local ${getStackCommandOpt[${opts},SPELL]}
	/declare recastDelayTimerName string local
	/declare targetDurationTimerName string local
	/declare cSpell spell local
	/declare targetID string local ${getStackCommandOpt[${opts},TARGETID]}

	/call getSpellFromCastRequest "${cSpellName}"
	/vardata cSpell ${Macro.Return}

	/varset recastDelayTimerName detriTimer_${cSpell.ID}
	/varset targetDurationTimerName detriTimer_${targetID}_${cSpell.ID}

	/if (${result.Find[FAIL_CAST_TAKEHOLD]}!=NULL) {
		/call lockout ${timerName} 120

	} else /if (${result.Find[PASS_]}!=NULL) {

		/if (${iniCombatSpells[${i},${bNoSitTimer}].NotEqual[-]}) {
			/call pauseAutoMed ${iniCombatSpells[${i},${bNoSitTimer}]}
		}

		/if (${iniCombatSpells[${i},${bRecastDelay}].NotEqual[-]}) {
			/call lockout ${recastDelayTimerName} ${iniCombatSpells[${i},${bRecastDelay}]}
		}

		/if (${cSpell.Duration.TotalSeconds}>0) {
			/call lockout ${targetDurationTimerName} ${Math.Calc[${cSpell.Duration.TotalSeconds}*10]}
		}

		|/if (${iniCombatSpells[${i},${bRecastDelay}].NotEqual[-]} || ${cSpell.Duration.TotalSeconds} > 0) {
		|}

	} else /if (${result.Equal[FAIL_CAST_RESIST]}) {
		|/varset currentResistCount ${Math.Calc[${currentResistCount}+1]}
		|/bc Cast Attempts: ${currentResistCount} of ${maxResists}
		|/if (${currentResistCount}<${maxResists}) {
		|	/while (${isCastReady[${cSpellName}]}==FALSE) {
		|		/delay 2
		|	}
		|	/goto :resistRetry
		|}
		|
		|/if (${currentResistCount}==${maxResists}) {
		|	/bc My spell was resisted too much.  Giving up...
		|	/g Giving up casting ${cSpellName} on ${spellKillID}, too many resists.
		|	/declare ${timerName} timer outer 5m
		|}
	} else /if (${result.Equal[FAIL_CAST_IMMUNE]}) {
		/bc Immune.  Giving up...
		/g Giving up casting ${cSpellName} on ${spellKillID}, target is immune.
		/call lockout ${timerName} 5m
	}

	/RETURN
}
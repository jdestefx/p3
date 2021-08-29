SUB p3_spellcombat_init {
	|/call p3RegisterCommand KILL combatKill 50
	|/call p3RegisterCommand STOP combatStop 50

	/declare spellTargetID int outer 0

	/RETURN
}

SUB spellcombatKill(string cmd, string opts) {
	/varset spellTargetID ${getStackCommandOpt[${opts},SPAWNID]}

	/RETURN
}

SUB spellcombatStop(string cmd, string opts) {
	/varset spellTargetID 0
	/attack off

	/RETURN
}

SUB spellcombatMaintenance() {
	/return

	/if (${spellTargetID}!=0) {
		/call checkCombatSpells
	}

	/RETURN
}

SUB checkCombatSpells {
	/if (${spellTargetID}==0) /return
	/if (${Me.Casting.ID}!=NULL) /return

	/declare i int local
	/declare castResult string local
	/declare cSpell spell local
	/declare cSpellName string local
	/declare cTimerName string local
	/declare cTimerValue int local 0
	/declare maxResists int local 5
	/declare currentResistCount int local 0
	/declare cancelConditions string local

	/declare lifeDeficit int local
	/declare lifeRatio int local

	/for i 1 to ${iniCombatSpells.Size}
		/varset cSpellName ${iniCombatSpells[${i},1]}
		/varset cTimerValue 0
		/varset currentResistCount 0
		/varset maxResists 5
		/varset cancelConditions TARGETDEAD

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
			/if (${Spawn[id ${spellTargetID}].Level} < ${iniCombatSpells[${i},${bTargetOverLevel}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetUnderLevel}].NotEqual[-]}) {
			/if (${Spawn[id ${spellTargetID}].Level} >= ${iniCombatSpells[${i},${bTargetUnderLevel}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bRequireBodyType}].NotEqual[-]}) {
			/if (${Select[${Spawn[id ${spellTargetID}].Body.Name},${iniCombatSpells[${i},${bRequireBodyType}]}]}==0) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetRace}].NotEqual[-]}) {
			/if (${Select[${Spawn[id ${spellTargetID}].Race},${iniCombatSpells[${i},${bTargetRace}]}]}==0) /continue
		}

		/if (${iniCombatSpells[${i},${bRequireMobsInProximity}].NotEqual[-]}) {
			/if (${SpawnCount[npc loc ${Me.X} ${Me.Y} radius 25]} < ${iniCombatSpells[${i},${bRequireMobsInProximity}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bRequireMaxMobsInProximity}].NotEqual[-]}) {
			/if (${SpawnCount[npc loc ${Me.X} ${Me.Y} radius 45]} > ${iniCombatSpells[${i},${bRequireMaxMobsInProximity}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bRequireTargetClass}].NotEqual[-]}) {
			/if (${Select[${Spawn[id ${spellTargetID}].Class.ShortName},${iniCombatSpells[${i},${bRequireTargetClass}]}]}==0) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetOverHP}].NotEqual[-]}) {
			/call fullTarget ${spellTargetID}
			/if (${Target.PctHPs} < ${iniCombatSpells[${i},${bTargetOverHP}]}) /continue
			/varset cancelConditions ${cancelConditions},TARGETUNDERHP:${iniCombatSpells[${i},${bTargetOverHP}]}
		}

		/if (${iniCombatSpells[${i},${bTargetUnderHP}].NotEqual[-]}) {
			/call fullTarget ${spellTargetID}
			/if (${Target.PctHPs} >= ${iniCombatSpells[${i},${bTargetUnderHP}]}) /continue
		}

		/if (${iniCombatSpells[${i},${bTargetLifeManaRatio}].NotEqual[-]}) {
			/call fullTarget ${spellTargetID}

			/varset lifeDeficit ${Math.Calc[100-${Target.PctHPs}]}
			/varset lifeRatio ${iniCombatSpells[${i},${bTargetLifeManaRatio}]}
			/if (${Me.PctMana} < ${Math.Calc[100-(${lifeDeficit}/${lifeRatio})]}) {
				/continue
			}
		}

		/if (${isCastReady[${cSpellName}]}==FALSE) /continue

		/call getSpellFromCastRequest "${cSpellName}"
		/vardata cSpell ${Macro.Return}

		/varset cTimerName detriTimer_${spellTargetID}_${cSpell.ID}
		/if (${isLockoutExpired[${cTimerName}]}==FALSE) /continue

		| TODO: MOVE TO CAST ROUTINE
		|/if (${iniCombatSpells[${i},${bMemInGem}].NotEqual[-]}) {
		|	/if (${Me.Gem[${cSpellName}]}==NULL) {
		|		/call memorizeSpellInGem "${cSpellName}" ${iniCombatSpells[${i},${bMemInGem}]}
		|	}
		|}

		/invoke (${cq.Append[CAST:SPELL=${cSpellName};TARGETID=${spellTargetID};CANCELCONDITIONS=${cancelConditions};SRC=combatSpells:SRCIDX=${i}]})

		/RETURN

		/call e3_cast ${spellTargetID} "${cSpellName}" "${cancelConditions}"

		/varset castResult ${Macro.Return}

		/if (${castResult.Equal[CAST_NOTAKEHOLD]}) {
			/declare ${cTimerName} timer outer 120
		} else /if (${castResult.Equal[CAST_SUCCESS]}) {
			| handle medbreak pause
			/if (${iniCombatSpells[${i},${bNoSitTimer}].NotEqual[-]}) {
				/call pauseMedBreak ${iniCombatSpells[${i},${bNoSitTimer}]}
			}

			/if (${iniCombatSpells[${i},${bRecastDelay}].NotEqual[-]} || ${cSpell.Duration.TotalSeconds} > 0) {
				/declare ${cTimerName} timer outer 0

				/if (${iniCombatSpells[${i},${bRecastDelay}].NotEqual[-]}) {
					/varset ${cTimerName} ${iniCombatSpells[${i},${bRecastDelay}]}
				}

				/if (${cSpell.Duration.TotalSeconds}>0) {
					/varset ${cTimerName} ${Math.Calc[${${cTimerName}} + ${cSpell.Duration.TotalSeconds}]}s
				}
			}

			/varset actionTaken TRUE
			/if (${stopAfterActionTaken}==TRUE) /break
		} else /if (${castResult.Equal[CAST_RESIST]}) {
			/varset currentResistCount ${Math.Calc[${currentResistCount}+1]}
			/bc Cast Attempts: ${currentResistCount} of ${maxResists}
			/if (${currentResistCount}<${maxResists}) {
				/while (${isCastReady[${cSpellName}]}==FALSE) {
					/delay 2
				}
				/goto :resistRetry
			}

			/if (${currentResistCount}==${maxResists}) {
				/bc My spell was resisted too much.  Giving up...
				/g Giving up casting ${cSpellName} on ${spellTargetID}, too many resists.
				/declare ${cTimerName} timer outer 5m
			}
		} else /if (${castResult.Equal[CAST_IMMUNE]}) {
			/bc Immune.  Giving up...
			/g Giving up casting ${cSpellName} on ${spellTargetID}, target is immune.
			/call lockout ${cTimerName} 5m
		}
	
	/next i	

	/RETURN
}
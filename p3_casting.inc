SUB p3_casting_init {
	/call p3RegisterCommand FOLLOW castingFollow 52
	/call p3RegisterCommand STOP castingStop 52
	/call p3RegisterCommand CAST castingCast 50

	/declare castingSpell spell outer
	/declare castingOpts string outer -
	/declare castingTargetID int outer 0
	/declare castingCancelConditions list outer
	/invoke ${castingCancelConditions.Delimiter[|]}

	/declare castingSpellEmpty spell outer
	/declare untrackedCastSpell spell outer

	/declare castResult string outer -

	/declare idleCastingTimerMax int outer 50
	/declare idleCastingTimer timer outer 0

	/declare cancelConditionDelay timer outer 0

	/RETURN
}

SUB castingSetIdle() {
	/if (${castingSpell.ID}!=0) {
		/vardata castingSpell castingSpellEmpty
		/varset castingOpts -
		/varset castResult -
		/varset castingTargetID 0
		/invoke ${castingCancelConditions.Clear}
	}

	/RETURN
}

SUB castingFollow(string cmd, string opts) {
	/if (${Me.Casting.ID} != NULL && ${Me.Class.ShortName.Equal[BRD]}==FALSE) {
		/echo stop for casting follow
		/varset movingTimer 20
		/interrupt
		/delay 5
	}
	/RETURN
}

SUB castingStop(string cmd, string opts) {
	/if (${Me.Casting.ID}!=NULL && ${amBard}==FALSE) {
		/echo stop cast seen.
		/interrupt
	}
	/RETURN
}

SUB castingCast(string cmd, string opts) {
	/call p3cast ${getStackCommandOpt[${opts},TARGETID]} "${getStackCommandOpt[${opts},SPELL]}" "${getStackCommandOpt[${opts},SRC]}" ${getStackCommandOpt[${opts},SRCIDX]}

	/if (${Macro.Return.Equal[PASS_STARTED]}) {
		/varset castingOpts ${opts}
		/vardata castingSpell ${getSpellFromCastRequest[${getStackCommandOpt[${castingOpts},SPELL]}]}
		/vardata untrackedCastSpell castingSpellEmpty
		/varset castingTargetID ${getStackCommandOpt[${castingOpts},TARGETID]}
		/invoke ${castingCancelConditions.Append[${getStackCommandOpt[${castingOpts},CANCELCONDITIONS]}]}
		/echo Cast Started [${castingSpell}].
	} else /if (${Macro.Return.Equal[PASS_INSTANT_CAST]}) {
		/echo Instant cast [${getStackCommandOpt[${opts},SPELL]}] done.
		/varset castingOpts ${opts}
		/vardata castingSpell ${getSpellFromCastRequest[${getStackCommandOpt[${castingOpts},SPELL]}]}
		/call castComplete ${castResult}
	} else /if (${Macro.Return.Find[FAIL_LOS]}!=NULL) {
		/echo Couldn't cast. No line of sight.
		/RETURN
	} else /if (${Macro.Return.Find[FAIL_]}!=NULL) {
		/echo Cast didn't start [${castingOpts}] => ${Macro.Return}
		/call castComplete ${Macro.Return}
	}

	/RETURN
}

SUB castingMaintenance() {
	/declare sourceArr string local
	/declare i int local

	/if (${Twist.Twisting}==TRUE && ${Me.Casting.ID}==${castingSpell.ID}) {
		/if (${Cast.Timing}==0) /twist off
		/RETURN RESET
	}

	/if (${Me.Casting.ID}!=NULL) {
		/if (${idleCastingTimer}<40) /varset idleCastingTimer ${idleCastingTimerMax}

		|/if (${movingTimer}>0 && ${canCastWhileMoving}==FALSE) {
		|	/echo Movement detected. Stopping cast.
		|	/interrupt
		|	/delay 3
		|	/return
		|}

		/if (${Me.Casting.ID} != ${castingSpell.ID}) {
			/vardata untrackedCastSpell Me.Casting
			|/echo Unknown Cast In Progress [${untrackedCastSpell}].
			/if (${castResult.Equal[IDLE]}) /varset castResult STARTED
			/call castingSetIdle
		} else {
			|/echo Casting ${castingSpell} as expected...
			/varset buffCheckTimer 10
			/if (${castingTargetID}==${Target.ID}) /call checkCancelConditions

			/if (${Macro.Return}==TRUE) {
				/BC Cancel Conditions met: [${getStackCommandOpt[${castingOpts},CANCELCONDITIONS]}]
				/interrupt
				/delay 3
				/RETURN RESET
			}
		}

		/if (${amBard}==FALSE) /RETURN RESET
	}

	/if (${Me.Casting.ID}==NULL) {
		/if (${castingSpell.ID} != NULL) {
			/echo Detected end of cast for [${castingSpell}]
			
			/varset sourceArr ${getStackCommandOpt[${castingOpts},SRC]}
			/varset i ${getStackCommandOpt[${castingOpts},SRCIDX]}

			/if (${castResult.Equal[STARTED]}) /varset castResult SUCCESS

			/if (${${sourceArr}[${i},${bAssumeSuccess}].NotEqual[-]}) {
				/echo Assuming Success: on ${${sourceArr}[${i},${bName}].Equal[-]}
				/varset castResult SUCCESS
			} else {
				| PROCESS CHAT EVENTS TO MAKE SURE RESISTS AND SUCH GET REGISTERED
				/doevents
				/delay 5
				/echo Result => ${castResult}
			}
			
			/if (${${sourceArr}[${i},${bNoSitTimer}].NotEqual[-]}) {
				/echo Pausing medbreak: ${${sourceArr}[${i},${bNoSitTimer}]}
				/call pauseAutoMed ${${sourceArr}[${i},${bNoSitTimer}]}
			}
		
			/if (${castResult.Equal[SUCCESS]} || ${${sourceArr}[${i},${bAssumeSuccess}].NotEqual[-]}) {
				/call castComplete PASS_${castResult}
			} else {
				/call castComplete FAIL_${castResult}
			}

			/call castingSetIdle
			/RETURN
		}

		/if (${untrackedCastSpell.ID}!=NULL) {
			/echo Detected end of unknown cast.
			/vardata untrackedCastSpell castingSpellEmpty
		}
	}

	
	/RETURN
}

SUB checkCancelConditions() {
	/declare i int local
	/declare e string local

	/for i 0 to ${Math.Calc[${castingCancelConditions.Count}-1]}
		/varset e ${castingCancelConditions.Item[${i}]}

		/if (${e.Find[TARGETDEAD]}!=NULL) {
			/if (${Target.Dead}==TRUE) /return TRUE
		}


		/if (${e.Find[TARGETISASSITTARGET]}!=NULL) {
			/if (${assistSpawnID}==${Target.ID}) /return TRUE
		}

		/if (${e.Find[TARGETUNDERHP]}!=NULL) {
			/if (${Target.PctHPs} < ${Math.Calc[${e.Token[2,:]}]}) {
				/return TRUE
			}
		}

		/if (${e.Find[TARGETOVERHP]}!=NULL) {
			/if (${NetBots[${Target.Name}].ID}!=NULL) {
				/if (${NetBots[${Target.Name}].PctHPs} > ${Math.Calc[${e.Token[2,:]}]}) /return TRUE
			} else {
				/if (${Target.PctHPs} > ${Math.Calc[${e.Token[2,:]}]}) /return TRUE
			}
		}

		/if (${e.Find[TARGETHASBUFF]}!=NULL) {
			/if (${Target.Buff[${e.Token[2,:]}].ID}!=NULL) /return TRUE
		}

		/if (${e.Find[NETBOTATTACKING]}!=NULL) {
			/if (${isNetBotAttackingID[${e.Token[2,:]}]}==TRUE) /return TRUE
		}
	/next i

	/RETURN FALSE
}

SUB p3Cast(int targetID, string castName, string arrName, int arrIdx) {
	/declare castType string local
	/declare castSpell spell local
	/declare bardWasTwisting bool local FALSE
	/declare ret string local
	/declare hasCastTime bool local true

	/call getCastTypeByCastRequest "${castName}"
	/if (${Macro.Return.Equal[UNKNOWN]}) /RETURN FAIL_UNKNOWN_SPELL
	/varset castType ${Macro.Return}

	| CHECK ABILITIES AND HANDLE RIGHT AWAY SINCE THEY DONT HAVE "SPELLS" 
	/if (${castType.Equal[ABILITY]}) /varset hasCastTime FALSE

	| ALL OTHER CAST TYPES INVOLVE SPELLS
	/call getSpellFromCastRequest "${castName}"

	/if (${Macro.Return.Equal[UNKNOWN]}) {
		| UNKNOWN is probably a combat ability like taunt, can allow.
	} else /if (${Macro.Return.NotEqual[ABILITY]}) {
		/vardata castSpell ${Macro.Return}
		/if (${Spawn[id ${targetID}].LineOfSight}==FALSE && ${castSpell.SpellType.Find[Detri]}) /RETURN FAIL_LOS
	}

	/if (${isCastReady[${castName}]}==FALSE) /RETURN FAIL_CAST_NOT_READY

	/if (${castType.Equal[ALTABILITY]} && ${AltAbility[${castName}].Spell.CastTime}==0) /varset hasCastTime FALSE
	/if (${castType.Equal[ITEM]} && ${FindItem[=${castName}].CastTime}==0) /varset hasCastTime FALSE
	/if (${castType.Equal[ABILITY]}) /varset hasCastTime FALSE
	/if (${castType.Equal[DISCIPLINE]}) /varset hasCastTime FALSE

	/echo Casting/Doing [${castName}] => ${Spawn[id ${targetID}].CleanName}

	/if (${hasCastTime}==TRUE) {
		/if (${canCastWhileMoving}==FALSE) {
			/call movementLockoutStick 15
		}

		/if (${Twist.Twisting}==TRUE) {
			/twist stop
			/delay 3
		}
	}

	/varset castResult STARTED

	/if (${castType.Equal[ABILITY]}) {
		/doability ${Me.Ability[${castName}]}
		/RETURN PASS_INSTANT_CAST
	
	} else /if (${castType.Equal[DISCIPLINE]}) {
		/disc ${castName}
		/RETURN PASS_INSTANT_CAST

	} else /if (${castType.Equal[ALTABILITY]}) {
		/alt act ${AltAbility[${castName}].ID}
		/if (${hasCastTime}==FALSE) /RETURN PASS_INSTANT_CAST

	} else /if (${castType.Equal[ITEM]}) {
		/useitem "${castName}"
		|/itemnotify "${castName}" rightmouseup
		/if (${hasCastTime}==FALSE) /RETURN PASS_INSTANT_CAST

	} else /if (${castType.Equal[SPELL]}) {
		/if (${Me.Class.ShortName.Equal[BRD]} && ${castType.Equal[SPELL]}) {
			|/CAST ${Me.Gem[${castName}]}
			/twist once ${Me.Gem[${castName}]}
		} else {
			/cast "${castName}"
		}
	}

	/delay 5

	/varset ret PASS_STARTED

	/RETURN ${ret}
}

SUB castComplete(string result) {

	/varset buffCheckTimer 10
	/declare funcName string local
	/declare i int local ${getStackCommandOpt[${castingOpts},SRCIDX]}
	/declare cSpellName string local ${getStackCommandOpt[${castingOpts},SPELL]}
	/declare recastDelayTimerName string local
	/declare targetDurationTimerName string local
	/declare cSpell spell local
	/declare targetID string local ${getStackCommandOpt[${castingOpts},TARGETID]}
	/declare trackRecast string local ${getStackCommandOpt[${castingOpts},TRACKRECAST]}

	/BC ${result} -> ${castingSpell} ${Spawn[id ${targetID}]}

	/call getSpellFromCastRequest "${cSpellName}"
	/vardata cSpell ${Macro.Return}

	/varset recastDelayTimerName detriTimer_${cSpell.ID}
	/varset targetDurationTimerName detriTimer_${targetID}_${cSpell.ID}

	|/if (${trackRecast.Equal[1]}) {
		/if (${result.Find[FAIL_NOHOLD]}!=NULL) {
			/call lockout ${recastDelayTimerName} 120

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

		} else /if (${result.Equal[FAIL_RESIST]}) {
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
			|	/declare ${recastDelayTimerName} timer outer 5m
			|}
		} else /if (${result.Equal[FAIL_IMMUNE]}) {
			/bc Immune.  Giving up...
			/g Giving up casting ${cSpellName} on ${spellKillID}, target is immune.
			/call lockout ${recastDelayTimerName} 5m
		}
	|}

	/varset funcName ${getStackCommandOpt[${castingOpts},ONRESULT]}

	/if (${funcName.Equal[-]}) /RETURN

	/call ${funcName} ${result} "${castingOpts}"

	/RETURN
}

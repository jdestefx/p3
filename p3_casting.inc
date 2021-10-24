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

	/declare castingResult string outer -

	/declare idleCastingTimerMax int outer 50
	/declare idleCastingTimer timer outer 0

	/declare cancelConditionDelay timer outer 0

	/RETURN
}

SUB castingSetIdle() {
	/if (${castingSpell.ID}!=0) {
		/vardata castingSpell castingSpellEmpty
		/varset castingOpts -
		/varset castingTargetID 0
		/invoke ${castingCancelConditions.Clear}
	}

	/RETURN
}

SUB castingFollow(string cmd, string opts) {
	/if (${Me.Casting.ID}!=NULL && ${Me.Class.ShortName.Equal[BRD]}==FALSE) {
		/echo stop for casting follow
		/interrupt
	}
	/RETURN
}

SUB castingStop(string cmd, string opts) {
	/if (${Me.Casting.ID}!=NULL) {
		/echo stop cast seen.
		/interrupt
	}
	/RETURN
}

SUB castingCast(string cmd, string opts) {
	/call p3cast ${getStackCommandOpt[${opts},TARGETID]} "${getStackCommandOpt[${opts},SPELL]}" "${getStackCommandOpt[${opts},SRC]}" ${getStackCommandOpt[${opts},SRCIDX]}

	/if (${Macro.Return.Equal[PASS_CAST_STARTED]}) {
		/varset castingOpts ${opts}
		/vardata castingSpell ${getSpellFromCastRequest[${getStackCommandOpt[${castingOpts},SPELL]}]}
		/vardata untrackedCastSpell castingSpellEmpty
		/varset castingTargetID ${getStackCommandOpt[${castingOpts},TARGETID]}
		/invoke ${castingCancelConditions.Append[${getStackCommandOpt[${castingOpts},CANCELCONDITIONS]}]}
		/echo Cast Started [${castingSpell}].
	} else /if (${Macro.Return.Equal[PASS_INSTANT_CAST]}) {
		/echo Instant cast [${getStackCommandOpt[${opts},SPELL]}] done.
		/call castComplete PASS_INSTANT_CAST
	} else /if (${Macro.Return.Find[FAIL_]}!=NULL) {
		/echo Cast didn't start [${castingOpts}] => ${Macro.Return}
		/call castComplete ${Macro.Return}
	}

	/RETURN
}

SUB castingMaintenance() {
	/declare sourceArr string local
	/declare i int local

	/if (${Twist.Twisting}==TRUE) /return

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
			/call castingSetIdle
		} else {
			|/echo Casting ${castingSpell} as expected...
			/varset buffCheckTimer 10
			/if (${castingTargetID}==${Target.ID}) /call checkCancelConditions

			/if (${Macro.Return}==TRUE) {
				/BC Cancel Conditions met: [${getStackCommandOpt[${castingOpts},CANCELCONDITIONS]}]
				/interrupt
				/delay 3
				/return
			}
		}

		/RETURN RESET
	}

	/if (${Me.Casting.ID}==NULL) {
		/if (${castingSpell.ID} != NULL) {
			/echo Detected end of cast for [${castingSpell}]

			/varset sourceArr ${getStackCommandOpt[${castingOpts},SRC]}
			/varset i ${getStackCommandOpt[${castingOpts},SRCIDX]}

			/if (${${sourceArr}[${i},${bAssumeSuccess}].NotEqual[-]}) {
				Assuming Success: on ${${sourceArr}[${i},${bName}].Equal[-]}
			} else {
				/delay 10
				/echo Result => ${Cast.Result}
			}
			
			/if (${${sourceArr}[${i},${bNoSitTimer}].NotEqual[-]}) {
				/echo Pausing medbreak: ${${sourceArr}[${i},${bNoSitTimer}]}
				/call pauseAutoMed ${${sourceArr}[${i},${bNoSitTimer}]}
			}
		
			/if (${Cast.Result.Equal[CAST_SUCCESS]} || ${${sourceArr}[${i},${bAssumeSuccess}].NotEqual[-]}) {
				/call castComplete PASS_${Cast.Result}
			} else {
				/call castComplete FAIL_${Cast.Result}
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
			/if (${assistTarget}==${Target.ID}) /return TRUE
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

	/echo cast: ${castName}

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
			/call movementLockoutStick 1s
		}

		/if (${Twist.Twisting}==TRUE) {
			/twist stop
			/delay 3
		}
	}


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
		/itemnotify "${castName}" rightmouseup
		/if (${hasCastTime}==FALSE) /RETURN PASS_INSTANT_CAST

	} else /if (${castType.Equal[SPELL]}) {
		/if (${Me.Class.ShortName.Equal[BRD]} && ${castType.Equal[SPELL]}) {
			/twist once ${Me.Gem[${castName}]}
		} else {
			/cast "${castName}"
		}
	}

	/delay 5

	/varset ret PASS_CAST_STARTED

	/RETURN ${ret}
}

SUB castComplete(string result) {
	/declare funcName string local
	/varset funcName ${getStackCommandOpt[${castingOpts},ONRESULT]}

	/varset buffCheckTimer 10

	/if (${funcName.Equal[-]}) /RETURN
	
	/call ${funcName} ${result} "${castingOpts}"

	/RETURN
}

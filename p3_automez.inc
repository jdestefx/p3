SUB p3_automez_init() {

	/call p3RegisterCommand KILL autoMezKillCheck 5

	/declare autoMezList list outer
	/declare autoMezCheckTimer timer outer 0
	/declare autoMezLastCastIndex int outer 0

	/declare autoMezActionTakenTimer timer outer 0
	/declare autoMezEnabled bool outer FALSE
	/declare autoMezSpell string outer NONE
	/declare autoMezTimerMarginSeconds string outer 10
	/declare autoMezMustHaveName string outer -
	/declare autoMezMobLevels string outer 10 65
	/declare autoMezMobRadius string outer 100
	/declare autoMezSpawnSearch string outer NPC LOS TARGETABLE

	/varset autoMezEnabled 				${getBotIniOption[AutoMez.AutoMezEnabled,${autoMezEnabled}]}
	/varset autoMezSpell 				${getBotIniOption[AutoMez.AutoMezSpell,${autoMezSpell}]}
	/varset autoMezMobLevels 			${getBotIniOption[AutoMez.AutoMezMobLevelRange,${autoMezMobLevels}]}
	/varset autoMezMobRadius 			${getBotIniOption[AutoMez.AutoMezMobRadius,${autoMezMobRadius}]}
	/varset autoMezTimerMarginSeconds 	${getBotIniOption[AutoMez.AutoMezTimerMarginSeconds,${autoMezTimerMarginSeconds}]}
	/varset autoMezMustHaveName 		${getBotIniOption[AutoMez.AutoMezMustHaveName,${autoMezMustHaveName}]}

	/varset autoMezSpawnSearch ${autoMezSpawnSearch} RADIUS ${autoMezMobRadius} RANGE ${autoMezMobLevels}
	
	/RETURN
}


SUB automezMaintenance() {

	/if (${autoMezEnabled}==TRUE) {

		/call checkAutoMez
		/if (${Macro.Return}==TRUE) /varset autoMezActionTakenTimer 200

		/if (${Macro.Return}==FALSE && ${autoMezActionTakenTimer}>0) {
			/call clearStackActions CAST
		}

		/if (${autoMezActionTakenTimer}>0) /RETURN RESET
	}

	/RETURN
}


SUB autoMezKillCheck() {
	/if (${autoMezActionTakenTimer}>0) {
		/RETURN RESET
	}

	/RETURN
}

SUB checkAutoMez() {
	/if (${autoMezCheckTimer}>0) /RETURN FALSE
	/if (${Stick.Active}==TRUE) /RETURN FALSE
	/if (${Me.Moving}==TRUE) /RETURN FALSE
	/if (${Me.Invis}==TRUE) /RETURN FALSE

	/varset autoMezCheckTimer 5

	/declare spawnsInRangeCount int local ${SpawnCount[${autoMezSpawnSearch}]}
	/declare i int local
	/declare j int local
	/declare xSpawn spawn local
	/declare targetID int local
	/declare xSpell spell local
	/declare useMezSpell string local
	/declare startIndex int local 0

	/call getReadyMezSpell

	/if (${Macro.Return.Equal[-]}) /RETURN
	/varset useMezSpell ${Macro.Return}

	/if (${isCastReady[${useMezSpell}]}==FALSE) /RETURN


	| SCAN AND ADD CANDIDATES
	/if (${spawnsInRangeCount}>0) {

		/for i 1 to ${spawnsInRangeCount}
			/vardata xSpawn NearestSpawn[${i},${autoMezSpawnSearch}]
			/if (${Spawn[id ${xSpawn.ID}].ID}==${assistSpawnID}) /continue
			/if (${Spawn[id ${xSpawn.ID}].Dead}==TRUE) /continue
			/if (${Spawn[id ${xSpawn.ID}].ID}==NULL) /continue
			/if (${Spawn[id ${xSpawn.ID}].Type.Equal[Pet]}) /continue
			/if (${isNetBotAttackingID[${xSpawn.ID}]}==TRUE) /continue
			/if (${assistSpawnID}==${xSpawn.ID}) /continue

			/if (${autoMezMustHaveName.NotEqual[FALSE]}) {
				/if (${checkAutoMezNameMatch[${xSpawn.CleanName}]}==FALSE) /continue
			}


			/if (${autoMezList.Index[${xSpawn.ID}]}==-1) {
				|/echo add to maint list: ${xSpawn.ID} ${xSpawn}
				/invoke ${autoMezList.Append[${xSpawn.ID}]}
			}

		/next i
	}


	| MAINTENANCE: CLEANUP ANYTHING THAT MIGHT BE DEAD OR GONE OR IS BEING ATTACKED BY A NETBOT
	/if (${autoMezList.Count}>0) {
		/for i ${Math.Calc[${autoMezList.Count}-1]} downto 0
			/if (${Spawn[id ${autoMezList.Item[${i}]}].Aggressive}==FALSE || ${autoMezList.Item[${i}]}==${assistSpawnID} || ${Spawn[id ${autoMezList.Item[${i}]}].Dead}==TRUE || ${Spawn[id ${autoMezList.Item[${i}]}].ID}==NULL || ${isNetBotAttackingID[${autoMezList.Item[${i}]}]}==TRUE) {
				/call wipeLockout mez_${autoMezList.Item[${i}]}
				/invoke ${autoMezList.Erase[${i}]}
				/continue
			}
		/next i
	}

	/if (${autoMezList.Count}==0) {
		/varset autoMezActionTakenTimer 0
		/RETURN FALSE
	}


	/if (${autoMezLastCastIndex} <= ${Math.Calc[${autoMezList.Count}-1]}) /varset startIndex ${autoMezLastCastIndex}

	| PROCESS LIST AND MEZ
	/for i ${startIndex} to ${Math.Calc[${autoMezList.Count}-1]}
		/varset targetID ${autoMezList.Item[${i}]}

		/if (${Spawn[id ${targetID}].Dead}) /continue

		/call fullTarget ${targetID}

		/if (${Target.Mezzed.ID}!=NULL) {
			/if (${Target.Mezzed.Duration.TotalSeconds} > ${autoMezTimerMarginSeconds}) {
				/echo detected already-mezzed. locking out for ${Math.Calc[${Target.Mezzed.Duration.TotalSeconds}-${autoMezTimerMarginSeconds}]}s
				/call lockout mez_${targetID} ${Math.Calc[${Target.Mezzed.Duration.TotalSeconds}-${autoMezTimerMarginSeconds}]}s
				/continue
			}
		}

		/if (${Target.Mezzed.ID}==NULL || ${isLockoutExpired[mez_${targetID}]}==TRUE) {
			/call clearStackActions CAST
			/invoke (${cq.Append[CAST;SPELL=${useMezSpell};TARGETID=${targetID};CANCELCONDITIONS=NETBOTATTACKING:${targetID}|TARGETISASSITTARGET;ONRESULT=mezCastRequestResult;]})
			/varset autoMezLastCastIndex ${i}
			/RETURN TRUE
		}
	/next i

	/varset autoMezLastCastIndex 0
	/RETURN FALSE
}

SUB mezCastRequestResult(string result, string opts) {
	/declare targetID int local ${getStackCommandOpt[${opts},TARGETID]}
	/declare cSpellName string local ${getStackCommandOpt[${opts},SPELL]}
	/declare cSpell spell local

	/call getSpellFromCastRequest "${cSpellName}"
	/vardata cSpell ${Macro.Return}

	/if (${result.Find[PASS_]}!=NULL) {
		/call lockout mez_${targetID} ${Math.Calc[${cSpell.Duration.TotalSeconds}-${autoMezTimerMarginSeconds}]}s
	}

	/RETURN
}

SUB checkAutoMezNameMatch(string n) {
	/declare i int local

	/for i 1 to ${Math.Calc[${autoMezMustHaveName.Count[,]}+1]}
		/if (${n.Find[${autoMezMustHaveName.Token[${i},,]]}]}!=NULL) {
			/RETURN TRUE
		}
	/next i

	/RETURN FALSE
}

SUB getReadyMezSpell() {
	/declare i int local 

	/if (${autoMezSpell.Find[,]}==0) {
		/if (${isCastReady[${autoMezSpell}]}==FALSE) {
			/RETURN -
		} else {
			/RETURN ${autoMezSpell}
		}
	} else {
		/for i 1 to ${Math.Calc[${autoMezSpell.Count[,]}+1]}
			/if (${isCastReady[${autoMezSpell.Token[${i},,]}]}==TRUE) {
				/RETURN ${autoMezSpell.Token[${i},,]}
			}
		/next i
	}

	/RETURN -
}


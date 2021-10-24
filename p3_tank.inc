SUB p3_tank_init() {
	|/call p3RegisterCommand STOP tankStop 40
	|/call p3RegisterCommand KILL tankKill 40
	
	/declare tankModeEnabled bool outer FALSE
	/declare dontTakeAggroFrom string outer NONE
	/declare tauntSuccessList list outer
	/declare tankCheckTimer timer outer 0

	/varset tankModeEnabled ${getBotIniOption[Tank.Enabled,${tankModeEnabled}]}
	/varset dontTakeAggroFrom ${getBotIniOption[Tank.DontTakeAggroFromNames,${dontTakeAggroFrom}]}

	/RETURN
}


SUB tankMaintenance() {
	/if (${tankModeEnabled}==TRUE && ${tankCheckTimer}==0) {
		/call checkTauntSuccessList
		/call checkTankStatus
		/varset tankCheckTimer 5
	}

	/RETURN
}

SUB tankKill(string cmd, string opts) {
	/RETURN
}

SUB tankStop(string cmd, string opts) {
	/RETURN
}


SUB checkTankStatus() {
 /if (${assistSpawnID}==0) /return
 	/echo checking tank stuff

	/declare i int local

	/if (${dontTakeAggroFrom.NotEqual[NONE]} && ${Me.TargetOfTarget.ID}!=NULL) {
		/call characterPassesAllowKeySeries ${Me.TargetOfTarget.CleanName.Upper} "${dontTakeAggroFrom.Upper}"
		/if (${Macro.Return}==TRUE) /return
	}

	/for i 1 to ${iniTankActions.Size}
		/if (${iniTankActions[${i},1].Equal[-]}) /return

		/if (${iniTankActions[${i},${bNotToT}].NotEqual[-]}) {
			/if (${Me.TargetOfTarget.ID}==${Me.ID}) /continue
		}

		/if (${iniTankActions[${i},${bAmToT}].NotEqual[-]}) {
			/if (${Me.TargetOfTarget.ID}!=${Me.ID}) /continue
		}

		/if (${iniTankActions[${i},${bOnlyAfterTaunt}].NotEqual[-]}) {
			/if (${tauntSuccessListContains[${assistSpawnID}_]}==FALSE) {
				/continue
			}
		}

		|/if (${iniTankActions[${i},${bDoAbility}].NotEqual[-]}) {
		|	/if (${Me.AbilityReady[${iniTankActions[${i},1]}]} == FALSE) /continue 
		|}

		|/if (${iniTankActions[${i},${bSelfNotHighestAggro}].NotEqual[-]}) {
		|	/if (${Me.PctAggro}==100) /continue
		|}

		/if (${iniTankActions[${i},${bCombatTimeOver}].NotEqual[-]}) {
			/if (${Math.Calc[(12000-${assistTime})/10]} < ${iniTankActions[${i},${bCombatTimeOver}]}) /continue
		}

		/if (${iniTankActions[${i},${bSelfOverEnd}].NotEqual[-]}) {
			/if (${Me.PctEndurance} < ${iniTankActions[${i},${bSelfOverEnd}]} ) /continue
		}

		|/if (${iniTankActions[${i},${bNextHighestAggroOver}].NotEqual[-]}) {
		|	/if (${Me.PctAggro} != 100) /continue
		|	/if (${Me.SecondaryPctAggro} < ${iniTankActions[${i},${bNextHighestAggroOver}]}) /continue
		|}

		/if (${iniTankActions[${i},${bTargetUnderDistance}].NotEqual[-]}) {
			/if (${Target.Distance} > ${iniTankActions[${i},${bTargetUnderDistance}]}) /continue
		}

		/if (${iniTankActions[${i},${bRequireMobsInProximity}].NotEqual[-]}) {
			/if (${SpawnCount[npc loc ${Me.X} ${Me.Y} radius 30]} < ${iniTankActions[${i},${bRequireMobsInProximity}]}) /continue
		}

		/if (${iniTankActions[${i},${bDoCommand}].NotEqual[-]}) {
			/call p3command "${iniTankActions[${i},${bDoCommand}]}"
		}

		/echo SPELL=${iniTankActions[${i},1]}
		/invoke (${cq.Append[CAST;SPELL=${iniTankActions[${i},1]};TARGETID=${assistSpawnID};]})

		/if (${Macro.Return.Equal[CAST_SUCCESS]}) /return

	/next i

	/RETURN
}

SUB checkTauntSuccessList() {
	/if (${tauntSuccessList.Count}==0) /return

	/declare i int local 
	/declare t int local

	/for i ${Math.Calc[${tauntSuccessList.Count}-1]} downto 0
		/varset t ${tauntSuccessList.Item[${i}].Token[2,_]}

		/if (${Math.Calc[${MacroQuest.Running} - ${t} >5000]}) {
			/invoke ${tauntSuccessList.Erase[${i}]}
		}  

	/next i

	/RETURN
}



SUB tauntSuccessListContains(string s)
   /if (${tauntSuccessList.Count}==0) /return FALSE
   /declare i int local

   /for i 0 to ${Math.Calc[${tauntSuccessList.Count}-1]}
      /if (${tauntSuccessList.Item[${i}].Find[${s}]}!=NULL) /return TRUE
   /next i

   /return FALSE
/RETURN

SUB p3_castorders_init() {
	/declare adhocCastListCheckTimer timer outer 0
	/declare adhocCastList list outer 
	/declare adHocCastPendingTimer timer outer 0

	/RETURN
}


SUB castordersMaintenance() {
	/if (${adhocCastList.Count}>0 && ${adHocCastPendingTimer}==0) {
		/call checkAdhocCastList
	}

	/RETURN ${Macro.Return}
}

SUB addCastOrder(string castName, string castTarget, bool trackRecastDuration) {
	/varset castName ${castName.Upper}

	/declare i int local
	/declare botName string local
	/declare botID int local
	/declare spellID int local
	/declare castSpell spell local

	/if (${castTarget.Upper.Equal[ALL]}) {
		/for i 1 to ${NetBots.Counts}
			/varset botName ${NetBots.Client[${i}]}
			/varset botID ${NetBots[${botName}].ID}
			/if (${adhocCastList.Contains[${castName}_${botID}]}==TRUE) /continue
			/invoke (${adhocCastList.Append[${castName}_${botID}]})
		/next i
	} else /if (${castTarget.Upper.Equal[SELF]}) {
		/if (${adhocCastList.Contains[${castName}_${Me.ID}]}==TRUE) {
			/bc Already have a cast request for ${castName} for ${Me.CleanName}
			/return
		}
		/invoke ${adhocCastList.Append[${castName}_${Me.ID}]}
	} else /if (${Spawn["${castTarget}"].ID}!=NULL) {
		/if (${adhocCastList.Contains[${castName}_${Spawn["${castTarget}"].ID}]}==TRUE) {
			/bc Already have a cast request for ${castName} for ${Spawn["${castTarget}"].CleanName}
			/return
		}
		/invoke ${adhocCastList.Append[${castName}_${Spawn["${castTarget}"].ID}_${If[${trackRecastDuration},1,0]}]}
	}
 
	/RETURN
}



SUB checkAdhocCastList() {
	/if (${adhocCastListCheckTimer}>0) /return
	/varset adhocCastListCheckTimer 3

	/declare i int local
	/declare castName string local
	/declare targetID string local
	/declare trackRecast string local
	/declare actionTaken bool local FALSE

	/if (${Me.Casting.ID}!=NULL) /return
	/if (${movingTimer}>0 && ${canCastWhileMoving}==FALSE) /return
	/if (${Me.Invis}==TRUE) /return
	/if (${Nav.Active}==TRUE) /return

	/for i ${Math.Calc[${adhocCastList.Count}-1]} downto 0
		/varset targetID ${adhocCastList.Item[${i}].Token[2,_]}
		/if (${Spawn[id ${targetID}].ID}==NULL || ${Spawn[id ${targetID}].Dead}==TRUE) {
			/invoke ${adhocCastList.Erase[${i}]}
		}
	/next i

	/if (${adhocCastList.Count}==0) {
		/RETURN FALSE
	}

	/for i 0 to ${Math.Calc[${adhocCastList.Count}-1]}
		/varset castName ${adhocCastList.Item[${i}].Token[1,_]}
		/varset targetID ${adhocCastList.Item[${i}].Token[2,_]}
		/varset trackRecast ${adhocCastList.Item[${i}].Token[3,_]}

		/if (${isCastReady[${castName}]}==FALSE) /continue
		/invoke (${cq.Append[CAST;SPELL=${castName};TARGETID=${targetID};ONRESULT=onAdhocCastResult;ADHOCINDEX=${i};CANCELCONDITIONS=TARGETDEAD;TRACKRECAST=${trackRecast}]})
		/varset adHocCastPendingTimer 5s
		/varset actionTaken TRUE
		/break
	/next i

	/RETURN ${actionTaken}
}

SUB onAdhocCastResult(string result, string opts) {
	/declare removeIndex string local ${getStackCommandOpt[${opts},ADHOCINDEX]}
	
	/if (${result.NotEqual[FAIL_INTERRUPT]} && ${result.NotEqual[FAIL_FIZZLE]}) {
		/invoke ${adhocCastList.Erase[${removeIndex}]}
	}

	/varset adHocCastPendingTimer 0	
	/RETURN
}

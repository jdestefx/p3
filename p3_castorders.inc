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


SUB checkAdhocCastList() {
	/if (${adhocCastListCheckTimer}>0) /return
	/varset adhocCastListCheckTimer 3

	/declare i int local
	/declare castName string local
	/declare targetID string local
	/declare actionTaken BOOL local FALSE

	/if (${Me.Casting.ID}!=NULL) /return
	/if (${movingTimer}>0 && ${canCastWhileMoving}==FALSE) /return
	/if (${Me.Invis}==TRUE) /return
	/if (${Stick.Active}==TRUE || ${Nav.Active}==TRUE) /return

	/for i ${Math.Calc[${adhocCastList.Count}-1]} downto 0
		/varset targetID ${adhocCastList.Item[${i}].Token[2,_]}
		/if (${Spawn[id ${targetID}].ID}==NULL || ${Spawn[id ${targetID}].Dead}==TRUE) {
			/invoke ${adhocCastList.Erase[${i}]}
		}
	/next i

	/for i 0 to ${Math.Calc[${adhocCastList.Count}-1]}
		/varset castName ${adhocCastList.Item[${i}].Token[1,_]}
		/varset targetID ${adhocCastList.Item[${i}].Token[2,_]}

		/if (${isCastReady[${castName}]}==FALSE) /continue
		/invoke (${cq.Append[CAST;SPELL=${castName};TARGETID=${targetID};ONRESULT=onAdhocCastResult;ADHOCINDEX=${i};CANCELCONDITIONS=TARGETDEAD;]})
		/varset adHocCastPendingTimer 10s
		/varset actionTaken TRUE
		/break
	/next i
 
	/RETURN ${actionTaken}
}

SUB onAdhocCastResult(string result, string opts) {
	/ECHO ADHOCRES: ${result} ${opts}

	/declare removeIndex string local ${getStackCommandOpt[${opts},ADHOCINDEX]}
	
	/if (${result.Find[PASS_]}!=NULL) {
		/ECHO .
		/invoke ${adhocCastList.Erase[${removeIndex}]}
	}

	/varset adHocCastPendingTimer 0	
	/RETURN
}

SUB p3_targetting_init {
	/call p3RegisterCommand STOP targettingStop 51
	/call p3RegisterCommand KILL targettingKill 51
	/call p3RegisterCommand CAST targettingCast 10

	/declare lockTargetID int outer 0
	/declare castTargetID int outer 0

	/declare keepTargetClearWhileIdle bool outer FALSE

	/varset keepTargetClearWhileIdle ${getBotIniOption[Misc.KeepTargetClearWhileIdle,${keepTargetClearWhileIdle}]}

	/RETURN
}

SUB targettingMaintenance() {

	| MAINTAIN CAST TARGET LOCK UNTIL NO LONGER CASTING
	/if (${castTargetID}!=0) {
		/if (${Me.Casting.ID}==NULL) {
			/varset castTargetID 0
		} else {
			/if (${Target.ID}!=${castTargetID}) {
				/tar id ${castTargetID}
			}
		}
		/RETURN
	}

	| HANDLE REGULAR TARGET LOCK

	/if (${Spawn[id ${lockTargetID}].ID}==NULL || ${Spawn[id ${lockTargetID}].Dead}==TRUE) {
		/varset lockTargetID 0
	}

	|/if (${lockTargetID}==0 && ${Target.ID}!=NULL && ${keepTargetClearWhileIdle}==TRUE) /target clear

	/if (${lockTargetID}==0) /RETURN
	
	/if (${Target.ID}!=${lockTargetID}) {
		/tar id ${lockTargetID}
	}

	/RETURN
}

SUB targettingCast(string cmd, string opts) {
	/varset castTargetID ${getStackCommandOpt[${opts},TARGETID]}

	/if (${Target.ID}!=${castTargetID}) {
		/tar id ${castTargetID}
		/delay 3
	}

	/RETURN
}

SUB targettingStop(string cmd, string opts) {
	/if (${useMeleeCombat}==TRUE) {
		/varset lockTargetID 0
	}

	/RETURN
}

SUB targettingKill(string cmd, string opts) {
	/if (${useMeleeCombat}==TRUE) {
		/varset lockTargetID ${getStackCommandOpt[${opts},SPAWNID]}
	}

	/RETURN
}

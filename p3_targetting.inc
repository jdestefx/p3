SUB p3_targetting_init {
	/call p3RegisterCommand STOP targettingStop 51
	/call p3RegisterCommand KILL targettingKill 51

	/declare lockTargetID int outer 0

	/RETURN
}

SUB targettingStop(string cmd, string opts) {
	/if (${useMeleeCombat}==TRUE) {
		/varset lockTargetID 0
	}
	/RETURN
}

SUB targettingKill(string cmd, string opts) {
	/declare targetID int local ${getStackCommandOpt[${opts},SPAWNID]}
	/varset lockTargetID ${targetID}
	/RETURN
}

SUB targettingMaintenance() {
	/if (${lockTargetID}==0 || ${Spawn[id ${lockTargetID}].ID}==NULL || ${Spawn[id ${lockTargetID}].Dead}==TRUE) {
		/if (${Target.ID}!=NULL) {
			/target clear
		}
	} else {
		/if (${Target.ID}!=${lockTargetID}) {
			/tar id ${lockTargetID}
		}
	}

	/RETURN
}
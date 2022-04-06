SUB p3_roam_init() {
	/call p3RegisterCommand ROAMLOCK roamRoam 50
	
	/declare roamTargetID int outer 0
	/declare roamMode bool outer FALSE
	/declare roamRefreshTimer timer outer 0

	/declare roamFlyDown bool outer FALSE
	/varset roamFlyDown ${getBotIniOption[Misc.RoamFlyDown,${roamFlyDown}]}

	/RETURN
}


SUB roamMaintenance() {
	/if (${roamMode}==TRUE) /call checkRoam
	/RETURN
}

SUB roamRoam(string cmd, string opts) {
	/declare targetID string local ${getStackCommandOpt[${opts},TARGETID]}

	/if (${targetID}==0) {
		/varset roamMode FALSE
		/bc Roam off.
	} else {
		/varset roamMode TRUE
		/varset roamTargetID ${targetID}
		/bc Roaming with ${Spawn[id ${roamTargetID}].CleanName}
	}

	/RETURN
}

SUB checkRoam() {
	/if (${roamRefreshTimer}==0 && ${Spawn[id ${roamTargetID}].Distance3D}>40) {
		/invoke ${cq.Append[ROAMMOVESTART;]}

		/if (${assistSpawnID}!=0) {
			/call assistStop STOP STOP;ASSISTHOLDTIMER=20
		}
		
		/if (${navPlugin}==TRUE) {
			/nav id ${roamTargetID}
			/if (${roamFlyDown}==TRUE) {
				/keypress CMD_MOVE_DOWN hold
				/timed 3 /keypress CMD_MOVE_DOWN
			}
		} else {
			/moveto loc ${Spawn[${roamTargetID}].Y} ${Spawn[${roamTargetID}].X} ${Spawn[${roamTargetID}].Z} 
		}
		/varset roamRefreshTimer 30
	}

	/RETURN
}


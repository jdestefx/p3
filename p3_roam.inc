SUB p3_roam_init() {
	/call p3RegisterCommand ROAMLOCK roamRoam 50
	/call p3RegisterCommand ROAMOFF roamRoamOff 50
	/call p3RegisterCommand CAMPON roamCampOn 60
	
	/declare roamTargetID int outer 0
	/declare roamMode bool outer FALSE
	/declare roamRefreshTimer timer outer 0
	/declare roamTargetZoneID int outer 0

	/declare roamFlyDown bool outer FALSE
	/varset roamFlyDown ${getBotIniOption[Misc.RoamFlyDown,${roamFlyDown}]}

	/RETURN
}


SUB roamMaintenance() {
	/if (${roamMode}==TRUE) /call checkRoam
	/RETURN
}

SUB roamRoamOff(string cmd, string opts) {
	/declare sourceName string local ${getStackCommandOpt[${opts},SOURCENAME]}
	/if (${Spawn[=${sourceName}].ID}==FALSE) /RETURN
	/if (${roamMode}==FALSE) /RETURN

	/call disableRoam

	/RETURN
}

SUB disableRoam() {
	/varset roamTargetZoneID 0
	/varset roamMode FALSE
	/bc Roam off.
	/RETURN
}

SUB roamCampOn(string cmd, string opts) {
	/if (${roamMode}==FALSE) /RETURN
	/call disableRoam
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
		/varset roamTargetZoneID ${Zone.ID}
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
		
		/if (${Zone.ID}!=${roamTargetZoneID}) /RETURN

		/if (${navPlugin}==TRUE) {
			/if (${Navigation.PathExists[id ${roamTargetID}]}==FALSE) {
				/bc Can't find path to my roam target. Trying face/walk for 1s.
				/face fast id ${roamTargetID}
				/keypress FORWARD HOLD
				/delay 10
				/keypress FORWARD
				/return
			}

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


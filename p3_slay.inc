SUB p3_slay_init {
	/call p3RegisterCommand STOP slayStop 50

	/declare slayProxMobs list outer
	/declare slayTankStates[16] string outer -
	/declare slayCheckTimer timer outer 0

	/declare slayEnabled bool outer 				FALSE
	/declare amSlayCoordinator bool outer		FALSE
	/declare slayMembers string outer			-
	/declare slaySearch string outer				npc los radius 50 range 1 80

	/varset amSlayCoordinator 		${getBotIniOption[Slay.Coordinator,${amSlayCoordinator}]}
	/varset slayMembers 				${getBotIniOption[Slay.Members,${slayMembers}]}
	/varset slaySearch 				${getBotIniOption[Slay.SpawnSearch,${slaySearch}]}

	/if (${amSlayCoordinator}==TRUE) {
		/varset slayEnabled TRUE
		/call clearTankStates
	}

	/RETURN
}

SUB slayStop(string cmd, string opts) {
	/call clearTankStates
	/return
}

SUB slayMaintenance {
	/if (${slayEnabled}==FALSE) /return
	/if (${amSlayCoordinator}==FALSE) /return
	/if (${slayCheckTimer}>0) /return
	/if (${Stick.Active}==TRUE && ${assistSpawnID}==0) /RETURN

	/varset slayCheckTimer 5

	/call verifySlayTankStates
	/call scanMobs
	/call checkTankAssignments

	/RETURN
}


SUB scanMobs() {
	/declare mobCount int local ${SpawnCount[${slaySearch} loc ${Me.X} ${Me.Y}]}
	/declare i int local
	/declare mobID int local
	/invoke ${slayProxMobs.Clear}

	/if (${mobCount}==0) /RETURN

	/for i 1 to ${mobCount}
		/varset mobID ${NearestSpawn[${i},${slaySearch}].ID}
		/invoke ${slayProxMobs.Append[${mobID}]}
	/next i

	/RETURN
}

SUB verifySlayTankStates() {
	/declare max int local ${Math.Calc[${slayMembers.Count[,]}+1]}
	/declare tankName string local
	/declare tankTargetID string local
	/declare i int local
	/declare e string local


	/for i 1 to ${max}
		/varset e ${slayTankStates[${i}]}
		/varset tankName ${e.Token[1,:]}
		/varset tankTargetID ${e.Token[2,:]}

		/if (${tankTargetID} != 0 && ${NetBots[${tankName}].TargetID} != ${tankTargetID}) {
			/varset slayTankStates[${i}] ${tankName}:0
		}

	/next i

	/RETURN
}


SUB clearTankStates() {
	/declare max int local ${Math.Calc[${slayMembers.Count[,]}+1]}
	/declare i int local

	/for i 1 to ${max}
		/varset slayTankStates[${i}] ${slayMembers.Token[${i},,]}:0
	/next i

	/RETURN
}

SUB checkTankAssignments() {
	/if (${slayProxMobs.Count}==0) /RETURN

	/declare i int local
	/declare mobID int local
	/declare freeTankIndex int local
	/declare freeTankName string local

	/for i 0 to ${Math.Calc[${slayProxMobs.Count}-1]}
		/varset freeTankIndex ${getFreeTankIndex[]}
		/varset freeTankName ${slayTankStates[${freeTankIndex}].Token[1,:]}
		/varset mobID ${slayProxMobs.Item[${i}]}]}

		/if (${freeTankIndex}==NULL) /RETURN

		/if (${arrayIndexOf[slayTankStates,:${slayProxMobs.Item[${i}]}]}==NULL) {
			/varset slayTankStates[${freeTankIndex}] ${freeTankName}:${mobID}
			/bct ${freeTankName} p3 kill ${mobID}
		}
	/next i

	/RETURN
}

SUB getFreeTankIndex() {
	/declare i int local

	/for i 0 to ${Math.Calc[${slayTankStates.Size}-1]}
		/if (${slayTankStates[${i}].Find[:0]}!=NULL) /return ${i}
	/next i

	/RETURN NULL
}

SUB arrayIndexOf(string arr, string search) {
	/declare i int local

	/for i 0 to ${Math.Calc[${${arr}.Size}-1]}
		/if (${${arr}[${i}].Find[${search}]}!=NULL) {
			/return ${i}
		}
	/next i

	/RETURN NULL
}
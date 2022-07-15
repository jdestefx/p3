SUB p3_slay_init {
	/call p3RegisterCommand STOP slayStop 50
	/call p3RegisterCommand KILL slayKill 55

	/declare slayProxMobs list outer
	/declare slayTankStates[16] string outer -
	/declare slayCheckTimer timer outer 0
	/declare slaySearch string outer

	/declare slayEnabled bool outer 			FALSE

	/declare amSlayLeader bool outer			FALSE
	/declare slayMembers string outer			-
	/declare slayLevelRange string outer		1 80
	/declare slayRadius string outer			50
	/declare slayExtra string outer

	/varset amSlayLeader 			${getBotIniOption[Slay.Leader,${amSlayLeader}]}
	/varset slayMembers 			${getBotIniOption[Slay.Members,${slayMembers}]}
	/varset slayLevelRange 			${getBotIniOption[Slay.MobLevelRange,${slayLevelRange}]}
	/varset slayRadius 				${getBotIniOption[Slay.ScanRadius,${slayRadius}]}
	/varset slayExtra 				${getBotIniOption[Slay.ExtraParams,${slayExtra}]}

	/varset slaySearch npc los radius ${slayRadius} range ${slayLevelRange} ${slayExtra}

	/if (${amSlayLeader}==TRUE) {
		|/varset slayEnabled TRUE
		/call clearTankStates
	}

	/RETURN
}


SUB slayKill(string cmd, string opts) {
	/varset slayCheckTimer 2
	/RETURN
}


SUB slayStop(string cmd, string opts) {
	/call clearTankStates
	/varset slayCheckTimer 30
	/return
}

SUB slayMaintenance {
	/if (${Me.Invis}==TRUE) /return
	/if (${slayEnabled}==FALSE) /return
	/if (${amSlayLeader}==FALSE) /return
	/if (${slayCheckTimer}>0) /return
	/if ((${Stick.Active}==TRUE || ${Nav.Active}==TRUE) && ${assistSpawnID}==0) /RETURN

	/varset slayCheckTimer 5

	/call verifySlayTankStates
	/call scanMobs
	/call checkTankAssignments

	/RETURN
}


SUB scanMobs() {
	/declare mobCount int local
	/declare i int local
	/declare xSpawn spawn local

	/invoke ${slayProxMobs.Clear}

	/varset mobCount ${SpawnCount[${slaySearch} loc ${Me.X} ${Me.Y}]}

	/if (${mobCount}>0) {

		/for i 1 to ${mobCount}
			/if (${NearestSpawn[${i},${slaySearch}].ID}!=NULL) {
				/vardata xSpawn NearestSpawn[${i},${slaySearch}]
				/if (${xSpawn.Aggressive}==FALSE) /continue
				/invoke ${slayProxMobs.Append[${xSpawn.ID}]}
			}
		/next i
	}

	/varset mobCount ${SpawnCount[${slaySearch} loc ${Me.X} ${Me.Y} npcpet]}
	/if (${mobCount}>0) {
		/for i 1 to ${mobCount}
			/if (${NearestSpawn[${i},${slaySearch}].ID}!=NULL) {
				/vardata xSpawn NearestSpawn[${i},${slaySearch} npcpet]
				/if (${xSpawn.Master.Type.Equal[PC]}) /continue
				/if (${xSpawn.Aggressive}==FALSE) /continue
				/if (${xSpawn.Charmed.ID}!==NULL) /continue
				/invoke ${slayProxMobs.Append[${xSpawn.ID}]}
			}
		/next i
	}

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
			/bct ${freeTankName} assist on ${mobID}
			/BC ${freeTankName} slay on ${mobID}
		}
	/next i

	/RETURN
}

SUB getFreeTankIndex() {
	/declare i int local
	/declare tankName string local

	/for i 0 to ${Math.Calc[${slayTankStates.Size}-1]}
		/varset tankName ${slayTankStates[${i}].Token[1,:]}
		/if (${Spawn[=${tankName}].ID}==NULL) /continue
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

SUB slayBuildTeam() {
	/declare i int local
	/declare j int local 
	/declare c string local
	/declare botName string local
	/declare finalMembers string local

	/declare prefTankOrder list local
	/invoke ${prefTankOrder.Append[WAR,PAL,SHD,MNK,RNG,ROG,BST,BER]}

	/for i 1 to ${Math.Calc[${prefTankOrder.Count}-1]}
		/varset c ${prefTankOrder.Item[${i}]}

		/for j 1 to ${NetBots.Counts}
			/varset botName ${NetBots.Client[${j}]}

			/if (${NetBots[${botName}].Class.ShortName.Equal[${c}]}) {
				/varset finalMembers ${finalMembers},${botName}
			}
		/next j
	/next i

	/varset finalMembers ${finalMembers.Right[-1]}
	/varset slayMembers ${finalMembers}

	/call writeBotIniOption "Slay.Members" "${finalMembers}"

	/echo New Slay Team/Order:  ${finalMembers}

	/RETURN
}


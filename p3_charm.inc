SUB p3_charm_init() {
	/declare charmCheckTimer timer outer 0
	/declare charmCheckTimerInterval int outer 10
	/declare charmCallAttackTimer timer outer 0
	/declare charmCallAttackTimerInterval outer 30

	/declare charmSpawnID int outer 0
	/declare charmUseSpell string outer NONE
	/declare charmRequireDebuffsImport string outer NONE
	/declare charmRequireDebuffs list outer
	/declare charmOnBreakCommand string outer NONE

	/varset charmUseSpell 				${getBotIniOption[Charm.UseSpell,${charmUseSpell}]}
	/varset charmRequireDebuffsImport	${getBotIniOption[Charm.RequireDebuffs,${charmRequireDebuffsImport}]}
	/varset charmOnBreakCommand 		${getBotIniOption[Charm.OnBreakCommand,${charmOnBreakCommand}]}

	/if (${Bool[${charmRequireDebuffsImport}]}!=FALSE) {
		/invoke ${charmRequireDebuffs.Append[${charmRequireDebuffsImport}]}
	}

	/RETURN
}


SUB charmMaintenance() {

	/if (${charmSpawnID}!=0) {
		/call checkCharmStatus
		/if (${Macro.Return}==FALSE) /RETURN FALSE
	}

	/if (${charmSpawnID}!=0 && ${assistSpawnID}!=0 && ${charmCallAttackTimer}==0) {
		/if (${Target.ID} != ${assistSpawnID}) {
			/tar id ${assistSpawnID}
			/delay 3
		}

		/if (${Pet.Target.ID}!=${assistSpawnID}) {
			/tar id ${assistSpawnID}
			/delay 10
			/pet attack
			/delay 10
			/varset charmCallAttackTimer ${charmCallAttackTimerInterval}
		}

	}

	/RETURN
}


SUB checkCharmStatus() {
	/if (${charmCheckTimer}>0) /RETURN
	/varset charmCheckTimer ${charmCheckTimerInterval}

	/echo Checking charm status...

	/if (${Me.Pet.ID}==${charmSpawnID}) /RETURN
	/if (${Me.Moving}==TRUE) /RETURN
	/if (${Me.Invis}==TRUE) /RETURN
	
	/if (${charmSpawnIsGone[]}==TRUE) {
		/varset charmSpawnID 0
		/RETURN
	}

	/if (${Me.Pet.ID}==NULL) {
		/call charmSpawnByID ${charmSpawnID}
		/RETURN FALSE
	}


	/RETURN
}


SUB charmSpawnIsGone() {
	/if (${Spawn[id ${charmSpawnID}].ID}==NULL) {
		/varset charmSpawnID 0
		/RETURN TRUE
	}

	/if (${Spawn[id ${charmSpawnID}].Dead}==TRUE) {
		/varset charmSpawnID 0
		/RETURN TRUE
	}

	/RETURN FALSE
}

SUB charmSpawnByID() {
	/declare i int local 0
	/declare debuffName string local

	/tar id ${charmSpawnID}
	/delay 10

	/if (${charmRequireDebuffs.Count}>0) {
		/for i 0 to ${Math.Calc[${charmRequireDebuffs.Count}-1]}
			/varset debuffName ${charmRequireDebuffs.Item[${i}]}

			/if (${Bool[${Target.Buff[${debuffName}]}]}==FALSE) {
				/bc Charm target requires ${debuffName}, casting...
				/invoke ${cq.Append[CAST;SPELL=${debuffName};TARGETID=${charmSpawnID};]}				

			}
		/next i

	}

	/invoke ${cq.Append[CAST;SPELL=${charmUseSpell};TARGETID=${charmSpawnID};]}

	/RETURN
}


SUB p3_autodispell_init() {
	/declare autoDispellEnabled bool outer FALSE
	/declare autoDispellCheckDelay int outer 10
	/declare autoDispellSpell string outer -

	/varset autoDispellEnabled 				${getBotIniOption[Misc.AutoDispellEnabled,${autoDispellEnabled}]}
	/varset autoDispellCheckDelay 			${getBotIniOption[Misc.AutoDispellCheckDelay,${autoDispellCheckDelay}]}
	/varset autoDispellSpell 				${getBotIniOption[Misc.AutoDispellSpell,${autoDispellSpell}]}

	/declare autoDispellCheckTimer timer outer 0

	/RETURN
}


SUB autodispellMaintenance() {
	/if (${autoDispellEnabled}==TRUE && ${autoDispellCheckTimer}==0) {
		/call checkAutoDispell
		/varset autoDispellCheckTimer ${autoDispellCheckDelay}
	}

	/RETURN
}


SUB checkAutoDispell() {
	/if (${assistSpawnID}==0) /return

	/if (${targetHasBeneficial[]}==TRUE) {
		/if (${isCastReady[${autoDispellSpell}]}==FALSE) /RETURN
		|/if (${Cast.Ready[${autoDispellSpell}]}==FALSE) /RETURN
		/invoke (${cq.Append[CAST;SPELL=${autoDispellSpell};TARGETID=${assistSpawnID};]})
	}

	/RETURN
}




SUB targetHasBeneficial() {
	| FOR SOME REASON NOT TARGETTING ANYTHING?
	/if (${Target.ID}==NULL) /return
	/if (${assistSpawnID}==0) /return
	/if (${Target.BuffsPopulated}==FALSE) /return

	/declare i int local
	/declare hasDispellable bool local FALSE

	/for i 1 to ${Target.BuffCount}
		/if (${Target.Buff[${i}].Beneficial}==TRUE && ${Target.Buff[${i}].Name.Upper.Find[MITIGATION OF THE MIGHTY]}==NULL) {
			/return TRUE
			|/varset hasDispellable TRUE
		}
	/next i

	/return FALSE

}


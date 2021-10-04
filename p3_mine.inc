SUB p3_mine_init() {
	/call p3RegisterCommand KILL onKillCommand 10
	
	/declare advPathPresent bool outer FALSE
	/if (${Plugin[MQ2AdvPath].Name.Length}!=NULL) /varset advPathPresent TRUE

	/RETURN
}

SUB mineMaintenance() {

	/RETURN
}


SUB onKillCommand(string cmd, string opts) {
	/if (${advPathPresent}==TRUE) /afollow off
	/RETURN
}


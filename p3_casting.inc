SUB p3_casting_init {
	/call p3RegisterCommand FOLLOW castingFollow 52
	/call p3RegisterCommand STOP castingStop 52
	/RETURN
}

SUB castingFollow (string cmd, string opts) {
	/if (${Me.Casting.ID}!=NULL) /interrupt
	/RETURN
}

SUB castingStop (string cmd, string opts) {
	/if (${Me.Casting.ID}!=NULL) /interrupt
	/RETURN
}

SUB castingMaintenance() {
	
	/RETURN
}
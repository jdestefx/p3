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

#Event OMMMask "#*#You feel a gaze of deadly power focusing on you#*#"
Sub EVENT_OMMMask
    /rsay IT'S ME!!!!
    |/declare strOrgMask string ${Me.Inventory[face]}
    /if (${Me.Class.ShortName.Equal[BRD]}) /twist off
    /if (${Cast.Timing}>1) /multiline ; /stopcast;/delay 1s
    |/exchange "Mirrored Mask" face
    /while (!${Me.Song[Reflective Skin].ID}) {
        /useitem "Mirrored Mask"
    }
    |/exchange "${strOrgMask}" face
    /if (${Me.Class.ShortName.Equal[BRD]}) /twist
    /rsay I clicked my mask.
/return


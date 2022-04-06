SUB p3_mine_init() {
	/call p3RegisterCommand KILL onMineKillCommand 10
	
    /declare rogSneakEnabled bool outer FALSE

	/declare advPathPresent bool outer FALSE
	/if (${Plugin[MQ2AdvPath].Name.Length}!=NULL) /varset advPathPresent TRUE

    /declare wuCheck bool outer FALSE
    /declare wuCheckTimer timer outer
    /declare wuRecheckTime int outer 30

	/RETURN
}

SUB mineMaintenance() {
    /if (${rogSneakEnabled}==TRUE) {
        /call checkRogSneak
    }

    /if (${wuCheck}==TRUE) {
        /if (${Me.Invis}==FALSE && ${Me.CombatAbilityReady[Fists of Wu]}==TRUE && ${Me.ActiveDisc.ID}==NULL) {
            /if (${Me.Song[Fists of Wu].ID}==NULL || ${Me.Song[Fists of Wu].Duration.TotalSeconds}<5) {
                /call checkWu
            }
        }
    }

	/RETURN
}

SUB checkWu() {
    /if (${wuCheckTimer}>0) /return
    /varset wuCheckTimer ${wuRecheckTime}

    /declare otherMonks list local
    /declare spawnID int local
    /declare spawnName string local
    /declare i int local

    /invoke ${otherMonks.Append[${Me.CleanName}]}

    /if (${Me.Grouped}==TRUE) {
        /for i 0 to 6
            /if (${Group.Member[${i}].Index}==NULL) /continue
            /varset spawnName ${Group.Member[${i}]}
            /if (${Spawn[pc =${spawnName}].Class.ShortName.Equal[MNK]}) {
                /invoke ${otherMonks.Append[${spawnName}]}
            }
        /next i
    }

    /declare topEndMonkName string local
    /declare topEndMonkAmt 0

    /echo ${otherMonks.Count}

    /for i 0 to ${Math.Calc[${otherMonks.Count}-1]} {
        /echo ${otherMonks.Item[${i}]} ${NetBots[${otherMonks.Item[${i}]}].CurrentEndurance} / ${topEndMonkAmt}
        /if (${NetBots[${otherMonks.Item[${i}]}].CurrentEndurance}>${topEndMonkAmt}) {
            /varset topEndMonkName ${otherMonks.Item[${i}]}
            /varset topEndMonkAmt ${NetBots[${otherMonks.Item[${i}]}].CurrentEndurance}
        }
    /next i

    /bc top monk: ${topEndMonkName} / ${topEndMonkAmt}

    /if (${topEndMonkName.Equal[${Me.CleanName}]}) {
        /disc fists of wu
        /delay 3
    }

    /RETURN
}


SUB onMineKillCommand(string cmd, string opts) {
    /if (${advPathPresent}==TRUE) /afollow off

    /if (${Me.Class.ShortName.Equal[ROG]}) {
        /call checkRogSurpriseAttack
    }

	/RETURN
}


SUB checkRogSneak() {
    /if (${Me.Class.ShortName.NotEqual[ROG]}) /RETURN

    /if (${assistSpawnID}==0) {
        /if (${Me.Sneaking}==FALSE) /doability sneak
        /if (${Me.Invis[SOS]}==FALSE && ${Me.AbilityReady[Hide]}==TRUE) {
            /makemevisible
            /delay 3
            /doability hide
            /em steps into the shadows.
        }
    }

    /RETURN
}

SUB checkRogSurpriseAttack() {
    /if (${Me.ActiveDisc.ID}==NULL) {
        /disc daggerfall
        /delay 5
    }

    /RETURN
}


#event rogueSneakToggle "[#*#(msg)] p3 sneak #1#"
#event rogueSneakToggle "<#*#> p3 sneak #1#"
SUB event_rogueSneakToggle(string line, string newMode)
    /if (${Me.Class.ShortName.NotEqual[ROG]}) /RETURN

    /if (${newMode.Upper.Equal[ON]}) {
        /varset rogSneakEnabled TRUE
        /bc Sneak on.
    } else /if (${newMode.Upper.Equal[OFF]}) {
        /varset rogSneakEnabled FALSE
        /bc Sneak off.
    } else {
        /bc Unrecognized sneak option. Use ON or OFF.
    }
/RETURN


#Event VishDT "#*#You sense your doom approaching#*#"
Sub Event_VishDT

    /nav rwp StartVish
    /delay 1s
    /stick off
    /rsay seeking a DT cure.
    /while (${Me.Buff[Creeping Doom].ID}) {
        /if (!${Target.CleanName.Equal["mournful spirit"]}) /tar mournful spirit
        /delay 1s
        /if (${Target.Distance}>29 && ${Target.CleanName.Equal["mournful spirit"]}) /nav target
        /if (${Target.Distance}<30 && ${Target.CleanName.Equal["mournful spirit"]}) /stick id ${Target.ID} 5 
        /if (${Target.Distance}<10 && ${Target.CleanName.Equal["mournful spirit"]}) /say Shoulder my burden
    }

    /nav wp StartVish
    /rsay Success!

/return




#Event OMMMask "#*#You feel a gaze of deadly power focusing on you#*#"
    Sub EVENT_OMMMask
        /rs IT'S ME!!!!  (CH rot may miss a beat)


        /if (${Me.Class.ShortName.Equal[BRD]}) {
            /twist off
            /delay 3
        }
        
        /if (${Cast.Timing}>1) {
            /interrupt
            /stopcast
            /delay 10
        }

        /stick off
        /moveto stop

        /while (!${Me.Song[Reflective Skin].ID}) {
            /useitem "Mirrored Mask"
            /delay 10
            /rs Trying to use mask... 
            /stick off
            /moveto stop
        }
        
        /if (${Me.Class.ShortName.Equal[BRD]}) /twist
        /rs Mask click confirmed.
    /return



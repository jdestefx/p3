SUB p3_mine_init() {
	/call p3RegisterCommand KILL onMineKillCommand 60
	
    /declare rogSneakEnabled bool outer FALSE

	/declare advPathPresent bool outer FALSE
	/if (${Plugin[MQ2AdvPath].Name.Length}!=NULL) /varset advPathPresent TRUE

    /declare wuCheck bool outer TRUE
    /declare wuCheckTimer timer outer
    /declare wuRecheckTime int outer 30

	/RETURN
}

SUB mineMaintenance() {
    /if (${rogSneakEnabled}==TRUE) {
        /call checkRogSneak
    }

    /if (${Me.Class.ShortName.Equal[MNK]} && ${wuCheck}==TRUE) {
        /if (${assistSpawnID}==0) /return

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



#event wuoff "<#1#> wu off"
#event wuoff "[#1#(msg)] wu off"
SUB EVENT_wuoff(string line, string chatSender) {
    /if (${Me.Class.ShortName.Equal[MNK]}) /bc Turning wuCheck off.
    /varset wuCheck FALSE
    /RETURN
}

#event wuon "<#1#> wu on"
#event wuon "[#1#(msg)] wu off"
SUB EVENT_wuon(string line, string chatSender) {
    /if (${Me.Class.ShortName.Equal[MNK]}) /bc Turning wuCheck on.
    /varset wuCheck TRUE

    /RETURN
}





#event Define_UquaKey "#*#The #1# must unlock the door to the next room.#*#"
SUB event_Define_UquaKey(string line, string KeyToUse)
    /rs >>^<< The ${KeyToUse} unlocks the door >>^<<
    /popup >>^<< The ${KeyToUse} unlocks the door >>^<<
/return


#event setClass_Warrior "#*#Brute force and brawn#*#"
#event setClass_Shaman "#*#cringe at the appearance of talismans#*#"
#event setClass_Beastlord "#*#Deep gashes of feral savagery#*#"
#event setClass_Necromancer "#*#Doom of death#*#"
#event setClass_Cleric "#*#Dread of celestial spirit#*#"
#event setClass_Shadowknight "#*#Dreads the strike of dead#*#"
#event setClass_Monk "#*#Focused tranquility#*#"
#event setClass_Bard "#*#Foreboding melody#*#"
#event setClass_Paladin "#*#Holy blade#*#"
#event setClass_Rogue "#*#Ignores anything behind it#*#"
#event setClass_Enchanter "#*#Mind and body vulnerable#*#"
#event setClass_Wizard "#*#Falters when struck with the power of the elements#*#"
#event setClass_Berserker "#*#Shies from heavy blades#*#"
#event setClass_Magician "#*#Summoned elements#*#"
#event setClass_Druid "#*#The creature seems weak in the face of the power of nature#*#"
#event setClass_Ranger "#*#True shots and fast blades#*#"

SUB event_setClass_Warrior
    /popup Spawn must be killed by a >>^<< WARRIOR >>^<<
    /rs Spawn must be killed by a >>^<< WARRIOR >>^<<
/return

SUB event_setClass_Shaman
    /popup Spawn must be killed by a >>^<< SHAMAN >>^<<
    /rs Spawn must be killed by a >>^<< SHAMAN >>^<<
/return

SUB event_setClass_Beastlord
    /popup Spawn must be killed by a >>^<< BEASTLORD >>^<<
    /rs Spawn must be killed by a >>^<< BEASTLORD >>^<<
/return

SUB event_setClass_Necromancer
    /popup Spawn must be killed by a >>^<< NECROMANCER >>^<<
    /rs Spawn must be killed by a >>^<< NECROMANCER >>^<<
/return

SUB event_setClass_Cleric
    /popup Spawn must be killed by a >>^<< CLERIC >>^<<
    /rs Spawn must be killed by a >>^<< CLERIC >>^<<
/return

SUB event_setClass_Shadowknight
    /popup Spawn must be killed by a >>^<< SHADOWKNIGHT >>^<<
    /rs Spawn must be killed by a >>^<< SHADOWKNIGHT >>^<<
/return

SUB event_setClass_Monk
    /popup Spawn must be killed by a >>^<< MONK >>^<<
    /rs Spawn must be killed by a >>^<< MONK >>^<<
/return

SUB event_setClass_Bard
    /popup Spawn must be killed by a >>^<< BARD >>^<<
    /rs Spawn must be killed by a >>^<< BARD >>^<<
/return

SUB event_setClass_Paladin
    /popup Spawn must be killed by a >>^<< PALADIN >>^<<
    /rs Spawn must be killed by a >>^<< PALADIN >>^<<
/return

SUB event_setClass_Rogue
    /popup Spawn must be killed by a >>^<< ROGUE >>^<<
    /rs Spawn must be killed by a >>^<< ROGUE >>^<<
/return

SUB event_setClass_Enchanter
    /popup Spawn must be killed by a >>^<< ENCHANTER >>^<<
    /rs Spawn must be killed by a >>^<< ENCHANTER >>^<<
/return

SUB event_setClass_Wizard
    /popup Spawn must be killed by a >>^<< WIZARD >>^<<
    /rs Spawn must be killed by a >>^<< WIZARD >>^<<
/return

SUB event_setClass_Berserker
    /popup Spawn must be killed by a >>^<< BERSERKER >>^<<
    /rs Spawn must be killed by a >>^<< BERSERKER >>^<<
/return

SUB event_setClass_Magician
    /popup Spawn must be killed by a >>^<< MAGICIAN >>^<<
    /rs Spawn must be killed by a >>^<< MAGICIAN >>^<<
/return

SUB event_setClass_Druid
    /popup Spawn must be killed by a >>^<< DRUID >>^<<
    /rs Spawn must be killed by a >>^<< DRUID >>^<<
/return

SUB event_setClass_Ranger
    /popup Spawn must be killed by a >>^<< RANGER >>^<<
    /rs Spawn must be killed by a >>^<< RANGER >>^<<
/return

#event startClass_Priests "#*#The creature cannot stand up to the power of healers#*#"
#event startClass_Hybrids "#*#The creature appears weak to the combined effort of might and magic#*#"
#event startClass_Casters "#*#The creature will perish under the strength of intelligent magic#*#"
#event startClass_Melees "#*#The creature appears weak to the combined effort of strength and cunning#*#"

SUB event_startClass_Priests
    /if (!${Defined[murauderKill]}) {
        /declare murauderKill int outer 3
    } else /if (${Defined[murauderKill]}) {
        /varset murauderKill 3
    }
    /rs Spawn must be killed by a >>^<< PRIEST >>^<<
/return

SUB event_startClass_Hybrids
    /if (!${Defined[murauderKill]}) {
        /declare murauderKill int outer 2
    } else /if (${Defined[murauderKill]}) {
        /varset murauderKill 2
    }
    /rs Spawn must be killed by a >>^<< HYBRID >>^<<
/return

SUB event_startClass_Casters
    /if (!${Defined[murauderKill]}) {
        /declare murauderKill int outer 1
    } else /if (${Defined[murauderKill]}) {
        /varset murauderKill 1
    }
    /rs Spawn must be killed by a >>^<< CASTER >>^<<
/return

SUB event_startClass_Melees
    /if (!${Defined[murauderKill]}) {
        /declare murauderKill int outer 4
    } else /if (${Defined[murauderKill]}) {
        /varset murauderKill 4
    }
    /rs Spawn must be killed by a >>^<< MELEE >>^<<
/return



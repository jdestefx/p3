SUB p3_global_init {
	/declare autoKeywordIgnoreValues ON,OFF,YES,NO,NONE
	/declare iniSectionsAutoProcess string outer Heals,CombatSpells,Buffs,AutoAbilities,BuffRequests,TankActions
	/declare debugMode bool outer FALSE
	/declare botIniFile string outer -
	/declare allNetbotNames string outer

	/call processCommandLine
	/call initializeIniTags
	/call loadIniSections
	/call gatherNetbotNames

	/declare bcsChannels string outer	${getDefaultBCSChannels[]}
	/varset bcsChannels 				${getBotIniOption[Misc.EQBCSChannels,${bcsChannels}]}

	/ECHO bccmd channels ${bcsChannels}
	/bccmd channels ${bcsChannels}

	/RETURN
}

SUB globalMaintenance() {

	/if (${Me.Dead}==TRUE || ${Me.State.Equal[HOVER]}) {
		/delay 10
		/RETURN RESET
	}

	/RETURN
}

SUB loadIniSections {
	/declare sections string local ${Ini[${botIniFile}]}
	/declare a string local
	/declare i int local 1
	/declare sectionName string local
	/declare shortSectionName string local
	/declare newArrayName string local

	/for i 1 to ${sections.Count[|]}
		/varset sectionName ${sections.Token[${i},|]}

		/if (${sectionName.Length}==0 || ${Bool[${sectionName}]}==NULL) /continue
		/if (${Select[${sectionName},${iniSectionsAutoProcess}]}==0) /continue

		/varset shortSectionName ${sectionName.Replace[ ,]}
		/varset newArrayName ini${shortSectionName} 

		/declare ${newArrayName}[96,96] string outer -
		/declare tmpArray[96] string outer
		/call getIniSectionToArray "${botIniFile},${shortSectionName}" tmpArray
		/call processArray tmpArray ${newArrayName}
		/deletevar tmpArray

	/next i

	/for i 1 to ${Math.Calc[${iniSectionsAutoProcess.Count[,]}+1]}
		/if (${Defined[ini${iniSectionsAutoProcess.Token[${i},,]}]}==FALSE) {
			/declare ini${iniSectionsAutoProcess.Token[${i},,]}[1,1] string outer -
		}
	/next i

	/RETURN
}

SUB initializeIniTags {
	/declare flagNamesByIndex list outer
	/invoke ${flagNamesByIndex.Append[bNull,bName,bValue,bTargets,bExcludeTargets,bTargetOverLevel,bCastableDuringAssist,bSelfOverHP,bSelfUnderHP,bSelfOverMana,bSelfUnderMana,bCastableOnlyResting,bCastOnlyDuringAssist,bSelfUnderEnd,bSelfOverEnd,bNoCancelChecks,bRequireGroupInRange,bTargetUnderHP,bTargetOverHP,bNotIfShortBuff,bNotIfLongBuff,bRecastDelay,bHealPct,bTargetRace,bCombatOnly,bTargetUnderLevel,bUseTeamIndex,bRequireMobsInProximity,bForceSpellID,bRequireTargetClass,bNoSitTimer,bRequireBodyType,bRequireDiseased,bRequireGroup,bRequireHurtPartyMembers,bRequireMaxMobsInProximity,bRequireAssistTarget,bRequirePoisoned,bDoCommand,bNotToT,bKeyword,bRebuffSeconds,bActivateBandolier,bRevertToBandolier,bFreeze,bInstructRemove,bMaxResists,bExtendedStackCheck,bIncludeXTargets,bNoCast,bIncludeGroup,bMemInGem,bOnlyXTargets,bRequireCastIdle,bOnlyAfterTaunt,bTargetLifeManaRatio,bAmToT,bRequireCursed,bRequireAssistTag,bNotIfAssistTag,bRequireUW,bCancelAtHP,bAssumeSuccess,bBurnKey,bRequireTeamIndexOver,bIgnoreTargetType,bNotIfMounted,bEnabled,bTagsInvalidated,bIncludeNetBotPets,bOnlyNetBotPets,bRequireTeamIndex,bTargetUnderDistance]}
	
	/declare i int local
	/for i 1 to ${Math.Calc[${flagNamesByIndex.Count}-1]}
		/declare ${flagNamesByIndex.Item[${i}]} string outer ${i}
	/next i

	/return  
}

SUB processArrayTargets(string arrayName, int defaultToNetBots) {
	/declare i int local
	/declare j int local
	/declare finalTargets list local
	/declare removeTargets list local
	/declare finalString string local

	/for i 1 to 64
		/if (${${arrayName}[${i},1].Equal[-]}) /break

		/invoke ${finalTargets.Clear}
		/invoke ${removeTargets.Clear}
		/varset finalString

		/if (${${arrayName}[${i},${bTargets}].NotEqual[-]}) {
			/call processTargets "${${arrayName}[${i},${bTargets}]}"
			/invoke ${finalTargets.Append[${Macro.Return}]}
		} else {
			/invoke ${finalTargets.Append[${allNetbotNames}]}
		}

		/if (${${arrayName}[${i},${bExcludeTargets}].NotEqual[-]}) {
			/call processTargets "${${arrayName}[${i},${bExcludeTargets}]}"
			/invoke ${removeTargets.Append[${Macro.Return}]}

			/for j 0 to ${Math.Calc[${removeTargets.Count}-1]}
				/invoke ${finalTargets.Remove[${removeTargets.Item[${j}]}]}
			/next j
		}

		/for j 0 to ${Math.Calc[${finalTargets.Count}-1]}
			/varset finalString ${finalString},${finalTargets.Item[${j}]}
		/next j
		/varset finalString ${finalString.Right[-1]}

		/varset ${arrayName}[${i},${bTargets}] ${finalString}

	/next i

	/RETURN
}

SUB processTargets(string targets) {
	/declare i int local 
	/declare e string local 
	/declare max int local ${Math.Calc[${targets.Count[,]}+1]}
	/declare ret string local
	/declare ret2 string local

	/for i 1 to ${max}
		/varset e ${targets.Token[${i},,]}
		/if (${e.Left[1].Equal[@]}) {
			/call getMatchesForKey "${e}"
			/if (${ret.Length}==0) {
				/varset ret ${Macro.Return}
			} else {
				/varset ret ${Macro.Return},${ret}
			}
		} else {
			/if (${ret.Length}==0) {
				/varset ret ${e}
			} else {
				/varset ret ${ret},${e}
			}
		}
	/next i

	/RETURN ${ret}
}

SUB processArray(string sourceArray, string destArray) {
	/declare i int local 1
	/declare e string local

	/declare j int local 1
	/declare sectionText string local 
	/declare sectionValue string local 
	/declare sectionName string local 
	/declare keyName string local
	/declare keyValue string local

	/for i 1 to 64
		/varset e ${${sourceArray}[${i}]}

		/if (${Bool[${e}]}==FALSE) /goto :loopesc

		/varset keyName ${e.Token[1,|]}
		/varset keyValue ${e.Right[-${Math.Calc[${keyName.Length}+1]}]}

		/varset ${destArray}[${i},${bName}] ${keyName}
		/varset ${destArray}[${i},${bValue}] ${keyValue}

		/for j 1 to ${Math.Calc[${keyValue.Count[|]}+1]}
			/varset sectionText ${keyValue.Token[${j},|]}
			/if (${sectionText.Length}==0 || ${Bool[${sectionText}]}==NULL) /goto :loopnexti

			/varset sectionName ${sectionText.Token[1,:]}
			/if (${Select[${sectionName},${autoKeywordIgnoreValues}]}>0) /goto :loopnextj

			/if (${sectionText.Count[:]}>0) {
				/varset sectionValue ${sectionText.Token[2,:]}
			} else {
				/varset sectionValue ${sectionText}
			}
			
			/if (${Defined[b${sectionName}]}==FALSE) {
				/bc Warning: >>> ${sectionName} <<< is not a recognized parameter.
				/continue
			}
			
			/varset ${destArray}[${i},${b${sectionName}}] ${sectionValue}

			:loopnextj
		/next j

		:loopnexti
	/next i
	:loopesc
	/RETURN
}

SUB getIniSectionToArray(string iniKey, string arrayName) {
	/declare iniSectionKeys string local
	/declare keyCount int local
	/declare i int local

	/deletevar ${arrayName}

	/varset iniSectionKeys ${Ini[${iniKey}]}

	| EMPTY SECTION? MAKE AN EMPTY ARRAY AND BAIL
	/if (${iniSectionKeys.Equal[NULL]}) {
		/declare ${arrayName}[0] string outer
		/return
	}

	/varset keyCount ${Math.Calc[${iniSectionKeys.Count[|]}-1]}

	| re-create the array with the appropriate size
	/declare ${arrayName}[${keyCount}] string outer

	| fetch and store the ini sections into the array
	/for i 1 to ${keyCount}
		/varset ${arrayName}[${i}] ${iniSectionKeys.Arg[${i},|].Arg[1,#]}|${Ini[${iniKey},${iniSectionKeys.Arg[${i},|]}]}
	/next i

	/RETURN
}

SUB getMatchesForKey(string allowKey) {
	/declare i int local 
	/declare netbotCount int local ${Math.Calc[${NetBots.Counts}]}
	/declare netbotName string local
	/declare netbotClass string local 
	/declare ret string local

	/for i 1 to ${netbotCount}
		/varset netbotName ${NetBots.Client[${i}]}
		/call characterPassesAllowKey ${netbotName} ${allowKey}
		/if (${Macro.Return}==TRUE) {
			/if (${ret.Length}==0) {
				/varset ret ${netbotName}
			} else {
				/varset ret ${ret},${netbotName}
			}
		}
	/next i

	/return ${ret}
}

SUB characterPassesAllowKey(string charName, string allowKey) {
	/if (${Spawn[pc ${charName}].ID}==NULL) /return FALSE

	/declare searchKey string

	/declare xspawn spawn local
	/vardata xspawn Spawn[pc ${charName}]

	/if (${allowKey.Equal[ALL]}) /return TRUE
	/if (${allowKey.Upper.Equal[@ALLNETBOTS]}) /return TRUE

	/if (${allowKey.Upper.Equal[@MANAREGENCLASSES]}) {
		/if (${manaRegenClasses.Find[${xspawn.Class.ShortName}]}!=NULL) /return TRUE
		/return FALSE
	}

	/if (${allowKey.Upper.Equal[@HASTECLASSES]}) {
		/if (${hasteClasses.Find[${xspawn.Class.ShortName}]}!=NULL) /return TRUE
		/return FALSE
	}

	/if (${allowKey.Upper.Equal[@MANACLASSES]}) {
		/if (${manaClasses.Find[${xspawn.Class.ShortName}]}!=NULL) /return TRUE
		/return FALSE
	}

	/if (${allowKey.Upper.Equal[@MELEECLASSES]}) {
		/if (${meleeClasses.Find[${xspawn.Class.ShortName}]}!=NULL) /return TRUE
		/return FALSE
	}

	/if (${allowKey.Upper.Equal[@MONKS]}) {
		/if (${xspawn.Class.ShortName.Equal[MNK]}) /return TRUE
		/return FALSE
	}

	/if (${allowKey.Upper.Equal[@KNIGHTS]}) {
		/if (${knightClasses.Find[${xspawn.Class.ShortName}]}!=NULL) /return TRUE
		/return FALSE
	}

	/if (${allowKey.Upper.Equal[@TANKS]}) {
		/if (${tankClasses.Find[${xspawn.Class.ShortName}]}!=NULL) /return TRUE
		/return FALSE
	}

	/if (${allowKey.Upper.Equal[@CLERICS]}) {
		/if (${xspawn.Class.ShortName.Equal[CLR]}) /return TRUE
		/return FALSE
	}

	/if (${allowKey.Upper.Equal[@BARDS]}) {
		/if (${xspawn.Class.ShortName.Equal[BRD]}) /return TRUE
		/return FALSE
	}

	/if (${allowKey.Upper.Left[5].Equal["ONLY:"]}==TRUE) {
		/varset searchKey ,${allowKey.Arg[2,:]},
		/if (${searchKey.Upper.Find[${charName.Upper}]}!=0) /return TRUE
	}

	/if (${allowKey.Upper.Equal[@GROUP]}==TRUE) {
		/if (${Group.Member[${charName}]}!=NULL) /return true
	}
	
	/RETURN
}

SUB characterPassesAllowKeySeries(string charName, string series) {
	/declare i int local 0
	/declare ikey string local

	/for i 1 to ${Math.Calc[${series.Count[,]}+1]}
		/varset ikey ${series.Token[${i},,]}

		/if (${ikey.Left[1].Equal[@]}) {
			/call characterPassesAllowKey ${charName} ${ikey}
			/if (${Macro.Return}==TRUE) /return TRUE
		} else {
			/if (${charName.Upper.Equal[${ikey.Upper}]}) /return TRUE
		}
	/next i

	/return FALSE
}

SUB gatherNetbotNames {
	/declare i int local 
	/declare netbotCount int local ${Math.Calc[${NetBots.Counts}]}
	/declare netbotName string local
	/declare ret string local

	/for i 1 to ${netbotCount}
		/if (${ret.Length}==0) {
			/varset ret ${NetBots.Client[${i}]}
		} else {
			/varset ret ${ret},${NetBots.Client[${i}]}
		}
	/next i

	/varset allNetbotNames ${ret}

	/RETURN
}

SUB validateTagsFromArray(string arrName) {
	/echo validating tags for ${arrName}

	/declare i int local 

	/for i 1 to 64
		/if (${${arrName}[${i},${bNotIfAssistTag}].Equal[-]} && ${${arrName}[${i},${bRequireAssistTag}].Equal[-]}) /continue

		/if (${${arrName}[${i},${bNotIfAssistTag}].NotEqual[-]}) {
			/if (${assistTags.Count}>0 || ${persistentAssistTags.Count}>0) {
				/echo bNotIfAssistTag: ${${arrName}[${i},${bNotIfAssistTag}]} TRUE
				/call validateAssistTags ${${arrName}[${i},${bNotIfAssistTag}]} TRUE
				/if (${Macro.Return}==TRUE) {
					/echo invalidating: ${${arrName}[${i},1]}
					/varset ${arrName}[${i},${bTagsInvalidated}] YES
				} else {
					/varset ${arrName}[${i},${bTagsInvalidated}] -
				}
			}
		}
		/if (${${arrName}[${i},${bRequireAssistTag}].NotEqual[-]}) {
			/if (${assistTags.Count}==0 && ${persistentAssistTags.Count}==0) /continue
			/ECHO bRequireAssistTag ${${arrName}[${i},${bRequireAssistTag}]}
			/call validateAssistTags ${${arrName}[${i},${bRequireAssistTag}]}
			/if (${Macro.Return}==FALSE) {
				/echo invalidating: ${${arrName}[${i},1]}
				/varset ${arrName}[${i},${bTagsInvalidated}] YES
			} else {
				/varset ${arrName}[${i},${bTagsInvalidated}] -
			}
		}

	/next i

	/RETURN
}

SUB validateSpellBasedArray(string arrName) {
	/declare i int local 

	/for i 1 to 64
		/if (${${arrName}[${i},1].Equal[-]}) /break

		/if (${isRealSpell[${${arrName}[${i},${bName}]}]}==FALSE) {
			/echo Error: >>>${${arrName}[${i},1]}<<< does not appear to be a valid spell/item/abilty in array [${arrName}]. Check spelling? Non-existant item/clicky? Disabling this spell entry until macro restart.
			/varset ${arrName}[${i},${bEnabled}] NO
		}

	/next i
	/RETURN
}

SUB isRealSpell(string request) {
	/declare castType string local
	/call getCastTypeByCastRequest "${request}"
	/varset castType ${Macro.Return}
	/if (${castType.Equal[UNKNOWN]}) /return FALSE
	/return TRUE
}

SUB memorizeSpellInGem(string spellName, int gemNumber) {
	/if (${Me.Casting.ID}!=NULL) {
		/echo interrupt.
		/interrupt
		/delay 5
	}

	/memo "${spellName}" ${gemNumber}

	/delay 3

	/while (${Window[SpellBookWnd]}==TRUE) {
		/delay 3
		/if (${Gem[${spellName}]}!=NULL) /break
	}

	/delay 35 ${Me.SpellReady[${spellName}]}

	/RETURN
}

SUB isNetBotAttackingID(int targetID) {
	/declare netbotCount int local ${NetBots.Counts}
	/declare i int local
	/declare netbotName string local


	/for i 1 to ${netbotCount}
		/varset netbotName ${NetBots.Client[${i}]}	
		/if (${NetBots[${netbotName}].Attacking}==TRUE && ${NetBots[${netbotName}].TargetID}==${targetID}) {
			/return TRUE
		}
	/next i

	/RETURN FALSE
}

SUB processCommandLine() {
	/if (${commandLine[${cmdDEBUG}].NotEqual[-]}) /varset debugMode TRUE

	/if (${commandLine[${cmdINI}].Length}>0) /varset commandLine[${cmdINI}] -${commandLine[${cmdINI}]}

	/if (${commandLine[${cmdDIRECTINI}].Length}>0) {
		/varset botIniFile p3_bots\${commandLine[${cmdDIRECTINI}]}.ini
	} else {
		/varset botIniFile p3_bots\${MacroQuest.Server}-${Me.CleanName}${commandLine[${cmdINI}]}.ini
	}

	/RETURN
}

SUB getDefaultBCSChannels() {
	/declare ret list local
	/declare retstr string local
	/declare i int local

	/invoke ${ret.Append[${Me.Class.Name.Lower}s]}
	/if (${meleeClasses.Find[${Me.Class.ShortName}]}!=NULL) {
		/invoke ${ret.Append[melee]}
	}
	/if (${tankClasses.Find[${Me.Class.ShortName}]}!=NULL) {
		/invoke ${ret.Append[tanks]}
	}
	/if (${rangedClasses.Find[${Me.Class.ShortName}]}!=NULL) {
		/invoke ${ret.Append[ranged]}
	}
	/if (${priestClasses.Find[${Me.Class.ShortName}]}!=NULL) {
		/invoke ${ret.Append[healers]}
	}

	/for i 0 to ${Math.Calc[${ret.Count}-1]}
		/varset retstr ${ret.Item[${i}]} ${retstr}
	/next i

	/RETURN ${retstr.Left[-1]}
}

SUB fullTarget(int targetID) {
	
	/while (${Target.BuffsPopulated}==0) {
		/if (${Spawn[id ${targetID}].ID}==NULL || ${Spawn[id ${targetID}].Dead}==TRUE) /RETURN FALSE
		/if (${Target.ID}!=${targetID}) /tar id ${targetID}
		/delay 1
	}

	/RETURN
}


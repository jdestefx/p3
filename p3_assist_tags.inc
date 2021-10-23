SUB p3_assist_tags_init {
	/declare assistTags list outer
	/declare tempCombatTags list outer

	/if (${commandLine[${cmdPAST}].Length}>0) {
		|/bc Adding PAST tags from startup: ${commandLine[${cmdPAST}].Replace[ ,]}
		/invoke ${assistTags.Append[${commandLine[${cmdPAST}].Replace[ ,]}]}
	}

	/RETURN
}



SUB assist_tagsMaintenance {
	/RETURN
}


SUB validateAssistTags(string tagsToCheck, bool asOrOperator) {
	/declare i int local
	/declare t string local 
	/declare matches int local 0
	/declare res bool local TRUE
	/declare checkTags list local
	/declare fullTags list local

	/if (${assistTags.Count}>0) {
		/vardata fullTags assistTags.Splice
	}

   /if (${tempCombatTags.Count}>0) {
      /for i 0 to ${Math.Calc[ ${tempCombatTags.Count}-1 ]}
         /invoke ${fullTags.Append[${tempCombatTags.Item[${i}]}]}
      /next i
   }

	/invoke ${checkTags.Append[${tagsToCheck}]}

	/for i 0 to ${Math.Calc[${checkTags.Count}-1]}
		/if (${fullTags.Contains[${checkTags.Item[${i}]}]}) {
			/varset matches ${Math.Calc[${matches}+1]}
		}
	/next i

	/if (${asOrOperator}==TRUE && ${matches}>0) {
		/varset res TRUE
	} else /if (${matches}>0 && ${matches} >= ${checkTags.Count}) {
		/varset res TRUE
	} else {
		/varset res FALSE
	}

	/return ${res}

	/RETURN
}



for ACTION in "000" "001" "010" "011" "100" "101" "110" "111"
do
	../../../../../espresso-logic-master/bin/espresso -Dexact "woods2q_a"$ACTION"_complete.pla" > "woods2q_a"$ACTION"_complete_minimized.pla" & 
done


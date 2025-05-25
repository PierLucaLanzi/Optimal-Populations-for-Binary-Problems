for DIRECTORY in woods1 woods2 maze4 maze5 maze6 woods14
do
	cd $DIRECTORY
	echo "PROCESSING Q-TABLE FOR $DIRECTORY"
	../../xcslib-1.3/executables/xcs-woods -f "$DIRECTORY"v
	echo "... DONE"
	cd ..
done
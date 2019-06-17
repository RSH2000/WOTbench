# This script runs multiple WoTbench tests by reading and parsing test parameters form the input csv file  
#!/usr/bin/env bash
TestParamFile=$1
OutputFilePrefix="./result_summaries/Out_"
OutputFile="$OutputFilePrefix$TestParamFile"
NumberofTests=$(wc -l < $TestParamFile)
#sed -n '1p' < $TestParamFile | awk '{split($0,header,",")}'
header=$(sed -n '1p' < $TestParamFile | tr '\r' ',')
NumParam=$(grep -o "," <<< "$header" | wc -l)
#echo ${header[1]}
outheader=$(echo $header | tr -d '\r')
outheader="${outheader},Response time,R-S,Send Lag,Send Delay,Expected ait,submitted ait,Probe R,CD R-S,Num of samples,Ave Proc/sec,Avg ctsw per sec,CPU Avg (all),CPU Max (all),CPU Avg (0),CPU Max (0),CPU Avg (1),CPU Max (1),CPU Avg (2),CPU Max (2),CPU Avg (3),CPU Max (3),CPU Avg (4),CPU Max (4),CPU Avg (5),CPU Max (5),CPU Avg (6),CPU Max (6),CPU Avg (7),CPU Max (7),"
for (( i=2; i<=$NumberofTests; i++ ))
do
	args=" "
	#sed -n "${i}p" < $TestParamFile | awk '{split($0,currenttest,",")}'
	currenttest=$(sed -n "${i}p" < $TestParamFile | tr '\r' ',' )
	#echo $NumParam
	namepos=""
	for (( j=$NumParam; j>=1; j-- ))
	do
		field=$(echo $currenttest | tr '\r' ','| cut -d ',' -f $j | tr -d ' ' )
		h=$(echo $header | cut -d ',' -f $j )
		#echo $j $h " = " $field
		if [ ! -z "$field" ] ; then
			args="${args} --${h} ${field}" 
			#echo $args
			if [ $j -ne 1 ]; then 
			namepos="${namepos}_${field}"
			else
			S="O${field}"
			fi
		fi
	done
	args="${args}${namepos}"
	echo ">>> Running next test with arguments: $args"
	rm tmp.log
	./WoTHarness.sh $args >> tmp.log 2>&1
	data=$(cat tmp.log | tail -n 1)
	out=$(echo $currenttest | tr -d '\r')
	#data="fg,gdf,dgdfg,"
	out="${out},${data}"
	echo $out
	if [ $i -eq 2 ]; then
		echo $outheader  > 	$OutputFile
	fi
	echo $out >> $OutputFile
	sleep 5
done
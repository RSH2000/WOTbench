# This script is used to Emulate the network behaviour. It runs the Pumba Docker network emulators based on netem.
#!/usr/bin/env bash
###
#	@param:	Mode	--> 0:None	1:Delay		2:Loss	3:Both
#	@param:	DelayFile	--> Name of the file that contains delays setup information							
#	@param:	LossFile	--> Name of the File that contains loss information
#	@param: NumSrvNode --> Number of Server Nodes
#	@param:	Duration	--> Duration of Network Emulation

#	Functionality: Setting up network Envirnment
#	Output:  none

###################### Delay File Template	######################
#### Mode: 			Constant			MultiHop		UserDefined							Random
#### Value:			const. value (ms)	per hop value	list of values (one per server)		mean value
#### Param: 		N/A					# of Hop levels	N/A									Distribution (normal/exponensial)
#### Distribution:	delay distr	delay distr		delay distr							delay distr

###################### Loss File Template	######################
####	Option1:	random PERCENT [ CORRELATION ]  
####	Option2:	state p13 [ p31 [ p32 [ p23 [ p14]]]] 
####	Option3: 	gemodel p [ r [ 1-h [ 1-k ]]]   


########################## Functions #############################
Parse_DelayFile()
{
DelayFile=$1

for (( i=1; i<=4; i++ ))
do
	line=$(cat $DelayFile | head -n $i | tail -n 1)
	header=$(echo $line | cut -d ':' -f 1 )
	case "$i" in
		1)
		if [ $header != "Mode" ]; then
		echo "Error: Corrupted Delay file in Mode line, Exiting Network Emulator"
		exit 0
		fi
		echo $header
		DMode=$(echo $line | cut -d ':' -f 2)
		echo $DMode
		;;
		2)
		if [ $header != "Value" ]; then
		echo "Error: Corrupted Delay file in Value line, Exiting Network Emulator"
		exit 0
		fi
		echo $header
		Value=$(echo $line | cut -d ':' -f 2)
		echo $Value
		;;
		3)
		if [ $header != "Param" ]; then
		echo "Error: Corrupted Delay file in Param line, Exiting Network Emulator"
		exit 0
		fi
		echo $header
		Param=$(echo $line | cut -d ':' -f 2)
		echo $Param
		;;
		4)
		if [ $header != "Distribution" ]; then
		echo "Error: Corrupted Delay file in Distribution line, Exiting Network Emulator"
		exit 0
		fi
		echo $header
		Distribution=$(echo $line | cut -d ':' -f 2)
		echo $Distribution
		;;
	esac
done
}

Set_Delay()
{
	ServerNum=$1
	DelayFile=$2
	ServerInfoFile=$3
	echo Setting up the delay for emulated network for $ServerNum instanses
	Parse_DelayFile	$DelayFile
	for (( i=1; i<=$ServerNum; i++ ))
	do
		let j=i+1
		out=$(head -n $j $ServerInfoFile | tail -1)
		insname=$(echo "$out" | awk '{split($0,a,","); print a[4]}')
		ip=$(echo "$out" | awk '{split($0,a,","); print a[3]}')
		echo Setting up delay for server: $insname with ip adress $ip
		case "$DMode" in
			Constant)
			echo "$DMode --> $i: Delay=$Value"
			pumba netem --duration ${Duration}m delay --time $Value --jitter 1 --distribution $Distribution $insname&
			;;
			MultiHop)
			p=$(($i * $Param))
			d=$(($Value * $p))
			echo "$i: Delay=$d"
			pumba netem --duration ${Duration}s delay --time $d --distribution $Distribution $insname&
			
			;;
			UserDefined)
			;;
			Random)
			;;
			*)
			echo Unknown Delay Mode
			;;
		esac
	done
			
}
Set_Loss()
{
	ServerNum=$1
	LossFile=$2
	params=$(cat $LossFile | head -n 1)	
	echo $params
	for (( i=1; i<=$ServerNum; i++ ))
	do
		let j=i+1
		out=$(head -n $j $ServerInfoFile | tail -1)
		insname=$(echo "$out" | awk '{split($0,a,","); print a[4]}')
		ip=$(echo "$out" | awk '{split($0,a,","); print a[3]}')
		echo Setting up loss for server: $insname with ip adress $ip
		pumba netem --duration ${Duration}s loss $params $insname&
	done
}


#####################Starting main Script ######################## 
while [[ $# > 1 ]]
do 
key="$1"
case $key in
    -m|--Mode)
    Mode="$2"
    shift 
    ;;
	-D|--DelayFile)
    DelayFile="$2"
    shift 
    ;;
    -L|--LossFile)
    LossFile="$2"
    shift 
    ;;
	-n|--NumSrvNode)
    NumSrvNode="$2"
    shift 
    ;;
    -d|--Duration)
    Duration="$2"
    shift  
    ;;
	-I|--ServerInfoFile)
    ServerInfoFile="$2"
    shift  
    ;;
    --default)
    DEFAULT=YES
    ;;
    *)
            # unknown option
    ;;
esac
shift # past argument or value
done

#### Setting up default values ######
DMode="Constant"
Value="100"
Param="N/A"
Distribution="normal"


echo ">>> Setting Up Emulated Network"
case "$Mode" in
	0)
	echo "No network emulation is requested ..."
	;;
	1)
	echo "Emulating network in Delay mode"
	Set_Delay $NumSrvNode $DelayFile $ServerInfoFile
	;;
	2)
	echo "Emulating network in Loss mode"
	Set_Loss $NumSrvNode $LossFile $ServerInfoFile
	;;
	3)
	echo "Emulating network in Delay and Loss modes"
	Set_Delay  $NumSrvNode $DelayFile $ServerInfoFile
	Set_Loss  $NumSrvNode $LossFile $ServerInfoFile
	;;
	*)
    echo Unkown network emulation mode        
    ;;
esac



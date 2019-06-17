# This script runs a "single" emulation scenario in WoTbench testbed.
#!/bin/bash

# Parsing Input Parameters
#	@param:	TestName	--> Uniqe Test identifier phrase (No Space)
#	@param:	ResultPath	--> Relative or absolut path to save result folder  -- default ="../results/"
#### Server Node Settings ####
#	@param: NumSrvNode --> Number of New Server Nodes
#	@param:	ThreadNum	--> Number of threads per server
#	@param: ServerInfoFile --> if killall=0 ->Container information file from previous test else info file prefix
#	@param: Killall --> Kill exising server nodes 1=Yes, 0=No -- default=1
#### Network Settings #####
#	@param: NetworkName --> Name of the container network
#	@param: NetworkIPv --> IP Version
#	@param: NetworkMode --> Network Emulation Mode 0=None 1=Delay 2=PacketLoss 3=Both  --default=0
#	@param: DelaySettingFile --> File that Contains Network Delay Setting	-- default=DelaySettings
#	@param: LossSettingFile --> File that Contains Network Loss Setting	-- default=LossSettings
#### Workload Settings ####
#	@param: WLFileName --> Prefix Name for the Workload File --- default=WL
#	@param: ResFileName --> Prefix Name for Resource Files --- default=Res
#	@param: StatFileName --> Prefix Name for Output Statistic File --- default=Stat
#	@param: UtilFileName --> Prefix Name for Output Resource Utilization File --- default=Util
#	@param: WLIAT --> Request Inter Arrival Time distribution,mean  e.g. e,50
#	@param: WLST --> Resource Service Time distribution,mean  e.g. e,50
#	@param:	WLBurstCorr	--> Burst Correction Factor
#	@param: Duration --> Duration of the Test
#	@param:	WLClientNum	--> Number of Client Instances	--- default=1


### Setting Up Default Parameters ###
ResultPath="../results/"
WLFileName="WL"
ResFileName="Res"
StatFileName="Stat"
UtilFileName="Util"
Killall=1
WLClientNum=1
NetworkMode=0
DelaySettingFile="DelaySettings"
LossSettingFile="LossSettings"
BusyCycle=1
CDServiceTime=10000
ServerAffinity=1
CDAffinity=0

#	Usage: CDBHarness.sh 
#		or
#	       CDBHarness.sh

#	Output: Replaces file.xml
while [[ $# > 1 ]]
do 
key="$1"
case $key in
	-sa|--ServerAffinity)
    ServerAffinity="$2"
    shift 
    ;;
	-pa|--CDAffinity)
    CDAffinity="$2"
    shift 
    ;;
    -tn|--TestName)
    TestName="$2"
    shift 
    ;;
	-rp|--ResultPath)
    ResultPath="$2"
    shift 
    ;;
    -sn|--NumSrvNode)
    NumSrvNode="$2"
    shift 
    ;;
	-tn|--ThreadNum)
    ThreadNum="$2"
    shift 
    ;;
    -SIF|--ServerInfoFile)
    ServerInfoFile="$2"
    shift 
    ;;
    -k|--Killall)
    Killall="$2"
    shift 
    ;;
    -nn|--NetworkName)
    NetworkName="$2"
    shift 
    ;;
    -nip|--NetworkIPv)
    NetworkIPv="$2"
    shift 
    ;;
    -nm|--NetworkMode)
    NetworkMode="$2"
    shift 
    ;;
	-bc|--BusyCycle)
    BusyCycle="$2"
    shift 
    ;;
    -ndf|--DelaySettingFile)
    DelaySettingFile="$2"
    shift 
    ;;
	-nlf|--LossSettingFile)
    LossSettingFile="$2"
    shift 
    ;;
	-wf|--WLFileName)
    WLFileName="$2"
    shift 
    ;;
    -rf|--ResFileName)
    ResFileName="$2"
    shift 
    ;;
	-ps|--CDServiceTime)
    CDServiceTime="$2"
    shift 
    ;;
	-pi|--CDIAT)
    CDIAT="$2"
    shift 
    ;;
    -sf|--StatFileName)
    StatFileName="$2"
    shift 
    ;;
    -uf|--UtilFileName)
    StatFileName="$2"
    shift 
    ;;
    -iat|--WLIAT)
    WLIAT="$2"
    shift 
    ;;
	-st|--WLST)
    WLST="$2"
    shift
    ;;
	-std|--STDIST)
    STDIST="$2"
    shift 	
    ;;
	-bc|--WLBurstCorr)
    WLBurstCorr="$2"
    shift 
    ;;
	-d|--Duration)
    Duration="$2"
    shift 
    ;;
	-cps|--CPUShare)
    cpushare="$2"
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


echo " >>> Starting test $TestName <<<"
mkdir $ResultPath$TestName
#./InitSystem.sh
ServerIPFile=$(echo "$ServerInfoFile.ip")
ServerInfoFile=$(echo "$ServerInfoFile.csv")

### Stop and remove previous instances ####
if [ $Killall -eq 1 ]; then 
	#docker stop $(docker ps -a -q)
	docker rm -f $(docker ps -a -q) 
	#sudo systemctl daemon-reload
	docker network rm $(docker network ls -q)
	#sudo systemctl restart docker
	sudo killall pumba
	docker network rm 1$TestName$NetworkName
	docker network create --driver bridge 1$TestName$NetworkName
	docker network rm 2$TestName$NetworkName
	#docker network create --ipv6 --driver bridge 2$TestName$NetworkName
	sleep 3
	echo ">>> Nework created <<<"
	mkdir Cids
	rm ./Cids/*.cid
	rm $ServerInfoFile
	rm $ServerIPFile
	rm ./$TestName*
	rm ../$TestName*
	rm ./*$ResFileName*
	rm ../*$ResFileName*
	current_dir=$(pwd)
	echo "Current dir: $current_dir"
	printf "Index,Cid,IP\n" >> $TestName$ServerInfoFile
	existing=0
else
	cp $ServerInfoFile $TestName$ServerInfoFile
	cp $ServerIPFile $TestName$ServerIPFile
	existing=$(docker ps | grep ago -c)
fi

sleep 1

###############  Initializing docker Client ###############  
echo ">>> Initiating a Client Instance ..."
docker  -D  --log-level=info  run -i -t --cpuset-cpus="1" --cpuset-mems="1" --net=1$TestName$NetworkName --ulimit nofile=262144:262144 --name=CLT$TestName -v `pwd`/..:`pwd`/.. -w `pwd`/.. -d --cidfile ./Cids/CLCID.cid rhashem/cdbclient
#docker network connect 2$TestName$NetworkName CLT$TestName
clcid=$(cat ./Cids/CLCID.cid)
echo "The client's CID is $clcid ..."
###############  Running the Server Dockers ###############  
echo ">>> Initiating $NumSrvNode docker instances of CoAP server ...."
let core=3
for (( i=1; i <= $NumSrvNode; i++ ))
do
	let existing=existing+1
	printf  "$existing," >> $TestName$ServerInfoFile
	echo ">>> Initiating instance number $i:"
	if [ $ServerAffinity -eq 0 ]; then
	m=$((i%2))
	#if [ $m -eq 0 ]; then
	#cpushare=10
	#else
	#cpushare=50
	#fi

	docker  -D  --log-level=info  run -i -t --cpu-shares=$cpushare --cpuset-cpus=3-7 --net=1$TestName$NetworkName --ulimit nofile=262144:262144 --name $TestName$i -v `pwd`/..:`pwd`/.. -w `pwd`/..  -d --cidfile ./Cids/SV$i.cid rhashem/cdbserver
	else
	docker  -D  --log-level=info  run -i -t --cpuset-cpus=$core --net=1$TestName$NetworkName --ulimit nofile=262144:262144 --name $TestName$i -v `pwd`/..:`pwd`/.. -w `pwd`/..  -d --cidfile ./Cids/SV$i.cid rhashem/cdbserver
	fi
	let core=$core+1
	if [ $core -eq 8 ]; then
		let core=3
	fi

	### Saving specifications of the current container ### 
	cid=$(cat ./Cids/SV$i.cid)
	printf  "$cid," >> $TestName$ServerInfoFile

	###IPVsix=$(docker inspect --format "'{{ .NetworkSettings.Networks.$TestName$NetworkName.GlobalIPv6Address }}'" $cid)
#### Saving the IP in the server IP file

	if [ $NetworkIPv -eq 6 ]; then
		IPVsix=$(docker exec -i -t $TestName$i ip -6 addr  show dev eth0 | sed -n 2p | tr '/' ' ' | awk -F ' ' '{print $2}')
		echo "Global IP v6 is: $IPVsix"
		printf  "$IPVsix,$TestName$i\n" >> $TestName$ServerInfoFile
		#IPSRV=$(echo "$IPVsix" | awk "sub(/[']/, x)")
		#IPSRV=$(echo "$IPSRV" | awk "sub(/[']/, x)")
		printf  "[$IPVsix]\n" >> $TestName$ServerIPFile
	else
		IPVfour=$(docker exec -i -t $TestName$i ifconfig | awk '/inet addr/{print substr($2,6)}' | sed -n 1P)
		echo "Global IP v4 is: $IPVfour"
		printf  "$IPVfour,$TestName$i\n" >> $TestName$ServerInfoFile
		printf  "$IPVfour\n" >> $TestName$ServerIPFile
	fi 
#printf  "[$IPVsix]\n" >> $TestName$ServerIPFile
done
N=$(docker ps | grep -c ago)
echo "We initialized $N Server Instances "
###############  Apply network Properties ###############  
NetDur=$(echo $((Duration+20)))
if [ $NetworkMode -ne 0 ]; then 
	echo ">>> Preparing network environment ..."
	echo "--Mode $NetworkMode --DelayFile $DelaySettingFile --LossFile $LossSettingFile -n $NumSrvNode -d $NetDur -I $TestName$ServerInfoFile"
	./SetNetwork.sh --Mode $NetworkMode --DelayFile $DelaySettingFile --LossFile $LossSettingFile -n $NumSrvNode -d $NetDur -I $TestName$ServerInfoFile
	sleep 1
fi
############### Generating workload ###############  
echo ">>> Generating workload ...."
cp $TestName$ServerIPFile ../WLGen/
echo " -b $STDIST -d $Duration -O $TestName$WLFileName -R $TestName$ResFileName -S $TestName$ServerIPFile -s $WLST -t $WLIAT -c $WLClientNum"
../WLGen/CDB-WLG -b $STDIST -d $Duration -O $TestName$WLFileName -R $TestName$ResFileName -S $TestName$ServerIPFile -s $WLST -t $WLIAT -c $WLClientNum
SrvDur=$(echo $((Duration+10)))
sleep 1

####### SETTING UP THE CD ########
echo ">>> Setting Up the Guard Server/Client"
if [ $CDAffinity -eq 0 ]; then
docker -D  --log-level=info  run -i -t --net=1$TestName$NetworkName --ulimit nofile=262144:262144 --name CD$TestName -v `pwd`/..:`pwd`/.. -w `pwd`/..  -d --cidfile ./Cids/CD.cid rhashem/cdbserver
else
docker  -D  --log-level=info  run -i -t --cpuset-cpus="2" --net=1$TestName$NetworkName --ulimit nofile=262144:262144 --name CD$TestName -v `pwd`/..:`pwd`/.. -w `pwd`/..  -d --cidfile ./Cids/CD.cid rhashem/cdbserver
fi
CD_IP=$(docker exec -i -t CD$TestName ifconfig | awk '/inet addr/{print substr($2,6)}' | sed -n 1P)
echo $CD_IP > $TestName-Ip-CD
echo "Generating Guard workload ..."
../WLGen/CDB-WLG -b d -d $Duration -O $TestName$WLFileName-CD -R $TestName$ResFileName-CD -S $TestName-Ip-CD -s $CDServiceTime -t $CDIAT -c 1
docker  -D  --log-level=info  run -i -t --cpuset-cpus="2" --net=1$TestName$NetworkName --ulimit nofile=262144:262144 --name=CDCLT$TestName -v `pwd`/..:`pwd`/.. -w `pwd`/.. -d --cidfile ./Cids/CDCLCID.cid rhashem/cdbclient
$(docker exec -t CD$TestName  /root/CoAP/Server/CDB-server -B $BusyCycle -T 1 -R $current_dir/$TestName$ResFileName-CD0 -D $SrvDur -O $current_dir/CDOut$TestName.csv> $current_dir/CDLog$TestName.csv &)
###########  Running Servers with specific resources ################
echo ">>> Running CBserver ..."
for (( i=1; i <= $NumSrvNode; i++ ))
	do
		let j=i-1
		echo ">>> Running CBServer on instance number $i with command:"
		$(docker exec -t $TestName$i  /root/CoAP/Server/CDB-server -B $BusyCycle -T $ThreadNum -R $current_dir/$TestName$ResFileName$j -D $SrvDur -O $current_dir/Serverlog$TestName$j.csv> $current_dir/Serverout$TestName$j.csv &)
	done
echo ">>> Starting collectl ..." 
./UtilCollector.sh $ResultPath$TestName/$TestName$UtilFileName.csv $ResultPath$TestName/Sar$TestName 
sleep 5
echo ">>> Submitting the Workload ..." 
echo "Stat file: $TestName$StatFileName.csv"
echo "Workload file: $TestName$WLFileName"
echo "Utilization file: $TestName$UtilFileName"
$(docker exec -t CDCLT$TestName   /root/CoAP/Client/CDB-client -c $WLBurstCorr -W $current_dir/$TestName$WLFileName-CD -O $current_dir/$TestName$StatFileName-CD.csv> $current_dir/CDCliLog$TestName.csv &)
docker exec -i -t CLT$TestName   /root/CoAP/Client/CDB-client -c $WLBurstCorr -W $current_dir/$TestName$WLFileName -O $current_dir/$TestName$StatFileName.csv
echo "Waiting for the Guard client to finish ... "
sleep 5
sudo killall collectl
sudo killall sar
###################### Collect Statistics #####################
echo ">>> Saving Statistics ..." 
./UtilGraphGenerator.sh $ResultPath$TestName/ $TestName$UtilFileName.csv 8 jpeg $Duration
utilsum=$(cat USummary-$TestName$UtilFileName.csv)
sadf -t -d $ResultPath$TestName/Sar$TestName  -- -n DEV >  $ResultPath$TestName/SarDEV$TestName.csv 
sadf -t -d $ResultPath$TestName/Sar$TestName  -- -w > $ResultPath$TestName/SarCS$TestName.csv
sed -i -e 's/;/,/g' $ResultPath$TestName/SarCS$TestName.csv
sed -i -e 's/;/,/g' $ResultPath$TestName/SarDEV$TestName.csv
cp ../*$WLFileName*  $ResultPath$TestName
sleep 10
##### Removing files ###############
echo ">>> House keeping ..." 
rm ./Cids/*.cid
mv  $TestName$ServerInfoFile $ResultPath$TestName
mv ./$TestName$ServerIPFile $ResultPath$TestName
mv  ./$TestName$WLFileName $ResultPath$TestName
##rm ../*$WLFileName*
mv ./$TestName$ResFileName* $ResultPath$TestName
###rm ../*$ResFileName*
mv ./$TestName* $ResultPath$TestName
mv ./USummary-$TestName$UtilFileName.csv $ResultPath$TestName
mv ./$TestName$StatFileName.csv $ResultPath$TestName
mv ./Serverout$TestName*.csv $ResultPath$TestName
mv ./Serverlog$TestName*.csv $ResultPath$TestName
mv ./CDOut$TestName.csv $ResultPath$TestName
mv ./CDLog$TestName.csv $ResultPath$TestName
mv ./CDCliLog$TestName.csv $ResultPath$TestName
mv ./$TestName$StatFileName*.csv $ResultPath$TestName
mv ./$TestName.csv $ResultPath$TestName

sudo killall pumba
sudo killall sar
sudo killall collectl

if [ $Killall -eq 1 ]; then 
docker rm -f $(docker ps -a -q) 
docker network rm 1$TestName$NetworkName
fi
echo ">>> Done with test $TestName  " 
out=$(cat $ResultPath$TestName/$TestName$StatFileName.csv | awk 'NR > 4 { print }' | awk -F',' '{sum7+=$7;sum9+=$9;sum8+=$8;sum10+=$10;sum11+=$11;sum12+=$12; ++n} END { print sum7/n","sum9/n","sum8/n","sum10/n","sum11/n","sum12/n"," }' | tr -d '\r')
outCD=$(cat $ResultPath$TestName/$TestName$StatFileName-CD.csv | awk 'NR > 4 { print }' | awk -F',' '{sum7+=$7;sum9+=$9; ++n}  $9>200 {count+=1} END { print sum7/n","sum9/n","count"," }' | tr -d '\r')
outcs=$(cat $ResultPath$TestName/SarCS$TestName.csv | awk 'NR > 6 { print }' | awk -F',' '{sum4+=$4;sum5+=$5; ++n} END { print sum4/n","sum5/n"," }' | tr -d '\r')
out="${out}${outCD}${outcs}${utilsum},"
for (( i=0; i < $NumSrvNode; i++ ))
	do
	srvout=$(cat $ResultPath$TestName/Serverout$TestName$i.csv | awk 'NR > 4 { print }' | awk  '{sum+=$1;++n} END { print sum/n"," }' | tr -d '\r')
	out="${out}${srvout}"
	done
echo $out
cp ./tmp.log $ResultPath$TestName
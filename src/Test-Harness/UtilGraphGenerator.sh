# The Purpose of This file is to generate CPU Utilization Graphs and Profiling 
#!/usr/bin/env bash
#############################################################
resultpath=$1
utilfilename=$2
cpunum=$3
t=$4
dur=$5
tempfile="./tmp/temp.txt"
let dur=dur+5
rm -r Plots
rm -r tmp
prevpath=$(pwd)
cd $resultpath
mkdir tmp
echo " moving back to $prevpath"
cd $prevpath
datfilename=$(echo "$utilfilename" | cut -d. --fields=1)
#echo "dat file name is $datfilename"
# calling the C program to calculate The Averages
#echo "the path is $resultpath"
./aggregator $resultpath $datfilename 5 $dur $cpunum >> USummary-$utilfilename
cd $resultpath
######################################Plotting#####################################
 tmpFile="gnuplot_input"
  rm -f $tmpFile
  #################################
  ## Graphs for Idle Time servers ##
  #################################

  # Plot CPU idle time
  echo Generating servers CPU time graph of type $t
  echo "set terminal "$t > $tmpFile;
  u=$(echo "./tmp/Total$datfilename.$t")
  echo set output '"'$u'"' >> $tmpFile;
  echo 'set title "'Total Utilization'"' >> $tmpFile;
  echo 'set xlabel "Time in seconds"' >> $tmpFile;
  echo 'set ylabel "Processor time in %"' >> $tmpFile;
  echo "set yrange [0:800]" >> $tmpFile;
  echo plot '"'./tmp/Total_Idle$datfilename.dat'"' title '"'Idle'"' with lines, '"'./tmp/Total_User$datfilename.dat'"' title '"'User'"' with lines, '"'./tmp/Total_Sys$datfilename.dat'"' title '"'System'"' with lines >> $tmpFile;
  gnuplot $tmpFile

####################################################################################
z=0
y=3
############################# Opening The filenames File ##############
for ((  i = 0 ;  i < cpunum;  i++  ))
do
cut -d, -f$y $utilfilename > ./tmp/CPU"$z"_User$datfilename.dat
sed '1d' ./tmp/CPU"$z"_User$datfilename.dat > ./tmp/tempdat.dat
sed '1d' ./tmp/tempdat.dat > ./tmp/NCPU"$z"_User$datfilename.dat

let y+=1
cut -d, -f$y $utilfilename > ./tmp/CPU"$z"_Sys$datfilename.dat
sed '1d' ./tmp/CPU"$z"_Sys$datfilename.dat > ./tmp/tempdat.dat
sed '1d' ./tmp/tempdat.dat > ./tmp/NCPU"$z"_Sys$datfilename.dat

let y+=1
cut -d, -f$y $utilfilename > ./tmp/CPU"$z"_Idle$datfilename.dat
sed '1d' ./tmp/CPU"$z"_Idle$datfilename.dat > ./tmp/tempdat.dat
sed '1d' ./tmp/tempdat.dat > ./tmp/NCPU"$z"_Idle$datfilename.dat

let y+=1

######################################Plotting#####################################
 tmpFile="gnuplot_input"
  rm -f $tmpFile
  #################################
  ## Graphs for Idle Time servers ##
  #################################

  # Plot CPU idle time
  echo Generating servers CPU time graph of type $t
  echo "set terminal "$t > $tmpFile;
  u=$(echo "./tmp/Core$z$datfilename.$t")
  echo set output '"'$u'"' >> $tmpFile;
  echo 'set title "'Core $z Utilization'"' >> $tmpFile;
  echo 'set xlabel "Time in seconds"' >> $tmpFile;
  echo 'set ylabel "Processor time in %"' >> $tmpFile;
  echo "set yrange [0:100]" >> $tmpFile;
  echo plot '"'./tmp/NCPU"$z"_Idle$datfilename.dat'"' title '"'Idle'"' with lines, '"'./tmp/NCPU"$z"_User$datfilename.dat'"' title '"'User'"' with lines, '"'./tmp/NCPU"$z"_Sys$datfilename.dat'"' title '"'System'"' with lines >> $tmpFile;
  gnuplot $tmpFile

####################################################################################
let z+=1
done


mkdir Plots
mv ./tmp/*.$t ./Plots
rm -r ./tmp

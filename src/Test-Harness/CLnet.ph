sub CLnetInit
{
  error("-s not allowed with 'rsh'")          if $userSubsys ne '';
  error("-f requires either --rawtoo or -P")     if $filename ne '' && !$rawtooFlag && !$plotFlag;
  error("-P or --rawtoo require -f")             if $filename eq '' && ($rawtooFlag || $plotFlag);
  $subsys=$userSubsys='cm';
}

sub CLnet
{
  my $line;
  if (($headerRepeat==0 && !$headersPrinted) || $headerRepeat==1 || ($headerRepeat>0 && $totalCounter ==1))
  {
    $line.="Sample,DateTime,";

        $line.=sprintf("cpu %d user,cpu %d system, cpu %d idle\n",$i,$i,$i);

    $headersPrinted=1;
  }
  $datetime='';
    ($ss, $mm, $hh, $mday, $mon, $year)=localtime($lastSecs);
    $datetime=sprintf("%02d:%02d:%02d", $hh, $mm, $ss);                
    $datetime=sprintf("%04d%02d%02d %s", $year+1900, $mon+1, $mday, $datetime);    
  $line.=sprintf("%s,%s",$totalCounter, $datetime);
	   my $rcv=$userP[$i]+$niceP[$i];
          my $snd=$sysP[$i]+$irqP[$i]+$softP[$i]+$stealP[$i];
        $line.=sprintf(",%s,%s,%s",$rcv,$snd,$idleP[$i]);
  $line.=sprintf("\n");	
  printText($line);
}
1;
 
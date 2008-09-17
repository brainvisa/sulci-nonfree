#!/pub_dom/perl/bin/perl

if( $#ARGV < 4 )
  {
    print $#ARGV, "\n";
    die "usage : ploterr filename grepfield errfield errmin outputfields";
  }

my $fo = 4;
my $file = @ARGV[0];
my $lrn = @ARGV[1]-1;
my $ef = @ARGV[2]-1;
my $th = @ARGV[3];
my $nout = $#ARGV - $fo + 1;
my $i;
my $chs;
my $file1 = "/tmp/toto1.dat";
my $file2 = "/tmp/toto2.dat";
my $file3 = "/tmp/toto3.dat";
my $plotfile = "/tmp/gnupl.gnc";
my $l = 0;

#print "entree : ", @lrn, "\n";
print "fichier : ", $file, "\n";
print "nb sorties : ", $nout, "\n";
print "erreur min : ", $th, "\n";

open (FILE1, ">$file1") or die "Cannot open file", $file1, " : $!\n";
open (FILE2, ">$file2") or die "Cannot open file", $file2, " : $!\n";
open (FILE3, ">$file3") or die "Cannot open file", $file3, " : $!\n";

open (INP, "<$file") or die "Cannot open file ", $file, " : $!\n";
print "Traçage des exemples...\n";
while (<INP>)
  {
    my @champ = split;
    if( @champ[$ef] >= $th )
      {
	my $err = 0;
	$i = 1;
#	print "champ :", @ARGV[2]-1, "\n";
	$chs = @champ[@ARGV[$fo]-1];
	if( $chs eq "" )
	  {
	    $err = 1;
	  }
	while( $err == 0 && $i < $nout )
	  {
#	    print "champ :", @ARGV[$i+2]-1, "\n";
	    if( @champ[@ARGV[$i+$fo]-1] eq "" )
	      {
		$err = 1;
	      }
	    else
	      {
		$chs = $chs."  ".@champ[@ARGV[$i+$fo]-1];
		++$i;
	      }
	  }
	$chs = $chs."\n";
	if( $nout == 1 )
	  {
	    $chs = $l."  ".$chs;
	  }

#	print $chs;

	if( $err == 0 )
	  {
	    if( @champ[$lrn] == 0 )
	      {
		print FILE3 $chs;
	      }
	    else
	      {
		if( @champ[$lrn] == 0.5 )
		  {
		    print FILE2 $chs;
		  }
		else
		  {
		    print FILE1 $chs;
		  }
	      }
	  }
	else
	  {
	    print "Ligne pourrie...\n";
	  }
      }
    ++$l;
  }
close( INP );
close( FILE1 );
close( FILE2 );
close( FILE3 );

print "OK.\n";

open( GNUPL, ">$plotfile" ) or die "Cannot open file ", $plotfile, " : $!\n";
if( $nout <= 2 )
  {
    print GNUPL "plot \"", $file1, "\", \"", $file2, "\", \"", $file3, "\"\n";
  }
else
  {
    print GNUPL "set parametric\n";
    print GNUPL "splot \"", $file1, "\" u 1:2:3, \"", $file2, "\" u 1:2:3, \"", $file3, "\" u 1:2:3\n";
  }
print GNUPL "pause -1\n";
close( GNUPL );

system( "gnuplot $plotfile" );
#getc;

print "Deleting tmp files...";
unlink( $plotfile, $file1, $file2, $file3 );
print "OK\n";

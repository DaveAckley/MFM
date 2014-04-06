#!/usr/bin/perl -Tw

my $file = shift @ARGV;
defined $file or die "Usage:\n$0 PPMFILE\n";
open(FILE,"<$file") or die "Can't read '$file': $!\n";
binmode FILE;
my ($count,$data);
$count = read (FILE, $data, 999999999);
close FILE or die;

$data =~ /\s*#\s*Max site events = (\d+)\n\s*(\d+)\s+(\d+)\s+(\d+)\s*\n(.+)$/
    or die "Can't find header info";
my ($eventmax,$wid,$hei,$max,$bytes) = ($1,$2,$3,$4,$5);
my @pixels = split(//,$bytes);

my $bpp = 1;
my ($x,$y) = (0,0);
for (my $i = 0; $i < $hei*$wid*$bpp; $i += $bpp) {
    printf("%d ", (ord($pixels[$i]) * $eventmax / 255) / 200);
    $x++;
    if(!(($x + 1) % ($wid + 1)))
    {
        $y++;
        $x = 0;
        printf("\n");
    }
}

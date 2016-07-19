#!/usr/bin/perl -w

die "Usage: $0 MASTERROW.svg\n" 
    unless scalar @ARGV == 1
    and $ARGV[0] =~ m!^(.*?/)?(([^/]+)([.]svg))$!;

die "Need inkscape to generate icon zsheet" unless findTool("inkscape","Inkscape");
die "Need ImageMagick to generate icon zsheet" unless findTool("convert","ImageMagick");

my ($path, $stem, $ext) = ($1, $3, $4);
my $in="$path$stem$ext";

my $zsuffix="-ZSHEET";

my $out="$path$stem$zsuffix.png";

die "Input '$in' already named as zsheet\n"
    if $stem =~ /.*$zsuffix$/;

open(my $in_fh,"<",$in) or die "Can't read '$in': $!\n";
my $svg = join('', <$in_fh>);
close $in_fh or die "Can't close '$in': $!\n";

my @rowheights = (
    200, 180, 162, 145, 130, 117, 105, 94, 84, 75, 67, 60,
    54, 48, 43, 38, 34, 30, 27, 24, 21, 19, 17, 15, 
    13, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1
    );
my @ystarts;
{
    my $tmp = 0;
    @ystarts = map { $tmp += $_ } @rowheights;
}


die "Can't find svg height in $in" unless $svg =~ /^(.*?<svg[^<]+height=")([\d]+)(".*)$/s;
my ($hfront,$inHeight,$hrest) = ($1,$2,$3);
die "svg height $inHeight is not 200" unless $inHeight==200;

# Update for zsheet height 
$svg = $hfront."2000".$hrest;

die "Can't find svg width in $in" unless $svg =~ /<svg[^<]+width="([\d]+)"/;
my $inWidth = $1;
die "svg width $inWidth is not multiple of 200" unless ($inWidth%200)==0;

my $inIcons = $inWidth / 200;

my $tag="ZSHEET_MASTER_ROW";
die "Can't find group with id $tag in $in" unless $svg =~ /<g[^<>]+id="$tag"/;

die "Can't find </g></svg> in $in" unless $svg =~ m!^(.+?)(</g>\s*</svg>\s*$)!s;
my ($front,$back) = ($1,$2);

## GENERATE OUTPUT

print STDERR "Warning: Overwriting '$out'\n" if -f $out;

my $finalheight = 2200;

print "Scaling: ";

my $convertcmd = "convert -size ${inWidth}x$finalheight xc:none \\\n";
my @tfiles;
for (my $y = 0; $y < scalar(@rowheights); ++$y) {
    my $scale = 1.0*$rowheights[$y]/$rowheights[0];
    my $ystart = $finalheight-$ystarts[$y];
    my $swidth = $inWidth*$scale;
    my $sheight = $inHeight*$scale;
    my $tmpfile = sprintf("/tmp/%s-%02d.png",$stem,$y);
    push @tfiles, $tmpfile;
    my $cmd = "inkscape --export-area-page --export-png=$tmpfile --export-width=$swidth --export-height=$sheight $in";
    $convertcmd .= "\\( $tmpfile \\) -geometry +0+$ystart -composite ";
    my $right = $inWidth-$inIcons*$sheight;
    my $dimystart = ($y>0)?$ystarts[$y-1]:0;
    $convertcmd .= "\\( $tmpfile +level 35% \\) -geometry +$right+$dimystart -composite ";
    print "$sheight ";
    `$cmd`;
}
print "Done\nMerging and dimming: ";
$convertcmd .= "$out";
`$convertcmd`;
print "Done\nCleaning: ";
for my $tf (@tfiles) {
    unlink $tf or die "Can't remove '$tf': $!";
}
print "Done\nWrote: $out\n";

sub findTool {
    my ($tool, $name) = @_;
    my $cmd = "$tool --version 2>&1";
    my $ret = `$cmd`;
    chomp $ret;
    if ($ret =~ /.*($name.*)/) {
        print "Found: $1\n";
        return 1;
    } else {
        return 0;
    }
}


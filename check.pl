#!/bin/sh --
eval 'PERL_BADLANG=x;export PERL_BADLANG;exec perl -x "$0" "$@";exit 1'
#!perl  # Start marker used by perl -x.
+0 if 0;eval("\n\n\n\n".<<'__END__');die$@if$@;__END__

BEGIN { $^W = 1 }
use integer;
use strict;

# By Daniel J. Bernstein on comp.lang.c.
# http://www.cse.yorku.ca/~oz/hash.html
sub hash32($) {
  my $s = $_[0];
  my $hash = 5381;
  my $ls = length($s);
  for (my $i = 0; $i < $ls; ++$i) {
    $hash = (($hash << 5) + $hash) + vec($s, $i, 8);
  }
  $hash &= 0xffffffff;
  $hash = ($hash - (1 << 32)) & 0xffffffff if $hash > 0 and $hash & 0x80000000;  # Works in both 32-bit and 64-bit Perl.
  $hash 
}  

die "Usage: $0 <as-filename>\n" if @ARGV != 1 or $ARGV[0] eq "--help";
my $fn = $ARGV[0];
# Filename fix for Perl 5.004_04 for filenames starting with whitespace etc.
sub fnopenq($) { $_[0] =~ m@[-+.\w]@ ? $_[0] : "./" . $_[0] }
die "fatal: error opening: $fn\n" if !open(F, "< " . fnopenq($fn));
binmode(F);
my $s = join("", <F>);
die "fatal: error reading: $fn\n" if !close(F);
die "fatal: bad file size: $fn\n" if length($s) != 69024;
die "fatal: bad checksum: $fn\n" if hash32($s) != 0x1abaf783;
die "fatal: bad reverse checksum: $fn\n" if hash32(reverse($s)) != 0x912ad703;

__END__

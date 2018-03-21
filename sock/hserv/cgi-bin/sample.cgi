#!/usr/bin/perl


print "Content-type: text/html\n";
print "\n";

print "<html>\n";
print "<head>\n";
print "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\n";
print "<title>cgi sample (answer)</title>\n";
print "</head>\n";
print "\n";
print "<body bgcolor=#ffffff text=#000000>\n";
print "\n";
print "<center>\n";
print "<font size=+2>\n";
print "<i><b><font color=#007744>cgi sample (answer)</font></i></b>\n";
print "</font><br>\n";
print "<br>\n";


read( STDIN, $buffer, $ENV{'CONTENT_LENGTH'} );

@pairs = split( /&/, $buffer );

foreach $pair( @pairs ){
	( $name, $value ) = split( /=/, $pair );
	$value =~ tr/+/ /;
	$value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C",hex($1))/eg;

	printf( "name: %s,  value: %s<br>\n", $name, $value );
}

print "\n";
print "</body>\n";
print "</html>\n";

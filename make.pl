#!/bin/perl

use strict;
use warnings;

my $CC = 'gcc';
my $FLAGS = '-g -O0';
my $SOURCES = 'main.c common.c diagnostic.c lexer.c parser.c semantic.c ir.c';
my $TARGET = 'bin/seal';

sub change_test 
{
    my ($i) = @_;
    my $dosya = 'main.c';

    open my $in, '<', $dosya or die $!;
    my @lines = <$in>;
    close $in;

    if ($i eq "lexer") 
    {
        for (@lines) 
        {
            s/run_parser\(\)/\/\/run_parser()/;
            s/print_ast\(1\)/\/\/print_ast(1)/;
            s/\/\/run_lexer_machine\(sources_list\)/run_lexer_machine(sources_list)/;
            s/\/\/print_tokens\(1\)/print_tokens(1)/;
        }
    }
    
    if ($i eq "parser") 
    {
        for (@lines) 
        {
            s/\/\/run_parser\(\)/run_parser()/;
            s/\/\/print_ast\(1\)/print_ast(1)/;
            s/print_tokens\(1\)/\/\/print_tokens(1)/;
        }
    }

	if ($i eq "close")
	{
		s/print_ast\(1\)/\/\/print_ast(1)/;
		s/print_tokens\(1\)/\/\/print_tokens(1)/;
	}
	
    if ($i eq "open_all")
    {
    	for (@lines)
    	{
    		s/\/\/run_parser\(\)/run_parser()/;
    		s/\/\/print_ast\(1\)/print_ast(1)/;
            s/\/\/run_lexer_machine\(sources_list\)/run_lexer_machine(sources_list)/;
            s/\/\/print_tokens\(1\)/print_tokens(1)/;
    	}
    }

    open my $out, '>', $dosya or die $!;
    print $out @lines;
    close $out;
}

sub print_out
{
	print "MAKESCRIPT: OUT -> ";
	system("echo $?");
}

if (scalar @ARGV < 1)
{
	my $command = "$CC $FLAGS $SOURCES -o $TARGET";

	system($command) == 0 or die "\nbuild error";
	print "MAKESCRIPT: $command\n";
	print_out();
	exit 0;
}

if ($ARGV[0] eq "clear")
{
	my $command = "rm $TARGET";
	
	system($command) == 0 or die "\n$TARGET not exists";
	print "MAKESCRIPT: $command\n";
	print_out();
	exit 0;
}

if ($ARGV[0] eq "run" || $ARGV[0] eq "mrun")
{
	my $command = "./bin/seal $ARGV[1]";

	if ($ARGV[0] eq "mrun")
	{
		$command = "$CC $FLAGS $SOURCES -o $TARGET && ./bin/seal $ARGV[1]";
	}
	
	system($command) == 0 or die "\n$TARGET not exists";
	print "MAKESCRIPT: $command\n";
	print_out();
	exit 0;
}

if ($ARGV[0] eq "test")
{
	change_test($ARGV[1]);
	print("Changed $ARGV[1]");
	exit 0;
}

print "Wrong arg\n";
exit 1;

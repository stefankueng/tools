import sys, os
from subprocess import call, check_output
import argparse

help_tabs = 'If specified, conversion will be spaces-to-tabs. If excluded, will be tabs-to-spaces.'
help_tab_size = 'Sets the size of tabs (number of spaces in a tab) when converting tabs-to-spaces'
help_check_only = 'Specifies that files are to be only checked for what conversions will take place. No actual changes will be made to any files.'
help_ignore_strings = 'If specified, all whitespace within strings (double-quotes) will be untouched.'
help_eol = 'Tells the tool to not remove trailing whitespace at the end of each line.'
help_exclude = 'If specified, the list of files will be treated as files to *avoid* changing. The default behavior is that it\'s an exclusive list.'
help_files = 'List of files to either include or exclude for processing. See the --exclude option.'

parser = argparse.ArgumentParser( description='Invoke Stefan\'s tabspace utility' )
parser.add_argument( '-t', '--tabs', action='store_true', help=help_tabs )
parser.add_argument( '-s', '--tab-size', type=int, help=help_tab_size )
parser.add_argument( '-c', '--check-only', action='store_true', help=help_check_only )
parser.add_argument( '--ignore-strings', action='store_true', help=help_ignore_strings )
parser.add_argument( '-e', '--eol', action='store_true', help=help_eol )
parser.add_argument( '--exclude', action='store_true', help=help_exclude )
parser.add_argument( 'file', nargs='+', help=help_files )
args = parser.parse_args()

cmd = [ 'tabspace64' ]

if args.check_only:
    cmd.append( '/checkonly' )
    
if args.tabs:
    cmd.append( '/usetabs' )
    
if args.tab_size:
    cmd.append( '/tabsize:{}'.format( args.tab_size ) )
    
if args.ignore_strings:
    cmd.append( '/cstyle' )
    
if args.eol:
    cmd.append( '/leaveeol' )

operation = 'exclude' if args.exclude else 'include'
    
cmd.append( '/{}:{}'.format( operation, ';'.join( args.file ) ) )

print( 'Running: {}'.format( ' '.join( cmd ) ) )
call( cmd )
print( 'Done.' )
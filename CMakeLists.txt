#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

cmake_minimum_required( VERSION 2.8 )


##### general package setup #####################

project( dolphin )


##### configure checks ##########################

include( ConfigureChecks.cmake )


##### source directories ########################

add_subdirectory( src )


##### write configure files #####################

configure_file( config.h.cmake config.h @ONLY )

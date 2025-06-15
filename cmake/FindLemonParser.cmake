#[======================================================================[.rst:
FindLemonParser
---------------

This module locates or, if necessary, builds the Lemon parser generator.
Lemon is part of the SQLite project, but it is not found by running
``find_package(SQLite3)``.

The following variables are defined:

``LemonParser_EXECUTABLE``
  Path to Lemon executable.
``LemonParser_FOUND``
  Whether the Lemon executable was found.

#]======================================================================]

include(FetchContent)

#[======================================================================[.rst:
.. command:: lemon_parser_find_and_build
   
   Finds the installed Lemon executable. If it cannot be found, then its
   sources are downloaded, configured, and built.

.. variable:: LemonParser_VERSION
   
   This is the version number as returned by the ``lemon -x``.
#}======================================================================]

function(lemon_parser_find_and_build)
	#cmake_parse_arguments(PARSE_ARGV 0 lemon_parser_
	#  <options> <one_value_keywords> <multi_value_keywords>)
	
	set(LemonParser_FOUND NO PARENT_SCOPE)
	set(lemonparser_src_dir "${FETCHCONTENT_BASE_DIR}/lemonparser-src")
	message(STATUS "Lemon parser source files will be downloaded to "
		${lemonparser_src_dir})
	
	set(LEMON_C_URL
		"https://raw.githubusercontent.com/sqlite/sqlite/refs/heads/master/tool/lemon.c")
	set(LEMPAR_C_URL
		"https://raw.githubusercontent.com/sqlite/sqlite/refs/heads/master/tool/lempar.c")
	
	# download `lemon.c`
	message(STATUS "Downloading lemon.c: ${LEMON_C_URL}")
	set(lemon_c_fpath  "${lemonparser_src_dir}/lemon.c")
	file(DOWNLOAD ${LEMON_C_URL} ${lemon_c_fpath} STATUS lemon_c_result
		TIMEOUT 5)
	list(GET lemon_c_result 0 lemon_c_result_code)
	if(NOT(${lemon_c_result_code} EQUAL 0))
		list(GET lemon_c_result 1 lemon_c_result_msg)
		message(FATAL_ERROR "lemon.c download failed with code "
			"${lemon_c_result_code}: ${lemon_c_result_msg}")
		return()
	endif()
	
	# download `lempar.c`
	message(STATUS "Downloading lempar.c: ${LEMPAR_C_URL}")
	set(lempar_c_fpath "${lemonparser_src_dir}/lempar.c")
	file(DOWNLOAD ${LEMPAR_C_URL} ${lempar_c_fpath} STATUS lempar_c_result
		TIMEOUT 5)
	list(GET lempar_c_result 0 lempar_c_result_code)
	if(NOT(${lempar_c_result_code} EQUAL 0))
		list(GET lempar_c_result 1 lempar_c_result_msg)
		message(FATAL_ERROR "lempar.c download failed with code "
			"${lempar_c_result_code}: ${lempar_c_result_msg}")
		return()
	endif()
	
	# compile from `lemon.c` and run
	set(CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE(
	try_run(run_result compile_result
		SOURCES ${lemon_c_fpath}
		COMPILE_OUTPUT_VARIABLE compile_var
		RUN_OUTPUT_VARIABLE run_var
		RUN_OUTPUT_STDOUT_VARIABLE run_stdout
		WORKING_DIRECTORY ${lemonparser_src_dir}
		ARGS -x
	)
	set(LemonParser_VERSION ${run_stdout} PARENT_SCOPE)
	#~ set(LemonParser_EXECUTABLE ${lemon} PARENT_SCOPE)
	
	unset(LemonParser_FOUND)
	set(LemonParser_FOUND YES PARENT_SCOPE)
	
endfunction()

#[======================================================================[.rst:
.. command:: lemon_parser_generate
   
   Generates a Lemon parser.
   
   ::
      
      lemon_parser_generate(
        <input_file> <output_file>
        [IFDEFS <macro_1> [<macro_2> ...]]
        [BASIS_ONLY]
        [NO_COMPRESS]
        [PREPROCESS_ONLY]
        [GRAMMAR_ONLY]
        [MAKEHEADERS]
        [NO_LINES]
        [SHOW_CONFLICTS]
        [NO_REPORT]
      )
   
   **Arguments**
   
   + ``<input_file>``: Path to input file. Equivalent to the single positional
     argument on the command line.
   + ``<output_file>``: Path to output file. Equivalent to using the ``-d``
     option, plus the suffix-changing behavior relative to the input file
     (i.e., ``parser.y`` becomes ``parser.c``).
   
   **Options**
   
   + ``BASIS_ONLY``: Print only the basis in report.
     Equivalent to ``-b`` on command line.
   + ``NO_COMPRESS``: Don't compress the action table.
     Equivalent to ``-c`` on command line.
   + ``IFDEFS <macro_1> [<macro_2> ...]``: Define ``%ifdef`` macros.
     Equivalent to ``-d<macro_1> -d<macro_2> ...``.
   + ``PREPROCESS_ONLY``: Print input file after preprocessing.
     Equivalent to ``-E`` on command line.
   + ``GRAMMAR_ONLY``: Print grammar without actions.
     Equivalent to ``-g`` on command line.
   + ``MAKEHEADERS``: Output a
     `makeheaders <https://github.com/compiler-dept/makeheaders>`_-compatible
     file. Equivalent to ``-m`` on command line.
   + ``NO_LINES``: Do not print ``#line`` statements.
     Equivalent to ``-l`` on command line.
   + ``SHOW_CONFLICTS``: Show conflicts resolved by precedence rules.
     Equivalent to ``-p`` on command line.
   + ``NO_REPORT``: Quiet mode; don't print the report file.
     Equivalent to ``-q`` on command line.
   + ``NO_RENUMBER``: Do not sort or re-number states.
     Equivalent to ``-r`` on command line.
   + ``SHOW_STATS``: Print parser statistics to standard output.
     Equivalent to ``-s`` on command line.
   + ``SQL_TABLES``: Generate the *.sql file describing the parser tables.
     Equivalent to ``-S`` on command line.
   + ``TEMPLATE_FILE <template_file>``: Specify a template file.
     Equivalent to ``-T<template_file>`` on command line.

#]======================================================================]

function(lemon_parser_generate)
	
	# process function arguments
	set(_options BASIS_ONLY NO_COMPRESS PREPROCESS_ONLY GRAMMAR_ONLY
		MAKEHEADERS NO_LINES SHOW_CONFLICTS NO_REPORT NO_RENUMBER SHOW_STATS
		SQL_TABLES)
	set(_oneval_keywords TEMPLATE_FILE)
	set(_multival_keywords IFDEFS)
	cmake_parse_arguments(PARSE_ARGV 0 arg
		${_options} ${_oneval_keywords} ${_multival_keywords})
	list(LENGTH arg_UNPARSED_ARGUMENTS num_unparsed)
	if(NOT (${num_unparsed} EQUAL 2))
		message(FATAL_ERROR
			"expected 2 positional arguments (not ${num_unparsed})")
	endif()
	list(GET arg_UNPARSED_ARGUMENTS 0 input_file_)
	list(GET arg_UNPARSED_ARGUMENTS 1 output_file_)
	
	# build command-line arguments list, starting with options
	set(cmdline_args )
	if(arg_BASIS_ONLY)
		list(APPEND cmdline_args "-b")
	endif()
	if(arg_NO_COMPRESS)
		list(APPEND cmdline_args "-c")
	endif()
	if(arg_IFDEFS)
		# for each `<element>` in arg_IFDEFS, append `-d<element>`
	endif()
	if(arg_PREPROCESS_ONLY)
		list(APPEND cmdline_args "-E")
	endif()
	if(arg_GRAMMAR_ONLY)
		list(APPEND cmdline_args "-g")
	endif()
	if(arg_MAKEHEADERS)
		list(APPEND cmdline_args "-m")
	endif()
	if(arg_NO_LINES)
		list(APPEND cmdline_args "-l")
	endif()
	if(arg_SHOW_CONFLICTS)
		list(APPEND cmdline_args "-p")
	endif()
	if(arg_NO_REPORT)
		list(APPEND cmdline_args "-q")
	endif()
	if(arg_NO_RENUMBER)
		list(APPEND cmdline_args "-r")
	endif()
	if(arg_SHOW_STATS)
		list(APPEND cmdline_args "-s")
	endif()
	if(arg_SQL_TABLES)
		list(APPEND cmdline_args "-S")
	endif()
	if(arg_TEMPLATE_FILE)
		# get `<path>` from key/value pair and append `-T<path>`
	endif()
	list(APPEND cmdline_args ${input_file_} ${output_file_})
	#~ add_custom_command(
		
	#~ )
	
endfunction()



#~ find_program(LemonParser_EXECUTABLE NAMES lemon)
#~ if(LemonParser_EXECUTABLE)
	#~ set(LemonParser_FOUND YES)
#~ else()
	lemon_parser_find_and_build()
#~ endif()

# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[======================================================================[.rst:
LemonParse
----------

This module wraps the
`Lemon <https://sqlite.org/src/doc/trunk/doc/lemon.html>`_ parser generator,
an alternative to Yacc and Bison.
It generates C code from a grammar specification file.

The name ``LemonParse`` was used instead of simply ``Lemon`` to avoid any
confusion with other software named "lemon".

The following variables are set by this module:

.. variable:: LemonParse_FOUND
   
   Whether the Lemon executable was found.

.. variable:: LemonParse_EXECUTABLE
   
   Path to the found Lemon executable.

.. variable:: LemonParse_VERSION
   
   Version number of the found Lemon executable.

.. todo::
   
   Finalize/debug support for downloading and compiling Lemon's sources.
   
   Harden handling of function arguments.

The following variable can be set to alter the behavior of this module:

.. variable:: LemonParse_FORCE_LOCAL
   
   Set this to ``YES`` to force building Lemon locally from downloaded sources.
   This is ``NO`` by default.

#]======================================================================]



if(NOT DEFINED LemonParse_FORCE_LOCAL)
	set(LemonParse_FORCE_LOCAL NO CACHE BOOL "Force building Lemon locally")
endif()
message(DEBUG "Force building Lemon locally: ${LemonParse_FORCE_LOCAL}")



#[======================================================================[.rst:
.. command:: LemonParse_download_lemon_sources

   Downloads the source files comprising the codebase of the Lemon parser
   generator, those being ``lemon.c`` and ``lempar.c``.
   
   ::
      
      LemonParse_download_lemon_sources(
          <lemon_c_file> <lempar_c_file>
          [DEST_DIR <dest_dir>]
      )
   
   On success, the ``<lemon_c_file>`` and ``<lempar_c_file>`` variables are set
   to the paths of the downloaded ``lemon.c`` and ``lempar.c`` files
   respectively.
   
   The only optional argument is:
   
   ``DEST_DIR <dest_dir>``
     Destination directory for the downloaded C source files.
     If this is not set, then the directory is ``lemonparse-src`` within the
     base directory created by CMake's ``FetchContent`` module.
#]======================================================================]
function(LemonParse_download_lemon_sources)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "DEST_DIR" "")
	if(NOT (${ARGC} STREQUAL 2))
		message(FATAL_ERROR "Expected 2 positional arguments, got ${ARGC}")
	endif()
	if(arg_DEST_DIR)
		set(dest_dir_ "${arg_DEST_DIR}")
	else()
		set(dest_dir_ "${FETCHCONTENT_BASE_DIR}/lemonparse-src")
	endif()
	message(DEBUG "Destination directory for Lemon sources: `${dest_dir_}`")
	set(lemon_c_url_
		"https://raw.githubusercontent.com/sqlite/sqlite/refs/heads/master/tool/lemon.c")
	set(lempar_c_url_
		"https://raw.githubusercontent.com/sqlite/sqlite/refs/heads/master/tool/lempar.c")
	set(lemon_c_destfile_  "${dest_dir_}/lemon.c")
	set(lempar_c_destfile_ "${dest_dir_}/lempar.c")
	if(NOT (EXISTS ${lemon_c_destfile_}))
		file(DOWNLOAD ${lemon_c_url_} ${lemon_c_destfile_}
			STATUS lemon_c_result_ TIMEOUT 5)
		list(GET lemon_c_result_ 0 lemon_c_result_code_)
		message(DEBUG "Downloading lemon.c  -> ${lemon_c_result_}")
		if(NOT (${lemon_c_result_code_} EQUAL 0))
			list(GET lemon_c_result_ 1 lemon_c_result_msg_)
			message(FATAL_ERROR "lemon.c download failed with code "
				"${lemon_c_result_code_}: ${lemon_c_result_msg_}")
			return()
		endif()
	endif()
	if(NOT (EXISTS ${lempar_c_destfile_}))
		file(DOWNLOAD ${lempar_c_url_} ${lempar_c_destfile_}
			STATUS lempar_c_result_ TIMEOUT 5)
		list(GET lempar_c_result_ 0 lempar_c_result_code_)
		message(DEBUG "Downloading lempar.c  -> ${lempar_c_result_}")
		if(NOT (${lempar_c_result_code_} EQUAL 0))
			list(GET lempar_c_result_ 1 lempar_c_result_msg_)
			message(FATAL_ERROR "lempar.c download failed with code "
				"${lempar_c_result_code_}: ${lempar_c_result_msg_}")
			return()
		endif()
	endif()
	set(${ARGV0} ${lemon_c_destfile_} PARENT_SCOPE)
	set(${ARGV1} ${lempar_c_destfile_} PARENT_SCOPE)
	message(STATUS "Successfully downloaded Lemon sources")
endfunction()



#[======================================================================[.rst:
.. command:: LemonParse_build_lemon
   
   Builds the Lemon parser generator executable from given sources.
   
   ::
      
      LemonParse_build_lemon(
          <executable> <lemon_c_file> <lempar_c_file>
      )
   
   The arguments are:
   
   ``<executable>``
     Path to compiled Lemon executable.
   ``<lemon_c_file>``
     Path to ``lemon.c`` file.
   ``<lempar_c_file>``
     Path to ``lempar.c`` file.
#]======================================================================]
function(LemonParse_build_lemon)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "" "")
	set(lemon_c_destfile_  "${ARGV1}")
	set(lempar_c_destfile_ "${ARGV2}")
	set(lemon_executable_ "${CMAKE_CURRENT_BINARY_DIR}/lemon")
	message(DEBUG "Building Lemon executable from `${lemon_c_destfile_}`")
	execute_process(
		COMMAND bash "-c"
			"${CMAKE_C_COMPILER} -o ${lemon_executable_} ${lemon_c_destfile_}"
		COMMAND_ECHO STDOUT
	)
	set(${ARGV0} ${lemon_executable_} PARENT_SCOPE)
endfunction()



if(${LemonParse_FORCE_LOCAL})
	set(LemonParse_FOUND NO)
else()
	find_program(LemonParse_EXECUTABLE lemon DOC "Lemon parser generator")
	if(DEFINED ${LemonParse_EXECUTABLE-NOTFOUND})
		set(LemonParse_FOUND NO)
	else()
		set(LemonParse_FOUND YES)
	endif()
endif()
if(NOT ${LemonParse_FOUND})
	LemonParse_download_lemon_sources(
		LemonParse_lemon_c_file
		LemonParse_lempar_c_file
	)
	LemonParse_build_lemon(
		LemonParse_EXECUTABLE
		${LemonParse_lemon_c_file}
		${LemonParse_lempar_c_file})
endif()
message(STATUS "Lemon executable is at `${LemonParse_EXECUTABLE}`")



#[======================================================================[.rst:
.. command:: LemonParse_get_version

   Gets the version number for the Lemon executable.
   
   ::
      
      LemonParse_get_version(<variable>)
   
   The ``<variable>`` argument is set to the version number found in the text
   returned by ``lemon -x``.
   
   .. note::
      
      This function is already run when this module is included, setting the
      ``LemonParse_VERSION`` variable.
#]======================================================================]
function(LemonParse_get_version)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "" "")
	if(NOT EXISTS ${LemonParse_EXECUTABLE})
		message(FATAL_ERROR "Lemon executable not found")
	endif()
	execute_process(
		COMMAND ${LemonParse_EXECUTABLE} -x
		OUTPUT_VARIABLE raw_version_
	)
	string(REGEX REPLACE "^Lemon[ \t]+[Vv]ersion[ \t]+" ""
		version_info_ ${raw_version_})
	string(REGEX MATCH "[0-9]+\.[0-9]+" version_number_ ${version_info_})
	set(${ARGV0} ${version_number_} PARENT_SCOPE)
endfunction()



#[======================================================================[.rst:
.. command:: LemonParse_generate_sources
   
   Generates C/C++ source and header files using the Lemon parser generator.
   
   ::
      
      LemonParse_generate_sources(
          <variable> <input_spec> <output_source>
          [OUTPUT_HEADER <output_header>]
          [NO_COMPRESS]
          [NO_LINES]
          [MAKEHEADERS]
          [NO_REPORT]
          [NO_RENUMBER]
          [SHOW_STATS]
          [SQL]
          [TEMPLATE <template_file>]
      )
   
   The ``<variable>`` argument is the target set by
   :command:`add_custom_target`, which is called internally.
   The ``<input_spec>`` argument is the path to the source file containing the
   parser specification, usually with ``.y`` (or maybe ``.yy``) suffix.
   The ``<output_source>`` argument is the path to the generated C source file.
   
   .. note::
      
      Lemon does not produce differently-optimized source code based on
      command-line arguments. Optimization is only provided by a compiler
      compiling that generated source code.
   
   The optional arguments are:
   
   ``OUTPUT_HEADER <output_header>``
     The generated C header file, containing the integral identifiers for
     parser terminals (tokens), is written to the path given at
     ``<output_header>``. By default, the header file is created in the same
   directory as the generated C source file, with the same basename as that
   source file.
  
   ``NO_COMPRESS``
     Do not compress the action table.
     Equivalent to ``lemon -c ...``.
   
   ``NO_LINES``
     Do not insert ``#line`` statements in the generated file.
     Equivalent to ``lemon -l ...``.
   
   ``MAKEHEADERS``
     Output a
     `makeheaders <https://compiler-dept.github.io/makeheaders/>`_-compatible
     file.
     Equivalent to ``lemon -m ...``.
   
   ``NO_REPORT``
     Do not generate a report file (with suffix ``.out``).
     Equivalent to ``lemon -q ...``.
   
   ``NO_RENUMBER``
     Do not sort or renumber states in the generated file.
     Equivalent to ``lemon -r ...``.
   
   ``SHOW_STATS``
     Print parser statistics to standard output.
   
   ``SQL``
     Generate the ``*.sql`` file describing the parser tables, within the same
     directory as the other outputs.
     The generated file has the same basename as that of the input file.
     Equivalent to ``lemon -S ...``.
   
   ``TEMPLATE <template_file>``
     Use the template file at ``<template_file>`` instead of the default
     template file (``lempar.c``).
     Equivalent to ``lemon -T<template_file> ...``.
   
   This function defines the ``YYCOVERAGE`` macro for the generated C/C++
   source file (at ``<output_source>``) in order to retain the ``yyTokenName``
   and ``yyRuleName`` arrays generated by Lemon. 
   
   Lemon generates source code with unused variables and function parameters.
   To suppress the associated compiler warnings, this function also sets these
   compilation options:
   
   + ``/wd4189`` for MSVC (Microsoft Visual C++); or
   + ``-Wno-unused-parameter`` and ``-Wno-unused-variable`` for all other
     compilers.
   
.. todo::
   
   Add support for ``lemon -g ...``, with a single-value option
   ``RAW_GRAMMAR``, to capture the command output at build-time. This will need
   to be done with a CMake script calling ``execute_process``, itself called
   from ``add_custom_command`` in the parent CMake file.
#]======================================================================]
function(LemonParse_generate_sources)
	set(opts NO_COMPRESS MAKEHEADERS NO_LINES NO_REPORT NO_RENUMBER SHOW_STATS
		SQL)
	set(sngval_args OUTPUT_HEADER TEMPLATE)
	set(mulval_args )
	cmake_parse_arguments(PARSE_ARGV 3 arg "${opts}" "${sngval_args}"
		"${mulval_args}")
	if(DEFINED arg_KEYWORDS_MISSING_VALUES)
		message(FATAL_ERROR "Keyword arguments are missing values: "
			"${arg_KEYWORDS_MISSING_VALUES}")
	endif()
	set(input_file_  "${ARGV1}")
	set(output_file_ "${ARGV2}")
	cmake_path(REPLACE_EXTENSION input_file_ LAST_ONLY ".c"
		OUTPUT_VARIABLE output_c_file_)
	cmake_path(REPLACE_EXTENSION input_file_ LAST_ONLY ".h"
		OUTPUT_VARIABLE pre_output_h_file_)
	if(DEFINED arg_OUTPUT_HEADER)
		set(output_h_file_ ${arg_OUTPUT_HEADER})
	else()
		set(output_h_file_ ${pre_output_h_file_})
	endif()
	if(arg_NO_COMPRESS)
		set(nocompress_cmdarg_ "-c")
	endif()
	if(arg_NO_LINES)
		set(nolines_cmdarg_ "-l")
	endif()
	if(arg_MAKEHEADERS)
		set(makeheaders_cmdarg_ "-m")
	endif()
	if(arg_NO_REPORT)
		set(noreport_cmdarg_ "-q")
	endif()
	if(arg_NO_RENUMBER)
		set(norenumber_cmdarg_ "-r")
	endif()
	if(arg_SHOW_STATS)
		set(showstats_cmdarg_ "-s")
	endif()
	if(arg_SQL)
		set(sql_cmdarg_ "-S")
	endif()
	if(arg_TEMPLATE)
		set(template_cmdarg_ "-T${arg_TEMPLATE}")
	endif()
	cmake_path(GET output_file_ PARENT_PATH output_dir_)
	message(DEBUG "Will generate C/C++ parser sources:")
	message(DEBUG "  executable path: `${LemonParse_EXECUTABLE}`")
	message(DEBUG "  variable:        ${ARGV0}")
	message(DEBUG "  input_spec:      ${input_file_}")
	message(DEBUG "  output source:   ${output_file_}")
	message(DEBUG "  output header:   ${output_h_file_}")
	message(DEBUG "  output dir:      ${output_dir_}")
	message(DEBUG "  no-compress arg: ${nocompress_cmdarg_}")
	message(DEBUG "  no-lines arg:    ${nolines_cmdarg_}")
	message(DEBUG "  makeheaders arg: ${makeheaders_cmdarg_}")
	message(DEBUG "  no-report arg:   ${noreport_cmdarg_}")
	message(DEBUG "  no-renumber arg: ${norenumber_cmdarg_}")
	message(DEBUG "  show-stats arg:  ${showstats_cmdarg_}")
	message(DEBUG "  SQL arg:         ${sql_cmdarg_}")
	message(DEBUG "  template file:   ${arg_TEMPLATE}")
	
	if(MSVC)
		set_source_files_properties(${output_file_} PROPERTIES
			COMPILE_OPTIONS "/wd4189"
			COMPILE_DEFINITIONS YYCOVERAGE)
	else()
		set_source_files_properties(${output_file_} PROPERTIES
			COMPILE_OPTIONS "-Wno-unused-parameter;-Wno-unused-variable"
			COMPILE_DEFINITIONS YYCOVERAGE)
	endif()
	
	add_custom_command(
		OUTPUT ${output_file_} ${output_h_file_}
		COMMAND ${LemonParse_EXECUTABLE} ${input_file_} -d${output_dir_}
			${nocompress_cmdarg_} ${nolines_cmdarg_} ${makeheaders_cmdarg_}
			${noreport_cmdarg_} ${norenumber_cmdarg_} ${showstats_cmdarg_}
			${sql_cmdarg_} ${template_cmdarg_}
		COMMAND ${CMAKE_COMMAND} -E rename ${output_c_file_} ${output_file_}
		COMMAND ${CMAKE_COMMAND} -E rename ${pre_output_h_file_}
			${output_h_file_}
		DEPENDS ${input_file_}
		COMMENT "Generating C/C+ parser sources: `${input_file_}` \
-> `${output_file_}`"
	)
endfunction()



LemonParse_get_version(LemonParse_VERSION)

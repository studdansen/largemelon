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
	
	# compile from `lemon.c`
	add_executable(lemon IMPORTED GLOBAL)
	target_sources(lemon INTERFACE ${lemon_c_fpath})
	set(LemonParser_EXECUTABLE ${lemon} PARENT_SCOPE)
	
	unset(LemonParser_FOUND)
	set(LemonParser_FOUND YES PARENT_SCOPE)
	
endfunction()



#~ find_program(LemonParser_EXECUTABLE NAMES lemon)
#~ if(LemonParser_EXECUTABLE)
	#~ set(LemonParser_FOUND YES)
#~ else()
	lemon_parser_find_and_build()
#~ endif()

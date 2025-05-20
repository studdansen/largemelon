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



function(lemon_parser_find_and_build)
	#cmake_parse_arguments(PARSE_ARGV 0 lemon_parser_
	#  <options> <one_value_keywords> <multi_value_keywords>)
	
	set(LEMON_C_URL  "https://sqlite.org/src/tool/lemon.c")
	set(LEMPAR_C_URL "https://sqlite.org/src/file/tool/lempar.c")
	set(lemon_build_dir "${PROJECT_BINARY_DIR}/_deps/lemon-parser")
	
	# download `lemon.c`
	set(lemon_c_fpath  "${PROJECT_BINARY_DIR}/lemon.c")
	file(DOWNLOAD ${LEMON_C_URL}  ${lemon_c_fpath}
		SHOW_PROGRESS STATUS lemon_c_result  TIMEOUT 5)
	list(GET ${lemon_c_result} 0 lemon_c_result_code)
	if(${lemon_c_result_code} NOTEQUAL 0)
		list(GET ${lemon_c_result} 1 lemon_c_result_msg)
		message(ERROR "Failed to download lemon.c: ${lemon_c_result_msg}")
		return()
	endif()
	
	# download `lempar.c`
	set(lempar_c_fpath "${PROJECT_BINARY_DIR}/lempar.c")
	file(DOWNLOAD ${LEMPAR_C_URL} ${lempar_c_fpath}
		SHOW_PROGRESS STATUS lempar_c_result TIMEOUT 5)
	list(GET ${lempar_c_result} 0 lempar_c_result_code)
	if(${lempar_c_result_code} NOTEQUAL 0)
		list(GET ${lempar_c_result} 1 lempar_c_result_msg)
		message(ERROR "Failed to download lempar.c: ${lempar_c_result_msg}")
		return()
	endif()
	
	# compile from `lemon.c`
	
	set(LemonParser_FOUND YES)
	
endfunction()



find_program(LemonParser_EXECUTABLE NAMES lemon)
if(LemonParser_EXECUTABLE)
	set(LemonParser_FOUND YES)
else()
	lemon_parser_find_and_build()
endif()

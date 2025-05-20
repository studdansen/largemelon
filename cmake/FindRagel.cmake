#[======================================================================[.rst:
FindRagel
---------

This module locates or, if necessary, builds the Ragel scanner generator.

The following variables are defined:

``Ragel_EXECUTABLE``
  Path to Ragel executable.
``Ragel_FOUND``
  Whether the Ragel executable was found.

#]======================================================================]



function(ragel_scanner_find_and_build)
	set(Ragel_FOUND NO)
endfunction()

find_program(Ragel_EXECUTABLE ragel)
if(RAGEL_EXECUTABLE)
	set(Ragel_FOUND YES)
else()
	ragel_scanner_find_and_build()
endif()

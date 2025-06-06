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

include(FetchContent)

#[======================================================================[.rst:
ragel_scanner_find_and_build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Finds the installed Ragel executable. If it cannot be found, then its sources
are downloaded, configured, and built.
#}======================================================================]

function(ragel_scanner_find_and_build)
	
	set(Ragel_FOUND NO PARENT_SCOPE)
	
	# download 
	
	unset(Ragel_FOUND)
	set(Ragel_FOUND YES PARENT_SCOPE)
	
endfunction()

#{======================================================================[.rst:
ragel_scanner_generate
^^^^^^^^^^^^^^^^^^^^^^

Generates a Ragel finite-state machine or scanner.

.. code-block:: cmake
   
   ragel_scanner_generate(
     OUTPUT_FILE <output_file>
   )

This function wraps at least most of the arguments to the ``ragel`` executable.

General options
"""""""""""""""

Use the ``KEEP_DUPLICATES`` option (``-d``) to not remove duplicates from
action lists.

Use ``OUTPUT_FILE <output-file>`` (``-o <file>``) to specify the path to the
output file.

Use ``IMPORT_DIRS <import-dir> [<import_dir> ...]`` (``-I <dir>``) to set the
list of directories searched for included and imported files.

Error-reporting
"""""""""""""""

Use ``ERROR_FORMAT <format>`` (``--error-format``) to set the error-reporting
format. ``<format>`` can be either ``gnu`` or ``msvc``.

FSM minimization
""""""""""""""""

Use ``MINIMIZE <mode>`` to set how Ragel minimizes the resultant FSM.

+	To skip performing minimization (``-n``), set ``<mode>`` to ``NONE``.
+	To minimize at the end of compilation (``-m``), set ``<mode>`` to ``END``.
+	To minimize after most operations (``-l``, the default option), set
	``<mode>`` to ``MOST``.
+	To minimize after every operation (``-e``), set ``<mode>`` to ``EVERY``.

Visualization
"""""""""""""

Use ``XML`` (``-x``) to only run the front-end, skipping FSM generation.

Use ``DOT`` (``-V``) to generate a ``*.dot`` file for Graphviz.

Use ``GV_PRINTABLE`` (``-p``) to display printable characters on labels.

Use ``SPEC`` (``-S <spec>``) to generate a finite-state machine specification
to output (for Graphviz output).

Use ``MACHINE <machine>`` to write the definition/instantiation of
``<machine>`` to output (for Graphviz output).

Host Language
"""""""""""""

Use ``LANGUAGE <language>`` to specify the host language, to which the
resultant is generated.

+	For C, C+, Objective-C, or Objective-C++ (``-C``, the default), use ``C``.
+	For D (``-D``), use ``D``.
+	For Go (``-Z``), use ``Go``.
+	For Java (``-J``), use ``Java``.
+	For Ruby (``-R``), use ``Ruby``.
+	For C-Sharp (``-A``), use ``C-Sharp``
+	For OCaml (``-O``), use ``OCaml``.

The following line directives and code styles are allowed per language:

.. list-table::
   :class: longtable
   :header-rows: 1
   
   * - CMake keyword option
     - Equivalent ``ragel`` option
     - Description
     - Languages
   * - ``NO_LINE_DIRS``
     - ``-L``
     - Inhibit writing of #line directives
     - C, D, Ruby, C#, OCaml
   * - ``TABLE_FSM``
     - ``-T0``
     - Table driven FSM (default)
     - C, D, Java, Ruby, C#, OCaml
   * - ``FAST_TABLE_FSM``
     - ``-T1``
     - Faster table driven FSM
     - C, D, Ruby, C#, OCaml
   * - ``FLAT_TABLE_FSM``
     - ``-F0``
     - Flat table driven FSM
     - C, D, Ruby, C#, OCaml
   * - ``FAST_FLAT_TABLE_FSM``
     - ``-F1``
     - Faster flat table-driven FSM
     - C, D, Ruby, C#, OCaml
   * - ``GOTO_FSM``
     - ``-G0``
     - Goto-driven FSM
     - C, D, C#, OCaml
   * - ``FAST_GOTO_FSM``
     - ``-G1``
     - Faster goto-driven FSM
     - C, D, C#, OCaml
   * - ``VERY_FAST_GOTO_FSM``
     - ``-G2``
     - Really fast goto-driven FSM
     - C, D
   * - ``SPLIT_GOTO_FSM``
     - ``-P<N>``
     - N-Way Split really fast goto-driven FSM
     - C, D

#]======================================================================]

function(ragel_scanner_generate)
	set(_options KEEP_DUPLICATES NO_MINIMIZE
		NO_LINE_DIRS TABLE_FSM FAST_TABLE_FSM FLAT_TABLE_FSM
		FAST_FLAT_TABLE_FSM GOTO_FSM FAST_GOTO_FSM VERY_FAST_GOTO_FSM
		SPLIT_GOTO_FSM
	)
	set(_oneval_keywords OUTPUT_FILE ERROR_FORMAT MINIMIZE)
	set(_multival_keywords IMPORT_DIRS)
	cmake_parse_arguments(PARSE_ARGV 0 Ragel_
	  ${_options} ${_oneval_keywords} ${_multival_keywords})
	
endfunction()

find_program(Ragel_EXECUTABLE ragel)
if(RAGEL_EXECUTABLE)
	set(Ragel_FOUND YES)
else()
	ragel_scanner_find_and_build()
endif()

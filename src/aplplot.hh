/*
    This file is part of GNU APL, a free implementation of the
    ISO/IEC Standard 13751, "Programming Language APL, Extended"

    Copyright (C) 2020 Chris Moller

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <Native_interface.hh>

/**
   This file demonstrates how to write native APL functions. A native
   APL function is a function that is written in C++ but can be called
   in APL like a user-defined APL function.
   
   In GNU APL a native function is created by using a variant of dyadic ⎕FX:

	soname ⎕FX funname

   soname is the path to a shared library
   funname is the name of the function

   For example:

   'native/.libs/lib_file_io.so' ⎕FX 'FILE_IO'
FILE_IO

   creates a native function FILE_IO. FILE_IO can then be called (the
   convention for FILE_IO is that the axis is a sub-function number (and
   sub-function 1 returns the error string for a numeric error code provided
   as right argument for FILE_IO):

      FILE_IO[1] 0   ⍝  error string for error code 0
Success

      FILE_IO[1] 1   ⍝  error string for error code 1
Operation not permitted

      FILE_IO[1] 2   ⍝  error string for error code 2
No such file or directory

      FILE_IO[1] 3   ⍝  error string for error code 3
No such process

   It is up to the native function to decide which function signatures
   it implements (and not implemented signatures will throw a SYNTAX ERROR).
   In the case of FILE_IO, the signatures Z←A FILE_IO[X] B and Z←FILE_IO[X] B
   i.e. monadic and dyadic function with axis were implemented.

   The native function implementated in this file is a template for your
   own native function. It implements all function signatures supported by
   GNU APL and simply returns ia character vector telling how it was called.

**/

// mandatory functios
extern "C" void * get_function_mux(const char * function_name);
static Fun_signature get_signature();
static Token eval_fill_B(Value_P B);
static Token eval_fill_AB(Value_P A, Value_P B);
static Token eval_ident_Bx(Value_P B, Axis x);

Fun_signature get_signature() { return SIG_Z_A_F2_B; }

static Token eval_B(Value_P B);
static Token eval_AB(Value_P A, Value_P B);
static Token eval_XB(Value_P X, Value_P B);
static Token eval_AXB(Value_P A, Value_P X, Value_P B);


/*
 * Copyright (c) 2020-2025, Rauli Laine
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
 #include <cstdlib>
 #include <iostream>

 #include <peelo/unicode/encoding/utf8.hpp>

 #include "snek/interpreter/resolve.hpp"
 #include "snek/parser/statement.hpp"

 #include "./utils.hpp"

 int
 main(int argc, char** argv)
 {
   using peelo::unicode::encoding::utf8::encode;

   snek::interpreter::Runtime runtime;
   bool first = true;

   if (argc != 2)
   {
     std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
     std::exit(EXIT_FAILURE);
   }
   LexFile(
     argv[1],
     [&runtime, &first](auto& lexer)
     {
       const auto statement = statement::Parse(lexer, true);
       const auto type = snek::interpreter::ResolveStatement(
         runtime,
         runtime.root_scope(),
         statement
       );

       if (first)
       {
         first = false;
       } else {
         std::cout << std::endl;
       }
       PrintNode(statement);
       if (type)
       {
         std::cout << encode(type->ToString()) << std::endl;
       } else {
         std::cout << "UNABLE TO RESOLVE" << std::endl;
       }
     }
   );

   return EXIT_SUCCESS;
 }

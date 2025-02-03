" Vim syntax file
" Language:    Snek
" Maintainer:  Rauli Laine <rauli.laine@iki.fi>
" Last Change: 2025 Feb 3

if exists("b:current_syntax")
  finish
endif

" Syntax: Comments
syn keyword snekTodo FIXME NOTE NOTES TODO XXX contained
syn match snekComment "#.*$" contains=snekTodo

" Syntax: Reserved keywords
syn keyword snekBoolean false true
syn keyword snekSpecial null
syn keyword snekStatement break continue pass return type
syn keyword snekConditional else if
syn keyword snekRepeat while
syn keyword snekInclude as export from import
syn keyword snekVariable let const

" Syntax: String literals
syn match snekStringEscape "\\["'\\/btnfr]" contained
syn match snekStringEscape "\\u\x\{4}" contained
syn region snekString start=/"/ end=/"/ contains=snekStringEscape
syn region snekString start=/'/ end=/'/ contains=snekStringEscape

" Syntax: Number literals
syn match snekNumber /\<[+-]\?\d\+\(.\d\+\)\?\([eE]\d\+\([+-]\d\+\)\?\)\?\>/

hi def link snekStatement Statement
hi def link snekConditional Conditional
hi def link snekRepeat Repeat
hi def link snekInclude Include
hi def link snekVariable Identifier
hi def link snekStringEscape Special
hi def link snekComment Comment
hi def link snekTodo Todo
hi def link snekBoolean Boolean
hi def link snekSpecial Constant
hi def link snekString String
hi def link snekNumber Number

let b:current_syntax = "snek"

" Vim syntax file
" Language:    Snek
" Maintainer:  Rauli Laine <rauli.laine@iki.fi>
" Last Change: 2020 Sep 02

if exists("b:current_syntax")
  finish
endif

" Syntax: Reserved keywords
syn keyword snekBoolean false true
syn keyword snekStatement break continue null return
syn keyword snekConditional else if
syn keyword snekRepeat while
syn keyword snekInclude as export from import

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
hi def link snekStringEscape Special
hi def link snekString String
hi def link snekBoolean Boolean

let b:current_syntax = "snek"

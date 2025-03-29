" Language:   Weft
" Maintainer: Alice Uu <alice@owl.ooo>
" Filenames:  *.weft,*.wft
" URL:        https://www.github.com/auul/weft

if exists('b:current_syntax')
	finish
endif

syn region weftComment start="#" end="$"
syn match weftNumber "\d\+\(\.\d\+\)\?\|\.\d\+"
syn region weftChar start="'" skip="\\\'" end="'"
syn region weftString start="\"" skip="\\\"" end="\""
syn region weftShuffle start="{" end="}"
syn match weftDefine "\S\+:"

hi def link weftComment Comment
hi def link weftNumber Constant
hi def link weftChar Constant
hi def link weftString Constant
hi def link weftShuffle Identifier
hi def link weftDefine Statement

let b:current_syntax = 'weft'

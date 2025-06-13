# Kalicomp
a barebones compiler implementation for the kaleidoscope language

Notes
1. kaleidoscope by default infers every variable to be of types double
1. Lexer takes text as input and breaks it into tokens along with token related metadata
2. invalid tokens if found will return ascii value  by lexer
3. AST captures behaviour such that's easier for the forthcoming stages of the compiler we thus would like to have one object for every construct in the language
4. in  kaleidoscope we have expression a prototype and a func object


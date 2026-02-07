obj-y := \
	rc.o \
	parser.tab.o \
	lexer.lex.o

# additional dependencies for generated headers
$(loc_build_tree)/rc.o: $(loc_build_tree)/parser.tab.c
$(loc_build_tree)/parser.tab.o: $(loc_build_tree)/lexer.lex.o
$(loc_build_tree)/lexer.lex.o: $(loc_build_tree)/parser.tab.c

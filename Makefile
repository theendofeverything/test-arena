CFLAGS := -Wall -Wextra -pedantic -std=c2x

EXE := test_MG_membuff
SRC := src/$(EXE).c
EXE := build/$(EXE)

$(EXE): $(SRC) | build
	$(CC) $(CFLAGS) $^ -o $@

build: ; @mkdir build

what-CC: ; @realpath `which $(CC)`

.PHONY: tags
tags:
	ctags -R .
	cscope -R -b

clean:
	-@rm $(EXE)

run: $(EXE) ; ./$(EXE)


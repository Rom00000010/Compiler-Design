CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -std=c99 -g

CFILES = $(shell find ./ -name "*.c")
OBJS = $(CFILES:.c=.o)
LFILE = $(shell find ./ -name "*.l")
YFILE = $(shell find ./ -name "*.y")
LFC = $(shell find ./ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
YFC = $(shell find ./ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
LFO = $(LFC:.c=.o)
YFO = $(YFC:.c=.o)

parser: syntax $(filter-out $(LFO),$(OBJS))
	$(CC) $(CFLAGS) $(filter-out $(LFO),$(OBJS)) -lfl -ly -o parser

syntax: lexical syntax-c
	$(CC) -c $(YFC) $(CFLAGS) -o $(YFO)

lexical: $(LFILE)
	$(FLEX) -o $(LFC) $(LFILE)

syntax-c: $(YFILE)
	$(BISON) -o $(YFC) -d -v $(YFILE)

-include $(patsubst %.o, %.d, $(OBJS))

# 定义的一些伪目标
.PHONY: clean test zip
test:
	./parser ../Test/test1.cmm
clean:
	rm -f parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
	rm -f *~
zip: clean
	@echo "Creating Code directory and copying files..."
	mkdir -p Code
	mkdir -p Code/include
	mkdir -p Code/src
	cp Makefile *.c *.l *.y Code/src
	cp -r ../include/ Code/
	@echo "Copying report files..."
	cp ../report/*.pdf ../report/README .
	@echo "Zipping files into submit.zip..."
	zip -r submit.zip Code README *.pdf
	@echo "Cleaning up..."
	rm -rf Code README *.pdf
debug:
	gdb -x ./init.gdb
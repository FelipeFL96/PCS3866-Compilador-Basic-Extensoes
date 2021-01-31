COMPILER = g++
FLAGS = -c -g -std=c++11

OBJPATH = bin

LEXSRC = lexic/sources
LEXHDR = lexic/headers
LEXOBJ = $(OBJPATH)/lexic

STXHDR = syntax/headers
STXSRC = syntax/sources
STXOBJ = $(OBJPATH)/syntax

SMTHDR = semantic/headers
SMTSRC = semantic/sources
SMTOBJ = $(OBJPATH)/semantic

GENHDR = generation/headers
GENSRC = generation/sources
GENOBJ = $(OBJPATH)/generation

MAINSRC = main/sources
MAINHDR = "main/headers"
MAINOBJ = $(OBJPATH)/main

SOURCES = \
	$(wildcard $(GENSRC)/*.cpp)\
	$(wildcard $(SMTSRC)/*.cpp)\
	$(wildcard $(STXSRC)/*.cpp)\
	$(wildcard $(LEXSRC)/*.cpp)\
	$(wildcard $(MAINSRC)/*.cpp)
HEADERS = \
	$(wildcard $(LEXHDR)/*.hpp)\
	$(wildcard $(STXHDR)/*.hpp)\
	$(wildcard $(SMTHDR)/*.hpp)\
	$(wildcard $(GENHDR)/*.hpp)\
	$(wildcard $(MAINHDR)/*.hpp)
OBJECTS = \
	$(subst .cpp,.o,\
	$(subst $(LEXSRC),$(LEXOBJ),\
	$(subst $(STXSRC),$(STXOBJ),\
	$(subst $(SMTSRC),$(SMTOBJ),\
	$(subst $(GENSRC),$(GENOBJ),\
	$(subst $(MAINSRC),$(MAINOBJ),\
	$(SOURCES)))))))


all: basicc

basicc: $(OBJECTS)
	$(COMPILER) $^ -o $(OBJPATH)/$@
	cp $(OBJPATH)/$@ ./$@

./$(MAINOBJ)/%.o: ./$(MAINSRC)/%.cpp
	mkdir -p $(MAINOBJ)
	$(COMPILER) $(FLAGS) -I $(LEXHDR) -I $(STXHDR) -I $(SMTHDR) -I $(GENHDR) -I $(MAINHDR) -o $@ $^

./$(LEXOBJ)/%.o: ./$(LEXSRC)/%.cpp ./$(LEXHDR)/%.hpp
	mkdir -p $(LEXOBJ)
	$(COMPILER) $(FLAGS) -I $(LEXHDR) -o $@ $<

./$(STXOBJ)/%.o: ./$(STXSRC)/%.cpp ./$(STXHDR)/%.hpp
	mkdir -p $(STXOBJ)
	$(COMPILER) $(FLAGS) -I $(LEXHDR) -I $(STXHDR) -o $@ $<

./$(SMTOBJ)/%.o: ./$(SMTSRC)/%.cpp ./$(SMTHDR)/%.hpp
	mkdir -p $(SMTOBJ)
	$(COMPILER) $(FLAGS) -I $(LEXHDR) -I $(STXHDR) -I $(SMTHDR) -I $(GENHDR) -o $@ $<

./$(GENOBJ)/%.o: ./$(GENSRC)/%.cpp ./$(GENHDR)/%.hpp
	mkdir -p $(GENOBJ)
	$(COMPILER) $(FLAGS) -I $(LEXHDR) -I $(STXHDR) -I $(SMTHDR) -I $(GENHDR) -o $@ $<

clean:
	rm -rf bin/*
	rm -f basicc
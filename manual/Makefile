SRC=installation
EXT=md

all: wiki pdf odt

pdf: $(SRC).$(EXT)
	pandoc -s --toc -i -t latex --template=default.latex --variable documentclass=book $(SRC).$(EXT) -o $(SRC).tex
	sed -i 's/\[<+->\]//' $(SRC).tex
	pdflatex $(SRC).tex
	pdflatex $(SRC).tex
	evince $(SRC).pdf

odt: $(SRC).$(EXT)
	pandoc --toc -s -t odt $(SRC).$(EXT) -o $(SRC).odt

wiki: $(SRC).$(EXT)
	pandoc -s -S -w mediawiki --toc $(SRC).$(EXT) -o $(SRC).wiki

clean:
	rm -f $(SRC).toc $(SRC).snm $(SRC).out $(SRC).nav $(SRC).log $(SRC).aux $(SRC).vrb $(SRC).pdf $(SRC).tex $(SRC).wiki $(SRC).odt

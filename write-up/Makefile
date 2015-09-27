SHELL = /bin/bash

all: template.pdf

template.pdf: *.tex *.bib 
	pdflatex template.tex
	bibtex template 
	touch template.tex
	pdflatex template.tex
	pdflatex template.tex

short: *.tex *.bib 
	pdflatex template.tex

clean:
	if (rm -f *.log *.aux *.bbl *.blg *.toc *.dvi *.ps *.brf *.lbl *pdf) then :; fi


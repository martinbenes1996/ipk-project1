
all: dokumentace.pdf

dokumentace.pdf:
		pdflatex dokumentace.tex
		biber dokumentace
		pdflatex dokumentace.tex
		pdflatex dokumentace.tex

.PHONY: clean
clean:
		rm dokumentace.pdf

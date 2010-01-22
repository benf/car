DOC_FILE=documentation
gen_sources:
	@./gen_sources.sh
doc: gen_sources
	pdflatex -output-format=pdf $(DOC_FILE).tex
clean:
	rm $(DOC_FILE).log $(DOC_FILE).pdf $(DOC_FILE).toc $(DOC_FILE).aux


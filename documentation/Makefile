DOC_FILE=documentation
doc:
	@./gen_sources.sh
	pdflatex -output-format=pdf $(DOC_FILE).tex
clean:
	rm $(DOC_FILE).log $(DOC_FILE).pdf $(DOC_FILE).toc $(DOC_FILE).aux


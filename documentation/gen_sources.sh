#!/bin/bash
for file in ../*.{c,h} ../Makefile ../pc/Makefile ../pc/joystick.c
do
	name=$(basename $file)
	echo '\lstset{language=C}' > $name.tex
	echo '\begin{lstlisting}' > $name.tex
	cat $file >> $name.tex
	#sed -i 's/[#&]/\\\0/g' $name.tex
	echo '\end{lstlisting}' >> $name.tex
done

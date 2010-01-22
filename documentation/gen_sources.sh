#!/bin/bash
echo -n > sources.tex
for file in ../*.{c,h} ../Makefile ../pc/joystick.c
do
	
	name=$(basename $file)
	echo '\newpage' >> sources.tex
	echo '\subsection{'$(echo $name | sed 's/_/\\\_/g')'}' >> sources.tex
	echo '\lstinputlisting{'$file'}' >> sources.tex


	#echo '\lstset{language=C}' > $name.tex
	#echo '\begin{lstlisting}' > $name.tex
	#cat $file >> $name.tex
	#sed -i 's/[#&]/\\\0/g' $name.tex
	#echo '\end{lstlisting}' >> $name.tex
done

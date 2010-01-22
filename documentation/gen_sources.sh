#!/bin/bash
for file in ../*.{c,h} ../Makefile ../pc/Makefile ../pc/joystick.c
do
	name=$(basename $file)
	echo "\lstset{language=C}" > $name.tex
	cat $file >> $name.text
	echo "\end{lstlisting}" >> $name.tex
done

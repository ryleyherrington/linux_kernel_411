#########################################################
#Original Author: Ryley Herrington and D. Kevin McGrath #
#Made October 22, 2012, 9:00am							#
#File: Makefile											#
#														#
#This is the makefile associated with Project 2			#
#########################################################

TARGET=template
HTML=main_html
CC_OPTIONS=-O

default: pdf compile

both: pdf html

dvi: ${TARGET}.tex 
#	pygmentize -f altex -o __${SRC}.tex ${SRC}
#	run latex twice to get references correct
	latex ${TARGET}.tex
	latex $(TARGET).tex
#	rm __${SRC}.tex

ps: dvi
	dvips -R -Poutline -t letter ${TARGET}.dvi -o ${TARGET}.ps

pdf: ps
	ps2pdf ${TARGET}.ps

html:
	cp ${TARGET}.tex ${HTML}.tex
	latex ${HTML}.tex
	latex2html -split 0 -show_section_numbers -local_icons -no_navigation ${HTML}

	sed 's/<\/SUP><\/A>/<\/SUP><\/A> /g' < ${HTML}/index.html > ${HTML}/index2.html
	mv ${HTML}/index2.html ${HTML}/index.html
	rm ${HTML}.*

compile:
	icpc -o file.c #put here whatever you usually use to compile. Make sure there is a tab at the beginning of the line.

clean:
	rm -f *.o *.html *.pdf *.log *.ps *.dvi *.out *.aux




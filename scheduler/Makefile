#########################################################
#Original Author: Ryley Herrington and D. Kevin McGrath #
#Made January 19th, 2012, 4:00pm			#
#File: Makefile						#
#Last Modified: January 23rd, 2012, 9:00pm, herringr	# 
#							#
#This is the makefile associated with Project 1		#
#########################################################

TARGET=template
HTML=main_html
CC_OPTIONS=-O

default: pdf copyprog

both: pdf html

dvi: ${TARGET}.tex 
#	run latex twice to get references correct
	latex ${TARGET}.tex
#	you can also have a bibtex line here
#	bibtex $(TARGET).tex
	latex $(TARGET).tex

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

copyprog: copyprog.c
        $(CC) -Wall -std=c99 $(CC_OPTIONS) -o $@ copyprog.c

test: copyprog
	time copyprog /tmp/foo /tmp/fee 1
	time copyprog /tmp/foo /tmp/fee 10
	time copyprog /tmp/foo /tmp/fee 100
	time copyprog /tmp/foo /tmp/fee 1000
	time copyprog /tmp/foo /tmp/fee 10000

clean:
	rm -f copyprog *.o core a.out




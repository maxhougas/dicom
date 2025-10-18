source := $(wildcard source/*.c)
#source/dcmdirectory.c source/dcmelement.c source/dcmendian.c source/dcmezbuff.c source/dcmlog.c source/dcmname.c source/dcmoutput.c source/dcmproc.c source/dcmspecialtag.c source/dcmtree.c source/dcmtypes.c source/dcmutil.c source/hougasargs.c source/sqtags.c source/soptable.c

all: dcmproc

dcmproc: $(source)
	@echo 'Compiling dcmproc'
	if [ -f /usr/include/dirent.h ]; then dirent="-D USEDIRENT=1"; fi &&\
	if [ -f /usr/include/stdint.h ]; then stdint="-D USESTDINT=1"; fi &&\
	gcc -Wall -Werror -ansi -Ofast $$dirent $$stdint -o dcmproc source/dcmproc.c

tmp/part6table.htm:
	@echo 'Grabbing html from https://dicom.nema.org/.../chtml/part6/chapter_{{7..9},6}.html'
	bash -c 'curl -s https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_{{7..9},6}.html' > tmp/part6table.htm

tmp/part6table-win.htm: tmp
	@echo 'WINDOWS: grabbing html from ../chtml/part6/chapter_{{7..9},6}.html'
	wget https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_7.html -outfile tmp/part7.htm
	wget https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_8.html -outfile tmp/part8.htm
	wget https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_9.html -outfile tmp/part9.htm
	wget https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_6.html -outfile tmp/part6.htm
	get-content part7.htm part8.htm part9.htm part6.htm | set-content tmp/part6table-win.htm

tmp/thetable: tmp/part6table.htm
	@echo 'Stripping HTML from part6table.htm'
	sed -z 's:\n *:__:g' tmp/part6table.htm |\
	grep -Po '<tbody>.*?</tbody>' |\
	grep -Po '<tr.*?</tr>' |\
	sed 's:<[^>]*>:___:g; s:^__*::; s: :_:g; s:\xe2\x80\x8b::g; s:&amp;:\\\\\\\&:g; s:(\|)\|'\'':\\\\\\&:g' |\
	awk -F '___+' -v OFS='___' '{gsub(/\\*\)|,/,"",$$1); gsub(/x/,"{{0..9},{A..F}}",$$1); gsub(/\\*\(/,"0x",$$1); print $$1,$$2,$$3,$$4,$$5}' |\
	while read line; do bash -c "echo `echo $$line`"; done |\
	sed 's: :\n:g; s:_: :g; s:   :  :g;' > tmp/thetable

source/sqtags.c: tmp/thetable
	@echo 'Selecting SQs from tmp/thetable'
	grep SQ tmp/thetable |\
	awk -F '  ' 'BEGIN{print "#ifndef DCMTYPES\n#include \"dcmtypes.c\"\n#endif\n\nconst byte4 SQTAGS[] =\n{"} {print $$1","}' |\
	sed '$$s:,$$:\n};\n\nconst int NSQTAGS = (sizeof(SQTAGS)/sizeof(byte4));:' > source/sqtags.c

source/thetable.c: tmp/thetable
	@echo 'Unrolling thetable'
	awk -F '  ' 'BEGIN{print "#ifndef _DCMTYPES\n#include \"dcmtypes.c\"\n#endif\n\nconst byte4 ALLTAGS[] =\n{"} {print $$1","}' tmp/thetable |\
	sed '$$s:,:\n};:' > source/thetable.c
	awk -F '  ' 'BEGIN{print "\n\nconst char *ALLNAMES[] = \n{"} {print "\""$$2"\","}' tmp/thetable |\
	sed '$$s:,:\n};:' >> source/thetable.c
	awk -F '  ' 'BEGIN{print "\n\nconst char *ALLKEYWORDS[] = \n{"} {print "\""$$3"\","}' tmp/thetable |\
	sed '$$s:,:\n};:' >> source/thetable.c
	awk -F '  ' 'BEGIN{print "\n\nconst char *ALLVRS[] = \n{"} {print "\""$$4"\","}' tmp/thetable |\
	sed '$$s:,:\n};:' >> source/thetable.c
	awk -F '  ' 'BEGIN{print "\n\nconst char *ALLVMS[] = \n{"} {print "\""$$5"\","}' tmp/thetable |\
	sed '$$s:,:\n};\n:' >> source/thetable.c
	echo 'const void *THETABLE[] = {ALLTAGS, ALLNAMES, ALLKEYWORDS, ALLVRS, ALLVMS};' >> source/thetable.c
	echo 'const int NTHETABLE = (sizeof(ALLTAGS)/sizeof(byte4));' >> source/thetable.c

source/soptable.c: tmp/soptable
	@echo 'Unrolling soptable'
	awk 'BEGIN{print "const char *SOPCLASSNAME[] =\n{"} {print "\""$$1"\","}' tmp/soptable |\
	sed '$$s:,:\n};\n:' > source/soptable.c
	awk 'BEGIN{print "const char *SOPCLASSUID[] =\n{"} {print "\""$$2"\","}' tmp/soptable |\
	sed '$$s:,:\n};\n:' >> source/soptable.c
	echo 'const unsigned int NSOP = sizeof(SOPCLASSNAME)/sizeof(char*);' >> source/soptable.c

tmp/soptable: tmp/part4sectB5.htm
	@echo 'Stripping HTML from part4secB5.htm'
	sed -z 's:\n:__:g' part4sectB5.htm |\
	grep -Po '<tbody>.*?</tbody>' |\
	grep -Po '<tr.*?</tr>' |\
	sed 's:<[^>]*>:___:g; s:  *:_:g;' |\
	awk -F '___+' '{print $$2,$$3}' > tmp/soptable

tmp/part4sectB5.htm:
	@echo 'Grabbing SOP class UID table from https://dicom.nema.org/.../chtml/part04/sect_B.5.html'
	curl -s -o tmp/part4sectB5.htm 'https://dicom.nema.org/medical/dicom/current/output/chtml/part04/sect_B.5.html'

clean:
	@echo 'Cleaning, but not removing tmp/*.htm'
	rm dcmproc tmp/thetable tmp/soptable source/sqtags.c source/thetable.c source/soptable.c

clean-win:
	@echo 'WINDOWS: cleaning'
	rm tmp/part7.htm tmp/part8.htm tmp/part9.htm tmp/part6.htm tmp/part6table-win.htm

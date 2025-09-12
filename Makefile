all: dcmproc source/sqtags.c

dcmproc: source/dcmelement.c source/dcmendian.c source/dcmezbuff.c source/dcmproc.c source/dcmtypes.c source/dcmspecialtag.c source/hougasargs.c
	echo 'Compiling dcmproc'
	gcc -ansi -o dcmproc source/dcmproc.c

tmp/part6table.htm: tmp
	echo 'grabbing html from .../chtml/part6/chapter_{{7..9},6}.html'
	bash -c 'curl -s https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_{{7..9},6}.html' > tmp/part6table.htm

tmp/part6table-win.htm: tmp
	echo 'WINDOWS: grabbing html from ../chtml/part6/chapter_{{7..9},6}.html'
	wget https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_7.html -outfile tmp/part7.htm
	wget https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_8.html -outfile tmp/part8.htm
	wget https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_9.html -outfile tmp/part9.htm
	wget https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_6.html -outfile tmp/part6.htm
	get-content part7.htm part8.htm part9.htm part6.htm | set-content tmp/part6table-win.htm

source/sqtags.c: tmp/part6table.htm
	echo 'stripping a lot of stuff akshewally'
	sed -z 's:\n *:!!:g; s:</tr>:</tr>\n:g' tmp/part6table.htm |\
	grep '<tr.*SQ.*</tr>' |\
	sed 's:).*:,:; s:,::; s:x:{{0..9},{A..F}}:g; s:.*(:0x:' |\
	while read line; do bash -c "echo `echo $$line`"; done |\
	sed 's:^0: 0:; s:, :,\n :g; 1s:^:#ifndef _DCMTYPES\n#include "dcmtypes.c"\n#endif\n\nconst byte4 SQTAGS[] =\n{\n:; $$s:,$$:\n};\n\nconst int NSQTAGS = (sizeof(SQTAGS)/sizeof(byte4));:' > source/sqtags.c

tmp/thetable: tmp tmp/part6table.htm
	sed -z 's:\n *:__:g' tmp/part6table.htm |\
	grep -Po '<tbody>.*?</tbody>' |\
	grep -Po '<tr.*?</tr>' |\
	sed 's:<[^>]*>:___:g; s:^__*::; s: :_:g; s:\xe2\x80\x8b::g; s:&amp;:\\\\\\\&:g; s:(\|)\|'\'':\\\\\\&:g' |\
	awk -F '___+' -v OFS='___' '{gsub(/\\*\)|,/,"",$$1); gsub(/x/,"{{0..9},{A..F}}",$$1); gsub(/\\*\(/,"0x",$$1); print $$1,$$2,$$3,$$4,$$5}' |\
	while read line; do bash -c "echo `echo $$line`"; done |\
	sed 's: :\n:g; s:_: :g; s:   :  :g;' > tmp/thetable

source/thetable.c: tmp/thetable
	awk -F '  ' 'BEGIN{print "#ifndef _DCMTYPES\n#include \"dcmtypes.c\"\n#endif\n\nconst byte4 ALLTAGS[] =\n{"} {print $$1","}' tmp/thetable |\
	sed '$$s:,:;\n};:' > source/thetable.c
	awk -F '  ' 'BEGIN{print "\n\nconst char *ALLNAMES[] = \n{"} {print "\""$$2"\","}' tmp/thetable |\
	sed '$$s:,:;\n};:' >> source/thetable.c
	awk -F '  ' 'BEGIN{print "\n\nconst char *ALLKEYWORDS[] = \n{"} {print "\""$$3"\","}' tmp/thetable |\
	sed '$$s:,:;\n};:' >> source/thetable.c
	awk -F '  ' 'BEGIN{print "\n\nconst char *ALLVRS[] = \n{"} {print "\""$$4"\","}' tmp/thetable |\
	sed '$$s:,:;\n};:' >> source/thetable.c
	awk -F '  ' 'BEGIN{print "\n\nconst char *ALLVMS[] = \n{"} {print "\""$$5"\","}' tmp/thetable |\
	sed '$$s:,:;\n};:' >> source/thetable.c
	echo -e '\nconst void *THETABLE[] = {ALLTAGS, ALLNAMES, ALLKEYWORDS, ALLVRS, ALLVMS}' >> source/thetable.c
	echo -e 'const int NTHETABLE = (sizeof(ALLVMS)/sizeof(byte4));' >> source/thetable.c

tmp:
	mkdir tmp

clean:
	echo 'cleaning'
	rm dcmproc tmp/part6table.htm source/sqtags.c

clean-win:
	echo 'WINDOWS: cleaning'
	rm tmp/part7.htm tmp/part8.htm tmp/part9.htm tmp/part6.htm tmp/art6table-win.htm

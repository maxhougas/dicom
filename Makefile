all: dcmproc source/sqtags.c

dcmproc: dcmproc.c hougasargs.c
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
	sed -z 's:\n *:!!:g; s:</tr>:</tr>\n:g' part6table.htm |\
	grep '<tr.*SQ.*</tr>' |\
	sed 's:).*:,:; s:.*(:0x:; s:,::; 1s:^:const int SQTAGS[] =\n{\n:; $$s:,:\n}:;' |\
	while read line; do bash -c "echo `echo $$line | sed 's:xx:{{0..9},{A..F}}{{0..9},{A..F}}:'`"; done < /dev/stdin |\
	sed 's:^0: 0:; s:, :,\n :g; $$s:$$:;:' > source/sqtags.c

tmp:
	mkdir tmp

clean:
	echo 'cleaning'
	rm dcmproc tmp/part6table.htm source/sqtags.c

clean-win:
	echo 'WINDOWS: cleaning'
	rm tmp/part7.htm tmp/part8.htm tmp/part9.htm tmp/part6.htm tmp/art6table-win.htm

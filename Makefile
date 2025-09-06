all: dcmproc source/sqtags.c

dcmproc: dcmproc.c hougasargs.c
	echo 'Compiling dcmproc'
	gcc -ansi -o dcmproc source/dcmproc.c

tmp/part6table.htm:
	echo 'grabbing html from .../chtml/part6/chapter_{{7..9},6}.html'
	mkdir -p tmp
	bash -c 'curl -s https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_{{7..9},6}.html' > tmp/part6table.htm

source/sqtags.c: tmp/part6table.htm
	echo 'stripping a lot of stuff akshewally'
	sed -z 's:\n *:!!:g; s:</tr>:</tr>\n:g' part6table.htm |\
	grep '<tr.*SQ.*</tr>' |\
	sed 's:).*:,:; s:.*(:0x:; s:,::; 1s:^:const int SQTAGS[] =\n{\n:; $$s:,:\n}:;' |\
	while read line; do bash -c "echo `echo $$line | sed 's:xx:{{0..9},{A..F}}{{0..9},{A..F}}:'`"; done < /dev/stdin |\
	sed 's:^0: 0:; s:, :,\n :g; $$s:$$:;:' > source/sqtags.c

clean:
	echo 'cleaning'
	rm dcmproc tmp/part6table.htm source/sqtags.c

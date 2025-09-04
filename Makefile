all: dcmproc sqs

dcmproc: dcmproc.c hougasargs.c
	echo 'Compiling dcmproc'
	gcc -ansi -o dcmproc dcmproc.c

sqs: pulltable.sh
	echo 'pulling tables from dicom.nema.org'
	./pulltable.sh

clean:
	echo 'cleaning'
	rm dcmproc part6table.htm tagtable sqs

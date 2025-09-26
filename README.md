<a top="top"/>

# DICOM
DICOM parser in ANSI compliant C
- Parses DICOM files into an array or array of trees of dcmels
- Renders that array as a YAML JSON, or CSV file
- Capable of processing multiple files in batch
- Makefile contains scripts which download tables from [dicom.nema.org](https://dicom.nema.org/mediacl/current/output), and generate .c files
- More than 99% of the code (by lines) is procedurally generated!

###### [Go to ToC](#table-of-contents)

## Todo
### Basic functionality
- [x] Create functionality to pull sqs from dicom.nema.org
- [x] Test issq functionality
- [x] Add single-pass functionality for tag-parsing
- [x] Add second-pass functionality for tree node hanging
- [x] Add functinality to dump data
  - [x] CSV
  - [x] JSON
  - [x] YAML
- [x] Support stdin and stdout
- [x] Fix CLI arg processing
- [ ] Tag -> English dictionary
  - [x] Make THE TABLE
  - [ ] Search THE TABLE
- [x] Add functionality for batch file processing
- [ ] Add functionality for serach/edit
### Code Organization
- [ ] Break subfunctionality into separate files
  - [x] dcmezbuff
  - [ ] dcmsmartbuff
    - [x] Back burner dcmsmartbuff
  - [x] dcmendian
  - [x] dcmspecialtag
  - [x] dcmtypes
  - [ ] logging
  - [x] dcmoutput
- [x] Pull pulltable.sh functionality into Makefile
### Meta stuff
- [x] Create develop branch
- [ ] Update README
- [x] Deal with types
  - [x] Decide whether to proceed with byte1 byte2 byte4 or use char short int (going with bytex)
  - [x] FIND EVERYWHERE TO CHANGE TYPES TO BYTEx!!! :) :) !! :| :| :) !!!!
- [x] Enable logging to file
- [x] Clean up dcmoutput.c redundant code

## Table of Contents
- [Todo](#todo)
- [Table of Contents](#top)
- [Usage](#usage)
- [Description](#description)
- [Building](#building)

## Usage
```
-h, --help    : this
-v, --version : version info (build date)
-c, --csv     : output in CSV format
    --CSV
-f, --file    : file to process; stdin is default
    --input
-j, --json    : output in JSON format
    --JSON
-l, --log     : logfile (append); some errors are printed to stderr anyway
                default is stderr
-o, --output  : file to write to (kablam!) stdout is default
-p, --prefix  : input file prefix
-r, --recurse : engage recursive mode; hang children
    --tree
-y, --yaml    : output in YAML format (default)
    --YAML
```
- Example DICOMs can be found at [3dicomviewer](https://3dicomviewer.com/dicom-library)
- Better examples can be found at [SlicerRtData](https://github.com/SlicerRt/SlicerRtData)
  - eclipse-8.1.20-phantom-breast/Original/RI.1.2.246.352.71.3.2088656855.2377794.20110920152340.dcm was used in testing
- A typical run line would be `./dcmproc -f "cooldicom.dcm\nanotherdicom.dcm" -jro cool.json`
  - This will parse the file `cooldicom.dcm`.
  - The internal representation of `cooldicom.dcm` will be recursively processed.
  - The file `cool.json` will be created if it does not exist or truncated if it does.
  - A JSON representation of the data from `cooldicom.dcm` will be written to `cool.json`.
  - The file `anotherdicom.dcm` will be parsed.
  - The internal representation of `anotherdicom.dcm` will be recursively processed.
  - A JSON representation of the data from `anotherdicom.dcm` will be appended to `cool.json`.
- This program is not capable of ennumerating a directory. If external means are used to do so, processing an entire directory is possible.
  - Such a line could be `./dcmproc -p path/to/dicoms/ -f "$(ls -1 path/to/dicoms)" -yro out.yml -l logfile`

###### [Go to Top](#top)
###### [Go to ToC](#table-of-contents)

## Description
- This program assumes 8-bit bytes
- This program will read and parse dicom files representing them internally as an array of elements.
- The element array can be "recursed" with the -r flag; childable nodes (those of VR SQ or tag 0xFFFEE000) will have their children arranged in a tree structure.
- Elements are maintained in a way that the original file is recoverable from it's representation.
- The representation can be written to a YAML or JSON format file with the -y or -j flags.
- If the represention remains flat i.e. the -r flag is not issued, it can be written to a CSV format file.
- Multiple DICOM files can be processed in batch by issuing a quoted, \n-delimited list to the -f flag.

###### [Go to Top](#top)
###### [Go to ToC](#table-of-contents)

## Building
  - Have make
  - Run `make`
  - Building tmp/part6table.htm will require interwebs

###### [Go to Top](#top)
###### [Go to ToC](#table-of-contents)

### The Table
- Makefile containes script equivalent to the following
```
echo 'grabbing html from .../chtml/part6/chapter_{{7..9},6}.html'
bash -c 'curl -s https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_{{7..9},6}.html' > part6table.htm

echo 'Stripping HTML from part6table.htm'
sed -z 's:\n *:__:g' part6table.htm |\
grep -Po '<tbody>.*?</tbody>' |\
grep -Po '<tr.*?</tr>' |\
sed 's:<[^>]*>:___:g; s:^__*::; s: :_:g; s:\xe2\x80\x8b::g; s:&amp;:\\\\\\\&:g; s:(\|)\|'\'':\\\\\\&:g' |\
awk -F '___+' -v OFS='___' '{gsub(/\\*\)|,/,"",$1); gsub(/x/,"{{0..9},{A..F}}",$1); gsub(/\\*\(/,"0x",$1); print $1,$2,$3,$4,$5}' |\
while read line; do bash -c "echo `echo $line`"; done |\
sed 's: :\n:g; s:_: :g; s:   :  :g;' > thetable

echo 'Unrolling thetable'
awk -F '  ' 'BEGIN{print "#ifndef _DCMTYPES\n#include \"dcmtypes.c\"\n#endif\n\nconst byte4 ALLTAGS[] =\n{"} {print $1","}' tmp/thetable |\
sed '$s:,:\n};:' > thetable.c
awk -F '  ' 'BEGIN{print "\n\nconst char *ALLNAMES[] = \n{"} {print "\""$2"\","}' thetable |\
sed '$s:,:\n};:' >> thetable.c
awk -F '  ' 'BEGIN{print "\n\nconst char *ALLKEYWORDS[] = \n{"} {print "\""$3"\","}' thetable |\
sed '$s:,:\n};:' >> thetable.c
awk -F '  ' 'BEGIN{print "\n\nconst char *ALLVRS[] = \n{"} {print "\""$4"\","}' thetable |\
sed '$s:,:\n};:' >> thetable.c
awk -F '  ' 'BEGIN{print "\n\nconst char *ALLVMS[] = \n{"} {print "\""$5"\","}' thetable |\
sed '$s:,:\n};\n:' >> thetable.c
echo 'const void *THETABLE[] = {ALLTAGS, ALLNAMES, ALLKEYWORDS, ALLVRS, ALLVMS};' >> thetable.c
echo 'const int NTHETABLE = (sizeof(ALLTAGS)/sizeof(byte4));' >> thetable.c
```
1. HTML is pulled from [dicom.nema.org](https://dicom.nema.org)
2. HTML tags are stripped yielding a plaintext table
3. Tag ranges indicated with 'x's are prepped for brace expansion
4. Brace expansion
5. The Table is unrolled into parallel constant arrays in compilable C.

###### [Go to Top](#top)
###### [Go to ToC](#table-of-contents)

### Linux
- Makefile assumes you have access to BASH--brace expansion is used excessively

###### [Go to Top](#top)
###### [Go to ToC](#table-of-contents)

### Windows
- Currently Makefile assumes access to GCC.
- The code should be platform independent, but you'll need to generate source/sqtags.c on your own.
  - Generated from tables @:
    - [chapter 7](https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_7.html)
    - [chapter 8](https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_8.html)
    - [chapter 9](https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_9.html)
    - [chapter 6](https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_6.html)
  - constant byte4 array SQTAGS
  - constant int NSQTAGS

###### [Go to Top](#top)
###### [Go to ToC](#table-of-contents)

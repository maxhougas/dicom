<a top="top"/>

# DICOM
DICOM parser in C
-More than half of the code (by lines) is procedurally generated!

[toc](#table-of-contents)

## Todo
### Basic functionality
- [x] Create functionality to pull sqs from dicom.nema.org
- [ ] Test issq functionality
- [x] Add single-pass functionality for tag-parsing
- [ ] Add second-pass functionality for tree node hanging
- [x] Add functinality to dump data
  - [x] CSV
  - [x] JSON
  - [x] YAML
- [x] Support stdin and stdout
- [x] Fix CLI arg processing
- [ ] Tag -> English dictionary
### Code Organization
- [ ] Break subfunctionality into separate files
  - [x] dcmezbuff
  - [ ] dcmsmartbuff
  - [x] dcmendian
  - [x] dcmspecialtag
  - [x] dcmtypes
  - [ ] logging
  - [ ] output
- [x] Pull pulltable.sh functionality into Makefile
### Meta stuff
- [x] Create develop branch
- [ ] Update README
- [x] Deal with types
  - [x] Decide whether to proceed with byte1 byte2 byte4 or use char short int (going with bytex)
  - [x] FIND EVERYWHERE TO CHANGE TYPES TO BYTEx!!! :) :) !! :| :| :) !!!!
- [x] Enable logging to file

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
-c, --csv     : output in CSV format (default)
    --CSV
-f, --file    : file to process stdin is default
    --input
-j, --json    : output in JSON format
    --JSON
-l, --log     : logfile (append); some errors are printed to stderr anyway
                default is stderr
-o, --output  : file to write to (kablam!) stdout is default
-y, --yaml    : output in YAML format
    --YAML
```
- Example DICOMs can be found at [3dicomviewer](https://3dicomviewer.com/dicom-library)

[top](#top) [toc](#table-of-contents)

## Description
- This program assumes 8-bit bytes

[top](#top) [toc](#table-of-contents)

## Building

[top](#top) [toc](#table-of-contents)

### Linux
- Makefile assumes you have access to BASH--shell expansion is used excessively

[top](#top) [toc](#table-of-contents)

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

[top](#top) [toc](#table-of-contents)

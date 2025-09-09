<a top="top"/>

# DICOM
DICOM parser in C

## Todo
### Basic functionality
- [x] Create functionality to pull sqs from dicom.nema.org
- [x] Test issq functionality
- [x] Add single-pass functionality for tag-parsing
- [ ] Add second-pass functionality for tree node hanging
- [ ] Add functinality to dump data
  - [x] CSV
  - [ ] JSON
### Code Organization
- [ ] Break subfunctionality into separate files
  - [x] dcmbuffer(dcmezbuff only)
  - [ ] dcmsmartbuff
  - [x] dcmendian
  - [x] dcmspecialtag
  - [x] dcmtypes
- [x] Pull pulltable.sh functionality into Makefile
### Meta stuff
- [x] Create develop branch
- [ ] Update README
- [ ] Deal with types
  - [x] Decide whether to proceed with byte1 byte2 byte4 or use char short int (going with bytex)
  - [ ] FIND EVERYWHERE TO CHANGE TYPES TO BYTEx!!! :) :) !! :| :| :) !!!!

## Table of Contents
- [Todo](#todo)
- [Table of Contents](#top)
- [Usage](#usage)
- [Description](#description)
- [Building](#building)

## Usage
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

[top](#top) [toc](#table-of-contents)

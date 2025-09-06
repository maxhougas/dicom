<a top="top"/>

# DICOM
DICOM parser in C

## Todo
- [x] Create functionality to pull sqs from dicom.nema.org
- [x] Create develop branch
- [ ] Update README
- [ ] Break subfunctionality into separate files
- [x] Pull pulltable.sh functionality into Makefile
- [ ] Finish issq functionality
- [ ] Add single-pass functionality for tag-parsing
- [ ] Add second-pass functionality for tree node hanging
- [ ] Add functinality to dump to ... json or something
- [ ] Decide whether to proceed with byte1 byte2 byte4 or use char short int

## Table of Contents
- [Todo](#todo)
- [Table of Contents](#top)
- [Usage](#usage)
- [Description](#description)
- [Building](#building)

## Usage

## Description

## Building

### Linux
- The make file assumes you have access to BASH--shell expansion is used excessively

### Windows
- Currently Makefile assumes access to GCC.
- The code should be platform independent, but you'll need to generate source/sqtags.c on your own.

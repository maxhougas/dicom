#!/bin/sh

curl -s https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_7.html | head -n 679 | tail -n +114 > part6table.htm
curl -s https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_8.html | head -n 648 | tail -n +114 >> part6table.htm
curl -s https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_9.html | head -n 134 | tail -n +112 >> part6table.htm
curl -s https://dicom.nema.org/medical/dicom/current/output/chtml/part06/chapter_6.html | head -n 146585 | tail -n +117 >> part6table.htm
sed -z 's:\xe2\x80\x8b::g; s:</a>\n *</p>:</a>null</p>\n:g; s:, :!:g; s:,::g; s:!:, :g' part6table.htm |\
 grep -Po '</tr>$|(?<=\()[0-9A-Zx]{8}(?=\))|(?<=>)[0-9A-Za-z ,]*?(?=<)' |\
 sed -z 's:\n:!:g; s:!</tr>!:\n:g' > tagtable
grep SQ tagtable | grep -o '[A-F0-9x]\{8\}' | sed 's:[A-F0-9x]\{8\}:&!&:g; s:^50xx:5000:; s:!50xx:!50FF:' > sqs

#curl -s https://dicom.nema.org/medical/dicom/current/output/html/part06.html > part06.html
#head -n 147423 part06.html | tail -n +953 > tagtable.html
#head -n 148085 part06.html | tail -n +147518 >> tagtable.html
#head -n 148694 part06.html | tail -n +148158 >> tagtable.html
#head -n 148789 part06.html | tail -n +148765 >> tagtable.html
#grep -Po '^ *</tr>|(?<=>).*(?=</span)|(?<=a>).*(?=</)' tagtable.html |\
# sed -z 's:\n:!:g; s:! *</tr>!:\n:g; s:^ *::g; s:\xe2\x80\x8b::g' |\
# sed 's:^(::g; s:)!:!:g; s:, :!!COMMA!! :g; s:,:\x00:g; s:!!COMMA!!:,:g; s:!:\x00:g; s:[^\x00]$:%\x00:g' > tagtable

#   953	                     <tbody>
#147423	                     </tbody>
#147518	                     <tbody>
#148085	                     </tbody>
#148158	                     <tbody>
#148694	                     </tbody>
#148765	                     <tbody>
#148789	                     </tbody>

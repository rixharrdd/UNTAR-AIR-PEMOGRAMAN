#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define JUMLAH_BARIS 15
#define JUMLAH_KOLOM 6
#define MAKS_RESERVASI 1000
#define MAKS_TIKET 90
#define FILE_KURSI "Kursi.txt"
#define FILE_HARGA "Harga.txt"
#define FILE_BAGASI "Bagasi.txt"
#define FILE_RESERVASI "Reservasi.txt"
#define FILE_ADMIN "Admin.txt"

typedef struct {
    char kode;
    char nama[30];
    int barisAwal;
    int barisAkhir;
    double harga;
} Kelas;

typedef struct {
    char kode;
    char keterangan[30];
    double biaya;
} Bagasi;

typedef struct {
    int idTransaksi;
    char namaPemesan[80];
    char kodeKelas;
    int baris;
    char kursi;
    char kodeBagasi;
    double hargaTiket;
    double biayaBagasi;
    char waktu[32];
    int aktif;
} Reservasi;

typedef struct {
    int baris;
    int kolom;
    char kodeBagasi;
    double biayaBagasi;
    double total;
} PesananSementara;

char statusKursi[JUMLAH_BARIS][JUMLAH_KOLOM];
Kelas daftarKelas[3];
Bagasi daftarBagasi[4];
Reservasi daftarReservasi[MAKS_RESERVASI];
int jumlahReservasi = 0;

/* ---------- Input helpers ---------- */
void hapusEnter(char teks[]) {
    size_t panjang = strlen(teks);
    while (panjang > 0 && (teks[panjang - 1] == '\n' || teks[panjang - 1] == '\r')) {
        teks[panjang - 1] = '\0';
        panjang--;
    }
}

void bacaTeks(const char *pesan, char hasil[], int ukuran) {
    int c;
    printf("%s", pesan);
    if (fgets(hasil, ukuran, stdin) == NULL) {
        hasil[0] = '\0';
        return;
    }
    if (strchr(hasil, '\n') == NULL) {
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    hapusEnter(hasil);
}

int bacaAngka(const char *pesan, int minimum, int maksimum) {
    char teks[100];
    char *akhir;
    long nilai;
    while (1) {
        bacaTeks(pesan, teks, sizeof(teks));
        nilai = strtol(teks, &akhir, 10);
        if (teks[0] != '\0' && *akhir == '\0' && nilai >= minimum && nilai <= maksimum) {
            return (int)nilai;
        }
        printf("Input tidak valid. Masukkan angka %d sampai %d.\n", minimum, maksimum);
    }
}

char bacaHuruf(const char *pesan) {
    char teks[40];
    while (1) {
        bacaTeks(pesan, teks, sizeof(teks));
        if (teks[0] != '\0' && teks[1] == '\0') {
            return (char)toupper((unsigned char)teks[0]);
        }
        printf("Masukkan satu huruf saja.\n");
    }
}

void bersihkanNama(char nama[]) {
    int i;
    for (i = 0; nama[i] != '\0'; i++) {
        if (nama[i] == '|') nama[i] = ' ';
    }
}

/* ---------- File handling ---------- */
void isiKursiKosong(void) {
    int i, j;
    for (i = 0; i < JUMLAH_BARIS; i++) {
        for (j = 0; j < JUMLAH_KOLOM; j++) {
            statusKursi[i][j] = '_';
        }
    }
}

void simpanKursi(void) {
    FILE *fp = fopen(FILE_KURSI, "w");
    int i, j;
    if (fp == NULL) {
        printf("Peringatan: file %s gagal disimpan.\n", FILE_KURSI);
        return;
    }
    for (i = 0; i < JUMLAH_BARIS; i++) {
        for (j = 0; j < JUMLAH_KOLOM; j++) {
            fprintf(fp, "%c%c", statusKursi[i][j], j == JUMLAH_KOLOM - 1 ? '\n' : ' ');
        }
    }
    fclose(fp);
}

void muatKursi(void) {
    FILE *fp = fopen(FILE_KURSI, "r");
    int i, j, berhasil = 1;
    char nilai;
    if (fp == NULL) {
        isiKursiKosong();
        simpanKursi();
        return;
    }
    for (i = 0; i < JUMLAH_BARIS; i++) {
        for (j = 0; j < JUMLAH_KOLOM; j++) {
            if (fscanf(fp, " %c", &nilai) != 1 ||
                (nilai != '_' && nilai != 'X' && nilai != 'A' && nilai != 'B' && nilai != 'C')) {
                berhasil = 0;
                break;
            }
            statusKursi[i][j] = nilai;
        }
        if (!berhasil) break;
    }
    fclose(fp);
    if (!berhasil) {
        printf("Isi Kursi.txt tidak sesuai format. Denah diinisialisasi ulang.\n");
        isiKursiKosong();
        simpanKursi();
    }
}

void buatHargaDefault(void) {
    FILE *fp = fopen(FILE_HARGA, "w");
    if (fp == NULL) return;
    fprintf(fp, "F First_Class 1 2 1500000\n");
    fprintf(fp, "B Business_Class 3 5 1000000\n");
    fprintf(fp, "E Economy_Class 6 15 600000\n");
    fclose(fp);
}

void muatHarga(void) {
    FILE *fp = fopen(FILE_HARGA, "r");
    int i;
    if (fp == NULL) {
        buatHargaDefault();
        fp = fopen(FILE_HARGA, "r");
    }
    if (fp == NULL) {
        printf("File Harga.txt tidak dapat dibuka.\n");
        exit(1);
    }
    for (i = 0; i < 3; i++) {
        if (fscanf(fp, " %c %29s %d %d %lf", &daftarKelas[i].kode,
                   daftarKelas[i].nama, &daftarKelas[i].barisAwal,
                   &daftarKelas[i].barisAkhir, &daftarKelas[i].harga) != 5) {
            fclose(fp);
            printf("Format Harga.txt tidak sesuai. Periksa isi file.\n");
            exit(1);
        }
    }
    fclose(fp);
}

void buatBagasiDefault(void) {
    FILE *fp = fopen(FILE_BAGASI, "w");
    if (fp == NULL) return;
    fprintf(fp, "T Tanpa_Bagasi 0\n");
    fprintf(fp, "A 0-5_kg 100000\n");
    fprintf(fp, "B Lebih_5-10_kg 175000\n");
    fprintf(fp, "C Lebih_10-20_kg 300000\n");
    fclose(fp);
}

void muatBagasi(void) {
    FILE *fp = fopen(FILE_BAGASI, "r");
    int i;
    if (fp == NULL) {
        buatBagasiDefault();
        fp = fopen(FILE_BAGASI, "r");
    }
    if (fp == NULL) {
        printf("File Bagasi.txt tidak dapat dibuka.\n");
        exit(1);
    }
    for (i = 0; i < 4; i++) {
        if (fscanf(fp, " %c %29s %lf", &daftarBagasi[i].kode,
                   daftarBagasi[i].keterangan, &daftarBagasi[i].biaya) != 3) {
            fclose(fp);
            printf("Format Bagasi.txt tidak sesuai. Periksa isi file.\n");
            exit(1);
        }
    }
    fclose(fp);
}

void buatAdminDefault(void) {
    FILE *fp = fopen(FILE_ADMIN, "w");
    if (fp == NULL) return;
    fprintf(fp, "admin untar123\n");
    fclose(fp);
}

void muatReservasi(void) {
    FILE *fp = fopen(FILE_RESERVASI, "r");
    char baris[300];
    if (fp == NULL) {
        fp = fopen(FILE_RESERVASI, "w");
        if (fp != NULL) fclose(fp);
        return;
    }
    while (fgets(baris, sizeof(baris), fp) != NULL && jumlahReservasi < MAKS_RESERVASI) {
        Reservasi r;
        if (sscanf(baris, "%d|%79[^|]|%c|%d|%c|%c|%lf|%lf|%31[^|]|%d",
                   &r.idTransaksi, r.namaPemesan, &r.kodeKelas, &r.baris,
                   &r.kursi, &r.kodeBagasi, &r.hargaTiket, &r.biayaBagasi,
                   r.waktu, &r.aktif) == 10) {
            daftarReservasi[jumlahReservasi++] = r;
        }
    }
    fclose(fp);
}

void simpanReservasi(void) {
    FILE *fp = fopen(FILE_RESERVASI, "w");
    int i;
    if (fp == NULL) {
        printf("Peringatan: file Reservasi.txt gagal disimpan.\n");
        return;
    }
    for (i = 0; i < jumlahReservasi; i++) {
        Reservasi r = daftarReservasi[i];
        fprintf(fp, "%d|%s|%c|%d|%c|%c|%.2f|%.2f|%s|%d\n",
                r.idTransaksi, r.namaPemesan, r.kodeKelas, r.baris,
                r.kursi, r.kodeBagasi, r.hargaTiket, r.biayaBagasi,
                r.waktu, r.aktif);
    }
    fclose(fp);
}

void inisialisasiFile(void) {
    FILE *fp;
    muatHarga();
    muatBagasi();
    muatKursi();
    muatReservasi();
    fp = fopen(FILE_ADMIN, "r");
    if (fp == NULL) buatAdminDefault();
    else fclose(fp);
}

/* ---------- Core data helpers ---------- */
int indeksKelasDariBaris(int baris) {
    int i;
    for (i = 0; i < 3; i++) {
        if (baris >= daftarKelas[i].barisAwal && baris <= daftarKelas[i].barisAkhir) return i;
    }
    return -1;
}

int indeksKolom(char huruf) {
    huruf = (char)toupper((unsigned char)huruf);
    if (huruf >= 'A' && huruf <= 'F') return huruf - 'A';
    return -1;
}

char hurufKolom(int kolom) {
    return (char)('A' + kolom);
}

int cariIndeksBagasi(char kode) {
    int i;
    kode = (char)toupper((unsigned char)kode);
    for (i = 0; i < 4; i++) {
        if (daftarBagasi[i].kode == kode) return i;
    }
    return -1;
}

int hitungKursiKosong(int indeksKelas) {
    int baris, kolom, jumlah = 0;
    if (indeksKelas < 0 || indeksKelas > 2) return 0;
    for (baris = daftarKelas[indeksKelas].barisAwal - 1;
         baris < daftarKelas[indeksKelas].barisAkhir; baris++) {
        for (kolom = 0; kolom < JUMLAH_KOLOM; kolom++) {
            if (statusKursi[baris][kolom] == '_') jumlah++;
        }
    }
    return jumlah;
}

int cariReservasiAktif(int baris, int kolom) {
    int i;
    for (i = jumlahReservasi - 1; i >= 0; i--) {
        if (daftarReservasi[i].aktif && daftarReservasi[i].baris == baris &&
            indeksKolom(daftarReservasi[i].kursi) == kolom) return i;
    }
    return -1;
}

int idTransaksiTerakhir(void) {
    int i, id = 0;
    for (i = 0; i < jumlahReservasi; i++) {
        if (daftarReservasi[i].idTransaksi > id) id = daftarReservasi[i].idTransaksi;
    }
    return id;
}

double hitungPendapatan(void) {
    int i;
    double total = 0;
    for (i = 0; i < jumlahReservasi; i++) {
        if (daftarReservasi[i].aktif) {
            total += daftarReservasi[i].hargaTiket + daftarReservasi[i].biayaBagasi;
        }
    }
    return total;
}

void ambilWaktu(char waktu[]) {
    time_t sekarang = time(NULL);
    struct tm *info = localtime(&sekarang);
    if (info != NULL) strftime(waktu, 32, "%Y-%m-%d_%H:%M:%S", info);
    else strcpy(waktu, "Waktu_tidak_tersedia");
}

/* ---------- Display functions ---------- */
void tampilkanDenah(void) {
    int i, j;
    printf("\n================ DENAH KURSI UNTAR AIR ================\n");
    printf("             A   B   C   D   E   F\n");
    for (i = 0; i < JUMLAH_BARIS; i++) {
        const char *kelas = i < 2 ? "First" : (i < 5 ? "Business" : "Economy");
        printf("Baris %-2d %-8s", i + 1, kelas);
        for (j = 0; j < JUMLAH_KOLOM; j++) printf(" %c  ", statusKursi[i][j]);
        printf("\n");
    }
    printf("Legenda: _ = kosong | X = dipesan tanpa bagasi | A/B/C = dipesan dengan bagasi\n");
    printf("=======================================================\n");
}

void tampilkanTarif(void) {
    int i;
    printf("\n=== HARGA TIKET ===\n");
    for (i = 0; i < 3; i++) {
        printf("%d. %s (baris %d-%d): Rp%.0f\n", i + 1, daftarKelas[i].nama,
               daftarKelas[i].barisAwal, daftarKelas[i].barisAkhir, daftarKelas[i].harga);
    }
    printf("\n=== PILIHAN BAGASI ===\n");
    for (i = 0; i < 4; i++) {
        printf("%c. %s: Rp%.0f\n", daftarBagasi[i].kode,
               daftarBagasi[i].keterangan, daftarBagasi[i].biaya);
    }
}

void tampilkanMenu(void) {
    printf("\n============== MENU UTAMA UNTAR AIR ==============\n");
    printf("1. Mencari info kursi tertentu\n");
    printf("2. Pemesanan kursi\n");
    printf("3. Membatalkan pemesanan\n");
    printf("4. Rekomendasi kursi terbaik\n");
    printf("5. Struk pemesanan tiket terakhir\n");
    printf("6. Statistik kursi dan total pendapatan\n");
    printf("7. Reset data (khusus admin)\n");
    printf("8. Keluar\n");
    printf("==================================================\n");
}

const char *deskripsiStatus(char status) {
    switch (status) {
        case '_': return "Masih kosong";
        case 'X': return "Sudah dipesan tanpa bagasi";
        case 'A': return "Dipesan dengan bagasi A (0-5 kg)";
        case 'B': return "Dipesan dengan bagasi B (>5-10 kg)";
        case 'C': return "Dipesan dengan bagasi C (>10-20 kg)";
        default: return "Status tidak dikenal";
    }
}

/* ---------- Menu features ---------- */
void cariInfoKursi(void) {
    int baris, kolom, indeksKelas;
    char huruf;
    baris = bacaAngka("Masukkan nomor baris (1-15): ", 1, 15);
    huruf = bacaHuruf("Masukkan huruf kursi (A-F): ");
    kolom = indeksKolom(huruf);
    if (kolom < 0) {
        printf("Huruf kursi tidak valid.\n");
        return;
    }
    indeksKelas = indeksKelasDariBaris(baris);
    printf("\nKursi %d%c\n", baris, huruf);
    printf("Kelas  : %s\n", daftarKelas[indeksKelas].nama);
    printf("Status : %c - %s\n", statusKursi[baris - 1][kolom],
           deskripsiStatus(statusKursi[baris - 1][kolom]));
}

int sudahDipilihSementara(PesananSementara sementara[], int jumlah, int baris, int kolom) {
    int i;
    for (i = 0; i < jumlah; i++) {
        if (sementara[i].baris == baris && sementara[i].kolom == kolom) return 1;
    }
    return 0;
}

const char *namaKelasDariKode(char kode) {
    int i;
    for (i = 0; i < 3; i++) {
        if (daftarKelas[i].kode == kode) return daftarKelas[i].nama;
    }
    return "Kelas tidak dikenal";
}

void cetakStrukTransaksi(int idTransaksi) {
    int i, jumlah = 0;
    double total = 0;
    printf("\n================ STRUK PEMESANAN ================\n");
    for (i = 0; i < jumlahReservasi; i++) {
        Reservasi r = daftarReservasi[i];
        if (r.idTransaksi == idTransaksi && r.aktif) {
            if (jumlah == 0) {
                printf("UNTAR AIR\nNama pemesan : %s\n", r.namaPemesan);
                printf("ID transaksi : %d\n", r.idTransaksi);
                printf("Waktu        : %s\n", r.waktu);
            }
            printf("Kursi %d%c | Kelas %s | Tiket Rp%.0f | Bagasi %c Rp%.0f\n",
                   r.baris, r.kursi, namaKelasDariKode(r.kodeKelas), r.hargaTiket,
                   r.kodeBagasi, r.biayaBagasi);
            total += r.hargaTiket + r.biayaBagasi;
            jumlah++;
        }
    }
    if (jumlah == 0) {
        printf("Tidak ada reservasi aktif pada transaksi tersebut.\n");
    } else {
        printf("Jumlah pemesanan : %d kursi\n", jumlah);
        printf("Total pembayaran : Rp%.0f\n", total);
        printf("Terima kasih telah memilih UNTAR AIR.\n");
    }
    printf("=================================================\n");
}

void prosesPemesanan(void) {
    char nama[80], konfirmasi;
    int pilihanKelas, jumlahTiket, indeksKelas, tersedia, k;
    int transaksiBaru;
    double hargaTiket, totalTransaksi = 0;
    char waktu[32];
    PesananSementara sementara[MAKS_TIKET];

    tampilkanTarif();
    bacaTeks("\nNama pemesan: ", nama, sizeof(nama));
    if (nama[0] == '\0') {
        printf("Nama pemesan tidak boleh kosong.\n");
        return;
    }
    bersihkanNama(nama);
    pilihanKelas = bacaAngka("Pilih kelas (1=First, 2=Business, 3=Economy): ", 1, 3);
    indeksKelas = pilihanKelas - 1;
    jumlahTiket = bacaAngka("Jumlah tiket yang ingin dipesan: ", 1, MAKS_TIKET);
    tersedia = hitungKursiKosong(indeksKelas);
    if (jumlahTiket > tersedia) {
        printf("Pemesanan ditolak. Kursi kosong pada kelas %s hanya %d.\n",
               daftarKelas[indeksKelas].nama, tersedia);
        return;
    }
    if (jumlahReservasi + jumlahTiket > MAKS_RESERVASI) {
        printf("Kapasitas catatan reservasi penuh.\n");
        return;
    }

    hargaTiket = daftarKelas[indeksKelas].harga;
    for (k = 0; k < jumlahTiket; k++) {
        int valid = 0;
        while (!valid) {
            int baris, kolom, kelasBaris;
            char huruf, kodeBagasi;
            baris = bacaAngka("Pilih nomor baris: ", 1, JUMLAH_BARIS);
            huruf = bacaHuruf("Pilih huruf kursi (A-F): ");
            kolom = indeksKolom(huruf);
            kelasBaris = indeksKelasDariBaris(baris);
            if (kolom < 0) {
                printf("Huruf kursi tidak valid.\n");
            } else if (kelasBaris != indeksKelas) {
                printf("Baris tersebut tidak sesuai dengan kelas yang dipilih.\n");
            } else if (statusKursi[baris - 1][kolom] != '_') {
                printf("Kursi %d%c sudah dipesan. Silakan pilih kursi lain.\n", baris, huruf);
            } else if (sudahDipilihSementara(sementara, k, baris, kolom)) {
                printf("Kursi tersebut sudah dipilih dalam transaksi ini.\n");
            } else {
                printf("Pilihan bagasi: T=Tanpa bagasi, A=0-5 kg, B=>5-10 kg, C=>10-20 kg\n");
                kodeBagasi = bacaHuruf("Masukkan pilihan bagasi (T/A/B/C): ");
                if (cariIndeksBagasi(kodeBagasi) < 0) {
                    printf("Kode bagasi tidak valid.\n");
                } else {
                    int ib = cariIndeksBagasi(kodeBagasi);
                    sementara[k].baris = baris;
                    sementara[k].kolom = kolom;
                    sementara[k].kodeBagasi = kodeBagasi;
                    sementara[k].biayaBagasi = daftarBagasi[ib].biaya;
                    sementara[k].total = hargaTiket + daftarBagasi[ib].biaya;
                    totalTransaksi += sementara[k].total;
                    valid = 1;
                }
            }
        }
    }

    printf("\n========== RINGKASAN PEMESANAN =========\n");
    printf("Nama pemesan : %s\n", nama);
    printf("Kelas        : %s\n", daftarKelas[indeksKelas].nama);
    printf("Jumlah tiket : %d\n", jumlahTiket);
    for (k = 0; k < jumlahTiket; k++) {
        printf("Kursi %d%c | Bagasi %c | Total Rp%.0f\n",
               sementara[k].baris, hurufKolom(sementara[k].kolom),
               sementara[k].kodeBagasi, sementara[k].total);
    }
    printf("Total transaksi: Rp%.0f\n", totalTransaksi);
    printf("========================================\n");
    konfirmasi = bacaHuruf("Konfirmasi pemesanan? (Y/T): ");
    if (konfirmasi != 'Y') {
        printf("Pemesanan dibatalkan. Data kursi tidak berubah.\n");
        return;
    }

    transaksiBaru = idTransaksiTerakhir() + 1;
    ambilWaktu(waktu);
    for (k = 0; k < jumlahTiket; k++) {
        Reservasi r;
        int idx = jumlahReservasi++;
        r.idTransaksi = transaksiBaru;
        strcpy(r.namaPemesan, nama);
        r.kodeKelas = daftarKelas[indeksKelas].kode;
        r.baris = sementara[k].baris;
        r.kursi = hurufKolom(sementara[k].kolom);
        r.kodeBagasi = sementara[k].kodeBagasi;
        r.hargaTiket = hargaTiket;
        r.biayaBagasi = sementara[k].biayaBagasi;
        strcpy(r.waktu, waktu);
        r.aktif = 1;
        daftarReservasi[idx] = r;
        statusKursi[r.baris - 1][sementara[k].kolom] =
            (r.kodeBagasi == 'T') ? 'X' : r.kodeBagasi;
    }
    simpanKursi();
    simpanReservasi();
    cetakStrukTransaksi(transaksiBaru);
    tampilkanDenah();
}

void prosesPembatalan(void) {
    int baris, kolom, indeks;
    char huruf, konfirmasi;
    double pengembalian;
    baris = bacaAngka("Masukkan nomor baris kursi yang dibatalkan: ", 1, JUMLAH_BARIS);
    huruf = bacaHuruf("Masukkan huruf kursi (A-F): ");
    kolom = indeksKolom(huruf);
    if (kolom < 0) {
        printf("Huruf kursi tidak valid.\n");
        return;
    }
    if (statusKursi[baris - 1][kolom] == '_') {
        printf("Kursi %d%c belum dipesan.\n", baris, huruf);
        return;
    }
    indeks = cariReservasiAktif(baris, kolom);
    if (indeks < 0) {
        printf("Catatan reservasi aktif tidak ditemukan. Hubungi admin.\n");
        return;
    }
    pengembalian = daftarReservasi[indeks].hargaTiket + daftarReservasi[indeks].biayaBagasi;
    printf("Nama pemesan : %s\n", daftarReservasi[indeks].namaPemesan);
    printf("Nilai pengembalian (asumsi pengembalian penuh): Rp%.0f\n", pengembalian);
    konfirmasi = bacaHuruf("Yakin membatalkan pemesanan? (Y/T): ");
    if (konfirmasi == 'Y') {
        statusKursi[baris - 1][kolom] = '_';
        daftarReservasi[indeks].aktif = 0;
        simpanKursi();
        simpanReservasi();
        printf("Pembatalan berhasil. Kursi %d%c kembali kosong.\n", baris, huruf);
        tampilkanDenah();
    } else {
        printf("Pembatalan dibatalkan. Data tidak berubah.\n");
    }
}

void rekomendasiKursi(void) {
    int pilihanKelas, indeksKelas, baris, kolom;
    int ditemukan = 0;
    pilihanKelas = bacaAngka("Pilih kelas (1=First, 2=Business, 3=Economy): ", 1, 3);
    indeksKelas = pilihanKelas - 1;
    for (baris = daftarKelas[indeksKelas].barisAwal - 1;
         baris < daftarKelas[indeksKelas].barisAkhir && !ditemukan; baris++) {
        for (kolom = 0; kolom < JUMLAH_KOLOM && !ditemukan; kolom++) {
            if (statusKursi[baris][kolom] == '_') {
                printf("Rekomendasi kursi terbaik: %d%c (%s)\n",
                       baris + 1, hurufKolom(kolom), daftarKelas[indeksKelas].nama);
                ditemukan = 1;
            }
        }
    }
    if (!ditemukan) printf("Tidak ada kursi kosong pada kelas tersebut.\n");
}

void tampilkanStrukTerakhir(void) {
    int i, id = 0;
    for (i = jumlahReservasi - 1; i >= 0; i--) {
        if (daftarReservasi[i].aktif) {
            id = daftarReservasi[i].idTransaksi;
            break;
        }
    }
    if (id == 0) printf("Belum ada transaksi aktif.\n");
    else cetakStrukTransaksi(id);
}

void tampilkanStatistik(void) {
    int i, j;
    int terisi[3] = {0, 0, 0};
    int totalTerisi = 0;
    int jumlahKelas[3] = {12, 18, 60};
    double pendapatan = hitungPendapatan();
    for (i = 0; i < JUMLAH_BARIS; i++) {
        int indeksKelas = indeksKelasDariBaris(i + 1);
        for (j = 0; j < JUMLAH_KOLOM; j++) {
            if (statusKursi[i][j] != '_') {
                terisi[indeksKelas]++;
                totalTerisi++;
            }
        }
    }
    printf("\n=========== STATISTIK KURSI UNTAR AIR ===========\n");
    printf("Jumlah kursi total : %d\n", JUMLAH_BARIS * JUMLAH_KOLOM);
    printf("Kursi terisi       : %d\n", totalTerisi);
    printf("Kursi kosong       : %d\n", JUMLAH_BARIS * JUMLAH_KOLOM - totalTerisi);
    printf("Keterisian First Class    : %.2f%% (%d/12)\n", terisi[0] * 100.0 / jumlahKelas[0], terisi[0]);
    printf("Keterisian Business Class : %.2f%% (%d/18)\n", terisi[1] * 100.0 / jumlahKelas[1], terisi[1]);
    printf("Keterisian Economy Class  : %.2f%% (%d/60)\n", terisi[2] * 100.0 / jumlahKelas[2], terisi[2]);
    printf("Total pendapatan berjalan: Rp%.0f\n", pendapatan);
    printf("=================================================\n");
}

int validasiAdmin(const char username[], const char password[]) {
    FILE *fp = fopen(FILE_ADMIN, "r");
    char userTersimpan[40], passTersimpan[40];
    int cocok = 0;
    if (fp == NULL) return 0;
    if (fscanf(fp, "%39s %39s", userTersimpan, passTersimpan) == 2) {
        if (strcmp(username, userTersimpan) == 0 && strcmp(password, passTersimpan) == 0) cocok = 1;
    }
    fclose(fp);
    return cocok;
}

void resetDataAdmin(void) {
    char username[40], password[40], konfirmasi;
    double pendapatanSebelum = hitungPendapatan();
    int i;
    printf("\n=== LOGIN ADMIN ===\n");
    bacaTeks("Username: ", username, sizeof(username));
    bacaTeks("Password: ", password, sizeof(password));
    if (!validasiAdmin(username, password)) {
        printf("Login gagal. Akses reset ditolak; data tidak berubah.\n");
        return;
    }
    printf("Pendapatan sebelum reset: Rp%.0f\n", pendapatanSebelum);
    konfirmasi = bacaHuruf("Konfirmasi reset seluruh data? (Y/T): ");
    if (konfirmasi != 'Y') {
        printf("Reset dibatalkan. Data tidak berubah.\n");
        return;
    }
    isiKursiKosong();
    for (i = 0; i < jumlahReservasi; i++) daftarReservasi[i].aktif = 0;
    jumlahReservasi = 0;
    simpanKursi();
    simpanReservasi();
    printf("Reset berhasil. Semua kursi kembali kosong.\n");
    printf("Pendapatan setelah reset: Rp0\n");
    tampilkanDenah();
}

/* ---------- Main program ---------- */
int main(void) {
    int pilihan;
    inisialisasiFile();

    printf("===============================================\n");
    printf("       SISTEM PEMESANAN KURSI UNTAR AIR\n");
    printf("===============================================\n");
    printf("Data kursi dan transaksi disimpan dalam file.\n");

    do {
        tampilkanDenah();
        tampilkanMenu();
        pilihan = bacaAngka("Pilih menu (1-8): ", 1, 8);

        switch (pilihan) {
            case 1: cariInfoKursi(); break;
            case 2: prosesPemesanan(); break;
            case 3: prosesPembatalan(); break;
            case 4: rekomendasiKursi(); break;
            case 5: tampilkanStrukTerakhir(); break;
            case 6: tampilkanStatistik(); break;
            case 7: resetDataAdmin(); break;
            case 8: printf("Terima kasih telah menggunakan UNTAR AIR.\n"); break;
            default: printf("Pilihan menu tidak valid.\n"); break;
        }
    } while (pilihan != 8);

    return 0;
}

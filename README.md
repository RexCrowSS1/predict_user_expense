# Prediksi Pengeluaran

Proyek menyediakan dua model regresi untuk memprediksi pengeluaran harian:

- `models/knn/`: KNN Regression dengan `k=3`.
- `models/linear_regression/`: Seasonal Linear Regression.

Kedua model menggunakan aplikasi, pembaca data, evaluasi, dan sembilan bagian
fitur yang sama. Status budget dihitung setelah nominal diprediksi sehingga
tidak menjadi target leakage.

## Menjalankan program

Jalankan perintah dari folder utama proyek:

```bash
make run-knn
make run-linear
```

Jika hanya menjalankan `make run`, model default-nya adalah Linear Regression.

## Input dan perhitungan bulanan

Satu periode bulanan pada aplikasi adalah **4 minggu / 28 hari**.

1. Input **Rent sekali** untuk satu bulan. Rent boleh nol dan dibagi empat
   untuk ditambahkan pada total masing-masing minggu.
2. Untuk setiap minggu, input **Laundry sekali** (total empat input). Nominal
   minggu tersebut dibagi tujuh dan dialokasikan ke setiap hari pada minggu
   yang sama. Contoh: Laundry Rp35.000 menghasilkan alokasi Rp5.000 per hari.
3. Input Food, Drinks, dan Transportation **cukup satu minggu (Senin–Minggu)**.
   Nominal pada hari yang sama digunakan kembali di minggu 2–4, sehingga
   tidak perlu diinput ulang. Laundry tetap mengikuti input masing-masing minggu.
4. Input budget harian (termasuk Laundry, tanpa Rent) dan budget empat minggu
   (termasuk Rent).

Perhitungan yang digunakan:

- Total harian = Food + Drinks + Transportation + Laundry minggu terkait / 7.
- Total mingguan = jumlah tujuh total harian + Rent bulanan / 4.
- Total bulanan = jumlah empat total mingguan; Rent hanya dihitung sekali.

Pembagian Laundry memakai presisi penuh saat dihitung; riwayat menampilkan dua
angka desimal, sedangkan ringkasan dibulatkan ke rupiah.

Rent tidak disimpan pada record harian dan tidak dipakai dalam training,
evaluasi model, prediksi harian, atau pemeriksaan budget harian. Statistik,
monitoring, dan peringatan mencakup empat minggu dengan asumsi pengeluaran
Food, Drinks, dan Transportation mengulang pola minggu pertama.

Model dilatih ulang dengan data input untuk memprediksi **empat minggu
berikutnya**. Laundry termasuk dalam pengeluaran harian yang dipelajari model.
Rent periode berikutnya diasumsikan sama dengan input bulanan dan ditambahkan
setelah prediksi, masing-masing seperempat ke setiap minggu.

## Dataset dan evaluasi

`data/train.csv` berisi minggu 1–24 dan `data/expense_test_fluctuated.csv`
berisi holdout minggu 25–28. Kolom Rent sudah dihapus dari keduanya.
`Total_Expense` dan `Budget_Status` CSV dihitung ulang tanpa Rent.

CSV menyimpan Laundry pada hari pembayaran aslinya. Saat dibaca, total Laundry
setiap minggu dibagi rata ke tujuh hari agar konsisten dengan input aplikasi.
Alokasi ini mempertahankan total mingguan; kolom total dan status di CSV tetap
menggambarkan pembayaran asli, bukan alokasi harian yang dipakai model.

Pemilihan KNN memakai training minggu 1–8 dan validasi minggu 9–12, dengan
hasil setelah Rent dihapus dan Laundry dialokasikan per hari:

| k | MAE validasi |
|--:|--------------:|
| 1 | Rp5.676 |
| 2 | Rp4.693 |
| 3 | Rp4.688 |
| 4 | Rp5.100 |
| 5 | Rp5.861 |

Karena itu model memakai `k=3`. Holdout minggu 25–28 tidak dipakai untuk memilih k.
Pada holdout, MAE Linear Regression adalah Rp2.811, KNN Rp7.418, dan baseline
musiman Rp6.857. KNN pada data baru belum mengalahkan baseline; pengujian
memeriksa validitas metrik dan pemilihan k, tanpa mengasumsikan keunggulan KNN.

## Menjalankan pengujian

Memerlukan compiler C++17, Make, dan Python 3 (untuk pengujian aplikasi).

```bash
make test
```

Pengujian mencakup kedua model, konsistensi dataset, alokasi Laundry historis,
input empat minggu, total mingguan/bulanan, serta independensi prediksi dan
status harian terhadap Rent.

## Struktur folder

```text
app/                         aplikasi dan sembilan bagian fitur
common/                      data, interface model, dan evaluasi
models/knn/                  implementasi KNN Regression
models/linear_regression/    implementasi Linear Regression
tests/                       pengujian model, data, dan aplikasi
data/                        dataset CSV tanpa Rent
build/                       binary lokal (dibuat otomatis)
```

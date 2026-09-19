# Prediksi Pengeluaran

Proyek menyediakan dua model regresi untuk memprediksi nominal pengeluaran:

- `models/knn/`: KNN Regression dengan `k=1`.
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

## Menjalankan pengujian

```bash
make test
```

Pemilihan KNN dilakukan pada validasi kronologis minggu 9-12:

| k | MAE validasi |
|--:|--------------:|
| 1 | Rp9.580 |
| 2 | Rp29.183 |
| 3 | Rp36.387 |
| 4 | Rp38.888 |
| 5 | Rp40.807 |

Karena itu model memakai `k=1`, bukan memaksakan `k=3`. Data minggu 13-14
tetap menjadi holdout terpisah untuk mengukur performa akhir.

## Struktur folder

```text
app/                         aplikasi dan sembilan bagian fitur
common/                      data, interface model, dan evaluasi
models/knn/                  implementasi KNN Regression
models/linear_regression/    implementasi Linear Regression
tests/                       pengujian masing-masing model
data/                        dataset CSV
build/                       binary lokal (dibuat otomatis)
```

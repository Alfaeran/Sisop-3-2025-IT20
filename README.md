# Sisop-3-2025-IT20

## Modul 3 Sistem Operasi 2025
- **Mey Rosalina NRP 5027241004**
- **Rizqi Akbar Sukirman Putra NRP 5027241044**
- **M. Alfaeran Auriga Ruswandi NRP 5027241115**

# Soal 1

# Soal 2
a. Mengunduh File Order dan Menyimpannya ke Shared Memory
```
if (argc == 1) {
    FILE *file = fopen("delivery_order.csv", "r");
    if (!file) {
        perror("Gagal membuka file CSV");
        exit(1);
    }

    int i = 0;
    while (fscanf(file, "%[^,],%[^,],%[^\n]\n",
                  orders[i].nama_penerima,
                  orders[i].alamat_tujuan,
                  orders[i].jenis_pengiriman) != EOF) {
        orders[i].status = 0;
        strcpy(orders[i].agen, "-");
        i++;
    }

    fclose(file);
    printf("Pesanan berhasil dimuat ke shared memory.\n");
}
```





# Soal 3

# Soal 4

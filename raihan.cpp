#include <iostream>
#include <ctime>
#include <sstream> //  library untuk stringstream atau mengubah string menjadi  type data  lain

// mendefinisikan trim sebagai  pengecek kosong atau tidak nya  input 
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r"); // cari karakter pertama bukan spasi/tab/newline
    if (start == std::string::npos) return "";    // jika semua spasi, kembalikan string kosong
    size_t end = s.find_last_not_of(" \t\n\r");   // cari karakter terakhir bukan spasi/tab/newline
    return s.substr(start, end - start + 1);      // ambil substring yang bersih
}

int main(){

// angka random pseudo
// s sebagai seed atau mengubah nilai random supaya tidak selalu sama ketika di run
// rand untuk menampilkan angka  random, jika tidak di seed maka hasil nya akan selalu sama
// time(NULL) mengambil waktu sekarang setiap  satu detik
// NULL adalah konstanta tidak yang berarti ada alamat / kosong / tidak menunjuk ke data apa pun 
// opsi lain bisa dibuat menjadi srand(time(0));
srand(time(NULL));

// rand adalah bilangan  acak semu
// nah pada c++ kita tidak dapat membuat angka random seperti di python yaitu dengan parameter
// namun kita bisa menggunakan modulo sebagai batas, jadi misal kita ingin membuat random angka 1 sampai 6 
// maka dibuat dengan rand() % 6 (nah namun dengan begini indeks masih di baca dari 0) maka random yang terjadi dari 0 - 5
// oleh karena itu di tambah dengan 1, supaya setiap angka akan  di tambah 1
int answer = (rand() % 6) + 1 ;
int num;
// MENDEFISINIKAN PERCOBAAN START DARI 0
int percobaan = 0;
std::string input;

std::cout << "*******************************************************************" << '\n';
std::cout << "************************* TEBAK - TEBAKAN *************************" << '\n';
std::cout << "*******************************************************************" << '\n';



// membuat game tebakkan  tebakkan angka 
do{   
    std::cout << "AYO TEBAK DARI ANGKA (1 - 6)? ";
    std::getline(std::cin, input);
    percobaan +=1;

    // trim input untuk mendeteksi spasi atau enter yang kosong
    input = trim(input);

    // cek kosong input atau tidak
    if(input.empty()){

        std::cout << '\n';
        std::cout << "                          MOHON UNTUK MENGISI INPUT ANDA :3                           " << '\n';
        continue;//  continue untuk melanjutkan ke program setelah nya
    }
    
    // mengubah input menjadi integer menggunakan ss (stringstream), nah  untuk menggunakan string stream harus menggunakan library yaitu <sstream>
    std::stringstream ss(input);
    // if(!(ss >>  num)) adalah jika ss >> num mengambil integer dari string, sedangkan ss.eof() cek apakah setelah angka, tidak ada karakter lain
    // ! (not) atau kondisi kebalikan
    // || (or) salah satu nya
    if (!(ss >> num) || !(ss.eof())) {
        std::cout << '\n';
        std::cout << "                          INPUT HARUS ANGKA ;3                       \n";
        continue;
        }
    
    // mengecek apakah input yang di masukkan angka (1 - 6)
    if(num < 1 || num > 6){
        std::cout << "              INPUT HANYA BOLEH  DARI (1 - 6);3 " << '\n';
        continue;
    }
    percobaan +=1;



    if(num > answer){
        std::cout << "ANGKA KAMU LEBIH TINGGI" << '\n';
    }

    else if(num < answer){
        std::cout << "ANGKA KAMU LEBIH RENDAH" << '\n';
        }
    else{
        std::cout << "KAMU BENAR JAWABAN NYA ADALAH " << answer << " DENGAN, " << percobaan << " PERCOBAAN" << '\n';
     } 

    }while (num != answer);


return 0;
}




    
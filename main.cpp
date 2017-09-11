///    Readme (Updated Sept 2017)
//   In order to crypt (and decrypt) images with this program, it requires to be compiled and to be specified two things:
// (0) The plain Image's sizes - width as "lung_x" and height as "lung_y"
// (1) The file's name - replace "1 test3.bmp" with your image's name. Make sure the image is in the same folder you compile the program in!
//  
//   Tip: try using CTRL+F to find the marked comments in the source code and quickly edit these values
//
//   Additional note: the output file names may also be changed - check marked comments (2) and (3)
//  



//    Observatii generale (dubios?)
//  -fisierele .bmp sunt citite de jos in sus, respectiv de la stanga la dreapta
//  -pixelii unui fisier .bmp sunt compusi din 3 octeti (BGR - Blue Green, Red)
//
//
//    *Valabil pentru citirea caracter cu caracter; in cazul citirii a cate 3 octeti intr-un int,
//  nu mai exista nicio problema. Sunt destul de sigur ca aceasta 'anomalie' apare din cauza little endianness-ului.
//

// 'Red' (din Paint) = 15539236 (RGB) sau 2366701 (GBR)

#include<iostream>
#include<fstream>
#include<iomanip>
#include<windows.h>
using namespace std;

#define key 1742839
#define key2 9319321
#define header_dim 54

// 1: 16 x 9
// 2: 200 x 200
// 3: 200 x 200

//#define poza1 1
#define poza2 1
//#define poza3 1



#ifdef poza1
    const int lung_x = 16;
    const int lung_y = 9;
#endif // poza1

#ifdef poza2
    const int lung_x = 200; /// Readme (0)
    const int lung_y = 200;
#endif // poza2

#ifdef poza3
    const int lung_x = 200;
    const int lung_y = 200;
#endif // poza3



// f1 = fisier necriptat
// f2 = fisier criptat
// f3 = fisier decriptat

fstream f1;
fstream f2;
fstream f3;
ofstream out("date.out");



struct pixel
{
    unsigned char red, green, blue;
    int poz1,poz2;

    void afis_clar()
    {
        out<<"("<<setw(3)<<int(red)<<", "<<setw(3)<<int(green)<<", "<<setw(3)<<int(blue)<<" | "<<poz1<<", "<<poz2<<")";
    }

    void afis_binar(fstream &f)
    {
        f.write((char*)&blue, 1);
        f.write((char*)&green, 1);
        f.write((char*)&red, 1);
    }

    void clone(pixel p)
    {
        red = p.red;
        green = p.green;
        blue = p.blue;
    }

    void xor_global()
    {
        int r;

        r=rand();
        red^=r;

        r=rand();
        green^=r;

        r=rand();
        blue^=r;
    }
};



char header[header_dim];
pixel mat[lung_y][lung_x];
pixel crypt[lung_y][lung_x];
pixel decrypt[lung_y][lung_x];

pixel permut[lung_y][lung_x]; // Doar pentru afisari
pixel depermut[lung_y][lung_x]; // Doar pentru afisari


void citesc_imagine(fstream &in, char header[], pixel matrice[lung_y][lung_x])
{
    unsigned char r,g,b;
    pixel *p;
    int i,j;

    i=0;
    j = -header_dim / 3; // 54 header / 3 octeti per pixel

    while(in.read((char*)&b, 1), in.read((char*)&g, 1), in.read((char*)&r, 1)) // Dimensiunea pozei .bmp = 54 + 3*nr_pixeli ==> divizibila cu 3
    {
        if(j>=0)
        {
            p = &mat[i][j];

            p->red = r;
            p->green = g;
            p->blue = b;

            if(j >= lung_x)
            {
                ++i;
                j=0;
            }
        }
        else
        {
            header[3*(j+18)] = b;
            header[3*(j+18) + 1] = g;
            header[3*(j+18) + 2] = r;
        }
        ++j;
    }
}



int main()
{
    unsigned char r,g,b;
    pixel *p;
    int i,j,nr;

    /// Citire
    f1.open("1 test3.bmp", ios::in | ios::binary); /// Readme (1)
    citesc_imagine(f1, header, mat);



    /// Criptare
    f2.open("2 Image crypted.bmp", ios::out | ios::binary);
    f2.write(header, header_dim);

    // Criptare Vernam
    srand(key);
    for(i=0; i<lung_y; ++i)
    {
        for(j=0; j<lung_x; ++j)
        {
            crypt[i][j].clone(mat[i][j]);
            crypt[i][j].xor_global();

            //crypt[i][j].afis_binar(f2);
        }
    }

    for(i=0; i<lung_y && i<1; ++i)
        for(j=0; j<lung_x; ++j)
            permut[i][j].clone(crypt[i][j]);

    // Permutare
    srand(key2);
    for(i=0; i<lung_y; ++i)
    {
        for(j=0;j<lung_x;++j) crypt[i][j].poz1=j;

        // Durstenfield
        for(j=lung_x-1; j>=1; --j)
        {
            nr=rand()%j;
            crypt[i][nr].poz2 = j;

            swap(crypt[i][j], crypt[i][nr]);
        }
    }

    for(i=0; i<lung_y; ++i)
        for(j=0; j<lung_x; ++j)
            crypt[i][j].afis_binar(f2);



    /// Decriptare
    f2.open("2 Image crypted.bmp", ios::out | ios::binary); /// Readme (2)
    f3.open("3 Image decrypted.bmp", ios::out | ios::binary); /// Readme (3)

    citesc_imagine(f2, header, crypt);
    f3.write(header, header_dim);

    // Permutare inversa
    int poz[lung_x];

    srand(key2);
    for(i=0; i<lung_y; ++i)
    {
        for(j=0;j<lung_x;++j) poz[j]=j;
        for(j=lung_x-1; j>=1; --j)
        {
            nr=rand()%j;
            swap(poz[j], poz[nr]);
        }

        for(j=0; j<lung_x; ++j) decrypt[i][poz[j]] = crypt[i][j];
    }

    for(i=0; i<lung_y && i<1; ++i)
        for(j=0; j<lung_x; ++j)
            depermut[i][j].clone(decrypt[i][j]);

    // Decriptare Vernam
    srand(key);
    for(i=0; i<lung_y; ++i)
    {
        for(j=0; j<lung_x; ++j)
        {
            decrypt[i][j].clone(decrypt[i][j]);
            decrypt[i][j].xor_global();

            decrypt[i][j].afis_binar(f3);
        }
    }



    /// Afisare
    if(0) // 1 pentru afisare; recomandabil pentru poza1 (dimensiuni mici)
        for(i=0; i<lung_y; ++i)
        {
            for(j=0; j<lung_x; ++j) // NORMAL    SEMI-CRIPTAT [verman]   CRIPTAT [verman+permutat]    SEMI-DECRIPTAT [verman]    DECRIPTAT
            {
                mat[i][j].afis_clar(); // NORMAL
                out<<"    ";
                permut[i][j].afis_clar(); // HALF-CRYPTED (VERMAN)
                out<<"    ";
                crypt[i][j].afis_clar(); // CRYPTED
                out<<"    ";
                depermut[i][j].afis_clar(); // HALF-DECRYPTED (VERMAN)
                out<<"    ";
                decrypt[i][j].afis_clar(); // DECRYPTED
                out<<"\n";
            }
            out<<"\n";
        }

    return 0;
}

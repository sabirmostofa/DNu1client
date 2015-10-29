
//http://www.linuxhowtos.org/C_C++/socket.htm
//http://www2.cs.uidaho.edu/~krings/CS270/Notes.S10/270-F10-28.pdf
//http://gnosis.cx/publish/programming/sockets.html
//
//      client.cpp
//

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
//#include <netdb.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <math.h>


using namespace std;

char *bit_send(char *buffer, int cnt) {
    //    int msg_size;
    char *buffer1, *ptr1, c;
    ptr1 = buffer1 = (char*) malloc(8 * cnt); //create a buffer
    while (cnt != 0) { //char is available
        c = *buffer; //get a char
        cnt--; //counter decrement
        for (int i = 0; i < 8; i++) { //show each bit as '0' or '1'
            if ((c & (0x80 >> i)) != 0) {
                *ptr1 = '1';
            } else {
                *ptr1 = '0';
            }
            ptr1++;
        }
        buffer++;
    }
    return buffer1; //return an address of a new string; 
    //new size is 8 * cnt
}


uint32_t lcg_seed = 1;

uint32_t lcg_rand() {
    //lcg_seed = ((uint64_t)lcg_seed * 279470273UL) % 4294967291UL;
    lcg_seed = rand();
    return lcg_seed;
}
// pseudo-random in [0,1]

double randf() {
    return lcg_rand() / (double) 0xFFFFFFFFUL;
}
const double PI = 3.14159265;
// pseudo-Rauschen gaussian

double randf_gauss() {
    double mu = 0.0; // mittelwert
    double sigma = 0.25; //!! varianz 0.45; try a values from 0.25 to 0.55
    return mu + sigma * sqrt(-2.0 * log(randf())) * cos(2 * PI * randf());
}

/****************** Analog Kanal Modell mit Rauschen *******************
 *                am Eingang ein originelles Bit aus den Datastream
 *                das Bit ist in analoge Grösse umgewandelt, gedämmt
 *                und mit dem Rauschen zusammengemischt.
 *                Das Ergebnis wird wieder als ein Bit dargestellt
 *                Am Ausgang ein Bit mit möglicher Störung             
 ***********************************************************************/
char analog_kanal_modell(char inputbit) { //add noise to the bit stream
    double input_signal_level = 0.1;
    char outputbit;
    double in, noise, out;
    /////////////////////  Digital to Analog conversion //////////////////
    if (inputbit != '0') {
        in = +input_signal_level;
    } else {
        in = -input_signal_level;
    }
    ///////////////////  Störungen im Kanal //////////////////    
    noise = 0.1 * randf_gauss(); // pseudo-zufällige Zahlen, Gauss Verteilung
    out = in + noise; // Analogsignal mit Rauschen
    /////////////////// Analog to Digital conversion //////////////////
    if (out > 0.0)
        outputbit = '1';
    else
        outputbit = '0';
    return outputbit;
}

int main() {
    cout << "test";

    //get file size
    ifstream file("sample.txt", ios::binary);
    file.seekg(0, ios::end);
    unsigned int fileSize = file.tellg();
    printf("File Size: %d ", fileSize);
    file.close();

    //get the file
    char* fileBuffer = new char[fileSize];
    file.open("sample.txt", ios::binary);
    file.seekg(0, ios::beg);
    file.read(fileBuffer, fileSize);
    file.close();
    int rc;
    int s;

    printf("file size: %d bytes", fileSize);

    struct sockaddr_in addr;

    // string st[4] = "";


    //    ifstream ifile("sample.txt");
    //
    //    for (int i = 0; i++; i++) {
    //        ifile.read(&*st.begin(), strlen(st) - 1);
    //
    //        cout << st;
    //    }


    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //addr.sin_addr.s_addr = inet_addr("192.168.2.2");
    addr.sin_port = htons(3334); // little endian -> big endian

    rc = connect(s, (struct sockaddr *) &addr, sizeof (addr));
    if (rc < 0) {
        printf("connect failed\n");
        return -1;
    }

    char* line;
    // char key[] = "exit";
    // char key1[] = "stop";


    const int FILE_CHUNK_SIZE = 1024*8;



    //send file in chunks
    unsigned int bytesSent = 0;
    int bytesToSend = 0;

    printf("file size: %d bytes", fileSize);


    fileSize *=8; //

    send(s, (const char*) &fileSize, sizeof (fileSize), 0);
    //
    
    char *zBuffer =  bit_send(fileBuffer,fileSize);
    int counter = 0;
    while (bytesSent < fileSize) {
        if (fileSize - bytesSent >= FILE_CHUNK_SIZE)
            bytesToSend = FILE_CHUNK_SIZE;
        else
            bytesToSend = fileSize - bytesSent;
        //
        //        char *converted = bit_send(line, strlen(line));
        //        printf("%s \n", converted);
        //        send(s, converted, strlen(converted), 0);
        send(s, zBuffer + bytesSent, bytesToSend, 0);
        bytesSent += bytesToSend;
        if (++counter == 3)
            break;
    }

    delete [] fileBuffer;




    shutdown(s, SHUT_RDWR);



    return 0;
}

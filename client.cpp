
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



unsigned int hackers_delight_crc32(unsigned char *message) 
{
	int i, j;
	unsigned int byte, crc, mask;
	i = 0;
	crc = 0xFFFFFFFF;
	while (message[i] != 0) {
		byte = message[i]; // Get next byte.
		crc = crc ^ byte;
		for (j = 7; j >= 0; j--) { // Do eight times.
			mask = -(int)(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
		i = i + 1;
	}
	return ~crc;
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
    
    //get crc sum
    
    unsigned int crcVal = hackers_delight_crc32((unsigned char *)fileBuffer);
    
    printf("crc val: %d\n", crcVal);

    send(s, (const char*) &fileSize, sizeof (fileSize), 0);
    
    //send crcVal
    send(s, (const char*) &crcVal, sizeof (crcVal), 0);
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

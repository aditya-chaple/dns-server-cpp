#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>

// Add the DNSHeader structure
struct DNSHeader {
    uint16_t id;       // Packet Identifier
    uint16_t flags;    // QR, OPCODE, AA, TC, RD, RA, Z, RCODE
    uint16_t qdCount;  // Number of questions
    uint16_t anCount;  // Number of answers
    uint16_t nsCount;  // Number of authority records
    uint16_t arCount;  // Number of additional records

    // Convert to network byte order
    void toNetworkOrder() {
        id = htons(id);
        flags = htons(flags);
        qdCount = htons(qdCount);
        anCount = htons(anCount);
        nsCount = htons(nsCount);
        arCount = htons(arCount);
    }
};

// Add the DNSQuestion structure
struct DNSQuestion {
    std::vector<uint8_t> name;  // Encoded domain name
    uint16_t type;              // Record type
    uint16_t qClass;            // Record class

    // Convert to network byte order
    void toNetworkOrder() {
        type = htons(type);
        qClass = htons(qClass);
    }

    // Serialize the question to a byte vector
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> output;
        output.insert(output.end(), name.begin(), name.end());
        output.push_back(0); // Null terminator for the domain name
        output.push_back((type >> 8) & 0xFF);
        output.push_back(type & 0xFF);
        output.push_back((qClass >> 8) & 0xFF);
        output.push_back(qClass & 0xFF);
        return output;
    }
};

int main() {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // Disable output buffering
    setbuf(stdout, NULL);

    std::cout << "Logs from your program will appear here!" << std::endl;

    // Original server code starts here
    int udpSocket;
    struct sockaddr_in clientAddress;

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << "..." << std::endl;
        return 1;
    }

    int reuse = 1;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "SO_REUSEPORT failed: " << strerror(errno) << std::endl;
        return 1;
    }

    sockaddr_in serv_addr = { .sin_family = AF_INET,
                              .sin_port = htons(2053),
                              .sin_addr = { htonl(INADDR_ANY) },
                            };

    if (bind(udpSocket, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) != 0) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        return 1;
    }

    int bytesRead;
    char buffer[512];
    socklen_t clientAddrLen = sizeof(clientAddress);

    while (true) {
        bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0,
                             reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddrLen);
        if (bytesRead == -1) {
            perror("Error receiving data");
            break;
        }

        buffer[bytesRead] = '\0';
        std::cout << "Received " << bytesRead << " bytes: " << buffer << std::endl;

        // Create DNS header
        DNSHeader dnsHeader{};
        dnsHeader.id = 1234;        // Packet Identifier
        dnsHeader.flags = 0x8000;  // QR=1, OPCODE=0, AA=0, TC=0, RD=0, RA=0, Z=0, RCODE=0
        dnsHeader.qdCount = 1;     // 1 question
        dnsHeader.anCount = 0;     // No answers
        dnsHeader.nsCount = 0;     // No authority records
        dnsHeader.arCount = 0;     // No additional records

        dnsHeader.toNetworkOrder();

        // Create DNS question
        DNSQuestion dnsQuestion;
        dnsQuestion.name = {0x0C, 'c', 'o', 'd', 'e', 'c', 'r', 'a', 'f', 't', 'e', 'r', 's', 0x02, 'i', 'o'};
        dnsQuestion.type = 1;    // A record
        dnsQuestion.qClass = 1; // IN class

        dnsQuestion.toNetworkOrder();
        std::vector<uint8_t> questionData = dnsQuestion.serialize();

        // Serialize the response
        std::vector<uint8_t> responseData;
        responseData.insert(responseData.end(), reinterpret_cast<uint8_t*>(&dnsHeader), reinterpret_cast<uint8_t*>(&dnsHeader) + sizeof(dnsHeader));
        responseData.insert(responseData.end(), questionData.begin(), questionData.end());

        // Send the response
        if (sendto(udpSocket, responseData.data(), responseData.size(), 0,
                   reinterpret_cast<struct sockaddr*>(&clientAddress), clientAddrLen) == -1) {
            perror("Failed to send response");
        } else {
            std::cout << "Sent DNS response with header and question section." << std::endl;
        }
    }

    close(udpSocket);
    return 0;
}

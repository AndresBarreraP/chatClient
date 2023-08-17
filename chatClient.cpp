#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h> // Necesario para inet_pton   
#include <thread>
#include <string>
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
void ReceiveThread(SOCKET clientSocket) {
    char buffer[4096];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        // Despliegue del mensaje recibido por el cliente
        if (bytesReceived > 0) {
            std::cout << buffer << std::endl;
        }
        else if (bytesReceived == 0) {
            std::cout << "Servidor desconectada." << std::endl;
            break;
        }
        else {
            std::cerr << "Error en la recepcion de datos." << std::endl;
            break;
        }
    }
}

int main() {
    WSADATA initWinsock;
    // Inicialización de la bibilioteca de sockets (Winsock v2.2)
    if (WSAStartup(MAKEWORD(2, 2), &initWinsock) != 0) {
        std::cerr << "Error al inicializar Winsock." << std::endl;
        return 1;
    }
    // Creacion del socket del lado cliente
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error al crear el socket del cliente." << std::endl;
        WSACleanup(); // Libera los recursos utilizados por Winsock.
        return 1;
    }

    // Configuracion de dirección y puerto del socket del lado servidor utilizando
    // la estructura sockaddr_in. 
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)); // Dirección IP del servidor (localhost)

    // Se intenta establecer una conexión con el servidor utilizando el socket de cliente creado.
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Fallo la conexion al servidor." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Conectado al servidor. Comience a escribir mensajes (escriba '/salir' para salir):" << std::endl;

    // Hilo creado para la recepcion de mensajes
    std::thread receiveThread(ReceiveThread, clientSocket);
    receiveThread.detach();

    char buffer[4096];
    while (true) {
        std::string message;
        // Esperando el mensaje enviado por el cliente
        std::getline(std::cin, message);

        if (message == "/salir") {
            send(clientSocket, message.c_str(), message.size() + 1, 0);
            break;
        }
        // Envio del mensaje del cliente hacia el servidor para su posterior difusión
        send(clientSocket, message.c_str(), message.size() + 1, 0);
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

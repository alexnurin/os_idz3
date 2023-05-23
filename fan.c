#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>


int client_socket;

void exit_program()
{
    printf("\nЗакрыте socket и завершение работы\n");
    close(client_socket);
    exit(0);
}

int generate_attractiveness(int num) {
    srand(time(NULL) * num);

    int date_attractiveness = rand() % 100;

    printf("Фанат %d отправил открытку, его предложение привлекательно на %d процентов \n", num + 1,
           date_attractiveness);
    return date_attractiveness;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Использование: <%s> <server ip> <sever port> <fan id>\n", argv[0]);
        exit_program();
    }

    int fan_id = atoi(argv[3]);
    struct sockaddr_in server_address;

    signal(SIGINT, exit_program);

    // Создание клиентского сокета
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Ошибка при создании сокета\n");
        exit_program();
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));

    // Преобразование IP-адреса из строкового представления в структуру sockaddr_in
    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        perror("Ошибка при конвертации IP адреса\n");
        exit_program();
    }

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Ошибка подключения к серверу\n");
        exit_program();
    }

    printf("Подключено к серверу\n");

    int result = generate_attractiveness(fan_id);
    send(client_socket, &result, sizeof(result), 0);

    int response;
    while (1) {
        if (recv(client_socket, &response, sizeof(int), 0) > 0) {
            if (response == 1) {
                printf("Я самый счастливый человек на земле!\n");
            }
            else if (response == 0){
                printf("О нет! Как жаль!\n");
            }
            break;
        }
    }
    exit_program();
}
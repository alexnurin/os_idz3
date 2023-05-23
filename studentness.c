#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

int server_socket;

void exit_program() {
    printf("\nЗакрытие socket и завершение программы.\n");
    close(server_socket);
    exit(0);
}

int get_best_fan_index(int *scores, int fans_count) {
    int mx = -1, mx_id = -1;
    printf("Просмотр открыток начат ...\n");
    for (int i = 0; i < fans_count; ++i) {
        if (scores[i] > mx) {
            mx = scores[i];
            mx_id = i;
        }
    }
    printf("Просмотр открыток завершен\n");
    return mx_id;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Использование: <%s> <port> <fan count>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int fans_count = atoi(argv[2]);

    if (fans_count < 1 || fans_count > 10000) {
        printf("Ошибка: количество фанатов должно быть от 1 до 10000 включительно\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, exit_program);

    ssize_t read_res;
    int fans_sockets[fans_count];
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Создание серверного сокета
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Ошибка при создании сокета\n");
        exit_program();
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Привязка серверного сокета к IP-адресу и порту
    if (bind(server_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Ошибка при соединении сокета\n");
        exit_program();
    }

    // Начало прослушивания входящих подключений
    if (listen(server_socket, fans_count) < 0) {
        perror("Ошибка в listen функции\n");
        exit_program();
    }

    printf("Студентка ожидает письма\n");

    int fans_scores[fans_count];
    for (int fan = 0; fan < fans_count; fan++) {
        fans_scores[fan] = -1;
    }

    for (int i = 0; i < fans_count; ++i) {
        // Принятие входящего подключения
        if ((fans_sockets[i] = accept(server_socket, (struct sockaddr *) &address, (socklen_t * ) & addrlen)) < 0) {
            printf("Ошибка в функции accept() для клиента %d\n", i);
            exit_program();
        }
        printf("Клиент %d подключен\n", i);
    }
    int letters_got = 0;

    while (letters_got < fans_count) {
        for (int fan = 0; fan < fans_count; fan++) {
            int recieved;
            if (recv(fans_sockets[fan], &recieved, sizeof(recieved), 0) > 0) {
                if (recieved > fans_scores[fan]) {
                    if (fans_scores[fan] == -1) {
                        letters_got++;
                    }
                    fans_scores[fan] = recieved;
                    printf("Получено письмо от фаната %d привлекательностью %d\n", fan + 1, recieved);
                }
            }
        }
    }
    int best_id = get_best_fan_index(fans_scores, fans_count);
    printf("Фанат %d — самый привлекательный!\n", best_id + 1);
    int good_answer = 1, bad_answer = 0;
    for (int i = 0; i < fans_count; ++i) {
        int answer = bad_answer;
        if (i == best_id) {
            answer = good_answer;
        }
        send(fans_sockets[i], &answer, sizeof(answer), 0);
    }

    int exit_code = -1;
    for (int i = 0; i < fans_count; ++i) {
        // Отправка кода клиенту для завершения работы программы
        send(fans_sockets[i], &exit_code, sizeof(exit_code), 0);
    }
    // Завершение программы:
    printf("День завершён.\n");
    exit_program();
}

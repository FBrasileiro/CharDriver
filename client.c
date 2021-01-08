#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define DEVICE_NAME "/dev/char_driver"

void read_d(int fd)
{
    int error = 0;
    char *data = (char *)malloc(1024 * sizeof(char));
    data[0] = '\0';
    if (read(fd, data, BUFFER_SIZE) < 0)
        printf("Error while reading\n");
    else if (data[0] != '\0')
        printf("Readed: %s\n", data);

    free(data);
}

void write_d(int fd)
{
    int error = 0;
    char *data = (char *)malloc(1024 * sizeof(char));
    printf("> ");
    scanf(" %[^\n]s", data);
    fflush(stdout);
    if (write(fd, data, strlen(data)) < 0)
        printf("Error while writing\n");
    free(data);
}

int main()
{
    system("clear");
    int option;
    if (access(DEVICE_NAME, F_OK) == -1)
    {
        printf("Can't find the module: %s\n", DEVICE_NAME);
        return -1;
    }

    while (1)
    {
        int fd;
        if ((fd = open(DEVICE_NAME, O_RDWR)) < 0)
        {
            printf("Could not open the module: %s\n", DEVICE_NAME);
            return 0;
        }
        printf("-=-=-=-=-=[ Select an option ]=-=-=-=-=-\n");
        printf("               1- Read\n");
        printf("               2- Write\n");
        printf("               3- Exit\n");
        printf("\nOPTION: ");
        scanf("%i", &option);
        fflush(stdin);
        system("clear");
        switch (option)
        {
        case 1:
            read_d(fd);
            break;
        case 2:
            write_d(fd);
            break;
        case 3:
            close(fd);
            return 0;
        default:
            break;
        }
        close(fd);
    }

    return 0;
}

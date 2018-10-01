#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define BYTE_PER_SECTOR 512

int adjust_in_sector_size(int fd, int source_size);
void write_kernel_information(int target_fd, int total_kernel_sector_count, int kernel32_sector_count);
int copy_file(int source_fd, int target_fd);

int main(int argc, char *argv[])
{
	int source_fd;
	int target_fd;
	int boot_loader_size;
	int kernel32_sector_count;
	int kernel64_sector_count;
	int source_size;

	if (argc < 4) {
		fprintf(stderr, "[ERROR] ImageMaker BootLoader.bin kernel32.bin kernel64.bin\n");
		exit(EXIT_FAILURE);
	}

	if ((target_fd = open("Disk.img", O_RDWR | O_CREAT | O_TRUNC,
					S_IREAD | S_IWRITE)) == -1) {
		fprintf(stderr, "[ERROR] Disk.img open fail.\n");
		exit(EXIT_FAILURE);
	}


	printf("[INFO] copy a boot loader to image file\n");
	if ((source_fd = open(argv[1], O_RDONLY)) == -1) {
		fprintf(stderr, "[ERROR] %s open fail.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	source_size = copy_file(source_fd, target_fd);
	close(source_fd);

	/* 파일 크기를 512바이트 단위로 맞추기 위해 0x00으로 채움 */
	boot_loader_size = adjust_in_sector_size(target_fd, source_size);
	printf("[INFO] %s size = [%d] and sector count = [%d]\n", argv[1], source_size, boot_loader_size);

	printf("[INFO] Copy the protected mode kernel to image file\n");
	if ((source_fd = open(argv[2], O_RDONLY)) == -1) {
		fprintf(stderr, "[ERROR] %s open fail.\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	source_size = copy_file(source_fd, target_fd);
	close(source_fd);

	kernel32_sector_count = adjust_in_sector_size(target_fd, source_size);
	printf("[INFO] %s size = [%d] and sector count = [%d]\n", argv[2], source_size, kernel32_sector_count);

	/* copy 64bit kernel */
	printf("[INFO] Copy IA-32e mode kernel to image file\n");
	if ((source_fd = open(argv[3], O_RDONLY)) == -1) {
		fprintf(stderr, "[ERROR] %s open fail.\n", argv[3]);
		exit(EXIT_FAILURE);
	}

	source_size = copy_file(source_fd, target_fd);
	close(source_fd);
	
	/* adjust kernel size */
	kernel64_sector_count = adjust_in_sector_size(target_fd, source_size);
	printf("[INFO] %s size = [%d] and sector count = [%d]\n", argv[3], source_size, kernel64_sector_count);

	/* change sector count of bootloader */
	printf("[INFO] Start to write kernel information\n");
	write_kernel_information(target_fd, kernel32_sector_count + kernel64_sector_count, kernel32_sector_count);
	printf("[INFO] Image file create complete\n");

	close(target_fd);

	return 0;
}


/* 현재 위치부터 512바이트 배수 위치까지 0x00으로 채움 */
int adjust_in_sector_size(int fd, int source_size)
{
	int i;
	int adjust_size_to_sector;
	char ch;
	int sector_count;

	adjust_size_to_sector = source_size % BYTE_PER_SECTOR;
	ch = 0x00;

	if (adjust_size_to_sector != 0) {
		adjust_size_to_sector = 512 - adjust_size_to_sector;
		printf("[INFO] File size [%u] and fill [%u] byte\n", source_size, adjust_size_to_sector);
		for (i = 0; i < adjust_size_to_sector; i++)
			write(fd, &ch, 1);
	}
	else
		printf("[INFO] File size is already aligned 512 byte\n");

	sector_count = (source_size + adjust_size_to_sector) / BYTE_PER_SECTOR;

	return sector_count;
}

void write_kernel_information(int target_fd, int total_kernel_sector_count, int kernel32_sector_count)
{
	unsigned short data;
	long position;

	if ((position = lseek(target_fd, 5, SEEK_SET)) == -1) {
		fprintf(stderr, "lseek fail. Return value = %ld, errno = %d, %d\n", position, errno, SEEK_SET);
		exit(EXIT_FAILURE);
	}

	data = (unsigned short) total_kernel_sector_count;
	write(target_fd, &data, 2);
	data = (unsigned short) kernel32_sector_count;
	write(target_fd, &data, 2);

	printf("[INFO] Total sector count except boot loader [%d]\n", total_kernel_sector_count);
	printf("[INFO] Total sector count of protected mode kernel [%d]\n", kernel32_sector_count);
}

int copy_file(int source_fd, int target_fd)
{
	int source_file_size;
	int read_count;
	int write_count;
	char buffer[BYTE_PER_SECTOR];

	source_file_size = 0;
	while (1) {
		read_count = read(source_fd, buffer, sizeof(buffer));
		write_count = write(target_fd, buffer, read_count);

		if (read_count != write_count) {
			fprintf(stderr, "[ERROR] read != write...\n");
			exit(EXIT_FAILURE);
		}
		source_file_size += read_count;

		if (read_count != sizeof(buffer))
				break;
	}

	return source_file_size;
}

		



#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <openssl/pem.h>
#include <arpa/inet.h>
#define MAX 1000000

int sockfd;
int padding = RSA_SSLV23_PADDING;
void* run(void* key)
{
	char buff[MAX];
	unsigned char encrypted_txt[MAX];
	RSA *rsa = RSA_new();
	rsa = PEM_read_RSA_PUBKEY(key, &rsa, NULL, NULL);
	if (rsa == NULL)
	{
		printf("RSA creation failed....\n");
		return 0;
	}

	while(1)
	{
		bzero(buff, MAX);
		scanf("%[^\n]%*c", buff);
		int cnt = RSA_public_encrypt(strlen(buff), (unsigned char*)buff, encrypted_txt, rsa, padding);
		write(sockfd, encrypted_txt, cnt);
		if ((strncmp(buff, "exit", 4)) == 0) break;
	}
	close(sockfd);
	fclose(key);
	exit(0);
}

int main(int argc, char *argv[])
{
	if (argc != 5)
	{
		printf("Command line arguments are not correct.\n");
		return 0;
	}
	char *ip = argv[1];
	int port = atoi(argv[2]);
	FILE *private_key = fopen(argv[3], "rb");
	FILE *public_key = fopen(argv[4], "rb");

	if(public_key == NULL){
		printf("There is no private key file of this name.\n");
		return 0;
	}
	if(public_key == NULL){
		printf("There is no public key file of this name.\n");
		return 0;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("Socket creation failed...\n");
		exit(1);
	}
	printf("Socket successfully created..\n");

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
	{
		printf("Connection with the server failed...\n");
		exit(1);
	}
	printf("Connected to the server..\n");

	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, run, public_key);

	char decrypted_txt[MAX];
	unsigned char encrypted_txt[MAX];

	RSA *rsa = RSA_new();
	rsa = PEM_read_RSAPrivateKey(private_key, &rsa, NULL, NULL);
	if(rsa == NULL)
	{
		printf("RSA creation failed....\n");
		return 0;
	}
	int cnt;
	bzero(decrypted_txt, MAX);
	bzero(encrypted_txt, MAX);
	while ((cnt=read(sockfd, encrypted_txt, MAX)) > 0)
    {
		RSA_private_decrypt(cnt, encrypted_txt, (unsigned char *)decrypted_txt, rsa, padding);
		if ((strncmp(decrypted_txt, "exit", 4)) == 0) break;

		printf("Cipher-Text: %s\nPlain-Text: %s\n", encrypted_txt, decrypted_txt);
		fflush(stdout);
		bzero(encrypted_txt, MAX);
		bzero(decrypted_txt, MAX);
	}
	close(sockfd);
	fclose(private_key);
	exit(0);
}

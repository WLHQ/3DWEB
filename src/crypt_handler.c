#include "handlers.h"
#include <string.h>

#define MAX_CRYPT_BYTES 0x400

static int is_hex_char(char c)
{
	return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9');
}

static const unsigned char hex_lookup[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
	0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int is_crypt_request(http_request *request)
{
	if (!startWith(request->path, "/crypt/") || strlen(request->path) < 8) // Ensure valid tokenization
		return 0;

	char *dup = strdup(request->path);
	int numParams = 0;
	int valid = 1;
	int l = 0;
	int ktype;
	char *saveptr;

	char *p = strtok_r(dup+1, "/", &saveptr);

	p = strtok_r(NULL, "/", &saveptr); // "crypt/"

	while (p != NULL)
	{
		numParams++;
		switch(numParams)
		{
			case 1:
			if (atoi(p) > 5)
				valid = 0;
			break;
			case 2:
				ktype = atoi(p);
				if (ktype < 0)
					valid = 0;
				else if (ktype >= 10 && ktype < 0x80)
					valid = 0;
				else if (ktype >= 0x100)
					valid = 0;
				break;
			case 3:
				if (strlen(p) != 0x20)
				{
					valid = 0;
					break;
				}
				for (int i = 0; i < 0x20; i++)
				{
					if (!is_hex_char(p[i]))
					{
						valid = 0;
						break;
					}
				}
				break;
			case 4:
				l = strlen(p);
				if (l % 2 != 0)
					valid = 0;
				if (l > 2 * MAX_CRYPT_BYTES) // 0x400 bytes per request, maximum.
					valid = 0;
				char *temp = p;
				while (*temp)
				{
					if (!is_hex_char(*temp))
					{
						valid = 0;
						break;
					}
					temp++;
				}
				break;
			case 5:
				if (strlen(p) != 0x20)
				{
					valid = 0;
					break;
				}
				for (int i = 0; i < 0x20; i++)
				{
					if (!is_hex_char(p[i]))
					{
						valid = 0;
						break;
					}
				}
				break;
			default:
				valid = 0;
				break;
		}
		p = strtok_r(NULL, "/", &saveptr);
	}
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
	if (numParams == 4 && !(ktype & 0x40))
	{
		// Do Nothing
	}
	else if (numParams == 5 && (ktype & 0x40))
	{
		// Do Nothing
	}
	else
	{
		valid = 0;
	}
	#pragma GCC diagnostic pop
	free(dup);
	return valid;
}
static int do_crypto_request(char *path, char *outbuf)
{
	unsigned char inbuf[MAX_CRYPT_BYTES];
	unsigned char iv[0x10];

	uint8_t *FCRAM = (uint8_t *)0x15000000;

	PS_AESAlgorithm algo = PS_ALGORITHM_CBC_ENC;

	PS_AESKeyType ktype = PS_KEYSLOT_0D;

	int buf_l = 0;

	memset(inbuf, 0, MAX_CRYPT_BYTES);
	memset(outbuf, 0, MAX_CRYPT_BYTES);

	char *dup = strdup(path);
	int param = 0;
	int l = 0;
	char *saveptr;

	char *p = strtok_r(dup+1, "/", &saveptr);

	p = strtok_r(NULL, "/", &saveptr); // "crypt/"

	while (p != NULL)
	{
		param++;
		switch (param)
		{
			case 1:
				algo = (PS_AESAlgorithm)atoi(p);
				break;
			case 2:
				ktype = (PS_AESKeyType)atoi(p);
				break;
			case 3:
				for (int i = 0; i < 0x10; i++)
				{
					iv[i] = (hex_lookup[(unsigned char)p[2*i]] << 4) | hex_lookup[(unsigned char)p[2*i+1]];
				}
				break;
			case 4:
				l = strlen(p);
				buf_l = l/2;
				for (int i = 0; i < buf_l; i++)
				{
					inbuf[i] = (hex_lookup[(unsigned char)p[2*i]] << 4) | hex_lookup[(unsigned char)p[2*i+1]];
				}
				break;
			case 5:
				for (int i = 0; i < 0x10; i++)
				{
					FCRAM[i] = (hex_lookup[(unsigned char)p[2*i]] << 4) | hex_lookup[(unsigned char)p[2*i+1]];
				}
				break;
			default:
				break;
		}
		p = strtok_r(NULL, "/", &saveptr);
	}

	const char * algos[6] = {"CBC Enc", "CBC Dec", "CTR Enc", "CTR Dec", "CCM Enc", "CCM Dec"};
	const int ktypes[10] = {0xD, 0x2D, 0x31, 0x38, 0x32, 0x39, 0x2E, 0x7, 0x36, 0x39};

	int usedkeyslot = (ktype & 0x80) ? (ktype & 0x3F) : (ktypes[ktype]);


	printTop("Crypto Request: %s, keyslot 0x%X, 0x%X bytes.\n", algos[algo], (int)usedkeyslot, buf_l);
	
	PS_EncryptDecryptAes((uint32_t)buf_l, (unsigned char *)inbuf, (unsigned char *)outbuf, algo, ktype, iv);


	free(dup);

	return buf_l;
}



http_response *get_crypt_handler_response(http_request *request)
{
	http_response *response = memalloc(sizeof(http_response));
	response->code = 200;
	response->content_type = strdup("Content-Type: application/octet-stream\r\n");
	char	*payload = memalloc(1024 * sizeof(char));
	int len = do_crypto_request(request->path, payload);
	response->payload = payload;
	response->payload_len = len;
	return response;
}

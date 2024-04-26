#include "handlers.h"

#define MAX_CRYPT_BYTES 0x400

static int is_hex_char(char c)
{
	return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9');
}

static unsigned char decode_hex(char c)
{
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= '0' && c <= '9')
		return c - '0';
	return 0; // pls
}

int is_crypt_request(http_request *request)
{
	if (!startWith(request->path, "/crypt/") || strlen(request->path) < 8) // Ensure valid tokenization
		return 0;

	char *dup = strdup(request->path);
	int numParams = 0;
	int valid = 1;
	int l = 0;
	int ktype;

	char *p = strtok(dup+1, "/");

	p = strtok(NULL, "/"); // "crypt/"

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
		p = strtok(NULL, "/");
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

	for (int i = 0; i < MAX_CRYPT_BYTES; i++)
	{
		inbuf[i] = 0;
		outbuf[i] = 0;
	}

	char *dup = strdup(path);
	int param = 0;
	int ofs = 0;
	int l = 0;

	char *p = strtok(dup+1, "/");

	p = strtok(NULL, "/"); // "crypt/"

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
				for (int i = 0; i < 0x20; i += 2)
				{
					iv[i/2] = (decode_hex(p[i]) << 4) | decode_hex(p[i+1]);
				}
				break;
			case 4:
				ofs = 0;
				l = strlen(p);
				buf_l = l/2;
				while (ofs < l)
				{
					inbuf[ofs/2] = (decode_hex(p[ofs]) << 4) | decode_hex(p[ofs+1]);
					ofs += 2;
				}
				break;
			case 5:
				for (int i = 0; i < 0x20; i += 2)
				{
					FCRAM[i/2] = (decode_hex(p[i]) << 4) | decode_hex(p[i+1]);
				}
				break;
			default:
				break;
		}
		p = strtok(NULL, "/");
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

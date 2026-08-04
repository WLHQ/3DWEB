/* Minimal PHP-proxy handler for WLHQ/3DWEB
   Idea: forward requests for "*.php" to an external HTTP server running PHP (php -S ...).
   Configure backend by editing PHP_BACKEND_HOST and PHP_BACKEND_PORT, or pass -DPHP_BACKEND_HOST and -DPHP_BACKEND_PORT at compile time.
   Not intended to be secure or production-grade; for controlled LAN/prank use only.
*/
#include "handlers.h"
#include "http_types.h"
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#ifndef PHP_BACKEND_HOST
#define PHP_BACKEND_HOST "192.168.0.100" /* <-- change to your laptop IP or define at compile time */
#endif
#ifndef PHP_BACKEND_PORT
#define PHP_BACKEND_PORT 8000
#endif

static int ends_with(const char *s, const char *suffix) {
    if (!s || !suffix) return 0;
    size_t sl = strlen(s), su = strlen(suffix);
    if (su > sl) return 0;
    return strcmp(s + sl - su, suffix) == 0;
}

int is_php_request(http_request *request)
{
    if (!request || !request->path) return 0;
    return ends_with(request->path, ".php");
}

/* return pointer to first char after "/sdcard" in path (or whole path if not sdcard) */
static const char *map_path_for_backend(const char *path)
{
    if (!path) return "/";
    const char *p = path;
    if (strncmp(path, "/sdcard", 7) == 0) {
        /* skip "/sdcard" */
        p = path + 7;
        if (*p == '\0') return "/";
        return p;
    }
    return path;
}

http_response *get_php_response(http_request *request)
{
    http_response *response = memalloc(sizeof(http_response));
    response->code = 502;
    response->content_type = memdup("Content-Type: text/html\r\n", sizeof("Content-Type: text/html\r\n"));
    response->payload = memdup("<html><body><h1>502 Bad Gateway</h1></body></html>", 44);
    response->payload_len = strlen(response->payload);

    /* Resolve backend address */
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PHP_BACKEND_PORT);
    if (inet_pton(AF_INET, PHP_BACKEND_HOST, &addr.sin_addr) != 1) {
        return response;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return response;
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return response;
    }

    /* Build a minimal HTTP request to backend */
    const char *mapped = map_path_for_backend(request->path);
    char reqbuf[1024];
    const char *method = "GET";
    if (request->payload && strlen(request->payload) > 0) method = "POST";

    /* Send request line and headers; we use HTTP/1.0 so backend will close */
    int len = snprintf(reqbuf, sizeof(reqbuf),
                       "%s %s HTTP/1.0\r\n"
                       "Host: " PHP_BACKEND_HOST "\r\n"
                       "Connection: close\r\n",
                       method,
                       mapped[0] ? mapped : "/");
    /* If POST, include content length and minimal content-type */
    if (strcmp(method, "POST") == 0) {
        char clen[64];
        size_t plen = request->payload ? strlen(request->payload) : 0;
        snprintf(clen, sizeof(clen), "Content-Length: %zu\r\n", plen);
        if ((len + (int)strlen(clen)) < (int)sizeof(reqbuf))
            len += snprintf(reqbuf + len, sizeof(reqbuf) - len, "%s", clen);
        if ((len + 32) < (int)sizeof(reqbuf))
            len += snprintf(reqbuf + len, sizeof(reqbuf) - len, "Content-Type: application/x-www-form-urlencoded\r\n");
    }

    /* end headers */
    if ((len + 4) < (int)sizeof(reqbuf))
        len += snprintf(reqbuf + len, sizeof(reqbuf) - len, "\r\n");

    /* write headers (and body if POST) */
    if (write(sock, reqbuf, len) != len) { close(sock); return response; }
    if (strcmp(method, "POST") == 0 && request->payload) {
        size_t towrite = strlen(request->payload);
        ssize_t w = write(sock, request->payload, towrite);
        (void)w;
    }

    /* Read response into dynamic buffer */
    size_t cap = 8192;
    size_t used = 0;
    char *buf = memalloc(cap);
    ssize_t r;
    while ((r = read(sock, buf + used, cap - used)) > 0) {
        used += r;
        if (cap - used < 1024) {
            cap *= 2;
            buf = memrealloc(buf, cap);
        }
    }
    close(sock);
    if (used == 0) {
        memfree(buf);
        return response;
    }

    /* Find header/body split */
    char *sep = NULL;
    for (size_t i = 0; i + 3 < used; ++i) {
        if (buf[i] == '\r' && buf[i+1] == '\n' && buf[i+2] == '\r' && buf[i+3] == '\n') {
            sep = buf + i + 4;
            break;
        }
    }

    if (!sep) {
        /* no header split: treat entire response as body */
        memfree(response->payload);
        response->payload = memalloc(used + 1);
        memcpy(response->payload, buf, used);
        response->payload[used] = '\0';
        response->payload_len = used;
        memfree(buf);
        response->code = 200;
        memfree(response->content_type);
        response->content_type = memdup("Content-Type: text/html\r\n", sizeof("Content-Type: text/html\r\n"));
        return response;
    }

    size_t header_len = (size_t)(sep - buf);
    size_t body_len = used - header_len;
    /* Look for Content-Type header */
    char *ct = NULL;
    buf[header_len - 1] = '\0'; /* temporarily null-terminate headers for strstr usage */
    char *ct_hdr = strstr(buf, "Content-Type:");
    if (ct_hdr) {
        ct_hdr += strlen("Content-Type:");
        while (*ct_hdr == ' ') ct_hdr++;
        char *end = strstr(ct_hdr, "\r\n");
        if (end) {
            size_t ctlen = end - ct_hdr;
            ct = memalloc(ctlen + sizeof("Content-Type: \r\n"));
            snprintf(ct, ctlen + sizeof("Content-Type: \r\n"), "Content-Type: %.*s\r\n", (int)ctlen, ct_hdr);
        }
    }

    /* set response */
    memfree(response->payload);
    response->payload = memalloc(body_len + 1);
    memcpy(response->payload, sep, body_len);
    response->payload[body_len] = '\0';
    response->payload_len = body_len;
    response->code = 200;
    memfree(response->content_type);
    if (ct) {
        response->content_type = ct;
    } else {
        response->content_type = memdup("Content-Type: text/html\r\n", sizeof("Content-Type: text/html\r\n"));
    }

    memfree(buf);
    return response;
}

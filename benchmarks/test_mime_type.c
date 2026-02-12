#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mime_type.h"

int failed_tests = 0;

void verify(const char *path, const char *expected) {
	const char *actual = get_mime_type(path);
	if (strcmp(actual, expected) != 0) {
		printf("FAILED: path='%s', expected='%s', got='%s'\n", path, expected, actual);
		failed_tests++;
	} else {
		printf("PASS: path='%s' -> '%s'\n", path, actual);
	}
}

int main() {
	printf("Running MIME type tests...\n");

	// Test known extensions
	verify("index.html", "text/html");
	verify("page.htm", "text/html");
	verify("style.css", "text/css");
	verify("script.js", "application/javascript");
	verify("data.json", "application/json");
	verify("image.png", "image/png");
	verify("photo.jpg", "image/jpeg");
	verify("photo.jpeg", "image/jpeg");
	verify("anim.gif", "image/gif");
	verify("image.webp", "image/webp");
	verify("favicon.ico", "image/x-icon");
	verify("readme.txt", "text/plain");
	verify("data.xml", "text/xml");
	verify("doc.pdf", "application/pdf");

	// Test case insensitivity
	verify("INDEX.HTML", "text/html");
	verify("Photo.Jpg", "image/jpeg");
	verify("script.JS", "application/javascript");

	// Test edge cases
	verify("file_no_ext", "application/octet-stream");
	verify("file.unknown", "application/octet-stream");
	verify("", "application/octet-stream");
	verify("folder.with.dots/file", "application/octet-stream");
	verify("archive.tar.gz", "application/octet-stream"); // .gz is not mapped, so default
	// If I wanted to support .gz, I would need to add it. But for now I test existing behavior.

	// Test multiple dots
	verify("jquery.min.js", "application/javascript");

	if (failed_tests > 0) {
		printf("\n%d tests failed!\n", failed_tests);
		return 1;
	}

	printf("\nAll tests passed!\n");
	return 0;
}

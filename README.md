# 3DWEB
An http server written in C made for the 3DS. A continuation of 3ds-httpd from dimaguy. https://github.com/dimaguy/3ds-httpd

Features:

Creates a directory called "Websites" & an index.html file when the server starts. You can add your own files in the Websites directory on your SD Card. The files put onto your SD Card in the Websites directory will not be overwritten.

TODO:

MIME Types Support: E.g PNGs, JPEGs, etc.

### Path Handlers
#### Default Handler  
	- Path: /  
	- Description:  
		Returns a hardcoded HTML response  
#### Favicon Handler  
	- Path: /favicon.ico  
	- Description:  
		Returns a hardcoded SVG favicon  
#### System Handler  
	- Path: /system/  
	- Description:  
		/system/exit - Exits app  
		/system/reboot - Reboots system  
#### SDMC Handler  
	- Path: /sdcard/  
	- Description:  
		Returns content of requested file in SD Card with content-type text/html, if file is missing returns 404. Path starts at SDMC root  
#### Memory R/W Handler  
	- Path: /readmem/ || /writemem/  
	- Description:  
		Reads and writes system memory. Honestly you're more likely to crash your 3DS. Check source code for details  
#### Encryption Handler  
	- Path: /crypt/  
	- Description:  
		Uses native AES Encryption/Decryption Algorithms to do CBC/CTR/CTM. Check source code for details.

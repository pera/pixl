/* 
 * Copyright (C) 2012 - Brian Gomes Bascoy
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#include "filesystem.h"

const char* PIXL_LoadTextFile(const char *file_name)
{
	FILE *fp;
	struct stat buf;
	char *source = NULL;

	if((stat(file_name, &buf)) < 0){
		fprintf(stderr, "Error: stat %s\n", file_name);
		//exit(EXIT_FAILURE);
	}

	if(!(fp = fopen(file_name, "r"))){
		fprintf(stderr, "Error: open %s\n", file_name);
		//exit(EXIT_FAILURE);
	}

	// Le sumamos 1 al tamaño del string para el null-terminated
	if(!(source = (char*) malloc ((sizeof(char) * buf.st_size) + 1))){
		fprintf(stderr, "Error: malloc\n");
		//exit(EXIT_FAILURE);
	}

	fread(source, sizeof(char), buf.st_size, fp);
	source[buf.st_size] = '\0'; // Esto nos asegura que el final del string sea null-terminated, sino podriamos obtener un ETB antes

	if(fclose(fp)){
		fprintf(stderr, "Error: close %s\n", file_name);
		//exit(EXIT_FAILURE);
	}

	return source;
}


/*  
	svg-line_2_gcode allow to convert svg file exported by librecad (MakerCAM SVG)
	as a Gcode file compatible with most CNC
	
    Copyright (C) 2023 Cyrille Henry

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/    

// compilation :
// gcc -o svg-line_2_gcode svg-line_2_gcode.c -lm

// usage : 
// svg-line_2_gcode file.svg
// output : file.gcode

// version 1.0

#include <stdio.h>
#include <string.h>

#define move_speed 5000
#define cut_speed 1000
#define clearance 5 // z hight of clearance level
#define default_Z_cut 1 // z cut if it's not provided by the user

#define min(i, j) (((i) < (j)) ? (i) : (j))
#define max(i, j) (((i) > (j)) ? (i) : (j))

#define max_pass_per_layer 10
#define MAX_LINE_LENGTH 1000
#define max_pass_size 100000

int main(int argc, char** argv) {
	FILE    *textfile, *newtextfile;
	char    textfilename[100];
    char    stringtextfile[max_pass_per_layer][max_pass_size];
	char    line[MAX_LINE_LENGTH];
    char    line_1[MAX_LINE_LENGTH]; // line n+1
    char    temp[MAX_LINE_LENGTH];
	int     i, nb_layer, nb_pass, nb_pass_max;
	int     oldline; // si on as une ligne en cour
	float   x1, x2, y1 ,y2, x2old, y2old, ymax, profondeur;
	
	for (i=0; i<max_pass_per_layer; i++) {
	    stringtextfile[i][0] = '\0';   // clear the string
	}
	                
	if ( argc <= 1 ) {
		printf("need 1 argument : the svg file to convert\n");
		return 0;
	}
	
    if (strlen(argv[1]) < 4) {
        printf("Not a .svg file\n");
		return 0;
	}
    
    char *ext = strrchr(argv[1], '.');
    if(!ext || strcmp(ext, ".svg")) {
        printf("Not a .svg file\n");
        return 0;
    }
    
    sprintf(textfilename, "%.*s%s", (int) (ext - argv[1]), argv[1], ".gcode");
    
    textfile = fopen(argv[1], "r");
	if(textfile == NULL) {
		printf("can not read svg file\n");
		return 0;
	}
	
    printf("loading the svg file\n");
    
    newtextfile = fopen(textfilename, "w"); // opening the gcode file to write	
	if(newtextfile == NULL){
      printf("Error!, can not write file\n");   
      return 1;
    }
    printf("opening the gcode file\n");
    
    // put prolog data in the Gcode
    fputs("G21 ; Set units to mm\n", newtextfile);
    fputs("G90 ; Absolute positioning\n", newtextfile);
    fputs("\n", newtextfile);
    
    oldline = 0;
    x1 = 0;
    y1 = 0;
    x2 = 0;
    y2 = 0;
    nb_layer = 0;
    ymax = 0;
    nb_pass = 1;
           
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1st pass to get Ymax : the svg origin isd top left, bug we want the origin to be on the bottom left. We need to revert the Y axe   
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    while(fgets(line, MAX_LINE_LENGTH, textfile)){ // for all line in the svg file
        char *temp = strstr(line, "line");
        if (temp) { // we have a 'line'
            if(sscanf(line, "%*s x1=\"%f%*s y1=\"%f%*s x2=\"%f%*s y2=\"%f",&x1,&y1,&x2,&y2) == 4) { // get the line data
                ymax = max(y1,ymax);
                ymax = max(y2,ymax);                
            } 
            else {
		        printf("can not read line the line data\n");
		        return 0;
            }
        }
        temp = strstr(line, "layername="); 
        if (temp) { // we have a new layer ('layername') 
          	nb_layer++;
          	//nb_pass = 1;
          	//profondeur = default_Z_cut;
            sscanf(line, "%*s lc:layername=\"%*s %f %d",&profondeur, &nb_pass);
        }
        nb_pass_max = max(nb_pass_max, nb_pass); // get the maximum number of pass per layer
    }
    
    fclose(textfile); // close and open the file again to read it again from the begining. TODO : ca we avoid this close/open???
    textfile = fopen(argv[1], "r");
	if(textfile == NULL) {
		printf("can not read svg file\n");
		return 0;
	}
    
    if (nb_pass_max > max_pass_per_layer) {
        nb_pass_max = max_pass_per_layer;
    	printf("the number of pass have been limited to %d, because of a limitation of this software.\n", nb_pass_max);
    }
    
    printf("found %d layer and a maximum of %d pass per layer\n", nb_layer, nb_pass_max);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 2eme pass for the actual conversion
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

  	while(fgets(line, MAX_LINE_LENGTH, textfile)){ // for all lines
  	    char *temp = strstr(line, "layername=");   
  	    if (temp) { // we have a layername
            profondeur = default_Z_cut;
  	        nb_pass = 1;
  	        sscanf(line, "%*s lc:layername=\"%*s %f %d",&profondeur, &nb_pass);
  	        oldline = 0;
  	        nb_pass = min(nb_pass, max_pass_per_layer); // TODO : remove this limitation with a memaloc for stringtextfile
            //printf("find layer\n");
            fputs("; layer ", newtextfile);
            i=11; // go to the begginig of the layer name
            while( (temp[i] != ' ') && (temp[i] != '"') ) { // copy the layer name as a comment in the Gcode
                fputc(temp[i], newtextfile);
                i++;
            }
            fprintf(newtextfile, ", pass : %d, Z : %f\n", nb_pass, profondeur);
        }
        temp = strstr(line, "line");
        if (temp) { // we get a new line
            if(sscanf(line, "%*s x1=\"%f%*s y1=\"%f%*s x2=\"%f%*s y2=\"%f",&x1,&y1,&x2,&y2) !=4) {
 		        printf("error reading a line data\n");
		        return 0;
	        } else {
	            y1 = ymax-y1; // Y axes miror
	            y2 = ymax-y2;
	            
	            for(i=0; i<nb_pass; i++) { // we write simultaneouslly all pass in memory
	                //printf("%f %f %f %f\n", x1, y1, x2, y2);
	                if ( oldline && (x1==x2old) && (y1==y2old) ) { // if we can continue the previous line
                        sprintf(temp, "G1 X%F Y%F\n", x2, y2 );
                   	    strcat(stringtextfile[i], temp);     
                   	    //sprintf(stringtextfile[i], "G1 X%F Y%F\n", x2, y2 );
                    } else {
	                    sprintf(temp, "G1 Z%i F%i ; Move to clearance level\n", clearance, move_speed); // create a command
	                    strcat(stringtextfile[i], temp); // write this command in memory for pass n°i
	                    sprintf(temp, "G1 X%F Y%F\n", x1, y1 );
	                    strcat(stringtextfile[i], temp);
	                    sprintf(temp, "G1 Z-%f F%i ; go down\n", (i+1)*profondeur/nb_pass, cut_speed);            
	                    strcat(stringtextfile[i], temp);
	                    sprintf(temp, "G1 X%F Y%F\n", x2, y2 );              
	                    strcat(stringtextfile[i], temp);
	                }
                }
                oldline = 1; 
                x2old = x2; // to test if we can continue this line
                y2old = y2;
	        }               
        }
        
        temp = strstr(line, "</g>");
        if (temp) { // we test for the end of a layer
            for(i=0; i<nb_pass; i++) { // copy all pass one after the other
                fprintf(newtextfile, "; pass N° : %i/%i, Z = %f\n", i+1, nb_pass, (i+1)*profondeur/nb_pass); // add a new pass comment in the gcode file
                fputs(stringtextfile[i], newtextfile); // copy pass N° i
                stringtextfile[i][0] = '\0';   // clear the string
                fputs("\n", newtextfile);
            }
        }          
    }

    // end of the file
    fprintf(newtextfile, "G1 Z%i F%i ; Move to clearance level\n", clearance, move_speed);
    fprintf(newtextfile, "G1 X0 Y0 ; go back to the origin\n");
    	                
    fclose(textfile);
	fclose(newtextfile);
	printf("convertion finised!\n");
	return 1;
}

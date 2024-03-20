//
//      NODE Navigator v0.8
//
//      Source for Watcom C++32 compiler
//
//      Frank Palazzolo
//      palazzol@msen.com
//
//      Comments and enhancements are welcome!!!
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <types.h>

#include <graph.h>

#include "nodenav.h"

#define min(a,b)  (((a) < (b)) ? (a) : (b))
#define max(a,b)  (((a) > (b)) ? (a) : (b))

FILE	*fp;
unsigned long int stack_ptr;

enum {BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, WHITE,
        GRAY, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED, LIGHTMAGENTA,
        YELLOW, BRIGHTWHITE};

#define LIGHTYELLOW    YELLOW

int main(int argc, char *argv[])
{

        char filename[80];
        char mission[80];

        if (argc < 2) {
           printf("Usage: %s mission \[wadfile\]\n",argv[0]);
           exit(-1);
        } else { if (argc == 2) 
                      strcpy(filename, "DOOM.WAD");
                 else
                      strcpy(filename, strupr(argv[2]));
        }

        strcpy(mission, strupr(argv[1]));

        DIRENTRYTYPE *seek_entry(DIRENTRYTYPE *,char *);
        int read_linedefs(LINEDEFTYPE *, DIRENTRYTYPE *);
        int read_sidedefs(SIDEDEFTYPE *, DIRENTRYTYPE *);
        int read_vertices(VERTEXTYPE *, DIRENTRYTYPE *);
        int read_segments(SEGMENTTYPE *, DIRENTRYTYPE *);
        int read_ssectors(SSECTORTYPE *, DIRENTRYTYPE *);
        int read_nodes(NODETYPE *, DIRENTRYTYPE *);
        int read_sectors(SECTORTYPE *, DIRENTRYTYPE *);

        HEADERTYPE wadheader;

        DIRENTRYTYPE direntry;
        LINEDEFTYPE *linedefs;
        SIDEDEFTYPE *sidedefs;
        VERTEXTYPE *vertices;
        SEGMENTTYPE *segments;
        SSECTORTYPE *ssectors;
        NODETYPE *nodes;
        SECTORTYPE *sectors;
 
        int num_linedefs = 0;
        int num_sidedefs = 0;
        int num_vertices = 0;
        int num_segments = 0;
        int num_ssectors = 0;
        int num_nodes = 0;
        int num_sectors = 0;

        if ((fp = fopen(filename,"rb")) == NULL) {
                fprintf(stderr,"No %s file found\n",filename);
                return(-1);
        }

        fread(&wadheader, sizeof(wadheader), 1, fp);

        printf("File ID: %c%c%c%c\n",wadheader.type[0],wadheader.type[1],
                wadheader.type[2],wadheader.type[3]);
        printf("# of Directory entries: %ld\n",wadheader.num_dir_entries);

        if (fseek(fp,wadheader.dirpointer,SEEK_SET))
                fprintf(stderr,"Directory Seek Failed!\n");

        if (seek_entry(&direntry,mission) == NULL) {
                printf("Could not find %s in %s\n",mission,filename);
                return(-1);
        } else
                printf("Found %s...\n",mission);

        if (seek_entry(&direntry,"LINEDEFS") == NULL) return(-1);
        else
                linedefs = (LINEDEFTYPE *)malloc(direntry.length);
                num_linedefs = read_linedefs(linedefs, &direntry);

        printf("Read in %d linedefs...\n",num_linedefs);

        if (seek_entry(&direntry,"SIDEDEFS") == NULL) return(-1);
        else
                sidedefs = (SIDEDEFTYPE *)malloc(direntry.length);
                num_sidedefs = read_sidedefs(sidedefs, &direntry);
 
        printf("Read in %d sidedefs...\n",num_sidedefs);

        if (seek_entry(&direntry,"VERTEXES") == NULL) return(-1);
        else
                vertices = (VERTEXTYPE *)malloc(direntry.length);
                num_vertices = read_vertices(vertices, &direntry);
 
        printf("Read in %d vertices...\n",num_vertices);

        if (seek_entry(&direntry,"SEGS\0\0\0\0") == NULL) return(-1);
        else
                segments = (SEGMENTTYPE *)malloc(direntry.length);
                num_segments = read_segments(segments, &direntry);

        printf("Read in %d segments...\n",num_segments);		

        if (seek_entry(&direntry,"SSECTORS") == NULL) return(-1);
        else
                ssectors = (SSECTORTYPE *)malloc(direntry.length);
                num_ssectors = read_ssectors(ssectors, &direntry);

        printf("Read in %d ssectors...\n",num_ssectors);

        if (seek_entry(&direntry,"NODES\0\0\0") == NULL) return(-1);
        else
                nodes = (NODETYPE *)malloc(direntry.length);
                num_nodes = read_nodes(nodes, &direntry);
 
        printf("Read in %d nodes...\n",num_nodes);

        if (seek_entry(&direntry,"SECTORS\0") == NULL) return(-1);
        else
                sectors = (SECTORTYPE *)malloc(direntry.length);
                num_sectors = read_sectors(sectors, &direntry);

        printf("Read in %d sectors...\n",num_sectors);

        int i;
        unsigned short int j;

        int key = 0;

        short int max_x, max_y, min_x, min_y, mid_x, mid_y;
        float scale_x, scale_y, scale;
        char L_type, R_type;
        unsigned short int up_node[600];

        up_node[num_nodes-1] = num_nodes-1;

        for(i=0;i<num_nodes;i++) {
                if (!(nodes[i].left_child & 0x8000))
                        up_node[nodes[i].left_child] = i;
                if (!(nodes[i].right_child & 0x8000))
                        up_node[nodes[i].right_child] = i;
        }

        _setvideomode( _VRES16COLOR);

        j = num_nodes - 1;
        while (key != 'Q') {

           _clearscreen(_GCLEARSCREEN);

        min_x = min(min(nodes[j].left_x_upper,nodes[j].left_x_lower),
                      min(nodes[j].right_x_upper,nodes[j].right_x_lower));

        min_y = min(min(nodes[j].left_y_upper,nodes[j].left_y_lower),
                      min(nodes[j].right_y_upper,nodes[j].right_y_lower));

        max_x = max(max(nodes[j].left_x_upper,nodes[j].left_x_lower),
                      max(nodes[j].right_x_upper,nodes[j].right_x_lower));

        max_y = max(max(nodes[j].left_y_upper,nodes[j].left_y_lower),
                      max(nodes[j].right_y_upper,nodes[j].right_y_lower));

        mid_x = (min_x + max_x) / 2;
        mid_y = (min_y + max_y) / 2;

        scale_x = ((float)320 / (float)(max_x - mid_x)) * (float)0.90;
        scale_y = ((float)240 / (float)(max_y - mid_y)) * (float)0.90;

        if (scale_x < scale_y) {
           scale = scale_x;
        } else {
           scale = scale_y;
        } /* endif */

             _setcolor(GRAY);
           for (i=0;i<num_linedefs;i++) {
              _moveto((vertices[linedefs[i].from_vertex].x-mid_x)*scale+320,
                        (vertices[linedefs[i].from_vertex].y-mid_y)*(-scale)+240);
              _lineto((vertices[linedefs[i].to_vertex].x-mid_x)*scale+320,
                        (vertices[linedefs[i].to_vertex].y-mid_y)*(-scale)+240);
        } 

     _setcolor(RED);
     _rectangle(_GBORDER,(nodes[j].left_x_upper-mid_x)*scale+320,
                        (nodes[j].left_y_upper-mid_y)*(-scale)+240,
                        (nodes[j].left_x_lower-mid_x)*scale+320,
                        (nodes[j].left_y_lower-mid_y)*(-scale)+240);
     _setcolor(BLUE);
     _rectangle(_GBORDER,(nodes[j].right_x_upper-mid_x)*scale+320,
                        (nodes[j].right_y_upper-mid_y)*(-scale)+240,
                        (nodes[j].right_x_lower-mid_x)*scale+320,
                        (nodes[j].right_y_lower-mid_y)*(-scale)+240);
     _setcolor(GREEN);
     _moveto((nodes[j].x-mid_x)*scale+320,
              (nodes[j].y-mid_y)*(-scale)+240);
     _lineto((nodes[j].x+nodes[j].dx-mid_x)*scale+320,
              (nodes[j].y+nodes[j].dy-mid_y)*(-scale)+240);
        
     _settextposition(1,1);

        if(nodes[j].left_child & 0x8000)
           L_type = 'S';
        else
           L_type = 'N';
        if(nodes[j].right_child & 0x8000)
           R_type = 'S';
        else
           R_type = 'N';

        printf("NodeNav v0.8            N:%#04x     %c:%#04hx   %c:%#04hx",j,
           L_type,nodes[j].left_child & 0x7fff,R_type,nodes[j].right_child & 0x7fff);
        fflush(stdout);
        key = toupper(getch());
        if (key == 'L' && L_type == 'N')
                j = nodes[j].left_child;
        if (key == 'R' && R_type == 'N')
                j = nodes[j].right_child;
        if (key == 'U')
                j = up_node[j];
     }
        _setvideomode( _DEFAULTMODE);
        return(0);
}


DIRENTRYTYPE *seek_entry(DIRENTRYTYPE *entry, char *name) {

        DIRENTRYTYPE *val = NULL;

        while(!feof(fp) && (val == NULL)) {
                fread(entry, sizeof(DIRENTRYTYPE),1,fp);
                if (!strncmp((char *)entry->name,name,strlen(name)))
                        val = entry;
        }
        return(val);
}

int read_linedefs(LINEDEFTYPE *ldarray, DIRENTRYTYPE *entry) {

        unsigned long int j;

        stack_ptr = ftell(fp);
        fseek(fp,entry->startaddr,SEEK_SET);

        for(j=0;j<entry->length/sizeof(LINEDEFTYPE);j++)
        {
                fread(&ldarray[j],sizeof(LINEDEFTYPE),1,fp);
        }

        fseek(fp,stack_ptr,SEEK_SET);
        return (entry->length/sizeof(LINEDEFTYPE));
}

int read_sidedefs(SIDEDEFTYPE *sdarray, DIRENTRYTYPE *entry) {

        unsigned long int j;

        stack_ptr = ftell(fp);
        fseek(fp,entry->startaddr,SEEK_SET);

        for(j=0;j<entry->length/sizeof(SIDEDEFTYPE);j++)
        {
                fread(&sdarray[j],sizeof(SIDEDEFTYPE),1,fp);
        }

        fseek(fp,stack_ptr,SEEK_SET);
        return (entry->length/sizeof(SIDEDEFTYPE));

}

int read_vertices(VERTEXTYPE *vtarray, DIRENTRYTYPE *entry) {

        unsigned long int j;

        stack_ptr = ftell(fp);
        fseek(fp,entry->startaddr,SEEK_SET);

        for(j=0;j<entry->length/sizeof(VERTEXTYPE);j++)
        {
                fread(&vtarray[j],sizeof(VERTEXTYPE),1,fp);
        }

        fseek(fp,stack_ptr,SEEK_SET);
        return (entry->length/sizeof(VERTEXTYPE));

}

int read_segments(SEGMENTTYPE *smarray, DIRENTRYTYPE *entry) {

        unsigned long int j;

        stack_ptr = ftell(fp);
        fseek(fp,entry->startaddr,SEEK_SET);

        for(j=0;j<entry->length/sizeof(SEGMENTTYPE);j++)
        {
                fread(&smarray[j],sizeof(SEGMENTTYPE),1,fp);
        }

        fseek(fp,stack_ptr,SEEK_SET);
        return (entry->length/sizeof(SEGMENTTYPE));

}

int read_ssectors(SSECTORTYPE *ssarray, DIRENTRYTYPE *entry) {

        unsigned long int j;

        stack_ptr = ftell(fp);
        fseek(fp,entry->startaddr,SEEK_SET);

        for(j=0;j<entry->length/sizeof(SSECTORTYPE);j++)
        {
                fread(&ssarray[j],sizeof(SSECTORTYPE),1,fp);
        }

        fseek(fp,stack_ptr,SEEK_SET);
        return (entry->length/sizeof(SSECTORTYPE));

}

int read_nodes(NODETYPE *noarray, DIRENTRYTYPE *entry) {

        unsigned long int j;

        stack_ptr = ftell(fp);
        fseek(fp,entry->startaddr,SEEK_SET);

        for(j=0;j<entry->length/sizeof(NODETYPE);j++)
        {
                fread(&noarray[j],sizeof(NODETYPE),1,fp);
        }

        fseek(fp,stack_ptr,SEEK_SET);
        return (entry->length/sizeof(NODETYPE));

}

int read_sectors(SECTORTYPE *starray, DIRENTRYTYPE *entry) {

        unsigned long int j;

        stack_ptr = ftell(fp);
        fseek(fp,entry->startaddr,SEEK_SET);

        for(j=0;j<entry->length/sizeof(SECTORTYPE);j++)
        {
                fread(&starray[j],sizeof(SECTORTYPE),1,fp);
        }

        fseek(fp,stack_ptr,SEEK_SET);
        return (entry->length/sizeof(SECTORTYPE));

}



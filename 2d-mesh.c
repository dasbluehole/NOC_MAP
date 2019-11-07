/*
 * 2d-mesh.c
 * 
 * Copyright 2013 Ashok Shankar Das <ashok.s.das@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
////////////////////////////////////////////////////////////////
// routines for 2d-mesh                                       //
// in our implementation we represent the 2d-mesh             //
// as a one dimentional array of size mesh_cols x mesh_rows   //
///////////////////////////////////////////////////////////////
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "setsl.c"
typedef struct mesh
{
	int rows;	// mesh rows
	int cols;	// mesh columns
	int *array; // array of routers
}mesh2d;
// creates a 2d mesh with rows and cols
// returns pointer to 2d mesh struct
mesh2d *create_2dmesh(int rows,int cols)
{
	mesh2d *m;
	m = (mesh2d*)malloc(sizeof(mesh2d));
	if(m == NULL)
	{
		printf("Error: Creating mesh datastructure\n");
		return NULL;	
	}
	m->rows = rows;
	m->cols = cols;
	m->array = (int*)malloc(sizeof(int)*rows*cols);
	if(m->array == NULL)
	{
		printf("Error: Creating mesh array of routers\n");
		free(m);
		return NULL;
	}
	memset(m->array,-1,sizeof(int)*rows*cols); // initialize to -1 'not assigned to any core'
	return (m);
}
// this function creates a replica of input mesh
mesh2d *copy_mesh(mesh2d *m)
{
	mesh2d *m1 = create_2dmesh(m->rows,m->cols);
	memcpy(m1->array,m->array,sizeof(int)*m->rows*m->cols);
	return m1; 
}
// frees a 2d-mesh
void free_2dmesh(mesh2d *m)
{
	if(m != NULL)
	{
		free(m->array);
		free(m);
	}
}
// finds hop distance between 2 cells in a 2d mesh 'm'
// and returns it
int find_distance(int cell1, int cell2, mesh2d *m)
{
	// as the cells are numbered in row major fashion
	// so any cell number = row_number * number of columns per row + columns in incomplete row.
	// assume in a 4x4 matrix cells are numbered from 0 to 15
	// so add 1 to any cell number, divide with 4 the remainder is recidual columns
	// interger divison result is the row number of cell
/*	|---|---|---|---|
	|---|---|---|---|
	|---|---|---|---|
	|---|---|---|---|
	|---|---|---|---|
*/
	// so distance between 2 cells = |x1-x2| + |y1 -y2|
	// which translates into (cell1 % nCols - cell2 % nCols) + (cell1/ncols - cell2/nCols)
	
	int x_distance,y_distance,x1,y1,x2,y2;
	x1 = cell1 % m->cols;
	y1 = cell1 / m->cols;
	x2 = cell2 % m->cols;
	y2 = cell2 / m->cols;
	if(x1>x2)
		x_distance = x1 - x2;
	else
		x_distance = x2 - x1;
	if(y1>y2)
		y_distance = y1 - y2;
	else
		y_distance = y2 - y1;	
	return (x_distance + y_distance);
}
//  finds degree of router r
static inline int find_degree_of_router(int r, int num_rows, int num_cols)
{
	if(r < 0 || r > num_cols * num_rows -1)
		return (-1);
	if((r == 0) || (r == num_rows * num_cols -1) || (r == num_cols - 1) || (r == num_cols * (num_rows - 1))) // top left and bottom right top right and bottom left
		return 2;
	if((r > 0 && r< num_cols) || (r > num_cols && (r % num_cols == num_cols - 1)) || ((r > num_cols * (num_rows -1)) && (r< num_cols * num_rows - 1)) || ((r % num_cols == 0)&&(r>0)))
		return 3;
	return 4;
}
// returns if a router is assigned or not
// if assigned then returns '0' else returns '1'
int is_router_free(int router, mesh2d *m)
{
	if(router >= m->rows*m->cols) // out of bound?
		return 0; // return false 
	if(m->array[router] == -1) // not assigned? 
		return 1; // return true
	return 0; // assigned so return false
}
// assign a core 'core' to router 'router' of mesh 'mesh'
// returns 1 if success
// else 0 if fails
int assign_router(int router, int core, mesh2d *mesh)
{
	if(router > mesh->rows*mesh->cols-1)
		return 0;
	if(mesh->array[router] == -1) // not assigned? 
	{
#ifdef DEBUG
		printf("Not assigned %d router assigning %d core\n",router,core);
#endif
		mesh->array[router] = core;
		return 1; // successfully assigned
	}
#ifdef DEBUG
	printf("Already assigned %d router\n",router);
#endif
	return 0;
}
// returns the core assigned to router
int get_router_value(int router, mesh2d *mesh)
{
	if(router >= mesh->rows*mesh->cols) // out of bound?
		return -1; // return false
	return(mesh->array[router]); 
}
// finds the core in 2d mesh and returns router
int find_core(int core, mesh2d *mesh)
{
	int i;
	for(i=0; i< mesh->rows*mesh->cols; i++)
	{
		if(mesh->array[i]==core)
			return(i);
	}
	return (-1);
}
// returns 1 if core is mapped
// else return 0 if not mapped
int is_core_mapped(int core, mesh2d *m)
{
	if(find_core(core, m)== -1)
		return 0;
	else
		return 1;
}
// this function will get all the neighbouring routers of the 'router' in mesh 'm' 
set *get_all_neighbours(int router, mesh2d *m)
{
	int j, pivot = router;
	set *nbr_set = create_set(m->rows*m->cols);
	int px,py;
	px = pivot % m->cols;
	py = pivot / m->cols;
#ifdef DEBUG
	printf("pivot at x = %d y = %d\n",px,py);
#endif
	for(j = 1; j<=m->rows+m->cols-2; j++)
	{	
		int routerl,routeru;
#ifdef DEBUG
		printf("hops = %d\n",j);
#endif
		int nx,nyu,nyl;
		int k=0;
		int l = 0;
		for(nx = px - j; nx <= px + j; nx++)
		{
			nyu = py + k;
			nyl = py - l;
			if(nx<px)
			{
				k++;
				l++;
			}
			else
			{
				k--;
				l--;
			}
			routerl = nx+nyu*m->cols;
			routeru = nx+nyl*m->cols;
			if(nx>=0 && nx< m->cols && nyu<=m->rows && routerl<m->rows*m->cols )
			{
#ifdef DEBUG
				printf("[%d %d]= %d ",nx,nyu,routerl);
#endif
				add_element(nbr_set,routerl);
			}
			if(nx>=0 && nx< m->cols && nyl<=m->rows && nyl>=0 && routeru<m->rows*m->cols )
			{
#ifdef DEBUG
				printf("[%d %d]= %d ",nx,nyl,routeru);
#endif
				add_element(nbr_set,routeru);
			}
		}
#ifdef DEBUG
		printf("\n");
#endif
	}
	return (nbr_set);
}
int find_nearest_free_router(int core, mesh2d *m)
{
	int pivot = find_core(core,m);
	if(pivot == -1)
	{
		printf("Error: couldnot find core in the mesh\n");
		return -1;
	}
	set *nbr = get_all_neighbours(pivot, m);
	if(nbr == NULL)
		return (-1);
	int k;
	for(k=0;k<nbr->num_elements;k++)
	{
		int rtr = get_element(k,nbr);
		if(is_router_free(rtr,m))
		{
			free(nbr);
			return(rtr);
		}
	}
	free(nbr);
	return(-1);
}
// finds neighbours of a router and populate an array.
int find_neighbours_with_distance(int current_router,int hops, int *narray,int num_rows,int num_cols)
{
	int start_x,start_y,next_x,next_y;
	int i=0,j,k=0;

	set *nb = create_set(num_rows*num_cols); // create a big set.

	start_x = current_router % num_cols;
	start_y = current_router / num_cols;
#ifdef DEBUG	
	printf("start = %d startx = %d starty = %d nrows = %d ncols = %d\n",current_router,start_x,start_y,num_rows,num_cols);
#endif
	// lower half
	for(j = hops; j >= -1*hops; j--)
	{
		//printf("[%d %d]", j, k);
		next_x = start_x + j;
		next_y = start_y + k;
		if(j>0)
			k++;
		else 	
			k--;
		if(next_x >= num_rows || next_y>=num_cols || next_x<0 || next_y<0)
			continue;
		int rtr = next_y * num_cols + next_x;
		add_element(nb,rtr);
		//narray[i] = next_y * num_cols + next_x;
		i++;
#ifdef DEBUG
		printf("%d -> [%d %d] = %d\n",hops,next_x, next_y, next_y * num_cols + next_x );
#endif
	}
	// upper half
	k=0;
	for(j = -1*hops; j<=hops; j++)
	{
		next_x = start_x + j;
		next_y = start_y - k;
		if(j<0)
			k++;
		else
			k--;
		if(next_x >= num_rows || next_y>=num_cols || next_x<0 || next_y<0)
			continue;
		int rtr = next_y * num_cols + next_x;
		add_element(nb,rtr);
		//narray[i] = next_y * num_cols + next_x;
		i++;
#ifdef DEBUG
		printf("%d -> [%d %d] = %d\n",hops,next_x, next_y, rtr );
#endif
	}
	// now read the set nb and populate the narray
	for(i = 0; i<nb->num_elements; i++)
	{
		narray[i] = get_element(i, nb);
		
	}
	free(nb);
	return(i);
}
int *request_n_routers(int n,int core, mesh2d *m)
{
	if(n > m->rows*m->cols) // requested ammount is greater than total number of routers
		return NULL;
	int *r_array = (int*)malloc(sizeof(int)*(n+1));
	if(r_array == NULL)
		return NULL;
	// find for which router we need to serve
	int cur_rtr = find_core(core,m);
	if(cur_rtr == -1)
		return NULL;
	int hops =1;	// initially search for the 1-hop neighbour
	int found = 0; // we have not found any good router yet
	while(1) // go into infinite loop WARNING Dangerous code region
	{
		int *narray = (int*)malloc(sizeof(int) * m->rows * m->cols);
		int a = find_neighbours_with_distance(cur_rtr,hops,narray,m->rows,m->cols);
		//iterate narray and check if the routers are un allocated
		int i;
		for(i = 0; i<a; i++)
		{
			if(is_router_free(narray[i],m) == 1)
			{
				// we found a free unused router in the mesh
				// add it to the r_array
				r_array[found] = narray[i];
				found++;
				if(found>=n)
				break;
			}		
		}
		free(narray);
		// ok check if requested number of routers are found or not
		if(found < n)
		{
			// we have not reached the requested ammount hence we have to re iterate 
			hops++;
		}
		else
		{
			// we got requested ammount of router
			// hence break out of the dangerous loop
			break; // break out of the infinite while loop		
		}
		if(hops > m->rows+m->cols-2)
		{
			printf("Error: illegal hop value.. should not happen[%d:%s]\n",__LINE__,__FILE__);
			break;
		}
	}
	return (r_array);
}
int *request_n_routers1(int n,int core, mesh2d *m)
{
	if(n > m->rows*m->cols) // requested ammount is greater than total number of routers
		return NULL;
	int *r_array = (int*)malloc(sizeof(int)*(n+1));
	if(r_array == NULL)
		return NULL;
	// find for which router we need to serve
	int cur_rtr = find_core(core,m);
	if(cur_rtr == -1)
		return NULL;
	int hops =1;	// initially search for the 1-hop neighbour
	int found = 1; // we have not found any good router yet
	while(hops<=m->rows+m->cols-2) 
	{
		int *narray = (int*)malloc(sizeof(int) * m->rows * m->cols);
		int a = find_neighbours_with_distance(cur_rtr,hops,narray,m->rows,m->cols);
		//iterate narray and check if the routers are un allocated
		int i;
		for(i = 0; i<a; i++)
		{
			if(is_router_free(narray[i],m) == 1)
			{
				// we found a free unused router in the mesh
				// add it to the r_array
				r_array[found] = narray[i];
				found++;
				if(found>=n)
				break;
			}		
		}
		free(narray);
		r_array[0] = found-1;
		// ok check if requested number of routers are found or not
		if(found < n)
		{
			// we have not reached the requested ammount hence we have to re iterate 
			hops++;
		}
		else
		{
			// we got requested ammount of router
			// hence break out of the dangerous loop
			break; // break out of the infinite while loop		
		}
		//if(hops > m->rows+m->cols-2)
		//{
		//	printf("Error: illegal hop value.. should not happen[%d:%s]\n",__LINE__,__FILE__);
		//	break;
		//}
	}
	return (r_array);
}
int *request_routers(int core, mesh2d *m)
{
	//if(n > m->rows*m->cols) // requested ammount is greater than total number of routers
	//	return NULL;
	int *r_array = (int*)malloc(sizeof(int) * m->rows * m->cols);
	if(r_array == NULL)
		return NULL;
	// find for which router we need to serve
	int cur_rtr = find_core(core,m);
	if(cur_rtr == -1)
		return NULL;
	//int hops =1;	// initially search for the 1-hop neighbour
	int found = 0; // we have not found any good router yet
	//while(1) // go into infinite loop WARNING Dangerous code regio;
	int j;
	for(j =0; j<m->rows+m->cols-2; j++)
	{
		int *narray = (int*)malloc(sizeof(int) * m->rows * m->cols);
		memset(narray,-1,sizeof(int) * m->rows * m->cols);
		int a = find_neighbours_with_distance(cur_rtr, j , narray, m->rows,m->cols);
		//iterate narray and check if the routers are un allocated
		int i;
		for(i = 0; i<a; i++)
		{
			if(is_router_free(narray[i],m) == 1)
			{
				// we found a free unused router in the mesh
				// add it to the r_array
				r_array[found] = narray[i];
				found++;
				//if(found>=n)
				//break;
			}		
		}
		free(narray);
	}
	return (r_array);
}
void reset_mesh(mesh2d *m)
{
	int i,j,k=0;
	//printf("\n The mesh is : \n");
	for(i = 0; i< m->rows; i++)
	{
		for(j = 0; j<m->cols; j++)
		{
			m->array[k] = -1;
			k++;		
		}
	}
}
// display the mesh with bit of formatting
void display_mesh(mesh2d *mesh)
{
	int i,j,k=0;
	char temp[20]="";
	printf("\n The mesh is : \n");
	for(i = 0; i< mesh->rows; i++)
	{
		for(j = 0; j<mesh->cols; j++)
		{
			if(mesh->array[k] != -1)
				sprintf(temp,"%3d ",mesh->array[k]+1);
			else
				sprintf(temp,"  * ");
			printf("%s ",temp);
			k++;		
		}
		printf("\n");
	}
	printf("\n");
}
void swap_rows(int r1, int r2, mesh2d *m)
{
	if(r1<0 || r1>m->rows || r2<0 || r2>m->rows || r1==r2)
		return; // do nothing
	int r1r,r2r;
	int i;
	int temp;
	for(i=0;i<m->cols;i++)
	{
		r1r = r1 * m->cols + i;
		r2r = r2 * m->cols + i;
		temp = m->array[r1r];
		m->array[r1r] = m->array[r2r];
		m->array[r2r] = temp;
	}
}
void swap_cols(int c1, int c2, mesh2d *m)
{
	if(c1 < 0 || c1>m->cols || c2<0 || c2>m->cols || c1 == c2)
		return;
	int c1r,c2r;
	int i;
	int temp;
	for(i = 0; i < m->rows; i++)
	{
		c1r = c1 + m->cols * i;
		c2r = c2 + m->cols * i;
		temp = m->array[c1r];
		m->array[c1r] = m->array[c2r];
		m->array[c2r] = temp;
	}
}
#ifdef TEST_MESH
// test 
int main(int argc, char *argv[])
{
	mesh2d *mesh = create_2dmesh(8,4);
	if(mesh == NULL)
	{
		printf("Error exiting\n");
		exit(0);	
	}
	int i=0,j=15;
	int *nbr_array = malloc(sizeof(int) *16);
	memset(nbr_array,-1,sizeof(int) * 16);
	find_neighbours_with_distance(5, 3,nbr_array,8,4);
	for(i=0;i<16;i++)
		printf("%d ",nbr_array[i]);
	printf("\n");
	printf("distance between %d and %d routers = %d hops\n",i,j,find_distance(i,j,mesh));
	for(i = 1; i< 31;i++)
	{
		if(assign_router(i,i,mesh)==0)
			printf("Warning cant assign router %d\n",i);
	}
	display_mesh(mesh);
	set *nbr = get_all_neighbours(15,mesh);
	print_set(nbr);
	printf("free router = %d\n",find_nearest_free_router(15,mesh));
	mesh2d *m1 = copy_mesh(mesh);
	swap_rows(7,0,m1);
	display_mesh(m1);
	swap_cols(3,0,m1);
	display_mesh(m1);
}
#endif

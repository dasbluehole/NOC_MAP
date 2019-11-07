/*
 * map_noc.c
 * 
 * Copyright 2012 Ashok Shankar Das <ashok.s.das@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
// standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//============================

// libconfig header
#include <libconfig.h>

// include utility files

#include "setsl.c" 	// set 
#include "mygraph.c"	// graph
#include "2d-mesh.c"	// 2d mesh

//====================================================

// search for first suitable router position
int find_best_router(int cur, graph *g, mesh2d *mesh)
{
	int best=-1;
	if(cur == -1) // i.e first core tobe mapped
	{
		best = (mesh->cols-1)/2 + (mesh->rows-1)/2 * mesh->cols;
	}
	else
	{
		//find the x coordinate and y coordinate
		int x,y;
		int i;
		//printf("current %d ",cur);
		x = cur % mesh->cols;
		y = cur / mesh->cols;
		//check east i.e advance x by 1 and check if it is feasible and available
		i = 1;

		do{
			if((x+i) < mesh->cols)
			{
				//printf("east ");
				best = (x+i) + y*mesh->cols; //east
				if(is_router_free(best,mesh) == 1)
					break;
				
			}
			if((y+i) < mesh->rows)
			{
				//printf("south ");
				best = x + (y+i)*mesh->cols; //south
				if(is_router_free(best,mesh) == 1)
					break;
			}
			if((x-i)>=0) //west
			{
				//printf("west ");
				best = (x -i)+ y*mesh->cols;
				if(is_router_free(best,mesh) == 1)
					break;
			}
			if((y-i)>=0) //north
			{
				//printf("north ");
				best = x + (y-i)*mesh->cols;
				if(is_router_free(best,mesh) == 1)
					break;			
			}
			i++;
		}while(i< mesh->cols + mesh->rows -2 );
		
	}
	return best;
}

void update_graph(graph *g)
{
	int i;
	for(i = 0; i<g->num_vertex; i++)
	{
		g->bw_array[i] =  g->bw_array[i] / g->degree_array[i];
	}
}
void map1(graph *g, mesh2d *m, int start_rtr)
{
	// the edges for every node of the graph is sorted according to weight
	// and the degree index is available to us
	// we will map every core and its neighbours in sequence
	// we will keep track of already mapped cores
	// if a core is already mapped then we will go to next core
	set *already_mapped = create_set(g->num_vertex);
	//update the graph by updating the bw_array
	//update_graph(g);
	// now we will start scanning the index array to get the cores
	int i;
	int currently_mapped_router = -1,currently_mapped_core;
	int *indexed = my_index((int*)g->bw_array, g->num_vertex,DESCENDING);
	for(i = 0; i<g->num_vertex; i++)
	{
		//printf("%d ",indexed[i]);

		// first map core i
		currently_mapped_core = indexed[i];
		if(is_exists(already_mapped,currently_mapped_core)!= -1)
		{
			//remove_element(tobe_placed,currently_mapped_core);
			continue;
		}
		if(currently_mapped_router == -1)
			currently_mapped_router = start_rtr;
		else
			currently_mapped_router = find_best_router(currently_mapped_router,g,m);
		if(assign_router(currently_mapped_router, currently_mapped_core, m) != 1)
		{
			printf("Some problem happened while mapping %d core to %d router\n",currently_mapped_core, currently_mapped_router);
			exit(0);
		}
		add_element(already_mapped,currently_mapped_core);
		// from now we will map the neighbours
		adj_list_node *itr = g->array[currently_mapped_core].head;
		while(itr)
		{
			currently_mapped_core = itr->dest;
			if(is_exists(already_mapped,currently_mapped_core)!= -1)
			{
				//remove_element(tobe_placed,currently_mapped_core);
				itr = itr->next;
				continue;
			}
			currently_mapped_router = find_best_router(currently_mapped_router,g,m);
			if(assign_router(currently_mapped_router, currently_mapped_core, m) != 1)
			{
				printf("Some problem happened while mapping %d core to %d router\n",currently_mapped_core, currently_mapped_router);
				exit(0);
			}
			add_element(already_mapped,currently_mapped_core);
			itr = itr->next;
		} // loop for every neighbour of a core
#ifdef DEBUG
		printf("============================\n");
		printf("already mapped: ");
		print_set(already_mapped);
		//print_set(tobe_placed);
		display_mesh(m);
		printf("============================\n");
#endif
	}// loop for every core
#ifdef DEBUG
	print_set(already_mapped);
#endif
	free(already_mapped);
}
void map2(graph *g, mesh2d *m)
{
	// the edges for every node of the graph 'g' is sorted according to weight
	// and the degree index is available to us
	// we will map every core and its neighbours in sequence
	// we will keep track of already mapped cores
	// if a core is already mapped then we will go to next core
	set *already_mapped = create_set(g->num_vertex);
	//update the graph by updating the bw_array
	//update_graph(g);
	// now we will start scanning the index array to get the cores
	int i;
	int currently_mapped_router = -1,currently_mapped_core;
	int *indexed = my_index((int*)g->bw_array, g->num_vertex,DESCENDING);
	int *nbr_list;
	int deg;
	int is_mapping_started = 0; // mapping flag to show wheather mapping is started or not
	// start the loop for all vertices
	for(i = 0; i<g->num_vertex; i++)
	{
		// get the core with next maximum weight from index
		currently_mapped_core = indexed[i];
		// check if it is already mapped
		if(is_exists(already_mapped,currently_mapped_core)!= -1)
		{
			printf("Core %d is already mapped\n",currently_mapped_core);
			printf("We should map its neighbours if thye are not mapped\n");
			goto lab1;
		}
		// else core is not mapped hence map it
		if(is_mapping_started == 0)
		{
			printf("first core is yet to be mapped ");
			currently_mapped_router = (m->cols)/2 + (m->rows)/2 * m->cols;
			is_mapping_started = 1; // show that already some core is mapped
			if(assign_router(currently_mapped_router, currently_mapped_core, m) != 1)
			{
				printf("Some problem: line %d in file %s while mapping %d core to %d router\n",__LINE__,__FILE__,currently_mapped_core, currently_mapped_router);
				display_mesh(m);
				//exit(EXIT_FAILURE);
				continue;
			}
			// add this core to already mapped set
			add_element(already_mapped,currently_mapped_core);
		}
lab1:		
		// we should map neighbours here
		// here first we will checck how many neighbours it has
		deg = get_degree(g, currently_mapped_core);
		// now get the neighbours list
		nbr_list = get_neighbours(currently_mapped_core, g);
		if(nbr_list == NULL)
		{
			printf("Error: Impossible... core doesn't have any neighbours!!!\n");
			exit(EXIT_FAILURE);		
		}
		// so we have number of neighbours and neighbours list.
		// get routers for the neighboour list at 1 hop distance.
		
		int *rtr_list = request_n_routers(deg,currently_mapped_core,m);
		if(rtr_list != NULL)
		{
			// so we have got our requested router list
			// now we will map the neighbouring cores from our task graph to routers in rtr_list
			int j,k=0;
			for(j=0;j<deg;j++)
			{
				printf("Mapping core %d  to %d router\n",nbr_list[j],rtr_list[j]);
				if(is_exists(already_mapped,nbr_list[j]) != -1)
				{
					printf("core %d is already mapped\n",nbr_list[j]);
					continue;
				}
				if(assign_router(rtr_list[k], nbr_list[j], m) != 1)
				{
					printf("Problem mapping core %d to %d router\n",nbr_list[j],rtr_list[j]);
					display_mesh(m);
					//exit(EXIT_FAILURE);
					break;
				}
				k++;
				add_element(already_mapped,nbr_list[j]);
			}
			printf("\n");
			print_set(already_mapped);
		}
		else
			printf("Some thing went wrong I am not so intelligent\n");
		free(rtr_list);
		free(nbr_list);
	}
}
void map3(graph *g, mesh2d *m,int start_rtr)
{
	int router,core;
	int mapping_started = 0;
	//int core_mapped = 0;
	int *nbr_array = NULL;
	int deg = 0;
	// first we will index the bw_array in side the graph g in descending order
	int * indexed = my_index((int*)g->bw_array,g->num_vertex, DESCENDING);
	// the indexed array now holds the nodes in order.
	
	// from this point our mapping logic is started
	// here we will maintain 2 sets one for mapped cores and another for unmapped cores
	set *mapped = create_set(g->num_vertex);
	set *unmapped = create_set(g->num_vertex);
	// put all node numbers in unmapped set from indexed
	int i;
	for(i =0; i <g->num_vertex; i++)
		add_element(unmapped,indexed[i]);
#ifdef DEBUG
	printf("Unmapped cores :");
	print_set(unmapped);

	for(i =0; i <g->num_vertex; i++)
		printf("%f ",g->bw_array[indexed[i]]);
	printf("\n");
#endif	
	for(i =0; i<g->num_vertex; i++)
	{
		core = indexed[i];
		if(is_exists(mapped,core) != -1)
			continue;	// if core is mapped then get next core
		// check if mapping is started
		if(mapped->num_elements != 0)
			mapping_started = 1;
		if(!mapping_started)
		{
			// first core is yet to be mapped/
			// choose the first router
			//router = (m->rows-1)/2 * m->cols + (m->cols-1)/2;
			router = start_rtr;
			
		}
		//else mapping already started
		// we have to check if the core already mapped
				
		if(mapping_started)
		{
			// we have to find a suitable core
			// for this purpose, we have to get all the neighbours of core
			deg = get_degree(g,core);
			if(!deg)
				return;
			nbr_array = get_neighbours(core, g);
#ifdef DEBUG
			printf("neighbours ");
#endif
			int j;
			int best_nbr = -1;
			float bw = 0.0;
			for(j=0; j<deg; j++)
			{
#ifdef DEBUG
				printf("%d ",nbr_array[j]);
#endif
				if(is_core_mapped(nbr_array[j],m))
				{
					float bw1,bw2;
					bw1 = get_weight(core,nbr_array[j],g);
					bw2 = get_weight(nbr_array[j],core,g);
					if((bw < bw1) || (bw<bw2))
						best_nbr = nbr_array[j];
				}
			}
#ifdef DEBUG
			printf("best neighbour = %d\n",best_nbr);
#endif
			// we found the best neighbour
			// now we have to ask for nearest router
			router = find_nearest_free_router(best_nbr,m);

		}
#ifdef DEBUG
		printf("-> Mapping core %d to router %d\n",core,router);
#endif
		if(assign_router(router, core, m) != 1)
		{
			printf("Some problem happened while mapping %d core to %d router\n",core, router);
			exit(0);
		}
		remove_element(unmapped,core);
		add_element(mapped,core);
#ifdef DEBUG
		print_set(unmapped);
		print_set(mapped);
#endif		
	}
}

void map4(graph *g, mesh2d *m,int start_rtr)
{
	int router,core;
	int mapping_started = 0;
	//int core_mapped = 0;
	int *nbr_array = NULL;
	int deg = 0;
	// first we will index the bw_array in side the graph g in descending order
	int * indexed = my_index((int*)g->bw_array,g->num_vertex, DESCENDING);
	// the indexed array now holds the nodes in order.
	
	// from this point our mapping logic is started
	// here we will maintain 2 sets one for mapped cores and another for unmapped cores
	set *mapped = create_set(g->num_vertex);
	set *unmapped = create_set(g->num_vertex);
	// put all node numbers in unmapped set from indexed
	int i,j;
	for(i =0; i <g->num_vertex; i++)
		add_element(unmapped,indexed[i]);
#ifdef DEBUG
	printf("Unmapped cores :");
	print_set(unmapped);

	for(i =0; i <g->num_vertex; i++)
		printf("%f ",g->bw_array[indexed[i]]);
	printf("\n");
#endif	
	for(i =0; i<g->num_vertex; i++)
	{	core = indexed[i];	
		if(is_exists(mapped,core) != -1)
		{
			goto map_nbr;	// try map its neighbour
			//continue;	// if core is mapped then get next core
		}
		// check if mapping is started
		if(mapped->num_elements != 0)
			mapping_started = 1;
		if(!mapping_started)
		{
			// first core is yet to be mapped/
			// choose the first router
			//router = (m->rows-1)/2 * m->cols + (m->cols-1)/2;
			router = start_rtr;
		}
		//else mapping already started
		// we have to check if the core already mapped
				
		if(mapping_started)
		{
			// we have to find a suitable core
			// for this purpose, we have to get all the neighbours of core
			deg = get_degree(g,core);
			if(!deg)
				return;
			nbr_array = get_neighbours(core, g);
#ifdef DEBUG
			printf("neighbours ");
#endif
			int j;
			int best_nbr = -1;
			float bw = 0.0;
			for(j=0; j<deg; j++)
			{
#ifdef DEBUG
				printf("%d ",nbr_array[j]);
#endif
				if(is_core_mapped(nbr_array[j],m))
				{
					float bw1,bw2;
					bw1 = get_weight(core,nbr_array[j],g);
					bw2 = get_weight(nbr_array[j],core,g);
					if((bw < bw1) || (bw<bw2))
						best_nbr = nbr_array[j];
				}
			}
			free(nbr_array);
#ifdef DEBUG
			printf("best neighbour = %d\n",best_nbr);
#endif
			// we found the best neighbour
			// now we have to ask for nearest router
			router = find_nearest_free_router(best_nbr,m);
		}
#ifdef DEBUG
		printf("-> Mapping core %d to router %d\n",core,router);
#endif
		if(assign_router(router, core, m) != 1)
		{
			printf("Some problem happened while mapping %d core to %d router\n",core, router);
			exit(0);
		}
		remove_element(unmapped,core);
		add_element(mapped,core);
#ifdef DEBUG
		print_set(unmapped);
		print_set(mapped);
#endif
		//goto not_now;
		// map the neighbours of the core
map_nbr:

#ifdef DEBUG	
		printf("Mapping neighbours of core = %d\n",core);
#endif
		deg = get_degree(g, core);
		nbr_array = get_neighbours(core, g);
		if(nbr_array == NULL) return;
		int j;
		for(j=0;j<deg;j++)
		{
			int nbr_core = nbr_array[j];
			if(is_exists(mapped,nbr_core) != -1)
			{
				continue;
			}
			router = find_nearest_free_router(core,m);
#ifdef DEBUG
			printf("-> Mapping core %d to router %d\n",nbr_core,router);
#endif
			if(assign_router(router,nbr_core , m) != 1)
			{
				printf("Some problem happened while mapping %d core to %d router\n",nbr_core, router);
				exit(0);
			}
			remove_element(unmapped,nbr_core);
			add_element(mapped,nbr_core);

#ifdef DEBUG
			print_set(unmapped);
			print_set(mapped);
#endif
//not_now:
//			printf("\n");
		}
	}
}
// this function will map using edges
void map5(graph *g, mesh2d *m,int start_rtr)
{
	int router,core;
	int mapping_started = 0;
	egraph *eg;
	int *nbr_array = NULL;
	eg = graph_to_egraph(g);
	sort_egraph(eg);
#ifdef DEBUG
	display_egraph(eg);
#endif
	//
	int * indexed = my_index((int*)g->bw_array,g->num_vertex, DESCENDING);
	int i;
	int deg = 0;
#ifdef DEBUG
	for(i = 0; i < g->num_vertex; i++)
		printf("%d ",indexed[i]+1);
	printf("\n");
#endif
	// from this point our mapping logic is started
	// here we will maintain 2 sets one for mapped cores and another for unmapped cores
	set *mapped = create_set(g->num_vertex);
	set *unmapped = create_set(g->num_vertex);
	// put all node numbers in unmapped set from indexed
	for(i =0; i <g->num_vertex; i++)
		add_element(unmapped,indexed[i]);
	// from here we will iterate through the egraph 
	for( i = 0; i < eg->num_edges; i++)
	{
		edge *e = get_edge_from_egraph(i,eg);
#ifdef DEBUG
		display_edge(e);
#endif
		float w1,w2;
		w1 = get_weight(e->vs,e->ve,g);
		w2 = get_weight(e->ve,e->vs,g);
#ifdef DEBUG
		printf("w1 = %f w2 = %f\n",w1,w2);
		printf(" bw_array[%d] = %f and bw_array[%d] = %f\n",e->vs,g->bw_array[e->vs],e->ve,g->bw_array[e->ve]);
#endif
		//we will map the highest communicating node of the current edge
		if(g->bw_array[e->vs] > g->bw_array[e->ve] && w1>w2)
			core = e->vs;
		else
			core = e->ve;
		if(is_exists(mapped,core) != -1)
		{
			goto map_nbr;	// try map its neighbour
		}
		// check if mapping is started
		if(mapped->num_elements != 0)
			mapping_started = 1;
		if(!mapping_started)
		{
			// first core is yet to be mapped/
			// choose the first router
			router = start_rtr;
		}
		//else mapping already started
		// we have to check if the core already mapped
		if(mapping_started)
		{
			// we have to find a suitable core
			// for this purpose, we have to get all the neighbours of core
			deg = get_degree(g,core);
			if(!deg)
				return;
			nbr_array = get_neighbours(core, g);
#ifdef DEBUG
			printf("neighbours ");
#endif
			int j;
			int best_nbr = -1;
			float bw = 0.0;
			for(j=0; j<deg; j++)
			{
#ifdef DEBUG
				printf("%d ",nbr_array[j]);
#endif
				if(is_core_mapped(nbr_array[j],m))
				{
					float bw1,bw2;
					bw1 = get_weight(core,nbr_array[j],g);
					bw2 = get_weight(nbr_array[j],core,g);
					if((bw < bw1) || (bw<bw2))
					{
						best_nbr = nbr_array[j];
						bw = (bw1>bw2)? bw1 : bw2;					
					}
				}
			}
			free(nbr_array);
#ifdef DEBUG
			printf("best neighbour = %d\n",best_nbr);
#endif
			// we found the best neighbour
			// now we have to ask for nearest router
			router = find_nearest_free_router(best_nbr,m);
		}
#ifdef DEBUG
		printf("-> Mapping core %d to router %d\n",core,router);
#endif
		if(assign_router(router, core, m) != 1)
		{
			printf("Some problem happened while mapping %d core to %d router\n",core, router);
			exit(0);
		}
		remove_element(unmapped,core);
		add_element(mapped,core);
#ifdef DEBUG
		print_set(unmapped);
		print_set(mapped);
#endif
		// map the neighbours of the core
map_nbr:

#ifdef DEBUG	
		printf("Mapping neighbours of core = %d\n",core);
#endif
		deg = get_degree(g, core);
		nbr_array = get_neighbours(core, g);
		if(nbr_array == NULL) return;
		int j;
		for(j=0;j<deg;j++)
		{
			int nbr_core = nbr_array[j];
			if(is_exists(mapped,nbr_core) != -1)
			{
				continue;
			}
			router = find_nearest_free_router(core,m);
#ifdef DEBUG
			printf("-> Mapping core %d to router %d\n",nbr_core,router);
#endif
			if(assign_router(router,nbr_core , m) != 1)
			{
				printf("Some problem happened while mapping %d core to %d router\n",nbr_core, router);
				exit(0);
			}
			remove_element(unmapped,nbr_core);
			add_element(mapped,nbr_core);

#ifdef DEBUG
			print_set(unmapped);
			print_set(mapped);
#endif
		}
	}
}

float find_weight(int core1, int core2, int router1, int router2, graph *g, mesh2d *mesh)
{
	float w;
	if((core1 == -1) || (core2 == -1) || (core1 == core2) || (router1 == router2))
		return 0.0;
	w = get_weight(core1,core2,g) * find_distance(router1,router2,mesh);
	return (w);
}
float sum_of_weight(mesh2d *mesh, graph *g)
{
	int size = mesh->rows * mesh->cols;
	int i,j;
	float sum = 0.0;
	for(i = 0; i<size; i++)
	{
		for(j = 0; j<size; j++)
		{
			sum = sum + find_weight(get_router_value(i,mesh), get_router_value(j,mesh),i,j,g,mesh);
		}
	}
	return (sum);
}
mesh2d *rearrange_mesh(mesh2d *m, graph *g)
{
	int i,j,k,l;
	mesh2d *minmesh,*m1,*m2;
	minmesh = copy_mesh(m);
	m1 = copy_mesh(m);
	m2 = copy_mesh(m);
	float tw,minw;
	tw=minw=sum_of_weight(m,g);
	for(i = 0; i<m->rows; i++)
	{
		for(j=i+1; j<m->rows;j++)
		{
			m1 = copy_mesh(m);
			swap_rows(i,j,m1);
			tw = sum_of_weight(m1,g);
			//printf(" row %d and row %d swapped, Weight = %f\n",i,j,tw);
			if(tw<minw)
			{
				minw = tw;
				printf("->row %d and row %d swapped, Weight = %f\n",i,j,tw);
				free_2dmesh(minmesh);
				minmesh = copy_mesh(m1);	
			}
				
			for(k=0;k<m->cols;k++)
			{
				for(l=k+1;l<m->cols;l++)
				{
					m2 = copy_mesh(m1);
					swap_cols(k,l,m2);
					tw = sum_of_weight(m2,g);
					//printf("  col %d and col %d swapped, Weight = %f\n",k,l,tw);
					if(tw<minw)
					{
						minw = tw;
						printf("->->col %d and col %d swapped, Weight = %f\n",k,l,tw);
						free_2dmesh(minmesh);
						minmesh = copy_mesh(m2);
					}
					//free_2dmesh(m2);
				}
				
			}
			//free_2dmesh(m1);
		}
	}
	free_2dmesh(m1);
	free_2dmesh(m2);
	return(minmesh);
}
// our main routine a file containing data set is passed as commandline parameter

int main(int argc, char *argv[])
{
	// check if we have a commandline paramater or not
	if(argc < 2)
	{
		printf("Please pass dataset file as command line parameter\n");
		printf("Syntax: %s <dataset.ext>\n",argv[0]);
		return(EXIT_FAILURE);	
	}
	// so we have a commandline parameter
	// we assume it as our dataset file.
	// hence using libconfig rooutines open it
	//================ Configuration file reading ===============
	config_t cfg, *cf;
	cf = &cfg;
	config_init(cf);
	//check if we can read the inputfile
	if(!config_read_file(cf,argv[1]))
	{
		fprintf(stderr,"%s: %d - %s\n",
		config_error_file(cf),
		config_error_line(cf),
		config_error_text(cf));
		config_destroy(cf);
		return(EXIT_FAILURE);
	}
	printf("\n==========================Dataset = [%s]================================\n\n",argv[1]);
	// variables needed for configuration setting readings

	int num_cores,num_rows,num_cols,num_edges;
	int n_froms,n_tos,n_weights;
	config_setting_t *froms,*tos,*weights;

	//read number of cores
	if(config_lookup_int(cf,"cores",&num_cores))
	{
		printf("Number of cores = %d\n",num_cores);
	}
	else
	{
		printf("Number of cores not found in configuration. Please Edit it\n");
		config_destroy(cf);
		return(EXIT_FAILURE);
	}
	// read number of rows on tile
	if(config_lookup_int(cf,"mesh_rows",&num_rows))
	{
		printf("Number of rows in mesh = %d\n",num_rows);
	}
	else
	{
		printf("Number of mesh rows not found in configuration. Please Edit it\n");
		config_destroy(cf);
		return(EXIT_FAILURE);
	}
	// read number of columns on tile
	if(config_lookup_int(cf,"mesh_cols",&num_cols))
		printf("Number of Columns in mesh = %d\n",num_cols);
	
	else
	{
		printf("Number of mesh columns not found in configuration. Please Edit it\n");
		config_destroy(cf);
		return(EXIT_FAILURE);
	}
	// read number of edges 
	if(config_lookup_int(cf,"edges",&num_edges))
		printf("Number of edges in graph = %d\n",num_edges);
	
	else
	{
		printf("Number of edges not found in configuration. Please Edit it\n");
		config_destroy(cf);
		return(EXIT_FAILURE);
	}
	// now we have enough info to create a 2d mesh tile and a task graph
	mesh2d *mesh1 = create_2dmesh(num_rows,num_cols);
	mesh2d *mesh2 = create_2dmesh(num_rows,num_cols);
	mesh2d *mesh3 = create_2dmesh(num_rows,num_cols);
	mesh2d *mesh4 = create_2dmesh(num_rows,num_cols);
	mesh2d *mesh5 = create_2dmesh(num_rows,num_cols);
	if(mesh1 == NULL || mesh2 == NULL || mesh3 == NULL || mesh4 == NULL || mesh5 == NULL)
	{
		printf("Error: Can't create a mesh of %d rows and %d cols\n", num_rows, num_cols);
		config_destroy(cf);
		return(EXIT_FAILURE);
	}
#ifdef DEBUG
	printf("Success: Mesh with %d rows and %d columns created\n",num_rows,num_cols);
#endif	
	graph *tcg = create_graph(num_cores); // create a graph with num_core vertices
	if(tcg == NULL)
	{
		printf("Error: Can't create a graph of %d vertices\n", num_cores);
		config_destroy(cf);
		free_2dmesh(mesh1);
		free_2dmesh(mesh2);
		free_2dmesh(mesh3);
		free_2dmesh(mesh4);
		free_2dmesh(mesh5);
		return(EXIT_FAILURE);
	}
#ifdef DEBUG
	printf("Success: Task graph with %d cores created\n", num_cores);
#endif
	// Now it is time to poppulate the graph with edges
	// to populate the graph we have to read the configuration file
	// we have 3 things to read 1) froms 2) tos 3) weights
	froms = config_lookup(cf,"froms");
	tos = config_lookup(cf,"tos");
	weights = config_lookup(cf,"weights");
	if(froms == NULL || tos == NULL || weights == NULL)
	{
		printf("Error: some problem in config file\n");
		config_destroy(cf);
		free_2dmesh(mesh1);
		free_2dmesh(mesh2);
		free_2dmesh(mesh3);
		free_2dmesh(mesh4);
		free_2dmesh(mesh5);
		free_graph(tcg);
		return(EXIT_FAILURE);
	}
	n_froms = config_setting_length(froms);
	n_tos = config_setting_length(tos);
	n_weights = config_setting_length(weights);
	if(n_froms != num_edges || n_tos != num_edges || n_weights != num_edges)
	{
		printf("Error: edges = %d defined but froms= %d tos =%d weights =%d given\n",num_edges,n_froms,n_tos,n_weights);
		config_destroy(cf);
		free_2dmesh(mesh1);
		free_2dmesh(mesh2);
		free_2dmesh(mesh3);
		free_2dmesh(mesh4);
		free_2dmesh(mesh5);
		free_graph(tcg);
		return(EXIT_FAILURE);
	}
	int i;
	for(i=0;i<num_edges;i++)
	{
		add_edge(tcg,config_setting_get_int_elem(froms,i),config_setting_get_int_elem(tos,i),config_setting_get_float_elem(weights,i));
	}
	// we dont need any further config 
	config_destroy(cf);
	// sort the edges per node NOT necessary for map3 method
	sort_graph_by_weight(tcg);
	//update_graph(tcg);  // if uncomented map1 will give better result but map3 gives bad result what ever the case map3 gives better
	// well we populated our tcg lets see if we are correct
	print_graph(tcg);
	egraph *eg = graph_to_egraph(tcg);
	sort_egraph(eg);
	display_egraph(eg);
	mesh2d *minmesh=NULL,*tempmesh;
	//display_mesh(mesh);
	// ok now we have the task core graph and the empty mesh
	// we will call the mapper function to map the graph to the mesh
	int st_rtr,best_rtr;
	float minw = 0.0, tw = 0.0;
	for(st_rtr = 0; st_rtr<num_rows*num_cols; st_rtr++)
	{
		/*
		printf("=============1st mapping method==========st rtr %d======\n",st_rtr);
		map1(tcg,mesh1,st_rtr);
		display_mesh(mesh1);
		printf("Total weight = %f\n",sum_of_weight(mesh1,tcg));
		reset_mesh(mesh1);*/
		/*		
		printf("=============2nd mapping method================\n");
		map2(tcg,mesh2);
		display_mesh(mesh2);
		printf("Total weight = %f\n",sum_of_weight(mesh2,tcg));
		*/
		/*
		printf("=============3rd mapping method==========st rtr %d======\n",st_rtr);
		map3(tcg,mesh3,st_rtr);
		display_mesh(mesh3);
		printf("Total weight = %f\n",sum_of_weight(mesh3,tcg));
		reset_mesh(mesh3);
		*/
		/*
		printf("=============4th mapping method==========st rtr %d======\n",st_rtr);
		map4(tcg,mesh4,st_rtr);
		display_mesh(mesh4);
		printf("Total weight = %f\n",sum_of_weight(mesh4,tcg));
		reset_mesh(mesh4);
		*/
		
		//printf("=============5th mapping method==========st rtr %d======\n",st_rtr);
		map5(tcg,mesh5,st_rtr);
		//display_mesh(mesh5);
		if(st_rtr == 0)
			minw = sum_of_weight(mesh5,tcg);
		tw = sum_of_weight(mesh5,tcg);
		printf("Total weight = %f\n",sum_of_weight(mesh5,tcg));
		if(tw<= minw)
		{
			minw = tw;
			best_rtr = st_rtr;
		}
		//if(st_rtr >= num_rows*num_cols) break;
		reset_mesh(mesh5);
	}
	// after this loop we found where we get minimum
	map5(tcg,mesh5,best_rtr);
	//printf("Till now minimum = %f\n",minw);
	display_mesh(mesh5);
	printf("weight = %f\n",sum_of_weight(mesh5,tcg));
	minmesh=rearrange_mesh(mesh5,tcg);
	display_mesh(minmesh);
	printf("after re arrange weight = %f\n",sum_of_weight(minmesh,tcg));
	// free the mesh and tcg 
	
	free_2dmesh(mesh1);
	free_2dmesh(mesh2);
	free_2dmesh(mesh3);
	free_2dmesh(mesh4);
	free_2dmesh(mesh5);
	free_graph(tcg);
	return(EXIT_SUCCESS);	
}

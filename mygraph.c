/*
 * mygraph.c
 * 
 * Copyright 2013 Ashok shankar Das <ashok.s.das@gmail.com>
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
// adjcency list implementation of graph                      //  
// copied and modified some portions from internet source.    //
// released as free collected by ashok.s.das@gmail.com        //
// some adition modification and adoption is done to the      //
// original code floating in internet sources                 //
// can be covered under GPL-v2, LGPL, or similar free licences//
////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
///////////////////////////////////////////////
// GRAPH implementation
///////////////////////////////////////////////
// A structure to represent an adjacency list node
typedef struct adj_list_node
{
	int dest; // destination node
	float weight; // weight of the edge
	struct adj_list_node* next;
}adj_list_node;
 
// A structure to represent an adjacency liat
typedef struct adjlist
{
	adj_list_node *head;  // pointer to head node of list
}adj_list;
 
// A structure to represent a graph. A graph is an array of adjacency lists.
// Size of array will be V (number of vertices in graph)
typedef struct graph
{
	int num_vertex; // number of vertices
	adj_list* array; // array of adjecency lists
	int *degree_array; // array of degree of nodes
	float *bw_array;
}graph;
 
// A utility function to create a new adjacency list node
adj_list_node* new_adj_list_node(int dest)
{
	struct adj_list_node* new_node = (struct adj_list_node*) malloc(sizeof(struct adj_list_node));
	new_node->dest = dest;
	new_node->next = NULL;
	new_node->weight =0.0;
	return new_node;
}
 
// A utility function that creates a graph of V vertices and returns a pointer to it
graph* create_graph(int num_nodes)
{
	graph* g = (graph*) malloc(sizeof(graph));
	g->num_vertex = num_nodes;
 
	// Create an array of adjacency lists.  Size of array will be num_nodes
	g->array = (adj_list*) malloc(num_nodes * sizeof(adj_list));
	
	
	int i;
	
	g->degree_array = (int *)malloc(num_nodes * sizeof(int));
	g->bw_array = (float *)malloc(num_nodes * sizeof(float));
	for (i = 0; i < num_nodes; ++i)
	{
		g->array[i].head = NULL; // Initialize each adjacency list as empty by making head as NULL
		g->degree_array[i] = 0;   // initialize degree of each node as 0
		g->bw_array[i] = 0.0;
	}
	return g;
}
void free_graph(graph *g)
{
	if(g != NULL)
	{
		free(g->degree_array);
		int i;
		for (i = 0; i < g->num_vertex; ++i)
		{
			adj_list_node *temp,*itr = g->array[i].head;
			while(itr)
			{
				temp = itr;
				itr = itr->next;
				free(temp);
			}
		}
	}
}
// Adds an edge 'src to dst' of weight 'w' to graph 'g' 
void add_edge(graph *g, int src, int dest, float w)
{
	// Add an edge from src to dest.  A new node is added to the adjacency
	// list of src.  The node is added at the begining
	adj_list_node* newNode = new_adj_list_node(dest);
	newNode->weight = w;
	newNode->next = g->array[src].head;
	g->array[src].head = newNode;
	g->degree_array[src]++; // increase the degree of source by one
	g->degree_array[dest]++; // increase the degree of dest by one
	g->bw_array[src] += w;
	g->bw_array[dest] += w;

/*  // uncomment if undirected graph is needed
    // Since graph is undirected, add an edge from dest to src also
    newNode = new_adj_list_node(src);
    newNode->next = g->array[dest].head;
    g->array[dest].head = newNode;
*/
}

// A utility function to print the adjacenncy list representation of graph
void print_graph(graph* g)
{
	int v;
	printf("The Graph represented by adjecent list is\n");
	for (v = 0; v < g->num_vertex; ++v)
	{
		adj_list_node* itr = g->array[v].head;
		printf("Vertex [%d]{Deg:%d} ", v+1,g->degree_array[v]);
		while (itr)
		{
			printf("-> %d( %f)", itr->dest+1,itr->weight);
			itr = itr->next;
		}
		printf("\n");
	}
	printf("=========================================\n");
}
void write_graph_to_file(graph *g, char *fname)
{
	FILE *fp = fopen(fname, "w");
	if(fp == NULL)
	{
		printf("Error: Unable to create file %s to write graph\n",fname);
		return;
	}
	char fromstr[14000]="", tostr[14000]="", wtstr[14000]="", temp[40]="";
	int numedges =0;
	fprintf(fp,"###The Graph represented by adjecent list is\n");
	int v;
	fprintf(fp,"cores = %d\n",g->num_vertex);
	for(v = 0; v<g->num_vertex; v++)
	{
		adj_list_node* itr = g->array[v].head;
		while(itr!=NULL)
		{
			sprintf(temp,"%d,",v);
			strcat(fromstr,temp);
			sprintf(temp,"%d,",itr->dest);
			strcat(tostr,temp);
			sprintf(temp,"%f,",itr->weight);
			strcat(wtstr,temp);
			numedges += 1;
			itr = itr->next;
		}
	}
	fromstr[strlen(fromstr)-1]='\0';
	tostr[strlen(tostr)-1]='\0';
	wtstr[strlen(wtstr)-1]='\0';
	fprintf(fp,"edges = %d\n",numedges);
	fprintf(fp,"froms  = [%s]\n",fromstr);
	fprintf(fp,"tos    = [%s]\n",tostr);
	fprintf(fp,"weights= [%s]\n",wtstr);
	fclose(fp);
}
// convert a graph 'g' to adjecency matrix 'adjmat'
void graph2adjmat(graph *g, float adjmat[g->num_vertex][g->num_vertex])
{
	int i,j;
	i = g->num_vertex;
	float w;
	
	bzero(adjmat, i * i * sizeof(float)); // clear out the region
	for(i=0; i<g->num_vertex; i++)
	{
		adj_list_node *itr = g->array[i].head;
		while(itr != NULL)
		{
			j= itr->dest;
			w = itr->weight;
			adjmat[i][j] = w;
			itr = itr->next;
		}
	}
	//return adjmat;
}
// converts an adjecent matrix 'mat' to a graph and returns a pointer to graph
graph *adjmat2graph(int rows,float mat[][rows])
{
	graph *g;
	int i,j;
	g = create_graph(rows);
	if(g == NULL)
	{
		printf("Error: Insufficient memory to allocate for graph\n");
		return(NULL);
	}
	for(i = 0; i<rows; i++)
	{
		for(j = 0; j<rows; j++)
		{
			if(mat[i][j] != 0.0)
				add_edge(g,i,j,mat[i][j]);
		}
	}
	return g;
}
#if 0
// NO MORE NEEDED AS WE INBUILT THE DEGREE ARRAY INTO THE STRUCTURE AND UPDATE IT EVERY TIME WE ADD AN EDGE
// creates and returns an array containing degree of all nodes in a graph 'g'
int *degree_of_nodes_array(graph *g)
{
	int i;
	int *arr = (int*)malloc(g->num_vertex * sizeof(int));
	if(arr == NULL)
	{
		printf("Error: Insufficient memory\n");
		return NULL;	
	}
	bzero(arr,g->num_vertex * sizeof(int));
	for(i = 0; i < g->num_vertex; i++)
	{
		
		adj_list_node *itr = g->array[i].head;
		while(itr)
		{
			arr[i]++;
			arr[itr->dest]++;
			itr = itr->next;
		}
	}
	return (arr);
}
#endif
// utility routine to print adjcent matrix
void print_adjmatrix(int dim,float mat[dim][dim])
{
	int i,j;
	printf("===========================Bandwidth matrix=============================\n\n");
	for(i = 0; i<dim; i++)
	{
		for(j = 0; j<dim; j++)
		{
			printf("%3.3f ",mat[i][j]);
		}
		printf("\n");
	}
}
// returns degree of node 'node' in graph 'g'
int get_degree(graph *g, int node)
{
	if(node<0 || node>g->num_vertex)
		return -1;
	return(g->degree_array[node]);
}
// returns weight of edge from src to dest in graph 'g'
float get_weight(int src, int dest, graph *g)
{
	adj_list_node* pCrawl = g->array[src].head;
	while (pCrawl)
	{
		if(pCrawl->dest == dest)
			return (pCrawl->weight);
		pCrawl = pCrawl->next;
	}
	return 0.0;
}
int *get_neighbours(int core, graph *g)
{
	int *neighbours = (int *)malloc(sizeof(int)*get_degree(g,core));
	int subscript = 0;
	if(neighbours == NULL)
	{
		return NULL;
	}
	adj_list_node *itr = g->array[core].head;
	while(itr)
	{
		neighbours[subscript] = itr->dest;
		subscript++;
		itr = itr->next;
	}
	int i;
	for(i = 0; i<g->num_vertex; i++)
	{
		itr = g->array[i].head;
		while(itr)
		{
			if(itr->dest == core)
			{
				neighbours[subscript] = i;
				subscript++;
			}
			itr = itr->next;
		}
	}
	return neighbours;
}
//////////////////////////////// GRAPH implementation ends //////////////////////////////////////

////////////////////////////////////////////////////////
///////////// MERGGE SORT //////////////////////////////
//////////////////////////////////////////////////////// 
// merge-sort to sort the adcency list

/* function prototypes */
adj_list_node* SortedMerge(adj_list_node* a, adj_list_node* b);
void FrontBackSplit(adj_list_node* source, adj_list_node** frontRef, adj_list_node** backRef);

/* sorts the linked list by changing next pointers (not data) */
void MergeSort(adj_list_node** headRef)
{
	adj_list_node* head = *headRef;
	adj_list_node* a;
	adj_list_node* b;
 
	/* Base case -- length 0 or 1 */
	if ((head == NULL) || (head->next == NULL))
	{
		return;
	}
 
	/* Split head into 'a' and 'b' sublists */
	FrontBackSplit(head, &a, &b);
 
	/* Recursively sort the sublists */
	MergeSort(&a);
	MergeSort(&b);
 
	/* answer = merge the two sorted lists together */
	*headRef = SortedMerge(a, b);
}

/* See http://geeksforgeeks.org/?p=3622 for details of this function */
adj_list_node* SortedMerge(adj_list_node* a, adj_list_node* b)
{
	adj_list_node* result = NULL;
 
	/* Base cases */
	if (a == NULL)
		return(b);
	else if (b==NULL)
		return(a);
 
	/* Pick either a or b, and recur */
	if (a->weight >= b->weight)
	{
		result = a;
		result->next = SortedMerge(a->next, b);
	}
	else
	{
		result = b;
		result->next = SortedMerge(a, b->next);
	}
	return(result);
}

/* Split the nodes of the given list into front and back halves,
     and return the two lists using the reference parameters.
     If the length is odd, the extra node should go in the front list.
     Uses the fast/slow pointer strategy.  */
void FrontBackSplit(adj_list_node* source,
          adj_list_node** frontRef, adj_list_node** backRef)
{
	adj_list_node* fast;
	adj_list_node* slow;
	if (source==NULL || source->next==NULL)
	{
		/* length < 2 cases */
		*frontRef = source;
		*backRef = NULL;
	}
	else
	{
		slow = source;
		fast = source->next;
 
		/* Advance 'fast' two nodes, and advance 'slow' one node */
		while (fast != NULL)
		{
			fast = fast->next;
			if (fast != NULL)
			{
				slow = slow->next;
				fast = fast->next;
			}
		}
 
		/* 'slow' is before the midpoint in the list, so split it in two at that point. */
		*frontRef = source;
		*backRef = slow->next;
		slow->next = NULL;
	}
}

////////////////// MergeSort ends ///////////////////////////
void sort_graph_by_weight(graph *g)
{
	int i;
	for(i = 0; i<g->num_vertex; i++)
	{
		MergeSort(&g->array[i].head);
	}
}
////////////////////////////////////////////////////////
///////////////// ARRAY INDEXER ////////////////////////
////////////////////////////////////////////////////////
#define ASCENDING 1
#define DESCENDING 0
// my_index takes an integer array, sizeof array, and order ASCENDING or DESCENDING
// returns the index array
int *my_index(int *array, int size, int order)
{
	int i,j,ctr=0;
	int *b = (int*)malloc(sizeof(int) * size);
	for(i = 0; i < size; i++ )
	{
		for(j=0;j<size;j++)
		{
			if(array[i]>array[j])
				ctr++;
		}
		for(j=0;j<i;j++)
		{
			if(array[i]==array[j] && i!=j)
			ctr++;
		}
		if(order == DESCENDING)
			b[size-ctr-1]=i;
		else
			b[ctr]=i;
		ctr=0;
	}
	return(b);
}
/////////////////// INDEXER ends ///////////////////////

typedef struct edge
{
	int vs,ve;
	float weight;
	struct edge *next;
}edge;
typedef struct egraph
{
	int num_edges;
	edge *head;
}egraph;
egraph *init_egraph()
{
	egraph *eg = (egraph *)malloc(sizeof(egraph));
	if(eg == NULL)
	{
		printf("Error: unable to allocate memory for egarph structure in %s at %d\n",__FILE__,__LINE__);
		return NULL;
	}
	eg->num_edges = 0;// currently no edges
	eg->head = NULL;  // -do-
	return (eg);
}
void free_egraph(egraph *eg)
{
	edge *temp,*temp1;
	temp = eg->head;
	while(temp)
	{
		temp1 = temp;
		temp = temp->next;
		free(temp1);
	}
	eg->num_edges = 0;
	eg->head = NULL;
	free(eg);
}
void add_egedge(egraph *eg,int vs, int ve, float w)
{
	edge *e = (edge *)malloc(sizeof(edge));
	if(e == NULL)
	{
		printf("Error: unable to allocate memory for edge in %s at %d\n",__FILE__,__LINE__);
		return;
	}
	e->vs = vs;
	e->ve = ve;
	e->weight = w;
	e->next = eg->head;
	eg->head = e;
	eg->num_edges++;
}
void display_egraph(egraph *eg)
{
	if(eg->num_edges <= 0)
		return;
	printf("Edge graph : ");
	printf("Number of edges = %d\n",eg->num_edges);
	edge *temp = eg->head;
	printf("Vs     Ve    wt\n");
	printf("======================\n");
	while(temp)
	{
		printf("[%3d %3d %9.4f]\n",temp->vs+1,temp->ve+1,temp->weight);
		temp = temp->next;
	}
	printf("=========================================\n");
}
edge *get_edge_from_egraph(int num,egraph *eg)
{
	edge *temp = eg->head;
	while(num)
	{
		temp = temp->next;
		num--;
	}
	return(temp);
}
void display_edge(edge *e)
{
	
	printf("[%3d %3d %9.4f]\n",e->vs+1,e->ve+1,e->weight);
}
egraph *graph_to_egraph(graph *g)
{
	egraph *eg;
	eg = init_egraph();
	int v;
	for (v = 0; v < g->num_vertex; ++v)
	{
		adj_list_node* itr = g->array[v].head;
		while (itr)
		{
			add_egedge(eg,v,itr->dest,itr->weight);
			itr = itr->next;
		}
	}
	return(eg);
}
edge *_move( edge *x )
{
    edge *n, *p, *ret;

    p = x;
    n = x->next;
    ret = n;
    while( n != NULL && x->weight < n->weight ) {
        p = n;
        n =  n->next;
    }
    /* we now move the top item between p and n */
    p->next = x;
    x->next = n;
    return ret;
}
edge* _sort(edge *start)
{
	
	if( start == NULL ) return NULL;
		start->next = _sort(start->next);
	if( start->next != NULL && start->weight < start->next->weight ) 
	{
		start = _move( start );
	}
	return start;
}
void sort_egraph(egraph *eg)
{
	eg->head = _sort(eg->head);
}
#ifdef TEST_SYSTEM
// main function to test the routines
int main()
{
	// create the graph given in above fugure
	int i,j;
	int a[20];
	// for test purpose pip data is used
	graph* g = create_graph(8);
	add_edge(g, 0, 4,64.0);
	add_edge(g, 1, 0,128.0);
	add_edge(g, 1, 2,64.0);
	add_edge(g, 2, 3,64.0);
	add_edge(g, 3, 6,64.0);
	add_edge(g, 4, 5,64.0);
	add_edge(g, 5, 6,64.0);
	add_edge(g, 6, 7,64.0);
	//add_edge(g, 4, 2,17.0);
 	
	// print the adjacency list representation of the above graph
	printf("The unsorted graph : \n");
	print_graph(g);
	float b[g->num_vertex][g->num_vertex]; // the adjecency matrix
	graph2adjmat(g,b);
	print_adjmatrix(g->num_vertex,b);
	graph *g1 = adjmat2graph(g->num_vertex, b);
	print_graph(g1);
	
	for(i=0 ; i<g1->num_vertex; i++)
	{
		printf("%d-> %d\n",i,get_degree(g1,i));
		int *neighbours = get_neighbours(i,g1);
		for(j = 0; j<get_degree(g1,i); j++)
		{
			printf(" %d ",neighbours[j]);
		}
		free (neighbours);
		printf("\n");
	}
	sort_graph_by_weight(g1);
	print_graph(g1);
	printf("\n");
	egraph *eg = graph_to_egraph(g1);
	display_egraph(eg);
	sort_egraph(eg);
	display_egraph(eg);
	//write_graph_to_file(g,"gtof.txt");
	free_egraph(eg);
	
	return 0;
}
#endif

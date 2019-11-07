/*
 * setsl.c
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
///////////////////////////////////////////////
// very basic set implementation using list  //
// Copyright (C) 2013, Ashok Shankar Das     //
// Released under GNU GPL-V3                 //
// for details of license visit www.gnu.org  //
///////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __SETSL_C__
#define __SETSL_C__
struct element
{
	int _val;
	struct element *next;
			
};
typedef struct element element;
typedef struct set
{
	element *elements; // list of elements
	unsigned int num_elements; // number of elements in the set
	unsigned int max_size;
}set;
typedef int (*compare_func)(void *,void*);
// copres 2 elements
int compare_elements(element *e1,element *e2)
{
	if( e1->_val < e2->_val)
		return -1;
	if(e1->_val > e2->_val)
		return 1;
	return 0;
};
// creates a set of number of elements = size
// size is maximum number of elements in the set
set* create_set( int size)
{
	set *myset; 
	myset = (set*)malloc(sizeof(set)*sizeof(char));
	if(myset ==NULL)
	{
		printf("ERROR: Insufficient Memory\n");
		return NULL;
	}
	myset->elements = NULL; // initialize elements to empty (NULL)
	myset->num_elements = 0;
	myset->max_size=size;
	return myset;
}
int is_exists(set *s, int v); // forward declaration
// adds an element v to set s
int add_element(set *s, int v)
{
	if(s->num_elements == s->max_size)
	{
		printf("Set is full\n");
		return -1; // we cant add elements
	}
	if(is_exists(s,v) > -1) // exists
	{
#ifdef DEBUG
		printf("already exists %d\n",v);
#endif
		return (s->num_elements);
	}
	
	element *temp,*e;
	
	e = (element*)malloc(sizeof(element));
	e->_val = v;
	e->next = NULL;
	if(s->elements == NULL) // first element
	{
		s->elements = e;
		s->num_elements++;
		return(s->num_elements);
	}
	temp = s->elements;
	while(temp->next != NULL)
		temp = temp->next;
	temp->next = e;
	e->next = NULL;
	s->num_elements++; // increase elements count 
	
	return s->num_elements; // return updated number of elements
}
// returns n'th element in a set 's'(non destructive)
int get_element(int num, set *s)
{
	element *e;
	e = s->elements;
	if(e == NULL)
		return -1;
	if(num > s->num_elements || num<0)
		return -1;
	while(num)
	{
		e = e->next;
		num--;	
	}
	return (e->_val);
}
// checks if v exists in set s
// if not found or error returns -1
// else returns the element number
int is_exists(set *s, int v)
{
	element *e;
	e = s->elements;
	if(e == NULL)
		return -1;
	int i=0;
	while(e != NULL)	
	{
		if(e->_val== v)
			return i;
		i++;
		e = e->next;
	}
	return -1;
}

// removes element v from set s
int remove_element(set *s, int v)
{
	element *e1,*e2;
	int t;
	t = is_exists(s,v);	
	if( t == -1)
		return -1;
	if(t == 0) // if it is the first element
	{
		e1 = s->elements->next;
		free(s->elements);
		s->elements = e1;
		s->num_elements--;
	}
	else
	{
		e1 = s->elements;
		while(e1)
		{
			e2 = e1;
			e1 = e1->next;
			if(e1->_val == v)
			{
				e2->next = e1->next;
				s->num_elements--;
				free(e1);
				e1 = NULL;		
			}
			
		}
	}
	return t;
}
// is t a subset of s
int is_a_subset(set *s, set *t)
{
	if(t->num_elements > s->num_elements)
		return -1;
	element *te;
	//se = s->elements;
	te = t->elements;
	while(te != NULL)
	{
		if(is_exists(s,te->_val) == -1)
			return -1;
		te = te->next;
	}
	return 1;
}
set *setintersection(set *s, set *t);
set *setdiff(set *s, set *t);
// make a union set of 2 sets s and t
// returns the union set
set *setunion(set *s, set *t)
{
	//copies all elements from both the sets  
	set *n;
	int size;
	size = s->num_elements + t->num_elements;
	n= create_set(size);
	element *e;
	e = s->elements;
	while(e != NULL)
	{
		if(is_exists(n,e->_val) == -1) // doesnt exist
			add_element(n,e->_val);
		e = e->next;
	}
	e = t->elements;
	while(e != NULL)
	{
		if(is_exists(n,e->_val) == -1) // doesnt exist
			add_element(n,e->_val);
		e = e->next;
	}
	return n;
	
}
// finds the intersection set of s and t
// returns the intersection set
set *setintersection(set *s, set *t)
{
	//copies only common elements
	set *n;
	n=create_set(s->num_elements>t->num_elements? t->max_size : s->max_size);
	element *e;
	e = s->elements;
	while(e)
	{
		if(is_exists(t,e->_val)>-1)
			add_element(n,e->_val);
		e = e->next;
	}
	return n;
}
// copy(assign) t to s
// i.e     s = t
set* set_copy(set *t)
{
	if(t == NULL)
		return NULL;
	set *s = create_set(t->max_size);
	if(s == NULL)
		return NULL;
	
	element *e = t->elements;
	while(e)
	{
		add_element(s,e->_val);
		e = e->next;	
	}
	return (s);
}
// finds the difference of set (s - t)
// returns the difference set
set *setdiff(set *s, set *t)
{
	set *n,*p;
	n = setintersection(s,t);
	p=set_copy(s);
	if(p == NULL)
		return NULL;
	
	element *e;
	
	e = n->elements;
	while(e)
	{
		remove_element(p,e->_val);
		e = e->next;
	}
	return p;
}

// printd the set containts
void print_set(set *s)
{
	printf("{");
	element *e = s->elements;
	while(e != NULL)
	{
		printf("%d ",e->_val);
		e = e->next;
	}
	printf("}\n");
}
#endif // __SETSL_C__
#ifdef TEST_SET
int main()
{
	set *a,*b,*c;
	a = create_set(5);
	b = create_set(3);

	add_element(a,1);
	add_element(a,2);
	add_element(a,3);
	add_element(a,4);
	add_element(a,1);
	add_element(b,2);
	add_element(b,5);
	add_element(b,5);
	
	print_set(a);
	print_set(b);

	printf("is b subset of a =");
	if(is_a_subset(a,b)==1)
		printf("True\n");
	else
		printf("false\n");
	printf("a U b =");
	c = setunion(a,b);
	print_set(c);
	add_element(c,9);
	if(add_element(c,20)== -1)
		printf("Can't add\n");
	print_set(c);
	//remove_element(c,3);
	//remove_element(c,1);
	//remove_element(c,5);


	printf(" intersection=");
	print_set(setintersection(a,b));
	
	printf(" a -b =");
	print_set(setdiff(a,b));
	printf("b - a =");
	print_set(setdiff(b,a));
	set *d;
	d=set_copy(c);
	print_set(d);
	
	free(a);
	free(b);
	free(c);
	free(d);
}
#endif

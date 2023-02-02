#include <Python.h>
#include <numpy/arrayobject.h>
#include <stdio.h>
#include <gsl_randist.h>
#include <gsl_rng.h>
#include <sys/time.h>
#include "complex.h"

#define N         2		// number of reaction
#define M         2		// number of chemical species
	
void init_two_state(int x[], int s[N][M]){

	// population of chemical species
	x[0] = 1;
	x[1] = 0;

    // N x M
	s[0][0] = -1;
	s[0][1] =  1;
	s[1][0] = -1;
	s[1][1] =  1;

}


void update_p_two_state(double t, double p[], double k_off, 
              double a, double b, double r1, double r2, int x[]){
	p[0] = a + b*(1-exp(-r1*t))*exp(-r2*t);
	p[1] = k_off;
	//printf("Time %f, k_on = %f, k_off = %f\n", t, p[0], p[1]);
}


int select_reaction_two_state(double p[], int pn, double sum_propencity, double r){
	int reaction = -1;
	double sp = 0.0;
	int i;
	r = r * sum_propencity;
	for(i=0; i<pn; i++){
		sp += p[i];
		if(r < sp){
			reaction = i;
			break;
		}
	}
	return reaction;
}

void update_x_two_state(int x[], int reaction){
	if (reaction == 0){
	   x[0] = 0; 
	   x[1] = 1;
	} else {
	   x[0] = 1;
	   x[1] = 0;
	} 
}

double sum_two_state(double a[], int n){
	int i;
	double s=0.0;
	for(i=0; i<n; i++) 
		s += a[i];
	return(s);
}

int two_state_ssa(int* x1, int* x2, double* times, double end_time, double time_step, double k_off, double a, double b, double r1, double r2){

	int x[M];			    // population of chemical species
	double c[N];			// reaction rates
	double p[N];			// propencities of reaction
	int s[N][M];			// data structure for updating x[]

	// init_two_stateialization
	double sum_propencity = 0.0;	// sum of propencities
	double tau=0.0;			// step of time
	double t=0.0;			// time
	double r;			// random number
	int reaction;			// reaction number selected

	init_two_state(x, s);
    int i = 0;
    time_t current_time = time(NULL);
    double seed = (double)current_time + (double)clock() / CLOCKS_PER_SEC;
    srand((unsigned int)(seed * 1000000.0));
    //printf("Seed: %f\n", seed);
	x1[0] = 1;
	x2[0] = 0;
		
	// main loop
	//printf("Time: 0 hours, x1=%d, x2=%d \n", x1[0], x2[0]);
	while(t < end_time){
		
		// update propencity
		update_p_two_state(t, p, k_off, a, b, r1, r2, x);
		sum_propencity = sum_two_state(p, N);
	    i += 1;

		// sample tau
		if(sum_propencity > 0){
			tau = -log((double)rand()/INT_MAX) / sum_propencity;
		}else{
			break;
		}
	
		// select reaction
		r = (double)rand()/INT_MAX;
		//printf("Random num: %f\n",r);
		reaction = select_reaction_two_state(p, N, sum_propencity, r);
		// update chemical species
		update_x_two_state(x, reaction);
		x1[i] = x[0];
		x2[i] = x[1];
        
		// time
		t += tau;
		times[i] = t;
	    //printf("%d Time: %f hours, Reaction: %d x1=%d, x2=%d \n", i, t, reaction, x1[i], x2[i]);	
	}
	
	return(0);
}


static PyObject* two_state(PyObject* Py_UNUSED(self), PyObject* args) {

  PyObject* list;

  if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &list))
    return NULL;
    
    double end_time = PyFloat_AsDouble(PyList_GetItem(list, 0));
    double time_step = PyFloat_AsDouble(PyList_GetItem(list, 1));
    double k_off = PyFloat_AsDouble(PyList_GetItem(list, 2));
    double a = PyFloat_AsDouble(PyList_GetItem(list, 3));
    double b = PyFloat_AsDouble(PyList_GetItem(list, 4));
    double r1 = PyFloat_AsDouble(PyList_GetItem(list, 5));
    double r2 = PyFloat_AsDouble(PyList_GetItem(list, 6));
    int Nt = PyLong_AsLong(PyList_GetItem(list, 7));
 
	int* x1 = calloc(Nt, sizeof(int));
	int* x2 = calloc(Nt, sizeof(int));
	double* times = calloc(Nt, sizeof(double));
	two_state_ssa(x1,x2,times,end_time,time_step,k_off,a,b,r1,r2);

	npy_intp dims[1] = {Nt}; //row major order
	PyObject *x1_out = PyArray_SimpleNew(1, dims, NPY_INT);
	PyObject *x2_out = PyArray_SimpleNew(1, dims, NPY_INT);
	PyObject *times_out = PyArray_SimpleNew(1, dims, NPY_DOUBLE);

    memcpy(PyArray_DATA(x1_out), x1, Nt * sizeof(int));
	memcpy(PyArray_DATA(x2_out), x2, Nt * sizeof(int));
	memcpy(PyArray_DATA(times_out), times, Nt * sizeof(double));

	return Py_BuildValue("(OOO)", x1_out, x2_out, times_out);
	free(x1);
	free(x2);
	free(times);
}

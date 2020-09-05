#include <stdio.h>
#include <stdlib.h>

#include "groundGridGeneration.hpp"

float * vertices_2;
int * indices_2;

//float * vertices; //pointer to array
int verticesSize_2 = 0;

//int * indices;  //pointer to array
int indicesSize_2 = 0;

void generateGroundGrid(int sc, int detail){
    //sc = squareCount
    //detail = size of the squares. larger number = smaller squares.

//check detail field:
    if(detail < 2){
        fprintf(stderr, "WARNING: Detail can not be set below 2. Setting to \
            default value.");
        detail = 2;
    }

//set up arrays
    delete[] vertices_2;  //if there was allready memory allocated, remove it.
    verticesSize_2 = (sc+1)*(sc+1)*3; //number of elements ((sc*2)+1)*((sc*2)+1)*3;
    vertices_2 = new float [verticesSize_2]; //allocate memory for the vertices array
    verticesSize_2 = verticesSize_2 * sizeof(float); //now a proper sizeof value
    delete[] indices_2;   //if there was allready memory allocated, remove it.
    indicesSize_2 = sc*sc*2*3; //number of elements
    indices_2 = new int[indicesSize_2];
    indicesSize_2 = indicesSize_2 * sizeof(int);//now a proper sizeof value



    float scf = (float)sc / detail;
    int index = 0;

    for(float i = scf/2; i >= -scf/2; i = i - (1/(float)detail)){

        for(float j = scf/2; j >= -scf/2; j = j - (1/(float)detail)){
            //printf("%ff, %ff, 0.0f,\n", i,  j);
            vertices_2[index++] = i;
            vertices_2[index++] = j;
            vertices_2[index++] = 0; //if Z is calculated on CPU, do it here
        }

    }

    index = 0; //reset index, reuse it

    for(int i = 0; i < sc; i++ ){

        for(int j = 0; j < sc; j++ ){
            //printf("%d, %d, %d,\n", i*(sc+1)+j , i*(sc+1)+j+1 , (i+1)*(sc+1)+j );
            //printf("%d, %d, %d,\n", i*(sc+1)+j+1 , (i+1)*(sc+1)+j+1 , (i+1)*(sc+1)+j );

            indices_2[index++] = i*(sc+1)+j;
            indices_2[index++] = i*(sc+1)+j+1;
            indices_2[index++] = (i+1)*(sc+1)+j;

            indices_2[index++] = i*(sc+1)+j+1;
            indices_2[index++] = (i+1)*(sc+1)+j+1 ;
            indices_2[index++] = (i+1)*(sc+1)+j;

        }
    }

}

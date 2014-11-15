/*********************************************************************************
 * FILENAME         distribution.c
 *
 * DESCRIPTION      These functions are part of the submission to exercises of 
 *                  the "Introduction to High Percformance Computing" (Intro HPC) 
 *                  lecture of the University of Heidelberg.
 * 
 *                  Exercise 4 - Parallel Matrix Multiplication
 * 
 * AUTHORS          Klaus Naumann
 *                  Christoph Klein
 *                  Günther Schindler
 *
 * LAST CHANGE      14. NOV 2014
 *
 ********************************************************************************/

#include "distribution.h"
#include <stdlib.h>
#include <stdio.h>

void vDistributeOutputMatrix( int iRows, int iCols, sJobList *pJobList) {

    int iJobCount = pJobList->iTotalJobs;

    /* initialize tree */
    sTree *pTree;

    pTree = (sTree *) malloc(sizeof(sTree*)); 
    if (pTree == NULL)
        exit(EXIT_FAILURE);
  
    /* set maximum height of the tree */
    pTree->iHeight = iTreeHeight(iJobCount); 

    /* 
    * total number of elements in a perfect balanced tree
    * with height H is equal to 2^(H+1) - 1
    * This is realized by bitwise shift operator.
    */
    pTree->iTreeSize = (1 << (pTree->iHeight + 1)) - 1;

    /* allocation for the elements */
    pTree->ppElement = (sElement **) malloc(pTree->iTreeSize * sizeof(sElement **)); 
    if (pTree->ppElement == NULL)
        exit(EXIT_FAILURE);

    /* initialization of the elements */
    int i;
    for (i = 0; i < pTree->iTreeSize; i++) {

        /* allocation of elements */
        pTree->ppElement[i] = (sElement *) malloc(sizeof(sElement *));
        if (pTree->ppElement[i] == NULL)
            exit(EXIT_FAILURE);

        /* mark every element as empty */
        pTree->ppElement[i]->iEmpty = 1;
    }
    
    /* mark first element */
    pTree->ppElement[0]->iEmpty = 0;
    pTree->ppElement[0]->iRowBegin = 0;
    pTree->ppElement[0]->iRowEnd = iRows;
    pTree->ppElement[0]->iColBegin = 0;
    pTree->ppElement[0]->iColEnd = iCols;

    /* add nodes to the tree until #leafs == #iJobCount */
    while (iCountLeafs(pTree) != iJobCount) {
        vDivideLeaf(pTree);
    }

    /* transform the tree structure to useful Job structure */
    vLeafs2Jobs(pTree, pJobList);
    
    /* free allocated memory */
    for (i = 0; i < pTree->iTreeSize; i++) {
        free(pTree->ppElement[i]);
    }
    free(pTree->ppElement);
    free(pTree);
}

void vDivideLeaf(sTree *pTree) {

    int iEmp = 0;
    int iFstEmp = 0;
    int iParent;
    int iBeginA, iBeginB, iEndA, iEndB;
    int iRowDiff, iColDiff;

    /* get index of first empty element */
    while (!iEmp) {

        if (iFstEmp >= pTree->iTreeSize)
            exit(EXIT_FAILURE);

        ++iFstEmp;
        iEmp = pTree->ppElement[iFstEmp]->iEmpty;
    }

    /* get parent of first empty index */
    iParent = (iFstEmp - 1) / 2;

    /* set children limits */
    iRowDiff = pTree->ppElement[iParent]->iRowEnd - pTree->ppElement[iParent]->iRowBegin;
    iColDiff = pTree->ppElement[iParent]->iColEnd - pTree->ppElement[iParent]->iColBegin;

    if (iRowDiff >= iColDiff) {
        /* divide the rows */
        iBeginA = pTree->ppElement[iParent]->iRowBegin;
        iEndA = iRowDiff/2 + pTree->ppElement[iParent]->iRowBegin;
        iBeginB = iEndA;
        iEndB = pTree->ppElement[iParent]->iRowEnd;

        /* set the values A */
        pTree->ppElement[iFstEmp]->iEmpty = 0;
        pTree->ppElement[iFstEmp]->iRowBegin = iBeginA;
        pTree->ppElement[iFstEmp]->iRowEnd = iEndA;
        pTree->ppElement[iFstEmp]->iColBegin = pTree->ppElement[iParent]->iColBegin;
        pTree->ppElement[iFstEmp]->iColEnd = pTree->ppElement[iParent]->iColEnd;

        /* 
        * set the values B 
        * (size of job B)  == ((size of job A) + 1Row) if (iRowDiff % 2 == 1)
        */
        pTree->ppElement[iFstEmp+1]->iEmpty = 0;
        pTree->ppElement[iFstEmp+1]->iRowBegin = iBeginB;
        pTree->ppElement[iFstEmp+1]->iRowEnd = iEndB;
        pTree->ppElement[iFstEmp+1]->iColBegin = pTree->ppElement[iParent]->iColBegin;
        pTree->ppElement[iFstEmp+1]->iColEnd = pTree->ppElement[iParent]->iColEnd;
    }
    else {
        /* divide the columns */
        iBeginA = pTree->ppElement[iParent]->iColBegin;
        iEndA = iColDiff/2 + pTree->ppElement[iParent]->iColBegin;
        iBeginB = iEndA;
        iEndB = pTree->ppElement[iParent]->iColEnd;

        /* set the values A */
        pTree->ppElement[iFstEmp]->iEmpty = 0;
        pTree->ppElement[iFstEmp]->iColBegin = iBeginA;
        pTree->ppElement[iFstEmp]->iColEnd = iEndA;
        pTree->ppElement[iFstEmp]->iRowBegin = pTree->ppElement[iParent]->iRowBegin;
        pTree->ppElement[iFstEmp]->iRowEnd = pTree->ppElement[iParent]->iRowEnd;

        /* 
        * set the values B 
        * (size of job B)  == ((size of job A) + 1Col) if (iColDiff % 2 == 1)
        */
        pTree->ppElement[iFstEmp+1]->iEmpty = 0;
        pTree->ppElement[iFstEmp+1]->iColBegin = iBeginB;
        pTree->ppElement[iFstEmp+1]->iColEnd = iEndB;
        pTree->ppElement[iFstEmp+1]->iRowBegin = pTree->ppElement[iParent]->iRowBegin;
        pTree->ppElement[iFstEmp+1]->iRowEnd = pTree->ppElement[iParent]->iRowEnd;
    }
}

int iTreeHeight(int iJobCount) {
    
    int i = 1; // loop var
    int h = 0; // tree height
    while (i < iJobCount) {
        i = i << 1;
        ++h;
    }
    return h;
}

int iCountLeafs(sTree *pTree) {

    int iLeafs = 0;
    int i;
    int iEmp;
    int iChild;
    int iSize = pTree->iTreeSize;

    for (i = 0; i < iSize; i++) {
        iEmp = pTree->ppElement[i]->iEmpty;
        if (iEmp) break;
        iChild = 2 * i + 1;
        if (iChild >= iSize) 
            iLeafs++;
        else if (iChild < iSize) {
            if (pTree->ppElement[iChild]->iEmpty)
                iLeafs++;
        }
    }
    return iLeafs;
}

void vLeafs2Jobs(sTree *pTree, sJobList *pJobList) {

    int i;
    int iEmp;
    int iChild;
    int iSize = pTree->iTreeSize;
    int iJobNr = 0;

    for (i = 0; i < iSize; i++) {
        iEmp = pTree->ppElement[i]->iEmpty;
        if (iEmp) break;
        iChild = 2 * i + 1;
        /* is leaf if the children index is outside the tree's size */
        if (iChild >= iSize) {
            /* get job properties from leaf */
            pJobList->ppJob[iJobNr]->iJob = iJobNr;
            pJobList->ppJob[iJobNr]->iRowBegin = pTree->ppElement[i]->iRowBegin;
            pJobList->ppJob[iJobNr]->iRowEnd = pTree->ppElement[i]->iRowEnd;
            pJobList->ppJob[iJobNr]->iColBegin = pTree->ppElement[i]->iColBegin;
            pJobList->ppJob[iJobNr]->iColEnd = pTree->ppElement[i]->iColEnd;
            iJobNr++; // increase job id
        }
            
        else if (iChild < iSize) {
            /* is leaf when the children are empty */
            if (pTree->ppElement[iChild]->iEmpty)
                /* get job properties from leaf */
                pJobList->ppJob[iJobNr]->iJob = iJobNr;
                pJobList->ppJob[iJobNr]->iRowBegin = pTree->ppElement[i]->iRowBegin;
                pJobList->ppJob[iJobNr]->iRowEnd = pTree->ppElement[i]->iRowEnd;
                pJobList->ppJob[iJobNr]->iColBegin = pTree->ppElement[i]->iColBegin;
                pJobList->ppJob[iJobNr]->iColEnd = pTree->ppElement[i]->iColEnd;
                iJobNr++; //increase job id
        }
    }
}

int vAllocateJobList(int iJobNr, sJobList *pJobList) {

    int i;
    /* allocate pointer to list struct */
    pJobList = (sJobList *) malloc(sizeof(sJobList *));
    if (pJobList == NULL) return 1;

    /* allocate pointer to job pointers */
    pJobList->ppJob = (sJob **) malloc(iJobNr * sizeof(sJob **));
    if (pJobList->ppJob == NULL) return 1;

    for (i = 0; i < iJobNr; i++) {
        
        /* allocate job pointer */ 
        pJobList->ppJob[i] = (sJob *) malloc(sizeof(sJob *));
        if (pJobList->ppJob[i] == NULL) return 1;
    }

    return 0;
}

void vFreeJobList(sJobList *pJobList) {
    int N = pJobList->iTotalJobs;
    int i;
    for (i = 0; i<N; i++) {
        free(pJobList->ppJob[i]);
    }
    free(pJobList->ppJob);
    free(pJobList);
}

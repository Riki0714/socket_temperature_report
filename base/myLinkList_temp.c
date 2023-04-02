/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  myLinkList.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(03/25/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/25/23 12:34:33"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "get_temp.h"

#include "myLinkList_temp.h"

//1.
//-------------- Initializes a linked list so that the header pointer is null
//------ pNode: the pointer of header pointer
void list_init(Node **pNode)
{
	*pNode = NULL;
	printf("Successful initialization\n");
}


//2. 
//-------------- Insert a new node from the header
//------ pNode: the pointer of header pointer
//		 insertElem: The data content of the new node
int list_insert_head(Node **pNode, Temp_str *tempStr, char * insertElem)
{
	Node *pIn;

	pIn = (Node *)malloc(sizeof(Node));
	memset(pIn, 0, sizeof(Node));
	//pIn = calloc(0, sizeof(Node));

	strncpy(pIn->element, insertElem, 64);
	//pIn->element = insertElem;
	pIn->next = *pNode;
	*pNode = pIn;

	return 1;
}


//3.
//-------------- Insert a new node from the tail
//------ pNode: the pointer of header pointer
//		 insertElem: The data content of the new node
int list_insert_tail(Node **pHead, Temp_str *tempStr, char * insertElem)
{
	Node *pIn=NULL;
	Node *pTmp=*pHead;
	Node *pHead_o=*pHead;  

	pIn = (Node *)malloc(sizeof(Node));
	memset(pIn, 0, sizeof(Node));

	strncpy(pIn->element, insertElem, 64);
	//pIn->element = insertElem;
	pIn->next = NULL;

	if( *pHead == NULL )
	{
		*pHead = pIn;
	}
	else
	{
		while( pTmp->next != NULL )
		{
			pTmp = pTmp->next;
		}
		pTmp->next = pIn;
		*pHead = pHead_o;
	}

	return 1;
}


//4.
//-------------- Find the node where the target value is located and return its content
//------ pHead: the hearder pointer
//		 x: What to look for
int list_find(Node *pHead, Temp_str *tempStr, char * x)
{
	int 	pos=0;

	if( NULL == pHead )
	{
		printf("The list is empty\n");
		return 0;
	}

	if( x==NULL )
	{
		printf("x is not allowed\n ");
		return 0;
	}

	while( (pHead->element != x) && (NULL != pHead->next) )
	{
		pHead = pHead->next;
		pos++;
	}

	if( (pHead->element != x) && (NULL != pHead) )
	{
		printf("can't find\n");
		return 0;
	}

	return pos;
}

//5.
//-------------- Find the node where the target value is located and return its address
//------ pHead: the hearder pointer
//		 x: What to look for
Node * list_findi_Node(Node *pHead, Temp_str *tempStr, char * x)
{
	if( NULL == pHead )
	{
		printf("The list is empty\n");
		return 0;
	}

	if( x<0 )
	{
		printf("x is not allowed\n ");
		return 0;
	}

	while( (pHead->element != x) && (NULL != pHead->next) )
	{
		pHead = pHead->next;
	}

	if( (pHead->element != x) && (NULL != pHead) )
	{
		printf("can't find\n");
		return 0;
	}

	return pHead;
}


//6.
//-------------- Find the node where the target value resides and return its contents
//------ pHead: the hearder pointer
//		 pos: The row number of the node
char * list_get(Node *pHead, int pos)
{
	int			  i=0;
	char	 *buf=NULL;

	if(pos<1)
	{
		printf("pos is not allowed\n");
		return buf;
	}
	if( NULL == pHead )
	{
		printf("The list is empty\n");
		return buf;
	}

	while( NULL != pHead )
	{
		i++;
		if( i==pos ) break;
		pHead = pHead->next;
	}
	if( i<pos )
	{
		printf("can't find\n");
		return buf;
	}

	return pHead->element;
}


//7.
//-------------- Find the node where the target value resides and return its address
//------ pHead: the hearder pointer
//		 pos: The row number of the node
Node * list_get_Node(Node *pHead, int pos)
{
	int			  i=0;
	Node		 *buf=NULL;

	if(pos<1)
	{
		printf("pos is not allowed\n");
		return buf;
	}
	if( NULL == pHead )
	{
		printf("The list is empty\n");
		return buf;
	}

	while( NULL != pHead )
	{
		i++;
		if( i==pos ) break;
		pHead = pHead->next;
	}
	if( i<pos )
	{
		printf("can't find\n");
		return buf;
	}

	return pHead;
}

/* 8.
	function: Deletes a node in the header

	parameter:
		(1) pNode: the pointer of header pointer

	return value:
		>0: succeed
		else: failure
*/
int list_drop_head(Node **pNode)
{
	Node *one, *ptr;

	one = *pNode;

	if( NULL == one )
	{
		printf("The list is empty\n");
		return -1;
	}
	else if( one->next == NULL )
	{
		ptr = one;
		one = NULL;
		free(ptr);
		return 1;
	}

	*pNode = one->next;
	free(one);

	return 2;
}


//9.
//-------------- Deletes a node in the tail
//
int list_drop_tail(Node **pNode)
{
	Node *pHead, *pTmp, *ptr;

	pHead = *pNode;
	pTmp = pHead;

	if( NULL == pHead )
	{
		printf("The list is empty\n");
		return -1;
	}
	else if( pHead->next == NULL )  //only one node
	{
		ptr = pHead;
		pHead = NULL;
		free(ptr);
		return 1;
	}

	while( NULL!=pHead->next )
	{
		ptr = pHead;
		pHead = pHead->next;
	}

	ptr->next = NULL;	
	free(pHead);
	*pNode = pTmp; //?

	return 2;
}


//10.
//-------------- Delete a specified node
int list_drop_pos(Node *pHead, int pos)
{
	Node 	*pFront, *pLast;
	int 	 i=0;

	if( pos<1 )
	{
		printf("pos is not allowed\n");
		return -1;
	}
	if( NULL == pHead )
	{
		printf("The list is empty\n");
		return -2;
	}

	while( NULL != pHead )
	{
		i++;
		if( i==pos ) break;

		pFront = pHead;
		pHead = pHead->next;
	}

	if( i<pos )
	{
		printf("can't find\n");
		return -3;
	}
	else if( i==pos )
	{
		pFront->next = NULL;
		free(pHead);
	}
	else 
	{
		pFront = pHead->next;
		free(pHead);
	}

	return 1;
}


//11.
//-------------- Clear linked list
//
int list_clear(Node *pHead)
{
	Node *pNext;

	if( NULL == pHead )
	{
		printf("The list is empty\n");
		return -1;
	}

	while( NULL!=pHead->next )
	{
		pNext = pHead->next;
		free(pHead);
		pHead = pNext;
	}

	return 1;
}


//12.
//-------------- Determines whether the linked list is empty
//
int list_isEmpty(Node *pHead)
{
	if( NULL == pHead)
	{
		printf("is empty\n");
		return 1;
	}
	printf("isn't empty\n");
	return -1;
}


//13.
//-------------- Print the contents of the linked list
//
int list_print(Node *pHead)
{
	if( NULL==pHead )
	{
		printf("The list is empty\n");
		return -1;
	}

	while( NULL != pHead )
	{
		printf("%s \n", pHead->element);
		pHead = pHead->next;
	}
	return 1;
}


//14.
//-------------- Calculate the length of the list
//
int list_size(Node *pHead)
{
	int len=0;

	if( NULL==pHead )
	{
		printf("The list is empty\n");
		return 0;
	}

	while( NULL!=pHead )
	{
		len++;
		pHead = pHead->next;
	}
	return len;
}

/*  
int main()
{
	Node *pList = NULL;
	char buf[32]="11";
	char buf1[32]="22";
	char buf2[32]="33";
	char buf3[32] = {0};
	elemType buf_get=NULL;

	list_insert_head(&pList, buf); //11

		//strncpy(buf, buf2, sizeof(buf));
	//list_insert_head(&pList, buf); //33

	list_insert_tail(&pList, buf1); //22
	list_insert_tail(&pList, buf2); //33
	
	strncpy(buf1, buf, sizeof(buf1));
	list_insert_head(&pList, buf1); //22


	//buf_get = *list_get(pList, 2);
	//printf("%s\n", buf_get);

	//printf("11111111111\n");
	//list_drop_tail(&pList);

	list_print(pList);
}
*/



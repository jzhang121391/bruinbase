#include <cstdio>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeNode.h"
#include "PageFile.h"

using namespace std;

const int SHOW_ERRORS = 1;

int test_nonleaf()
{
	PageFile p;									//	page file
	int opened;									//	check if page file was opened
	unsigned char *buffer = (unsigned char*) malloc(PageFile::PAGE_SIZE);	//	buffer for reading
	int i = 0;
	int a;

	BTNonLeafNode root;
	root.initializeRoot(1, 50, 2);

	BTNonLeafNode root_2;
	root_2.initializeRoot(3, 60, 4);
	root_2.insert(-70, -5); // Insert negative inputs
	root_2.insert(80, 6); // Insert on full node (should fail)

	BTNonLeafNode nonleaf_1;
	nonleaf_1.initializeRoot(3, 25, 4);
	nonleaf_1.insert(80, 7);

	//nonleaf_1.insert(25, 8);

	BTNonLeafNode nonleaf_2;
//	nonleaf_2.initializeRoot(5, 75, 6);
	nonleaf_2.insert(80, 7);
	nonleaf_2.insert(70, 8);
	nonleaf_2.insert(90, 9);
	nonleaf_2.insert(72, 10);
	nonleaf_2.insert(81, 11);

	//nonleaf_2.insert(999999,7);

	//	result should be <70>8<72>10<80>7<81>11<90>9

	BTNonLeafNode empty;
	PageId new_page = 12;
	nonleaf_2.insertAndSplit(82, new_page, empty, a);
	//printf("midKey result #1: %i\n", a);

	BTNonLeafNode nonleaf_4, empty_2;
	PageId new_page_2 = 13;
	//nonleaf_4.insert(50, 5);
	if(SHOW_ERRORS)
		printf("insertAndSplit on empty node: %i\n", nonleaf_4.insertAndSplit(40, new_page_2, empty_2, a));
	//printf("midKey result #2: %i\n", a);

	/*
		result should be:
			- initially <70>8<72>10<80>7<81>11<82>12<90>9
			- split into <70>8<72>10<80>7 <81> 11<82>12<90>9
	*/

	BTNonLeafNode nonleaf_3;

	if(SHOW_ERRORS) {
		//printf("%i\n", nonleaf_3.getKeyCount());
		printf("locateChildPtr on empty node: %i\n", nonleaf_3.locateChildPtr(5, a));
		//printf("%i\n", a);
	}

	//  result should be -1012

	for(int i = 0; i < 200; i++) {
		if(nonleaf_3.insert(i,i) != 0) {
			if(SHOW_ERRORS)
				printf("insert on full node: %i\n", nonleaf_3.insert(i,i));
			break;
		}
	}

	//  result should be -1010

	int e;

	if(SHOW_ERRORS)
		printf("insertAndSplit with non-empty sibling: %i\n", nonleaf_3.insertAndSplit(200, new_page+1, nonleaf_3, e));

	//  result should be -1014

	BTNonLeafNode locate;
	locate.initializeRoot(10, 20, 30);
//	locate.insert(20,10);

	int b, c, d;

	locate.locateChildPtr(5, a);
	locate.locateChildPtr(15, b);
	locate.locateChildPtr(2, c);
	locate.locateChildPtr(35, d);

	fprintf(stderr, "a = %i, b = %i, c = %i, d = %i\n", a, b, c, d);

	//	open the page file

	opened = p.open("a.txt", 'w');

	if(!opened)
	{

		//	store the root non-leaf node in the first page file

		if(root.write(0, p))
		{
			fprintf(stderr, "Bad write\n");
			p.close();
			free(buffer);
			return -1;
		}


		//  store full non-leaf root node

		if(root_2.write(4, p))
		{
			fprintf(stderr, "Bad write\n");
			p.close();
			free(buffer);
			return -1;
		}

		//	store left non-leaf node

		if(nonleaf_1.write(1, p))
		{
			fprintf(stderr, "Bad write\n");
			p.close();
			free(buffer);
			return -1;
		}

		//	store right non-leaf node

		if(nonleaf_2.write(2, p))
		{
			fprintf(stderr, "Bad write\n");
			p.close();
			free(buffer);
			return -1;
		}

		if(empty.write(12, p))
		{
			fprintf(stderr, "Bad write\n");
			p.close();
			free(buffer);
			return -1;
		}

		// store full non-leaf node

		if(nonleaf_3.write(3, p))
		{
			fprintf(stderr, "Bad write\n");
			p.close();
			free(buffer);
			return -1;
		}

//		if(!root.read(0, p) && !p.read(0, buffer))
		if(!p.read(0, buffer))
		{
			fprintf(stderr, "The buffer for page %i is: ", 0);

			root.printItems();
		}
		else
			fprintf(stderr, "Bad read\n");

//		if(!nonleaf_1.read(1, p) && !p.read(1, buffer))
		if(!p.read(1, buffer))
		{
			fprintf(stderr, "The buffer for page %i is: ", 1);

			nonleaf_1.printItems();
		}
		else
			fprintf(stderr, "Bad read\n");

//		if(!nonleaf_2.read(1, p) && !p.read(2, buffer))
		if(!p.read(2, buffer))
		{
			fprintf(stderr, "The buffer for page %i is: ", 2);

			nonleaf_2.printItems();
		}
		else
			fprintf(stderr, "Bad read\n");

		if(!p.read(12, buffer))
		{
			fprintf(stderr, "The buffer for page %i is: ", 12);

			empty.printItems();
		}
		else
			fprintf(stderr, "Bad read\n");

		if(!p.read(3, buffer))
		{
			fprintf(stderr, "The buffer for page %i is: ", 3);

			nonleaf_3.printItems();
		}
		else
			fprintf(stderr, "Bad read\n");

		if(!p.read(4, buffer))
		{
			fprintf(stderr, "The buffer for page %i is: ", 4);

			root_2.printItems();
		}
		else
			fprintf(stderr, "Bad read\n");

		p.close();
	}
	else
		fprintf(stderr, "Cannot open file\n");

	free(buffer);
	return 0;
}

int test_leaf()
{
	PageFile p;									//	page file
	int opened;									//	check if page file was opened
	void *buffer = malloc(PageFile::PAGE_SIZE);	//	buffer for reading
	RecordId rid_1, rid_2, rid_3, rid_4, rid_5, rid_get;
	int key;

	rid_1.pid = 1;
	rid_1.sid = 2;
	rid_2.pid = 3;
	rid_2.sid = 4;
	rid_3.pid = 5;
	rid_3.sid = 6;
	rid_4.pid = 7;
	rid_4.sid = 8;
	rid_5.pid = 9;
	rid_5.sid = 10;

	//	locate testing

	int a, b, c, d, e;

	BTLeafNode leaf_1, leaf_2;

	leaf_1.insert(2, rid_2);
//	leaf_1.insert(4, rid_4);
	leaf_1.insert(3, rid_3);
	leaf_1.insert(1, rid_1);

	leaf_1.locate(1, a);
	leaf_1.locate(2, b);
	leaf_1.locate(3, c);
//	leaf_1.locate(4, d);

//	fprintf(stderr, "a = %i, b = %i, c = %i, d = %i\n", a, b, c, d);
	leaf_1.readEntry(1 - 1, key, rid_get);
	fprintf(stderr, "eid = %i, key = %i, rid.pid = %i, rid.sid = %i\n", 1, key, rid_get.pid, rid_get.sid);
	leaf_1.readEntry(2 - 1, key, rid_get);
	fprintf(stderr, "eid = %i, key = %i, rid.pid = %i, rid.sid = %i\n", 2, key, rid_get.pid, rid_get.sid);
	leaf_1.readEntry(3 - 1, key, rid_get);
	fprintf(stderr, "eid = %i, key = %i, rid.pid = %i, rid.sid = %i\n", 3, key, rid_get.pid, rid_get.sid);
//	leaf_1.readEntry(4 - 1, key, rid_get);
//	fprintf(stderr, "eid = %i, key = %i, rid.pid = %i, rid.sid = %i\n", 4, key, rid_get.pid, rid_get.sid);

	leaf_1.insertAndSplit(5, rid_5, leaf_2, d);

	BTLeafNode leaf_3;
	printf("%i\n", leaf_3.getKeyCount());

	for(int i = 1; i < 86; i++) {
		RecordId rid_temp;
		rid_temp.pid = i*2-1;
		rid_temp.sid = i*2;
		if(leaf_3.insert(i,rid_temp) != 0) {
			if(SHOW_ERRORS)
				printf("insert on full node: %i\n", leaf_3.insert(i,rid_temp));
			break;
		}
	}

	BTLeafNode leaf_4;

	if(SHOW_ERRORS) {
		printf("insertAndSplit with non-empty sibling: %i\n", leaf_3.insertAndSplit(5, rid_5, leaf_1, e));
		printf("locate with large searchKey: %i\n", leaf_3.locate(9999,e));
		printf("locate in empty leaf: %i\n", leaf_4.locate(0, e));
		//printf("%i\n", leaf_3.readEntry(0, key, rid_get));
		//printf("%i\n", leaf_3.getKeyCount());
		printf("readEntry with out of bounds eid (-1): %i\n", leaf_3.readEntry(-1, key, rid_get));
		printf("readEntry with out of bounds eid (9999): %i\n", leaf_3.readEntry(9999, key, rid_get));
		//printf("setNextNodePtr on invalid pid: %i\n", leaf_3.setNextNodePtr(99999999));
	}

	//	open the page file

	opened = p.open("a.txt", 'w');

	if(!opened)
	{

		//	store the leaf_1

		if(leaf_1.write(0, p))
		{
			fprintf(stderr, "Bad write\n");
			p.close();
			free(buffer);
			return -1;
		}

//		if(!leaf_1.read(0, p) && !p.read(0, buffer))
		if(!p.read(0, buffer))
		{
			fprintf(stderr, "The buffer for page %i is: ", 0);

			leaf_1.printItems();
		}
		else
			fprintf(stderr, "Bad read\n");

		//	store the leaf_2

		if(leaf_2.write(1, p))
		{
			fprintf(stderr, "Bad write\n");
			p.close();
			free(buffer);
			return -1;
		}

//		if(!leaf_2.read(1, p) && !p.read(1, buffer))
		if(!p.read(1, buffer))
		{
			fprintf(stderr, "The buffer for page %i is: ", 1);

			leaf_2.printItems();
		}
		else
			fprintf(stderr, "Bad read\n");

		if(leaf_3.write(2, p))
		{
			fprintf(stderr, "Bad write\n");
			p.close();
			free(buffer);
			return -1;
		}

		if(!p.read(2, buffer))
		{
			fprintf(stderr, "The buffer for page %i is: ",2);

			leaf_3.printItems();
		}
		else
			fprintf(stderr, "Bad read\n");

		p.close();
	}
	else
		fprintf(stderr, "Cannot open file\n");

	free(buffer);
	return 0;
}

int main()
{
	return test_nonleaf();
//	return test_leaf();
}

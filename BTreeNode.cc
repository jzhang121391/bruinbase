#include "BTreeNode.h"

using namespace std;

/*
	retrieve hex string from buffer and convert to integer
*/
int getInteger(unsigned char* buffer)
{

	//	get the integer

	return ((buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3]);
}

/*
	convert integer into 4-digit hex string and insert into buffer
*/
int setInteger(unsigned char* buffer, int integer)
{

	//	store the integer

	buffer[0] = (integer >> 24) & 0xFF;
	buffer[1] = (integer >> 16) & 0xFF;
	buffer[2] = (integer >> 8) & 0XFF;
	buffer[3] = (integer & 0XFF);

	return 0;
}

//	leaf constructor
BTLeafNode::BTLeafNode()
{
	num_keys = 0;
	PageId sibling = -1;
	next_leaf = sibling;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{
	return (pf.read(pid, (void*) buffer));
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{
	return (pf.write(pid, (void*) buffer));
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	return num_keys;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	// max keys/entries is PAGE_SIZE - left-most pointer, divided by size of entries

	//printf("%i\n", num_keys);

    //if(num_keys >= (PageFile::PAGE_SIZE-sizeof(PageId))/(sizeof(RecordId)+sizeof(int)));
    //	return RC_NODE_FULL;	

	int n_keys = 0;
	int next_key = 0;
	unsigned char *buffer_insert = (unsigned char*) malloc(3 * sizeof(int));
	unsigned char *temp_buffer = NULL;

	//	rid.pid rid.sid key

	setInteger(buffer_insert, rid.pid);
	setInteger(buffer_insert + sizeof(int), rid.sid);
	setInteger(buffer_insert + 2 * sizeof(int), key);

	while(n_keys < num_keys)
	{
		next_key = getInteger(buffer + (3 * n_keys + 2) * sizeof(int));

		//	check if the pair should be inserted here

		if(key <= next_key)
		{

			//	copy the second half of the buffer into a temporary buffer

			temp_buffer = (unsigned char*) malloc(3 * (num_keys - n_keys) * sizeof(int));
			memcpy(temp_buffer, buffer + 3 * n_keys * sizeof(int), 3 * (num_keys - n_keys) * sizeof(int));

			//	append the pair to the first half of the buffer

			memcpy(buffer + (3 * n_keys) * sizeof(int), buffer_insert, 3 * sizeof(int));

			//	append the second half of the buffer to the result

			memcpy(buffer + (3 * n_keys) * sizeof(int) + 3 * sizeof(int), temp_buffer, (3 * (num_keys - n_keys)) * sizeof(int));

			//	update members

			num_keys++;

			//	free arrays

			free(buffer_insert);
			free(temp_buffer);

			return 0;
		}

		//	proceed to parse the next key and id

		n_keys++;
	}

	//	end of the buffer

	//	append pair to the buffer

	memcpy(buffer + 3 * n_keys * sizeof(int), buffer_insert, 3 * sizeof(int));

	//	update members

	num_keys++;

	//	free arrays

	free(buffer_insert);

	return 0;
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{

	//	check if the sibling is empty

	if(sibling.num_keys || !num_keys)
		return RC_INVALID_ATTRIBUTE;

	unsigned char *temp_buffer_total = (unsigned char*) malloc(PageFile::PAGE_SIZE + 3 * sizeof(int));
	memcpy(temp_buffer_total, buffer, PageFile::PAGE_SIZE);

	int half = -1;
	int half_num_keys = ((num_keys + 1) / 2) + ((num_keys + 1) % 2);

	int n_keys = 0;
	int next_key = 0;
	unsigned char *buffer_insert = (unsigned char*) malloc(3 * sizeof(int));
	unsigned char *temp_buffer = NULL;

	//	rid.pid rid.sid key

	setInteger(buffer_insert, rid.pid);
	setInteger(buffer_insert + sizeof(int), rid.sid);
	setInteger(buffer_insert + 2 * sizeof(int), key);

	while(n_keys < num_keys)
	{
		next_key = getInteger(buffer + (3 * n_keys + 2) * sizeof(int));

		//	check if the pair should be inserted here

		if(key <= next_key)
		{

			//	copy the second half of the buffer into a temporary buffer

			temp_buffer = (unsigned char*) malloc(3 * (num_keys - n_keys) * sizeof(int));
			memcpy(temp_buffer, buffer + 3 * n_keys * sizeof(int), 3 * (num_keys - n_keys) * sizeof(int));

			//	append the pair to the first half of the buffer

			memcpy(temp_buffer_total + (3 * n_keys) * sizeof(int), buffer_insert, 3 * sizeof(int));

			//	append the second half of the buffer to the result

			memcpy(temp_buffer_total + (3 * n_keys) * sizeof(int) + 3 * sizeof(int), temp_buffer, (3 * (num_keys - n_keys)) * sizeof(int));

			//	update members

			num_keys++;

			//	free arrays

			free(buffer_insert);
			free(temp_buffer);
		}

		//	proceed to parse the next key and id

		n_keys++;
	}

	//	end of the buffer

	if(n_keys == num_keys)
	{

		//	append pair to the buffer

		memcpy(temp_buffer_total + 3 * n_keys * sizeof(int), buffer_insert, 3 * sizeof(int));

		//	update members

		num_keys++;

		//	free arrays

		free(buffer_insert);
	}

	//	copy the first half of the buffer into the nonleaf

	memcpy(buffer, temp_buffer_total, PageFile::PAGE_SIZE);

	//	copy the second half of the buffer into the sibling's buffer

	memcpy(sibling.buffer, temp_buffer_total + (3 * half_num_keys) * sizeof(int), (3 * (num_keys - half_num_keys)) * sizeof(int));

	//	get first key of sibling

	siblingKey = getInteger(sibling.buffer + 2 * sizeof(int));

	//	update members

	sibling.num_keys = num_keys - half_num_keys;
	num_keys = half_num_keys;

	//	free buffers

	free(temp_buffer_total);

	return 0;
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{

	/*
		- repeatedly parse the file by looking for id key
		- get key
		- if searchKey is less than or equal to key, get the entry number of the key; otherwise keep going
		- if we arrive at the end (i.e., key>pid), we fail to find it
	*/

	int n_keys = 0;
	int key = 0;
	int entry = 0;

	while(n_keys < num_keys)
	{
		key = getInteger(buffer + (n_keys * 3 + 2) * sizeof(int));

		if(searchKey <= key)
		{
			eid = n_keys;
			return 0;
		}

		n_keys++;
	}

	//	end of the buffer

	return RC_NO_SUCH_RECORD;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{

	//	check if the entry exists

	if(eid > num_keys || eid < 0)
		return RC_INVALID_CURSOR;

	rid.pid = getInteger(buffer + (eid * 3) * sizeof(int));
	rid.sid = getInteger(buffer + (eid * 3 + 1) * sizeof(int));
	key = getInteger(buffer + (eid * 3 + 2) * sizeof(int));

	return 0;
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{
	return next_leaf;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	//if(/*some check*/)
	//	return RC_INVALID_PID;

	next_leaf = pid;
	return 0;
}

/*
	test function used to print keys and id's
*/
void BTLeafNode::printItems()
{
	int n_keys = 0;

	//	print in the format rid.pid,rid.sid<key>rid.pid,rid.sid<key>...

	while(n_keys < num_keys)
	{

		fprintf(stderr, "%i,", getInteger(buffer + (3 * n_keys) * sizeof(int)));
		fprintf(stderr, "%i", getInteger(buffer + (3 * n_keys + 1) * sizeof(int)));
		fprintf(stderr, "<%i>", getInteger(buffer + (3 * n_keys + 2) * sizeof(int)));

		n_keys++;
	}

	fprintf(stderr, "\n");
}

//	nonleaf constructor
BTNonLeafNode::BTNonLeafNode()
{
	num_keys = 0;
	root = 0;
	left = 0;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
	return (pf.read(pid, (void*) buffer));
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
	return (pf.write(pid, (void*) buffer));
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	return num_keys;
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{

	//	check if the node is a full

	if((root && num_keys == 2) || (num_keys >= (PageFile::PAGE_SIZE-sizeof(PageId))/(sizeof(PageId)+sizeof(int))))
		return RC_NODE_FULL;

	int n_keys = 0;
	int next_key = 0;
	unsigned char *buffer_insert = (unsigned char*) malloc(2 * sizeof(int));
	unsigned char *temp_buffer = NULL;

	//	key pid

	setInteger(buffer_insert, key);
	setInteger(buffer_insert + sizeof(int), pid);

	while(n_keys < num_keys)
	{
		next_key = getInteger(buffer + (left + 2 * n_keys) * sizeof(int));

		//	check if the pair should be inserted here

		if(key < next_key)
		{

			//	copy the second half of the buffer into a temporary buffer

			temp_buffer = (unsigned char*) malloc((left + 2 * (num_keys - n_keys)) * sizeof(int));
			memcpy(temp_buffer, buffer + (left + 2 * n_keys) * sizeof(int), (left + 2 * (num_keys - n_keys)) * sizeof(int));

			//	append the pair to the first half of the buffer

			memcpy(buffer + (left + 2 * n_keys) * sizeof(int), buffer_insert, 2 * sizeof(int));

			//	append the second half of the buffer to the result

			memcpy(buffer + (left + 2 * n_keys) * sizeof(int) + 2 * sizeof(int), temp_buffer, (left + 2 * (num_keys - n_keys)) * sizeof(int));

			//	update members

			num_keys++;

			//	free arrays

			free(buffer_insert);
			free(temp_buffer);

			return 0;
		}

		//	proceed to parse the next key and id

		n_keys++;
	}

	//	end of the buffer

	//	append pair to the buffer

	memcpy(buffer + (left + 2 * n_keys) * sizeof(int), buffer_insert, 2 * sizeof(int));

	//	update members

	num_keys++;

	//	free arrays

	free(buffer_insert);

	return 0;
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{

	//	check if the sibling is empty

	if(sibling.num_keys || !num_keys)
		return RC_INVALID_ATTRIBUTE;

	int place = 0;
	int next_key = 0;
	unsigned char *buffer_insert = (unsigned char*) malloc(2 * sizeof(int));
	unsigned char *temp_buffer = NULL;
	unsigned char *temp_buffer_total = (unsigned char*) malloc(PageFile::PAGE_SIZE + 2 * sizeof(int));
	memcpy(temp_buffer_total, buffer, PageFile::PAGE_SIZE);

	int n_keys = 0;
	int extra = 0;
	int half_num_keys = ((num_keys + 1) / 2);

	if(half_num_keys == 1)
		extra = 0;
	else
		extra = 1;

	//	key pid

	setInteger(buffer_insert, key);
	setInteger(buffer_insert + sizeof(int), pid);

	while(n_keys < num_keys)
	{
		next_key = getInteger(temp_buffer_total + (left + 2 * n_keys) * sizeof(int));

		//	check if the pair should be inserted here

		if(key < next_key)
		{

			//	copy the second half of the buffer into a temporary buffer

			temp_buffer = (unsigned char*) malloc((left + 2 * (num_keys - n_keys)) * sizeof(int));
			memcpy(temp_buffer, temp_buffer_total + (left + 2 * n_keys) * sizeof(int), (left + 2 * (num_keys - n_keys)) * sizeof(int));

			//	append the pair to the first half of the buffer

			memcpy(temp_buffer_total + (left + 2 * n_keys) * sizeof(int), buffer_insert, 2 * sizeof(int));

			//	append the second half of the buffer to the result

			memcpy(temp_buffer_total + (left + 2 * n_keys) * sizeof(int) + 2 * sizeof(int), temp_buffer, (left + 2 * (num_keys - n_keys)) * sizeof(int));

			//	update members

			num_keys++;

			//	free arrays

			free(buffer_insert);
			free(temp_buffer);

			break;
		}

		//	proceed to parse the next key and id

		n_keys++;
	}

	//	end of the buffer

	if(n_keys == num_keys)
	{

		//	append pair to the buffer

		memcpy(temp_buffer_total + (left + 2 * n_keys) * sizeof(int), buffer_insert, 2 * sizeof(int));

		//	free arrays

		free(buffer_insert);
	}

	//	copy the first half of the buffer into the nonleaf

	memcpy(buffer, temp_buffer_total, PageFile::PAGE_SIZE);

	//	copy the second half of the buffer into the sibling's buffer

	memcpy(sibling.buffer, temp_buffer_total + (left + 2 * half_num_keys + 1) * sizeof(int), (2 * (num_keys - half_num_keys - extra) + 1) * sizeof(int));

	//	set midKey

	midKey = getInteger(buffer + (left + 2 * half_num_keys) * sizeof(int));

	//	update members

	sibling.num_keys = num_keys - half_num_keys - extra;
	num_keys = half_num_keys;

	sibling.left = 1;

	//	free buffers

	free(temp_buffer_total);

	return 0;
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{

	/*
		- repeatedly get id and key
		- if searchKey is less than or equal to key, set pid to id; otherwise keep going
		- if we arrive at the end, get id and set pid to id
	*/

	int i = 0;
	int n_keys = 0;
	int key = 0;
	int pid_marker = 0;

	while(n_keys < num_keys)
	{

		key = getInteger(buffer + (left + 2 * n_keys) * sizeof(int));

		if(searchKey < key)
		{

			//	check if the pointer exists

			if(!n_keys && !left)
				return RC_NO_SUCH_RECORD;

			pid = getInteger(buffer + (left + 2 * n_keys - 1) * sizeof(int));

			return 0;
		}

		n_keys++;
	}

	//	end of the buffer

	pid = getInteger(buffer + (left + 2 * n_keys - 1) * sizeof(int));

	return 0;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{

	//	check if the node is empty

	if(num_keys)
		return RC_INVALID_ATTRIBUTE;

	//	insert the values into the buffer

	setInteger(buffer, pid1);
	setInteger(buffer + sizeof(int), key);
	setInteger(buffer + 2*sizeof(int), pid2);

	//	update members

	root = 1;
	left = 1;
	num_keys = 1;

	return 0;
}

/*
	test function used to print keys and id's
*/
void BTNonLeafNode::printItems()
{
	int i = 0;

	//	print in the format id<key>id<key>...

	while(i < num_keys)
	{
		if(!i && left)
			fprintf(stderr, "%i", getInteger(buffer));

		fprintf(stderr, "<%i>", getInteger(buffer + (left + 2 * i) * sizeof(int)));
		fprintf(stderr, "%i", getInteger(buffer + (left + 2 * i + 1) * sizeof(int)));

		i++;
	}

	fprintf(stderr, "\n");
}

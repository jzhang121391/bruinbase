Alexander Lin
203882489
alexander.c.lin@gmail.com

Partner

Jason Zhang
503888927
jzhang121391@yahoo.com

Notes:

	Extra functions:
		- setInteger: convert integer into 4-digit hex string and insert into buffer
		- getInteger: retrieve hex string from buffer and convert to integer
		- printItems (BTLeafNode and BTNonLeafNode): print keys and id's in the nodes

	Nonleaf nodes:

		- Extra members:
			- num_keys indicates the number of keys in the node
			- root_key indicates the root of the node
			- constructor initializes these members
			- printItems prints the items in the node in the format <key>id<key>id...

	Leaf nodes
		- Extra members:
			- num_keys indicates the number of keys in the node
			- next_leaf indicates the node's sibling
			- constructor initializes these members
			- printItems prints the items in the node in the format rid.pid,rid.sid<key>rid.pid,rid.sid<key>...

	buffer:
		- to store the keys and id's as integers, the buffer had to be changed into unsigned char*
		- this type was used to store the integers as 4-byte hex strings to ensure that each integer only uses 4 bytes

	insert and split:
		if the number of keys results in an odd number, the first node will have one more key than the sibling
		Example: (key,key,key) -> (key,key) (key)

Teamwork

	- Alexander Lin
		- setInteger
		- getInteger
		BTLeafNode:
			- BTLeafNode (constructor)
			- insert
			- insertAndSplit
			- locate
			- readEntry
			- getNextNodePtr
			- setNextNodePtr
			- getKeyCount
			- read
			- write
			- printItems
		BTNonLeafNode:
			- BTNonLeafNode (constructor)
			- insert
			- insertAndSplit
			- locateChildPtr
			- initializeRoot
			- getKeyCount
			- read
			- write
			- printItems

	- For team collaboration, it would be helpful if we set up a repository on Github as soon as possible for quick access to updated files

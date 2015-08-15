
This assignment is made by Utsav Sinha, 12775

To see the conclusions, please see conclusions.txt

Run the bPlusTree.cpp using

g++ bPlusTree.cpp -o bPlusTree

PRE_PROCESSING stage:
	The program checks the existence of root node and creates one if no root exists.
	It also reads bplustree.config to get MAX_NUM_KEYS.

	Root node has MAX_NUM_KEYS appended to its name.
	If value of MAX_NUM_KEYS is changed in bplustree.config after a database has already been
	indexed, then the program reads the new value from bplustree.config and rebuilds the tree
	using this new MAX_NUM_KEYS

	But before rebuilding the tree, the program selects one possible root node from the current 
	directory (which has file name having ROOT_NODE_PATTERN) and uses that tree to build a data dump
	of the previously created database into bplus_dump_file.txt

	Then it builds the tree using that file as its data.


EXECUTION stage:
There are 3 possible commands which can be executed by entering the inputs:

1
	This deletes all existing root, leaf and internal nodes of the tree. Then it builds the tree from 
	scratch using the file BPLUS_TREE_DUMP_FILE (which is by default set to assgn2_bplus_data.txt) 

	All statistics and intermediate results can be found in build_results.txt and build_stats.txt

2
	This takes QUERY_FILE (which is by default set to querysample.txt) and performs those queries on 
	the existing database (bplus-tree). 
	If no bplus-tree exists prior to this (that is command 1 has not been executed), then it creates a new bplus-tree and performs these queries on it. 

	All statistics and intermediate results are stored in query_results.txt and query_stats.txt

3
	This enters into a query loop. The queries have similar format as in querysample.txt i.e.

	Operation  		Code  		Details   
	Insertion  		0  			Point   
	Point Query  	1  			Point to Search   
	Range query  	2  			Query center  		Range 

	To exit the query loop, enter -1

	All statistics and intermediate results are printed on the terminal itself.


STRUCTURE of B-PLUS TREE

The nodes are distinguished into internal nodes and leaf nodes. Leaf nodes are stored in
files having names leaf<number>.txt and internal nodes (that do not store data but only keys)
are stored in files internal<number>.txt

These numbers are generated sequentially.

root node is always stored in root<number>.txt where number is MAX_NUM_KEYS as said earlier

Every node has the general structure:

Line 1>Type of Node
Line 2>Number of Keys stored in this node
Line 3>A node pointer (name of a file)

Line 4 onwards> <Key> <data> pairs

Type of node can be either leaf or internal
Number of keys is always <= MAX_NUM_KEYS and >= MAX_NUM_KEYS/2 (except for the root which can have less than MAX_NUM_KEYS/2)

The node pointer in third line carries seperate meaning for internal and leaf nodes:

Internal node - It points to the file that have keys less than the lowest key in this node
Leaf node 	  - It points to the next file in sequential order of key magnitude (maintaining the the linked list of leaf nodes)

Line 4 onwards contain key data pairs separated by \t
<this-line-key> <this-line-data>
The keys are floating point keys which are used for indexing while the data has separate meanings for internal and leaf nodes

Internal node - It points to the file that has keys of magnitude greater than equal to <this-line-key>
Leaf node 	  - It stores the dtring data corresponding to <this-line-key>

CONCLUSIONS

Please see conclusions.txt for inferences drawn from the statistics

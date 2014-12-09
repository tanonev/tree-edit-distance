tree-edit-distance
==================

To run the code on the sample data, do the following:

    make
    bin/matching ast_0.json ast_1.json octave_builtins.txt

The map will be written to standard out. The first line contains two numbers: the edit distance d and the number of matched nodes n. Then follow n lines with the node ids of the matched nodes.


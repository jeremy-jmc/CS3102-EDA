btree:
	g++ btree.cpp -std=c++17 -o main && main
bptree:
	g++ b+tree.cpp -std=c++17 -o main && main
clean:
	del *.exe
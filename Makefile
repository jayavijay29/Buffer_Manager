all:output

output:dberror.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o test_assign2_1.o 
	gcc dberror.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o test_assign2_1.o -o output

dberror.o: dberror.c
	gcc -c dberror.c

storage_mgr.o: storage_mgr.c
	gcc -c storage_mgr.c

buffer_mgr_stat.o: buffer_mgr_stat.c
	gcc -c buffer_mgr_stat.c

buffer_mgr.o: buffer_mgr.c
	gcc -c buffer_mgr.c

test_assign2_1.o: test_assign2_1.c
	gcc -c test_assign2_1.c

	
clean: 
	rm -rf *o output
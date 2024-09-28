CC=gcc
SRC= dberror.c storage_mgr.c buffer_mgr.c buffer_mgr_stat.c test_assign2_1.c 
OFILES= dberror.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o test_assign2_1.o 

# SRC= dberror.c storage_mgr.c buffer_mgr.c buffer_mgr_stat.c test_assign2_2.c 
# OFILES= dberror.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o test_assign2_2.o 

assignment2: $(OFILES)
		$(CC) -o test_assign2 $(OFILES)

$(OFILES): $(SRC)
		$(CC) -g -c $(SRC)

run: assignment2
		./test_assign2

clean:
		rm -rf test_assign2 *.o
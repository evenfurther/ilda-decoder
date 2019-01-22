all:: test_ilda display_ilda max_records

display_ilda: display_ilda.o ilda_decoder.o
	$(CC) -o display_ilda $^ -lSDL2

test_ilda: test_ilda.o ilda_decoder.o
	$(CC) -o test_ilda $^

max_records: max_records.o ilda_decoder.o
	$(CC) -o max_records $^

ilda_decoder.o: ilda_decoder.h

clean:
	$(RM) *.o test_ilda display_ilda max_records

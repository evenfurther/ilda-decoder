test_ilda: test_ilda.o ilda_decoder.o
	$(CC) -o test_ilda $^

ilda_decoder.o: ilda_decoder.h

clean:
	$(RM) *.o test_ilda

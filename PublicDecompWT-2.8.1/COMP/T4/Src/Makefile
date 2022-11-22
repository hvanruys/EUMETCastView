LIB_OBJ= CBitBuffer.o CompressT4.o T4Coder.o T4Codes.o T4Decoder.o

CPPFLAGS+= -I../Inc  -I../../Inc -I../../../DISE

all: libT4.a

clean:
	rm -f *.o
	rm -f libT4.a

libT4.a: $(LIB_OBJ)
	ar r libT4.a $(LIB_OBJ)
	ranlib libT4.a
	@touch libT4.a

.cpp.o:
	g++ $(CPPFLAGS) -c -m$(BITS) $<

depend:
	makedepend -- $(CPPFLAGS) -- $(MAKEDEPFLAGS) -- $(SOURCES)

LIB_OBJ= CxRITFile.o CxRITFileHeaderRecords.o CxRITAnnotation.o ErrorHandling.o TimeSpan.o UTCTime.o

CPPFLAGS+= -I. 

all: libDISE.a

clean:
	rm -f *.o
	rm -f libDISE.a

libDISE.a: $(LIB_OBJ)
	ar r libDISE.a $(LIB_OBJ)
	ranlib libDISE.a
	@touch libDISE.a

.cpp.o:
	g++ $(CPPFLAGS) -c -m$(BITS) $<

depend:
	makedepend -- $(CPPFLAGS) -- $(MAKEDEPFLAGS) -- $(SOURCES)

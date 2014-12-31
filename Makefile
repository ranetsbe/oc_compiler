# $Id: Makefile,v 1.11 2013-10-15 16:37:56-07 - - $

MKFILE    = Makefile
DEPSFILE  = ${MKFILE}.deps
NOINCLUDE = ci clean spotless
NEEDINCL  = ${filter ${NOINCLUDE}, ${MAKECMDGOALS}}
VALGRIND  = valgrind --leak-check=full --show-reachable=yes

# Definitions of list of files:
HSOURCES  = ast.h lyutils.h auxlib.h stringset.h oc.h symtable.h ralib.h astutils.h
CSOURCES  = ast.cc lyutils.cc auxlib.cc stringset.cc oc.cc symtable.cc ralib.cc astutils.cc
LSOURCES  = scanner.l
YSOURCES  = parser.y
ETCSRC    = README ${MKFILE} ${DEPSFILE}
CLGEN     = yylex.cc
HYGEN     = yyparse.h
CYGEN     = yyparse.cc
CGENS     = ${CLGEN} ${CYGEN}
ALLGENS   = ${HYGEN} ${CGENS}
EXECBIN   = oc
ALLCSRC   = ${CSOURCES} ${CGENS}
OBJECTS   = ${ALLCSRC:.cc=.o}
LREPORT   = yylex.output
YREPORT   = yyparse.output
IREPORT   = ident.output
REPORTS   = ${LREPORT} ${YREPORT} ${IREPORT}
ALLSRC    = ${ETCSRC} ${YSOURCES} ${LSOURCES} ${HSOURCES} ${CSOURCES}
LISTSRC   = ${ALLSRC} ${HYGEN}

# Definitions of the compiler and compilation options:
GCC       = g++ -O0 -g -Wall -Wextra -std=gnu++0x
MKDEPS    = g++ -MM -std=gnu++0x

# The first target is always ``all'', and hence the default,
# and builds the executable images
all : ${EXECBIN}

# Build the executable image from the object files.
${EXECBIN} : ${OBJECTS}
	${GCC} -o${EXECBIN} ${OBJECTS}

# Build an object file form a C source file.
%.o : %.cc
	${GCC} -c $<

# Build the scanner.
${CLGEN} : ${LSOURCES}
	flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}
	- grep -v '^  ' ${LREPORT}

# Build the parser.
${CYGEN} ${HYGEN} : ${YSOURCES}
	bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

# Check sources into an RCS subdirectory.
ci :
	ci -u ${CSOURCES} ${HSOURCES} ${LSOURCES} ${YSOURCES}

co :
	co -l ${CSOURCES} ${HSOURCES} ${LSOURCES} ${YSOURCES}


# Make a listing from all of the sources
lis : ${LISTSRC} tests
	mkpspdf List.source.ps ${LISTSRC}
	mkpspdf List.output.ps ${REPORTS} \
		${foreach test, ${TESTINS:.in=}, \
		${patsubst %, ${test}.%, in out err}}

# Clean and spotless remove generated files.
clean :
	- rm -f ${OBJECTS} ${ALLGENS} ${REPORTS} ${DEPSFILE} *~

spotless : clean
	- rm -f ${EXECBIN}

# Build the dependencies file using the C preprocessor
deps : ${ALLCSRC}
	@ echo "# ${DEPSFILE} created `date` by ${MAKE}" >${DEPSFILE}
	${MKDEPS} ${ALLCSRC} >>${DEPSFILE}

${DEPSFILE} :
	@ touch ${DEPSFILE}
	${MAKE} --no-print-directory deps

# Everything
again :
	gmake --no-print-directory spotless deps ci all lis
	
ifeq "${NEEDINCL}" ""
include ${DEPSFILE}
endif


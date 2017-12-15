#include <isa/riscvISA.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <map>
#include <portability.h>

std::map<CORE_INT(16), FILE*> fileMap;
FILE **inStreams, **outStreams;
int nbInStreams, nbOutStreams;

void stb(CORE_UINT(32) addr, CORE_INT(8) value){
}

void sth(CORE_UINT(32) addr, CORE_INT(16) value){
}

void stw(CORE_UINT(32) addr, CORE_INT(32) value){
	stb(addr+3, value.SLC(8,24));
	stb(addr+2, value.SLC(8,16));
	stb(addr+1, value.SLC(8,8));
	stb(addr+0, value.SLC(8,0));
}

void _std(CORE_UINT(32) addr, CORE_INT(32) value){
	stb(addr+7, value.SLC(8,56));
	stb(addr+6, value.SLC(8,48));
	stb(addr+5, value.SLC(8,40));
	stb(addr+4, value.SLC(8,32));
	stb(addr+3, value.SLC(8,24));
	stb(addr+2, value.SLC(8,16));
	stb(addr+1, value.SLC(8,8));
	stb(addr+0, value.SLC(8,0));
}

CORE_INT(8) ldb(CORE_UINT(32) addr){
	CORE_INT(8) result = 0;
	return result;
}

CORE_INT(16) ldh(CORE_UINT(32) addr){
	CORE_INT(16) result = 0;
	result.SET_SLC(8, ldb(addr+1));
	result.SET_SLC(0, ldb(addr));
	return result;
}

CORE_INT(32) ldw(CORE_UINT(32) addr){
	CORE_INT(32) result = 0;
	result.SET_SLC(24, ldb(addr+3));
	result.SET_SLC(16, ldb(addr+2));
	result.SET_SLC(8, ldb(addr+1));
	result.SET_SLC(0, ldb(addr));
	return result;
}

CORE_INT(32) ldd(CORE_UINT(32) addr){
	CORE_INT(32) result = 0;
	result.SET_SLC(56, ldb(addr+7));
	result.SET_SLC(48, ldb(addr+6));
	result.SET_SLC(40, ldb(addr+5));
	result.SET_SLC(32, ldb(addr+4));
	result.SET_SLC(24, ldb(addr+3));
	result.SET_SLC(16, ldb(addr+2));
	result.SET_SLC(8, ldb(addr+1));
	result.SET_SLC(0, ldb(addr));
	return result;
}

CORE_UINT(32) doRead(CORE_UINT(32) file, CORE_UINT(32) bufferAddr, CORE_UINT(32) size){
	//printf("Doign read on file %x\n", file);
	int localSize = size.SLC(32,0);
	char* localBuffer = (char*) malloc(localSize*sizeof(char));
	CORE_UINT(32) result;
	if (file == 0){
		if (nbInStreams == 1)
			result = fread(localBuffer, 1, size, inStreams[0]);
		else
			result = fread(localBuffer, 1, size, stdin);
	}
	else{
		FILE* localFile = fileMap[file.SLC(16,0)];
		result = fread(localBuffer, 1, size, localFile);
		if (localFile == 0)
			return -1;
	}
	for (int i=0; i<result; i++){
		stb(bufferAddr + i, localBuffer[i]);
	}
	return result;
}

CORE_UINT(32) doWrite(CORE_UINT(32) file, CORE_UINT(32) bufferAddr, CORE_UINT(32) size){
	int localSize = size.SLC(32,0);
	char* localBuffer = (char*) malloc(localSize*sizeof(char));
	for (int i=0; i<size; i++)
		localBuffer[i] = ldb(bufferAddr + i);
	if (file < 5){
		CORE_UINT(32) result = 0;
		int streamNB = (int) file-nbInStreams;
		if (nbOutStreams + nbInStreams > file)
			result = fwrite(localBuffer, 1, size, outStreams[streamNB]);
		else
			result = fwrite(localBuffer, 1, size, stdout);
		return result;
	}
	else{
		FILE* localFile = fileMap[file.SLC(16,0)];
		if (localFile == 0)
			return -1;
		CORE_UINT(32) result = fwrite(localBuffer, 1, size, localFile);
		return result;
	}
}

CORE_UINT(32) doOpen(CORE_UINT(32) path, CORE_UINT(32) flags, CORE_UINT(32) mode){
	int oneStringElement = ldb(path);
	int index = 0;
	while (oneStringElement != 0){
		index++;
		oneStringElement = ldb(path+index);
	}
	int pathSize = index+1;
	char* localPath = (char*) malloc(pathSize*sizeof(char));
	for (int i=0; i<pathSize; i++)
		localPath[i] = ldb(path + i);
	char* localMode;
	if (flags==0)
		localMode = "r";
	else if (flags == 577)
		localMode = "w";
	else if (flags == 1089)
		localMode = "a";
	else if (flags == O_WRONLY|O_CREAT|O_EXCL)
		localMode = "wx";
	else{
		fprintf(stderr, "Trying to open files with unknown flags... \n");
		exit(-1);
	}
	FILE* test = fopen(localPath, localMode);
	int result; //= (int) test;
	CORE_INT(32) result_ac = result;
	//For some reasons, newlib only store last 16 bits of this pointer, we will then compute a hash and return that.
	//The real pointer is stored here in a hashmap
	CORE_INT(32) returnedResult = 0;
	returnedResult.SET_SLC(0, result_ac.SLC(15,0) ^ result_ac.SLC(15,16));
	returnedResult[15] = 0;
	fileMap[returnedResult.SLC(16,0)] = test;
	return returnedResult;
}

CORE_UINT(32) doOpenat(CORE_UINT(32) dir, CORE_UINT(32) path, CORE_UINT(32) flags, CORE_UINT(32) mode){
	fprintf(stderr, "Syscall openat not implemented yet...\n");
	exit(-1);
}

CORE_UINT(32) doClose(CORE_UINT(32) file){
	if (file > 2 ){
		FILE* localFile = fileMap[file.SLC(16,0)];
		int result = fclose(localFile);
		return result;
	}
	else
		return 0;
}

CORE_INT(32) doLseek(CORE_UINT(32) file, CORE_UINT(32) ptr, CORE_UINT(32) dir){
	if (file>2){
		FILE* localFile = fileMap[file.SLC(16,0)];
		if (localFile == 0)
			return -1;
		int result = fseek(localFile, ptr, dir);
		return result;
	}
	else
		return 0;
}

CORE_UINT(32) doStat(CORE_UINT(32) filename, CORE_UINT(32) ptr){
	int oneStringElement = ldb(filename);
	int index = 0;
	while (oneStringElement != 0){
		index++;
		oneStringElement = ldb(filename+index);
	}
	int pathSize = index+1;
	char* localPath = (char*) malloc(pathSize*sizeof(char));
	for (int i=0; i<pathSize; i++)
		localPath[i] = ldb(filename + i);
	struct stat fileStat;
	int result = stat(localPath, &fileStat);
	//We copy the result in simulator memory
	for (int oneChar = 0; oneChar<sizeof(struct stat); oneChar++)
		stb(ptr+oneChar, ((char*)(&stat))[oneChar]);
	return result;
}

CORE_UINT(32) doSbrk(CORE_UINT(32) value){
	return 0;
}

CORE_UINT(32) doGettimeofday(CORE_UINT(32) timeValPtr){
	timeval* oneTimeVal;
	struct timezone* oneTimeZone;
	int result = gettimeofday(oneTimeVal, oneTimeZone);
	return result;
}

CORE_UINT(32) doUnlink(CORE_UINT(32) path){
	int oneStringElement = ldb(path);
	int index = 0;
	while (oneStringElement != 0){
		index++;
		oneStringElement = ldb(path+index);
	}
	int pathSize = index+1;
	char* localPath = (char*) malloc(pathSize*sizeof(char));
	for (int i=0; i<pathSize; i++)
		localPath[i] = ldb(path + i);
	int result = unlink(localPath);
	return result;
}

CORE_UINT(32) solveSysCall(CORE_UINT(32) syscallId, CORE_UINT(32) arg1, CORE_UINT(32) arg2,
 CORE_UINT(32) arg3, CORE_UINT(32) arg4, CORE_UINT(2) *sys_status){
	CORE_UINT(32) result = 0;
	switch (syscallId){
		case SYS_exit:
			*sys_status = 1; //Currently we break on ECALL
			break;
		case SYS_read:
			result = doRead(arg1, arg2, arg3);
			break;
		case SYS_write:
			result = doWrite(arg1, arg2, arg3);
			break;
		case SYS_brk:
			result = doSbrk(arg1);
			break;
		case SYS_open:
			result = doOpen(arg1, arg2, arg3);
			break;
		case SYS_openat:
			result = doOpenat(arg1, arg2, arg3, arg4);
			break;
		case SYS_lseek:
			result = doLseek(arg1, arg2, arg3);
			break;
		case SYS_close:
			result = doClose(arg1);
			break;
		case SYS_fstat:
			result = 0;
			break;
		case SYS_stat:
			result = doStat(arg1, arg2);
			break;
		case SYS_gettimeofday:
			result = doGettimeofday(arg1);
			break;
		case SYS_unlink:
			result = doUnlink(arg1);
			break;
		default:
			*sys_status = 2;
			break;
	}
	return result;
}

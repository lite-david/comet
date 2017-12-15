// vim: set ts=4 nu ai:
#include <cache.h>
#include <assert.h>
#include <iostream>

Cache::Cache(Dram* dram){
	assert(TAGBITS + SETBITS + IDBITS == 32);
	assert(1 << SETBITS == SETS);
	assert(1 << IDBITS == CACHEBLOCKBYTES);
	dram_location = dram;
	n_cache_miss = 0;
	n_load = 0;
	n_store = 0;
	n_dram_writes = 0;
	n_dram_reads = 0;
	int i =0,j=0;
	for(i=0;i<SETS;i++){
		for(j=0;j<CACHEBLOCKBYTES;j++){
			cache[i][j] = 0;
		}
		index[i].tag = 0;
		index[i].dirtybit=0;
		index[i].invalid = 1;
	}
}

void Cache::store(CORE_UINT(32) address, CORE_INT(32) value, CORE_UINT(2) op, CORE_UINT(2)* cache_miss){
	// For store byte, op = 0
	// For store half word, op = 1
	// For store word, op = 3
	
	n_store++;
	CORE_UINT(TAGBITS) tag = getTag(address);
	CORE_UINT(SETBITS) set = getSet(address);
	CORE_UINT(IDBITS) id = getId(address);

	CORE_UINT(8) byte0 = value.SLC(8,0);
	CORE_UINT(8) byte1 = value.SLC(8,8);
	CORE_UINT(8) byte2 = value.SLC(8,16);
	CORE_UINT(8) byte3 = value.SLC(8,24);

	CORE_UINT(1) tag_match = (tag == index[set].tag) ? 1 : 0;
	CORE_UINT(1) dirty = index[set].dirtybit;
	CORE_UINT(32) dram_address = 0;
	CORE_UINT(IDBITS+1) id_counter;

	if(tag_match == 0 || index[set].invalid==1){
		n_cache_miss++;
		n_dram_reads++;
		*cache_miss = 1;
		if(dirty){
			*cache_miss = 2;
			n_dram_writes++;
			dram_address.SET_SLC(IDBITS,set);
			dram_address.SET_SLC(IDBITS+SETBITS,index[set].tag);
			for(id_counter = 0; id_counter<CACHEBLOCKBYTES; id_counter++){
				dram_location->setMemory(dram_address+id_counter, cache[set][id_counter]);
			}
		}
		dram_address.SET_SLC(IDBITS,set);
		dram_address.SET_SLC(IDBITS+SETBITS, tag);
		for(id_counter = 0; id_counter<CACHEBLOCKBYTES; id_counter+=1){
			cache[set][id_counter] = dram_location->getMemory(dram_address+id_counter);
		}	
	}

	cache[set][id] = byte0;
	if(op & 1){
		cache[set][id+1] = byte1;
	}
	if(op & 2){
		cache[set][id+2] = byte2;
		cache[set][id+3] = byte3;
	}
	index[set].tag = tag;
	index[set].dirtybit = 1;
	index[set].invalid = 0;
}

CORE_INT(32) Cache::load(CORE_UINT(32) address, CORE_UINT(2) op, CORE_UINT(1) sign, CORE_UINT(2)* cache_miss){
	// For load byte, op = 0
	// For load half word, op = 1
	// For load word, op = 3
	n_load++;
	CORE_UINT(TAGBITS) tag = getTag(address);
	CORE_UINT(SETBITS) set = getSet(address);
	CORE_UINT(IDBITS) id = getId(address);
	CORE_UINT(1) tag_match = (tag == index[set].tag) ? 1 : 0;
	CORE_UINT(1) dirty = index[set].dirtybit;
	CORE_INT(32) result;
	result = sign ? -1 : 0;
	CORE_UINT(32) dram_address = 0;
	CORE_UINT(8) byte0, byte1, byte2, byte3;
	CORE_UINT(IDBITS+1) id_counter;

	if(tag_match && index[set].invalid == 0){
		byte0 = cache[set][id];
		byte1 = cache[set][id+1];
		byte2 = cache[set][id+2];
		byte3 = cache[set][id+3];
	}
	else{
		n_dram_reads++;
		n_cache_miss++;
		*cache_miss = 1;
		if(dirty){
			*cache_miss = 2;
			n_dram_writes++;
			dram_address.SET_SLC(IDBITS,set);
			dram_address.SET_SLC(IDBITS+SETBITS,index[set].tag);
			for(id_counter = 0; id_counter<CACHEBLOCKBYTES; id_counter++){
				dram_location->setMemory(dram_address+id_counter,cache[set][id_counter]);
			}
		}

		dram_address.SET_SLC(IDBITS,set);
		dram_address.SET_SLC(IDBITS+SETBITS, tag);
		for(id_counter = 0; id_counter<CACHEBLOCKBYTES; id_counter+=1){
			cache[set][id_counter] = dram_location->getMemory(dram_address+id_counter);
		}	
		//reading blocks into cache
		byte0 = cache[set][id]; 
		byte1 = cache[set][id+1]; 
		byte2 = cache[set][id+2];
		byte3 = cache[set][id+3];
		index[set].tag = tag;
		index[set].dirtybit = 0;
		index[set].invalid = 0;
	}
	switch(address % 4){
		case 0:
			break;
		case 1:
			byte0 = byte1;
			break;
		case 2:
			byte0 = byte2;
			byte1 = byte3;
			break;
		case 3:
			byte0 = byte3;
			break;
	}
	result.SET_SLC(0,byte0);
	if(op & 1){
		result.SET_SLC(8,byte1);
	}
	if(op & 2){
		result.SET_SLC(16,byte2);
		result.SET_SLC(24,byte3);
	}
	return result;
}		


CORE_UINT(TAGBITS) Cache::getTag(CORE_UINT(32) address){
	return address.SLC(TAGBITS,IDBITS+SETBITS);
}	

CORE_UINT(SETBITS) Cache::getSet(CORE_UINT(32) address){
	return address.SLC(SETBITS,IDBITS);
}

CORE_UINT(IDBITS) Cache::getId(CORE_UINT(32) address){
	return address.SLC(IDBITS,0);
}

CORE_UINT(32) Cache::getNumberCacheMiss(){
	return n_cache_miss;
}


CORE_UINT(32) Cache::getNumberDramReads(){
	return n_dram_reads;
}


CORE_UINT(32) Cache::getNumberDramWrites(){
	return n_dram_writes;
}


CORE_UINT(32) Cache::getNumberLoads(){
	return n_load;
}


CORE_UINT(32) Cache::getNumberStores(){
	return n_store;
}

#ifndef SRC_DATAMEMORYCATAPULT_H_
#define SRC_DATAMEMORYCATAPULT_H_

#include "portability.h"

class DataMemory {

	public:
		
		CORE_INT(32) memory[8192];
	
		void set(CORE_UINT(32) address, CORE_INT(32) value, CORE_UINT(2) op){
			CORE_UINT(13) wrapped_address = address % 8192;
			memory[wrapped_address >> 2] = value;
		}

		CORE_INT(32) get(CORE_UINT(32) address, CORE_UINT(2) op,
		CORE_UINT(1) sign){
			CORE_UINT(13) wrapped_address = address % 8192;
			CORE_UINT(2) offset = wrapped_address & 0x3;
			CORE_UINT(13) location = wrapped_address >> 2;
			CORE_INT(32) result;
			result = sign ? -1 : 0;
			CORE_INT(32) mem_read = memory[location];
			result=mem_read;
			//CORE_INT(8) byte0 = mem_read.SLC(8,0);
			//CORE_INT(8) byte1 = mem_read.SLC(8,8);
			//CORE_INT(8) byte2 = mem_read.SLC(8,16);
			//CORE_INT(8) byte3 = mem_read.SLC(8,24);
			/*switch(offset){
				case 0:
					break;
				case 1:
					byte0 = byte1;
					break;
				case 2:
					byte0 = byte1;
					byte1 = byte3;
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
			}*/
			return result;
		}
	
};

#endif /* SRC_DATAMEMORYCATAPULT_H_ */

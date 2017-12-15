/*
 * DataMemory.h
 *
 *  Created on: Sep 15, 2017
 *      Author: emascare
 */

#ifndef SRC_DATAMEMORY_H_
#define SRC_DATAMEMORY_H_

#include "portability.h"

//NOTE: Only supports aligned accesses
class DataMemory {

	public:

		CORE_INT(8) memory[8192][4];

		void set(CORE_UINT(32) address, CORE_INT(32) value, CORE_UINT(2) op){
			// For store byte, op = 0
			// For store half word, op = 1
			// For store word, op = 3

			CORE_UINT(13) wrapped_address = address % 8192;
			CORE_INT(8) byte0 = value.SLC(8,0);
			CORE_INT(8) byte1 = value.SLC(8,8);
			CORE_INT(8) byte2 = value.SLC(8,16);
			CORE_INT(8) byte3 = value.SLC(8,24);
			CORE_UINT(2) offset = wrapped_address & 0x3;
			CORE_UINT(13) location = wrapped_address >> 2;
			memory[location][offset] = byte0;
			if(op & 1){
				memory[location][offset+1] = byte1;
			}
			if(op & 2){
				memory[location][2] = byte2;
				memory[location][3] = byte3;
			}
		}

		CORE_INT(32) get(CORE_UINT(32) address, CORE_UINT(2) op, CORE_UINT(1) sign){
			// For load byte, op = 0
			// For load half word, op = 1
			// For load word, op = 3
			CORE_UINT(13) wrapped_address = address % 8192;
			CORE_UINT(2) offset = wrapped_address & 0x3;
			CORE_UINT(13) location = wrapped_address >> 2;
			CORE_INT(32) result;
			result = sign ? -1 : 0;
			CORE_INT(8) byte0 = memory[location][0];
			CORE_INT(8) byte1 = memory[location][1];
			CORE_INT(8) byte2 = memory[location][2];
			CORE_INT(8) byte3 = memory[location][3];
			switch(offset){
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

};

#endif /* SRC_DATAMEMORY_H_ */

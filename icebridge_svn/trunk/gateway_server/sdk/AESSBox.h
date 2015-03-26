/*
 * AESBox.h
 *
 *  Created on: Sep 11, 2013
 *      Author: root
 */

#ifndef AESBOX_H_
#define AESBOX_H_
#include <cstring>
#define  TOTALBOXES 1
#define EXCHANGETIMES 64

class AESSBox {
public:
	typedef short unsigned int DBYTE;
	typedef unsigned char BYTE;
	AESSBox();
	virtual ~AESSBox();
	void generateBox(DBYTE mx);

	static void GenerateOriginalBoxe(DBYTE mx = 0x0102){
			 AESSBox *pBox = new AESSBox();

			    //for(int i = 0x0100; i < 512 ; i++){
			        pBox->generateBox(mx);
			    //}

			    delete pBox;
		}

	static void getBox(unsigned char * pBox,unsigned char * &pRandoms);

	static void formNewBoxes(unsigned char * pBox,unsigned char * &pRandoms);

	//static AESBox pBox;
	/*因为m(x)的最高项是8次方，所以总共有9项，所以就用2个字节来表示 */

	int seekat7(BYTE temp) {
		if (temp & 0x80)
			return 1;
		else
			return 0;
	}

	int seekat6(BYTE temp) {
		if (temp & 0x40)
			return 1;
		else
			return 0;
	}
	int seekat5(BYTE temp) {
		if (temp & 0x20)
			return 1;
		else
			return 0;
	}
	int seekat4(BYTE temp) {
		if (temp & 0x10)
			return 1;
		else
			return 0;
	}
	int seekat3(BYTE temp) {
		if (temp & 0x08)
			return 1;
		else
			return 0;
	}
	int seekat2(BYTE temp) {
		if (temp & 0x04)
			return 1;
		else
			return 0;
	}
	int seekat1(BYTE temp) {
		if (temp & 0x02)
			return 1;
		else
			return 0;
	}
	int seekat0(BYTE temp) {
		if (temp & 0x01)
			return 1;
		else
			return 0;
	}

	int length(DBYTE temp) {

		int i = 0;
		while (1) {

			if (temp == 0)
				break;
			temp = temp >> 1;
			i++;
		}

		return i;

	}
	/************************************************一个双字节数模m(x)后的结果***********************************/
	BYTE mod(DBYTE temp, DBYTE mx) {
		DBYTE bemod;

		while (temp > 256 || temp == 256) {
			bemod = mx;
			bemod = bemod << (length(temp) - 9);
			temp = temp ^ bemod;
		}

		return (BYTE) temp;
	}

	DBYTE mul(BYTE temp) {
		DBYTE out = 0;
		if (seekat7(temp))
			out += 0x4000;
		if (seekat6(temp))
			out += 0x1000;
		if (seekat5(temp))
			out += 0x0400;
		if (seekat4(temp))
			out += 0x0100;
		if (seekat3(temp))
			out += 0x0040;
		if (seekat2(temp))
			out += 0x0010;
		if (seekat1(temp))
			out += 0x0004;
		if (seekat0(temp))
			out += 0x0001;

		return out;
	}
	/*******************************************************************************************************/

	/****两个字节在伽瓦罗域下的乘积，事实上和上面那个函数是一样的，只不过上面的那个函数是两个相同的字节相乘，结果可以简化********/
	/**********************************在这里没办法化简，所以只能老老实实一位一位的列出来****************************/
	DBYTE multwo(BYTE temp1, BYTE temp2) {
		DBYTE out = 0;
		if (seekat7(temp1) * seekat7(temp2))
			out += 0x4000;
		if ((seekat7(temp1) * seekat6(temp2))
				^ (seekat6(temp1) * seekat7(temp2)))
			out += 0x2000;
		if ((seekat7(temp1) * seekat5(temp2))
				^ (seekat6(temp1) * seekat6(temp2))
				^ (seekat5(temp1) * seekat7(temp2)))
			out += 0x1000;
		if ((seekat7(temp1) * seekat4(temp2))
				^ (seekat6(temp1) * seekat5(temp2))
				^ (seekat5(temp1) * seekat6(temp2))
				^ (seekat4(temp1) * seekat7(temp2)))
			out += 0x0800;
		if ((seekat7(temp1) * seekat3(temp2))
				^ (seekat6(temp1) * seekat4(temp2))
				^ (seekat5(temp1) * seekat5(temp2))
				^ (seekat4(temp1) * seekat6(temp2))
				^ (seekat3(temp1) * seekat7(temp2)))
			out += 0x0400;
		if ((seekat7(temp1) * seekat2(temp2))
				^ (seekat6(temp1) * seekat3(temp2))
				^ (seekat5(temp1) * seekat4(temp2))
				^ (seekat4(temp1) * seekat5(temp2))
				^ (seekat3(temp1) * seekat6(temp2))
				^ (seekat2(temp1) * seekat7(temp2)))
			out += 0x0200;

		if ((seekat7(temp1) * seekat1(temp2))
				^ (seekat6(temp1) * seekat2(temp2))
				^ (seekat5(temp1) * seekat3(temp2))
				^ (seekat4(temp1) * seekat4(temp2))
				^ (seekat3(temp1) * seekat5(temp2))
				^ (seekat2(temp1) * seekat6(temp2))
				^ (seekat1(temp1) * seekat7(temp2)))
			out += 0x0100;

		if ((seekat7(temp1) * seekat0(temp2))
				^ (seekat6(temp1) * seekat1(temp2))
				^ (seekat5(temp1) * seekat2(temp2))
				^ (seekat4(temp1) * seekat3(temp2))
				^ (seekat3(temp1) * seekat4(temp2))
				^ (seekat2(temp1) * seekat5(temp2))
				^ (seekat1(temp1) * seekat6(temp2))
				^ (seekat0(temp1) * seekat7(temp2)))
			out += 0x0080;

		if ((seekat6(temp1) * seekat0(temp2))
				^ (seekat5(temp1) * seekat1(temp2))
				^ (seekat4(temp1) * seekat2(temp2))
				^ (seekat3(temp1) * seekat3(temp2))
				^ (seekat2(temp1) * seekat4(temp2))
				^ (seekat1(temp1) * seekat5(temp2))
				^ (seekat0(temp1) * seekat6(temp2)))
			out += 0x0040;

		if ((seekat5(temp1) * seekat0(temp2))
				^ (seekat4(temp1) * seekat1(temp2))
				^ (seekat3(temp1) * seekat2(temp2))
				^ (seekat2(temp1) * seekat3(temp2))
				^ (seekat1(temp1) * seekat4(temp2))
				^ (seekat0(temp1) * seekat5(temp2)))
			out += 0x0020;

		if ((seekat4(temp1) * seekat0(temp2))
				^ (seekat3(temp1) * seekat1(temp2))
				^ (seekat2(temp1) * seekat2(temp2))
				^ (seekat1(temp1) * seekat3(temp2))
				^ (seekat0(temp1) * seekat4(temp2)))
			out += 0x0010;

		if ((seekat3(temp1) * seekat0(temp2))
				^ (seekat2(temp1) * seekat1(temp2))
				^ (seekat1(temp1) * seekat2(temp2))
				^ (seekat0(temp1) * seekat3(temp2)))
			out += 0x0008;

		if ((seekat2(temp1) * seekat0(temp2))
				^ (seekat1(temp1) * seekat1(temp2))
				^ (seekat0(temp1) * seekat2(temp2)))
			out += 0x0004;

		if ((seekat1(temp1) * seekat0(temp2))
				^ (seekat0(temp1) * seekat1(temp2)))
			out += 0x0002;
		if (seekat0(temp1) * seekat0(temp2))
			out += 0x0001;

		return out;
	}

	BYTE matrix(BYTE temp) {
		BYTE out = 0;
		if ((seekat0(temp) * 1) ^ (seekat1(temp) * 0) ^ (seekat2(temp) * 0)
				^ (seekat3(temp) * 0) ^ (seekat4(temp) * 1)
				^ (seekat5(temp) * 1) ^ (seekat6(temp) * 1)
				^ (seekat7(temp) * 1) ^ 1)
			out += 1;

		if ((seekat0(temp) * 1) ^ (seekat1(temp) * 1) ^ (seekat2(temp) * 0)
				^ (seekat3(temp) * 0) ^ (seekat4(temp) * 0)
				^ (seekat5(temp) * 1) ^ (seekat6(temp) * 1)
				^ (seekat7(temp) * 1) ^ 1)
			out += 2;
		if ((seekat0(temp) * 1) ^ (seekat1(temp) * 1) ^ (seekat2(temp) * 1)
				^ (seekat3(temp) * 0) ^ (seekat4(temp) * 0)
				^ (seekat5(temp) * 0) ^ (seekat6(temp) * 1)
				^ (seekat7(temp) * 1) ^ 0)
			out += 4;
		if ((seekat0(temp) * 1) ^ (seekat1(temp) * 1) ^ (seekat2(temp) * 1)
				^ (seekat3(temp) * 1) ^ (seekat4(temp) * 0)
				^ (seekat5(temp) * 0) ^ (seekat6(temp) * 0)
				^ (seekat7(temp) * 1) ^ 0)
			out += 8;
		if ((seekat0(temp) * 1) ^ (seekat1(temp) * 1) ^ (seekat2(temp) * 1)
				^ (seekat3(temp) * 1) ^ (seekat4(temp) * 1)
				^ (seekat5(temp) * 0) ^ (seekat6(temp) * 0)
				^ (seekat7(temp) * 0) ^ 0)
			out += 16;
		if ((seekat0(temp) * 0) ^ (seekat1(temp) * 1) ^ (seekat2(temp) * 1)
				^ (seekat3(temp) * 1) ^ (seekat4(temp) * 1)
				^ (seekat5(temp) * 1) ^ (seekat6(temp) * 0)
				^ (seekat7(temp) * 0) ^ 1)
			out += 32;
		if ((seekat0(temp) * 0) ^ (seekat1(temp) * 0) ^ (seekat2(temp) * 1)
				^ (seekat3(temp) * 1) ^ (seekat4(temp) * 1)
				^ (seekat5(temp) * 1) ^ (seekat6(temp) * 1)
				^ (seekat7(temp) * 0) ^ 1)
			out += 64;
		if ((seekat0(temp) * 0) ^ (seekat1(temp) * 0) ^ (seekat2(temp) * 0)
				^ (seekat3(temp) * 1) ^ (seekat4(temp) * 1)
				^ (seekat5(temp) * 1) ^ (seekat6(temp) * 1)
				^ (seekat7(temp) * 1) ^ 0)
			out += 128;

		return out;

	}

};

#endif /* AESBOX_H_ */

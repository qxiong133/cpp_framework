/*
 * RandomData.hpp
 *
 *  Created on: Jan 13, 2014
 *      Author: zhongbing
 */

#ifndef RANDOMDATA_HPP_
#define RANDOMDATA_HPP_
#include <vector>
#include <map>
#include <iostream>
#include <ctime>
#include <stdlib.h>

template <class T>
class RandomData
{
private:

	std::vector<T> mydata;

	std::vector<T> back_up_data;

public:

	RandomData(){
		srandom(time(NULL));
	};

	void AddMember(T& member)
	{
		mydata.push_back(member);
	}

	bool GetRandomData(T & member)
	{
		//srandom(time(NULL));
		if(mydata.size() > 0)
		{
			member = mydata[rand()%mydata.size()];
			return true;
		}
		return false;
	}

	bool GetSomeData(std::vector<T>& data_list,int n)
	{
		data_list.clear();
		int size = mydata.size();
		if(size == 0)
		{
			return false;
		}
		if(size <= n)
		{
			data_list.insert(data_list.begin(),mydata.begin(),mydata.end());
			return true;
		}

		for(int i = 0;i< n;i++)
		{
			T t;
			if(GetRandomOnlyData(t))
			{
				data_list.push_back(t);
			}
			else
			{
				break;
			}
		}
		if(data_list.size() == 0)
		{
			return false;
		}
		mydata.insert(mydata.begin(),back_up_data.begin(),back_up_data.end());
		back_up_data.clear();
		return true;
	}

	bool GetRandomOnlyData(T & member)
	{
		int size = mydata.size();
		if(size > 1)
		{
			int index = rand()%size;
			int last_index = size - 1;
			member = mydata[index];
			if(index == last_index)
			{
				back_up_data.push_back(member);
				mydata.pop_back();
			}
			else
			{
				mydata[index] = mydata[last_index];
				back_up_data.push_back(member);
				mydata.pop_back();
			}
			return true;
		}
		if(size == 1)
		{
			member = mydata[0];
			back_up_data.push_back(member);
			mydata.pop_back();
			return true;
		}
		return false;
	}

	static void test()
	{
		RandomData<int> test;
		for(int i = 0;i< 10;i++)
		{
			test.AddMember(i);
		}
		for(int i = 0;i<20;i++)
		{
			int random_data;
			if(test.GetRandomData(random_data))
			{
				printf("\n~~~~~~~~~~~~~~~~~~~  %d \n",random_data);
			}
			else
			{
				break;
			}
		}
		for(int i = 0;i<10;i++)
		{
			int random_data;
			if(test.GetRandomOnlyData(random_data))
			{
				printf("\n+++++++++++++++++++  %d \n",random_data);
			}
			else
			{
				break;
			}
		}
	}
};
#endif /* RANDOMDATA_HPP_ */

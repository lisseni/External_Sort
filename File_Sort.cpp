// File_Sort.cpp: 
//
 
#include "stdafx.h"
#include <cstdint>
#include <Windows.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <random>  

#define FILE_SIZE         1073741824
#define MEM_LIMIT         268435456
#define BUF_NUMBER        FILE_SIZE/MEM_LIMIT
#define BUF_LENTH         MEM_LIMIT/sizeof(uint32_t)
#define BUF_SIZE          MEM_LIMIT

using namespace std;

//Generate binary file size of 1 Gb
//If random_digits.bin exists in project path, 
//then file will be replaced
void generate_bin_file(char * file_name)
{
	uint32_t i=0, j=0;
	
	cout << "Start generation " << file_name <<" . . ." << endl;

	//random generator	
	srand((int)time(NULL));  		
	// open file to record 
	ofstream outstrm (file_name, ios_base::trunc | std::ios::binary);

	if(outstrm.is_open())
	{
		uint32_t *buf = new uint32_t[BUF_LENTH];
		random_device rd;   
		mt19937 gen(rd());
		//record fandom digits into file
		for (j = 0; j < BUF_NUMBER; j++)
		{
			for(i = 0; i < BUF_LENTH; i++)
				*(buf+i) = (uint32_t)gen();

			outstrm.seekp(BUF_SIZE * j);
			outstrm.write((char *)buf, BUF_SIZE);
		}
		delete(buf);
		outstrm.close();
	}
	else
		cout << "File " << file_name <<" wasn't opened" << endl;
	cout << "file generation is finished" << endl;
	cout << "  " << endl;
}

int comp(const void *i, const void *j)
{
	return *(int *)i - *(int *)j;
}

//Read file by blocks. Each block is sorted standard function qsort and
//then will be written into new temp file random_digits_sorted.bin
//Such way there is partly sorted file.
int part_sort(char *file_name)
{
	uint32_t * buffer = new uint32_t[BUF_LENTH];
	char sorted_file_name[120];
	cout << "Start spliting on sorted blocks . . ." << endl;

	ifstream instrm (file_name, std::ios::ate | std::ios::binary);
	if (((uint32_t)instrm.tellg()) != FILE_SIZE)
	{
		cout << "Wrong size of "<< file_name <<" = "<< instrm.tellg() << endl;
		cout << "It has to be = " << FILE_SIZE << endl;
		return 1;
	}

	sprintf_s(sorted_file_name,120,"sorted_%s",file_name);

	ofstream outstrm (sorted_file_name, ios_base::trunc | std::ios::binary);

	if(instrm.is_open() )
	{
		for (int j=0; j<BUF_NUMBER; j++)
		{
			instrm.seekg(BUF_SIZE * j);
			for (int i=0; i<BUF_LENTH; i++)
				instrm.read((char *)(buffer+i), sizeof(uint32_t));

			qsort(buffer, BUF_LENTH, sizeof(uint32_t), comp);

			if(outstrm.is_open())
			{
				outstrm.seekp(BUF_SIZE * j);
				outstrm.write((char *)(buffer),BUF_SIZE);
			}
			else
			{
				cout << "File "<< sorted_file_name <<" wasn't opend" <<endl;
				cout << "  " << endl;
				delete buffer;
				return 1;
			}
		}
		instrm.close();
		if(outstrm.is_open())
			outstrm.close();
	}
	else
	{
		cout << "File "<< file_name <<" wasn't opend" <<endl;
		cout << "  " << endl;
		delete buffer;
		return 1;
	}
	delete buffer;
	cout << "New file "<< sorted_file_name <<" is created" << endl;
	cout << "  " << endl;
	return 0;
}

//in-place merge
void merge_buffer(uint32_t *buf)
{
	uint32_t * temp_buf = new uint32_t[BUF_LENTH];

	uint32_t i=0, j=0;
	for (int k=0; k<BUF_LENTH; k++)
	{
		if (i > BUF_LENTH/2-1)
		{
			temp_buf[k] = *(buf+BUF_LENTH/2+j);
			j++;
		}
		else if (j > BUF_LENTH/2-1)
		{
			temp_buf[k] = *(buf+i);
			i++;
		}
		else if (*(buf+i) < *(buf+BUF_LENTH/2+j))
		{
			temp_buf[k] = *(buf+i);
			i++;
		}
		else
		{
			temp_buf[k] = *(buf+BUF_LENTH/2+j);
			j++;
		}
	}

	memcpy(buf, temp_buf, BUF_SIZE);
	delete temp_buf;
}

//External sort
int merge_sort(char *file_name)
{

	uint32_t i=BUF_NUMBER;//Counter sorted half-block from the end of file
	uint32_t j=0;	//Counter sorted half-block from the start of file		
	char sorted_file_name[120];
	const uint32_t half_buf_size = BUF_SIZE/2;
	sprintf_s(sorted_file_name,120,"sorted_%s",file_name);
	cout << "Start external sort of "<< sorted_file_name <<" . . ." << endl;
	
	fstream sorted_file(sorted_file_name, std::ios::in | std::ios::out | std::ios::binary);

	if (sorted_file.is_open())
	{
		uint32_t * buff_sorted = new uint32_t[BUF_LENTH];		
		while (i>0)
		{
			//cout << "tail counter i=" << i << endl;
			sorted_file.seekg(half_buf_size * j);
			sorted_file.read((char *)buff_sorted, half_buf_size);

			for (uint32_t k=1; k<i; ++k)
			{
				uint32_t temp=half_buf_size * j + BUF_SIZE * k;
				sorted_file.seekg(half_buf_size * j + BUF_SIZE * k);
				sorted_file.read((char *) (buff_sorted + BUF_LENTH/2), half_buf_size);	
				//in-place merge of buff_sorted[]
				merge_buffer(buff_sorted);
				sorted_file.seekp(half_buf_size * j + BUF_SIZE * k);
				sorted_file.write((char *) (buff_sorted + BUF_LENTH/2), half_buf_size);
			}
			sorted_file.seekp(half_buf_size * j);
			sorted_file.write((char *) (buff_sorted), half_buf_size);

			sorted_file.seekg(half_buf_size * (j-1) + BUF_SIZE * i);
			sorted_file.read((char *) (buff_sorted + BUF_LENTH/2), half_buf_size);
			if (i > 1)
				for (int64_t k = i - 2; k >= 0; --k)
				{
					sorted_file.seekg(half_buf_size * (j + 1) + BUF_SIZE * k);
					sorted_file.read((char *) buff_sorted, half_buf_size);
					merge_buffer(buff_sorted);
					sorted_file.seekp(half_buf_size * (j + 1) + BUF_SIZE * k);
					sorted_file.write((char *) buff_sorted, half_buf_size);
				}

				sorted_file.seekp(half_buf_size * (j-1) + BUF_SIZE * i);
				sorted_file.write((char *) (buff_sorted + BUF_LENTH/2), half_buf_size);

				--i;
				++j;

				for (uint32_t p = 0; p < i; ++p) 
				{
					sorted_file.seekg(half_buf_size * j + BUF_SIZE * p);
					sorted_file.read((char *) buff_sorted, BUF_SIZE);
					merge_buffer(buff_sorted);
					sorted_file.seekp(half_buf_size * j + BUF_SIZE * p);
					sorted_file.write((char *) buff_sorted, BUF_SIZE);
				}
		}		
		sorted_file.close();
		delete buff_sorted;
		cout << "External sort is done" << endl;
		cout << "  " << endl;
		return 0;
	}else
		cout << "File "<< sorted_file_name <<" wan't opened" << endl;
	cout << "  " << endl;
	return 1;	
}

//Check sorting
int check(char *file_name)
{
	uint32_t base_val=0;
	uint32_t i, j;
	char sorted_file_name[120];
	cout << "Start file check . . . " << endl;
	sprintf_s(sorted_file_name,120,"sorted_%s",file_name);
	ifstream checked_file (sorted_file_name, std::ios::binary);

	if(checked_file.is_open())
	{
		uint32_t *buffer = new uint32_t[BUF_LENTH];
		for(i=0; i<BUF_NUMBER; ++i)
		{
			if (*buffer >= base_val)
			{
				for (j=1; j<BUF_LENTH; j++)
					if (*(buffer+j) < *(buffer+j-1))
					{
						cout << "Sorting is wrong" << endl;
						cout << "Error with Byte number = " << ((j-1)*i) << endl;
						cout << "and Byte number = " << (j*i) << endl;
						checked_file.close();
						return 1;
					}
			}
			else
			{
				cout << "Sorting is wrong" << endl;			
				checked_file.close();
				return 1;
			}
			base_val=*(buffer+BUF_NUMBER-1);
		}		
		checked_file.close();
		delete buffer;
		cout << "all is ok" << endl;
		cout << "  " << endl;
	}else
	{
		cout << "file "<< sorted_file_name <<" wasn't opened" << endl;
		cout << "  " << endl;
		return 1;
	}
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int gen_file_flag = 0;
	char file_name[80];
	
	cout << "Please, Type binary file name. The size should be 1 Gb" << endl;
	cout << "If there is no this file in current directory, It will be created." << endl;
	cin >> file_name;
	
	FILE *f;
	fopen_s(&f,file_name, "r");
    if (f == NULL)				
		generate_bin_file(file_name);
	else fclose(f);
	// Split file for some files that can be sorted with qsort,
	//Block sizes are < 300 Mb.
	//Temporary sorted files are into project directory temp_files
	if (part_sort(file_name))
		cout << "Error in split" << endl;
	else
	{
		if (merge_sort(file_name)) //merge sorting	
			cout << "Error in externel sort" << endl;
		else if (check(file_name)) //sort checking				
			cout << "Check error" << endl;
	}
	getchar();
	return 0;
}


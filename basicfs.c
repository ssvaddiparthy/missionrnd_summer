/* File System implementation with max file size being 16kb */
/* Author : V Sandeep Srivastav */
/* Date: 1st October 2014 */

#include<stdio.h>
#include<string.h>
#define ENDFILE 1024000
#define MAXBLOCKS 256
#define ENDMETA 8192
#define BEGDATA 8192

struct bit_vector
{
	int array[MAXBLOCKS];
};
typedef struct bit_vector bit_vector;

struct metadata
{
	char name[20];
	int id;
	int size;
	int blocks[4];
};
typedef struct metadata metadata;

void my_format(char *str)
{
	FILE *fp;
	bit_vector bvector;
	int iterator=0;
	
	
	fp=fopen(str,"wb+");
	fwrite(&iterator,sizeof(int),1,fp);			//setting number of files to zero
	fwrite(&iterator,sizeof(int),1,fp);			//setting file ids to zero
	
		iterator=0;
		while(iterator<MAXBLOCKS)
		{
			bvector.array[iterator]=0;
			iterator++;
		}
	
	bvector.array[0]=1;
	bvector.array[1]=1;
	bvector.array[2]=1;
	fwrite(&bvector,sizeof(bit_vector),1,fp);	//setting all blocks as free
	
	fflush(fp);
	fclose(fp);
	
}	

int getidfrom(char *str)
{
	FILE *fp;
	int id=-1;
	
	fp=fopen(str,"rb+");
	fseek(fp,sizeof(int),SEEK_SET);
	fread(&id,sizeof(int),1,fp);
		
	id++;
	
	return id;

}

int get_num_blocks(int size)
{
	if(size%4==0)
		return size/4;
		
	else
		return (size/4);


}

int setblocks(char *drive,int size,int *array)
{
	int blocks=0,iterator=0,count=0;
	FILE *fp;
	bit_vector b1;
	
	blocks=get_num_blocks(size);
	
	fp=fopen(drive,"rb+");
	fseek(fp,2*sizeof(int),SEEK_SET);
	fread(&b1,sizeof(bit_vector),1,fp);
	
	
		iterator=0;
		while(iterator<256 && count<blocks)
		{
			if(b1.array[iterator]==0)
			{
				array[count]=iterator;
				b1.array[iterator]=1;
				count++;
			}
		
			iterator++;
		}
	
	fseek(fp,2*sizeof(int),SEEK_SET);
	fwrite(&b1,sizeof(bit_vector),1,fp);
	fclose(fp);
	return blocks;	
	
}

void printer_of_metadata(metadata *m1)
{
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("name: %s  %d\n ",m1->name,m1->id);
	printf("\n %d %d %d %d\n",m1->blocks[0],m1->blocks[1],m1->blocks[2],m1->blocks[3]);
	printf("\nsize is %d\n",m1->size);

}

long get_freemdb(char *drive)
{
	FILE *fp;
	int n_f,c_fid;
	metadata m1;
	
	m1.id=0;
	
	fp=fopen(drive,"rb+");
	fread(&n_f,sizeof(int),1,fp);
	fread(&c_fid,sizeof(int),1,fp);
	fseek(fp,sizeof(bit_vector),SEEK_CUR);
	
	if(n_f==0)
		return ftell(fp);
		
	if(n_f==c_fid)
	{
		fseek(fp,n_f*sizeof(metadata),SEEK_CUR);
		return ftell(fp);
	
	}	
	
	else
	{
		
		while(m1.id!=-1)
		{
			fread(&m1,sizeof(metadata),1,fp);
		}
		
		if(ftell(fp)>8192)
		{
			return -1;
		}
		
		else
		{
			fseek(fp,-sizeof(metadata),SEEK_CUR);
			return ftell(fp);
		}
	}
	fclose(fp);
}

void update_integers(int id,char *drive)
{	
	FILE *fp;
	int temp=0;
	
	fp=fopen(drive,"rb+");
	fread(&temp,sizeof(int),1,fp);
	fseek(fp,0,SEEK_SET);
	temp++;
	fwrite(&temp,sizeof(int),1,fp);
	fwrite(&id,sizeof(int),1,fp);
	fclose(fp);

}

void addfile(char *fname,int size,char *sysname)
{
	FILE *src,*drive;
	long pos_md=-23;
	metadata *m1;
	int counter=0,blocks=0,ch_read;
	char array[4096],end="~";
	
	m1=(metadata*)malloc(sizeof(metadata)*1);
	strcpy(m1->name,fname);
	m1->size=size;
	m1->id=getidfrom(sysname);
	blocks=setblocks(sysname,size,m1->blocks);

	pos_md=get_freemdb(sysname);
	
	drive=fopen(sysname,"rb+");
	
	fseek(drive,pos_md,SEEK_SET);
	fwrite(m1,sizeof(metadata),1,drive);
	
	fseek(drive,0,SEEK_SET);
	
	src=fopen(fname,"rb+");
	
		while(counter<blocks)
		{
			ch_read=fread(&array,sizeof(char),4096,src);
			fseek(drive,m1->blocks[counter]*4096,SEEK_SET);
			fwrite(&end,sizeof(char),4096,drive);
			fseek(drive,-(4096),SEEK_CUR);
			fwrite(&array,sizeof(char),ch_read,drive);
			counter++;
		}
		
	
	fclose(drive);	
	fclose(src);
	update_integers(m1->id,sysname);

}

void display(char *file,char *sysname)
{
	FILE *fp;
	int max_id=-1,counter=0,read;
	metadata m1;
	char temp[4096];
	
	fp=fopen(sysname,"rb+");
	fseek(fp,sizeof(int),SEEK_SET);
	fread(&max_id,sizeof(int),1,fp);
	fseek(fp,sizeof(bit_vector),SEEK_CUR);
	
			counter=0;
			while(counter<max_id)
			{
				fread(&m1,sizeof(metadata),1,fp);
					if(strcmp(m1.name,file)==0)
					{
						break;
					}
				counter++;
			}
			
			if(counter>max_id)
			{
				printf("\n\nFile not found\n\n\n\n");
				return;
			}
			
			if(m1.id!=-1)
			{
				counter=0;
				printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Display Function~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				while(m1.blocks[counter]!=0)
				{
					fseek(fp,m1.blocks[counter]*4096,SEEK_SET);
					read=fread(&temp,sizeof(char),4096,fp);
					fwrite(&temp,sizeof(char),read,stdout);
					counter++;
				}			
			}
			

}

void deletefile(char *file,char *sysname)
{
	FILE *fp;
	int max_id=-1,counter=0,counter2=0;
	metadata m1;
	char temp[4096];
	bit_vector bv;
	
	fp=fopen(sysname,"rb+");
	fseek(fp,sizeof(int),SEEK_SET);
	fread(&max_id,sizeof(int),1,fp);
	fseek(fp,sizeof(bit_vector),SEEK_CUR);
	
			counter=0;
			while(counter<max_id)
			{
				fread(&m1,sizeof(metadata),1,fp);
				
					if(strcmp(m1.name,file)==0)
					{
						break;
					}
				counter++;
			}
			
			if(counter>max_id)
			{
				printf("\n\tFile not found");
				return;
			}
			
			else
			{
				printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Deleting the file~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				
				fseek(fp,-sizeof(metadata),SEEK_CUR);
				m1.id=-1;
				fwrite(&m1,sizeof(metadata),1,fp);
				fseek(fp,-sizeof(metadata),SEEK_CUR);
				
				counter=0;
				
				fseek(fp,2*sizeof(int),SEEK_SET);
				fread(&bv,sizeof(bit_vector),1,fp);
				
				bv.array[m1.blocks[0]]=0;
				bv.array[m1.blocks[1]]=0;
				bv.array[m1.blocks[2]]=0;
				bv.array[m1.blocks[3]]=0;
				bv.array[0]=1;

				fseek(fp,2*sizeof(int),SEEK_SET);
				fwrite(&bv,sizeof(bit_vector),1,fp);
				
				fseek(fp,0,SEEK_SET);
				fread(&counter,sizeof(int),1,fp);
				counter--;
				fseek(fp,0,SEEK_SET);
				fwrite(&counter,sizeof(int),1,fp);
						
			}
			
	fclose(fp);

}

void main()
{
	
}